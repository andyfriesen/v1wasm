
#include <cstdio>
#include <string>
#include <memory>
#include <stdexcept>
#include <pthread.h>
#include "ppapi/c/ppb_file_io.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/file_system.h"
#include "ppapi/cpp/file_ref.h"
#include "ppapi/cpp/file_io.h"
#include "ppapi/cpp/url_request_info.h"
#include "ppapi/cpp/input_event.h"
#include "ppapi/cpp/url_loader.h"
#include "ppapi/cpp/graphics_2d.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/size.h"
#include "ppapi/cpp/var.h"

#include "main.h"
#include "nacl.h"
#include "render.h"
#include "vc.h"
#include "fs.h"

void MiscSetup();
void PutOwnerText();
void initvga(IFramebuffer* fb);
void InitItems();

struct ScopedLock {
    ScopedLock(pthread_mutex_t& m)
        : mutex(m)
    {
        auto result = pthread_mutex_lock(&mutex);
        assert(0 == result && "Unable to lock mutex");
    }

    ~ScopedLock() {
        auto result = pthread_mutex_unlock(&mutex);
        assert(0 == result && "Unable to unlock mutex");
    }

private:
    pthread_mutex_t& mutex;
};

struct Downloader {
    explicit Downloader(pp::Instance* instance)
        : instance(instance)
        , urlRequestInfo(instance)
        , ccFactory(this)
        , urlLoader(instance)
    {
        urlRequestInfo.SetMethod("GET");
    }

    bool IsError(int32_t result) {
        return ((PP_OK != result) && (PP_OK_COMPLETIONPENDING != result));
    }

    int32_t get(const std::string& url, pp::CompletionCallback cb) {
        data.resize(0);
        this->url = url;
        onComplete = cb;
        urlRequestInfo.SetURL(url);

        auto cc = ccFactory.NewCallback(&Downloader::onOpen);
        auto res = urlLoader.Open(urlRequestInfo, cc);
        if (PP_OK_COMPLETIONPENDING != res) {
            cc.Run(res);
        }

        return !IsError(res);
    }

    void onOpen(int32_t result) {
        if (result < 0) {
            onError(result);
            return;
        }

        readBody();
    }

    void readBody() {
        auto cc = ccFactory.NewCallback(&Downloader::onRead);
        int32_t res = urlLoader.ReadResponseBody(buffer, sizeof(buffer), cc);
        if (PP_OK_COMPLETIONPENDING != res) {
            cc.Run(res);
        }
    }

    void onRead(int32_t result) {
        if (result < 0) {
            onError(result);
            return;
        } else if (result == 0) {
            urlLoader.Close();
            onComplete.Run(PP_OK);
        } else {
            auto bytes = result < bufferSize ? result : bufferSize;
            data.reserve(data.size() + bytes);
            data.insert(data.end(), buffer, buffer + bytes);
            readBody();
        }
    }

    void onError(int32_t result) {
        printf("Downloader::onError(%s) result = %i\n", url.c_str(), result);
        onComplete.Run(result);
    }

    const char* getData() const {
        return &*data.begin();
    }

    const size_t getLength() const {
        return data.size();
    }
private:
    pp::Instance* instance;
    std::string url;
    pp::URLRequestInfo urlRequestInfo;
    pp::CompletionCallbackFactory<Downloader> ccFactory;
    pp::CompletionCallback onComplete;
    pp::URLLoader urlLoader;

    static const int32_t bufferSize = 4096;
    char buffer[bufferSize];
    std::vector<char> data;
};

struct GameDownloader {
    GameDownloader(pp::Instance* instance, pp::FileSystem* fs)
        : instance(instance)
        , ccFactory(this)
        , fileSystem(fs)
    { }

    void start(pp::CompletionCallback oc) {
        onComplete = oc;
        printf("Fetching manifest\n");
        downloader.reset(new Downloader(instance));
        auto cc = ccFactory.NewCallback(&GameDownloader::gotManifest);
        downloader->get("http://localhost:5013/v1/sully/manifest.txt", cc);
    }

    void gotManifest(int32_t result) {
        if (result != PP_OK) {
            printf("Failed to get manifest :( result=%i\n", result);
            return;
        }

        auto data = std::string(downloader->getData(), downloader->getLength());
        do {
            auto p = data.find('\n');
            if (p == std::string::npos) {
                appendToManifest(data);
                break;
            }

            auto file = data.substr(0, p);
            appendToManifest(file);
            data.erase(0, p + 1);
        } while (true);

        getNextFile();
    }

    void appendToManifest(const std::string& s) {
        if (s.length() > 0) {
            printf("manifest\t'%s'\n", s.c_str());
            manifest.push_back(s);
        }
    }

    void getNextFile() {
        if (manifest.size() == 0) {
            onComplete.Run(PP_OK);
            return;
        }

        auto next = manifest.back();
        printf("next asset is '%s'\n", next.c_str());
        manifest.pop_back();
        auto url = "http://localhost:5013/v1/sully/" + next;
        auto cc = ccFactory.NewCallback(&GameDownloader::gotFile, next);
        downloader.reset(new Downloader(instance));
        downloader->get(url, cc);
    }

    void gotFile(int32_t result, std::string currentFile) {
        if (result != PP_OK) {
            printf("Failed ;_;\n");
            return;
        }

        printf("GameDownloader::gotFile '%s' OK\n", currentFile.c_str());

        auto d = downloader->getData();
        auto l = downloader->getLength();
        verge::vset(currentFile, verge::DataVec(d, d + l));
        getNextFile();

        /*
        auto fr = pp::FileRef(*fileSystem, ("/" + currentFile).c_str());
        fio.reset(new pp::FileIO(instance));

        auto cb = ccFactory.NewCallback(&GameDownloader::openedFile, currentFile);
        fio->Open(fr, PP_FILEOPENFLAG_CREATE | PP_FILEOPENFLAG_WRITE | PP_FILEOPENFLAG_TRUNCATE, cb);
        */
    }

    void openedFile(int32_t result, std::string currentFile) {
        if (result != PP_OK) {
            printf("FileIO::Open failed %i\n", result);
            return;
        }

        auto cb = ccFactory.NewCallback(&GameDownloader::fileWriteComplete, currentFile);
        fio->Write(0, downloader->getData(), downloader->getLength(), cb);
    }

    void fileWriteComplete(int32_t writeResult, std::string currentFile) {
        if (writeResult < 0) {
            printf("FileIO::Write failed result=%i\n", writeResult);
            return;
        }

        auto bytesWritten = writeResult;
        printf("GameDownloader::Wrote %i bytes to %s OK\n", bytesWritten, currentFile.c_str());

        getNextFile();
    }

private:
    pp::Instance* instance;
    pp::CompletionCallbackFactory<GameDownloader> ccFactory;
    pp::FileSystem* fileSystem;
    std::shared_ptr<Downloader> downloader;
    std::shared_ptr<pp::FileIO> fio;
    std::vector<std::string> manifest;
    pp::CompletionCallback onComplete;
};

struct V1naclInstance
    : public pp::Instance
    , IFramebuffer
{
    explicit V1naclInstance(PP_Instance instance, pp::Module* module)
        : pp::Instance(instance)
        , module(module)
        , ccfactory(this)
        , fileSystem(this, PP_FILESYSTEMTYPE_LOCALTEMPORARY)
        , gameDownloader(this, &fileSystem)
        , graphics(0)
        , backBuffer(0)
    {
        pthread_mutex_init(&bbMutex, 0);
        pthread_mutex_init(&inputMutex, 0);

        RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE);
        RequestFilteringInputEvents(PP_INPUTEVENT_CLASS_KEYBOARD);
    }

    virtual ~V1naclInstance() {
    }

    virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]) {
        graphics = new pp::Graphics2D(this, pp::Size(320, 240), true);
        auto result = BindGraphics(*graphics);
        if (!result) {
            printf("BindGraphics failed %i\n", result);
            return false;
        }

        backBuffer = new pp::ImageData(
            this,
            PP_IMAGEDATAFORMAT_BGRA_PREMUL,
            pp::Size(320, 200),
            false
        );

        auto cb = ccfactory.NewCallback(&V1naclInstance::fileSystemIsOpen);
        fileSystem.Open(5000, cb);
        return true;
    }

    virtual bool HandleInputEvent(const pp::InputEvent& event) {
        verge::InputEvent ie;

        switch (event.GetType()) {
            case PP_INPUTEVENT_TYPE_KEYDOWN:
                ie.type = verge::EventType::KeyDown;
                break;
            case PP_INPUTEVENT_TYPE_KEYUP:
                ie.type = verge::EventType::KeyUp;
                break;
            default:
                // Don't care.
                return false;
        }

        pp::KeyboardInputEvent kie(event);
        ie.keyCode = kie.GetKeyCode();



        ScopedLock sl(inputMutex);
        eventQueue.push_back(ie);

        return true;
    }

    void fileSystemIsOpen(int32_t result) {
        if (result != 0) {
            printf("FileSystem::Open failed %i\n", result);
            return;
        }

        auto cb = ccfactory.NewCallback(&V1naclInstance::downloadComplete);
        gameDownloader.start(cb);
    }

    virtual void HandleMessage(const pp::Var& var_message) {
    }

    void downloadComplete(int32_t result) {
       if (result != PP_OK) {
            printf("Download failed :(\n");
            return;
       } else {
           printf("Download complete!  Starting...\n");

           auto result = pthread_create(&thread, 0, &_run, this);
           assert(0 == result && "pthread_create failed");
       }
    }

    static void* _run(void* ctx) {
        auto self = reinterpret_cast<V1naclInstance*>(ctx);
        return self->run();
    }

    void* run() {
        MiscSetup();
        PutOwnerText();
        initvga(this);
        InitItems();

        while (1) {
            qabort = 0;
            /* -- ric: 01/Jun/98 --
             * These variables set to allow the vc layer functions to work
             * by preventing the engine from trying to draw a non-existant
             * map
             */
            cameratracking = 0;
            layer0 = 0;
            layer1 = 0;
            drawparty = 0;
            drawentities = 0;

            StartupScript();
        }
        return 0;
    }

    uint32_t _8to32(unsigned char c, unsigned char* pal) {
        return 0xFF000000
            | pal[c * 3] << 16
            | pal[c * 3 + 1] << 8
            | pal[c * 3 + 2];
    }

    virtual void getInputEvents(std::vector<verge::InputEvent>& events) {
        ScopedLock sl(inputMutex);

        events.swap(eventQueue);
        eventQueue.resize(0);
    }

    virtual void vgadump(unsigned char* framebuffer, unsigned char* palette) {
        printf("vgadump\n");

        if (0 != pthread_mutex_lock(&bbMutex)) {
            assert(!"Failed to acquire mutex\n");
        }

        auto src = framebuffer;
        uint32_t* dst = (uint32_t*)backBuffer->data();
        assert(0 == (backBuffer->stride() % sizeof(uint32_t)));
        const auto stride = backBuffer->stride() / sizeof(uint32_t);

        for (int y = 0; y < YRES; ++y) {
            for (int x = 0; x < XRES; ++x) {
                dst[x] = _8to32(*src, palette);
                //printf("%i,%i = %i / %08X\n", x, y, *src, dst[x]);
                src++;
            }
            dst += stride;
        }

        module->core()->CallOnMainThread(0, pp::CompletionCallback(&V1naclInstance::_present, this));
    }

    static void _present(void* data, int32_t blah) {
        auto self = static_cast<V1naclInstance*>(data);
        self->present();
    }

    void present() {
        graphics->PaintImageData(*backBuffer, pp::Point());
        pp::CompletionCallback cb = ccfactory.NewCallback(&V1naclInstance::presentComplete);
        graphics->Flush(cb);
    }

    void presentComplete(int32_t result) {
        if (0 != pthread_mutex_unlock(&bbMutex)) {
            printf("Failed to release mutex\n");
        }
    }

private:
    enum { XRES = 320, YRES = 200 };

    pp::Module* module;
    pp::CompletionCallbackFactory<V1naclInstance> ccfactory;
    pp::FileSystem fileSystem;
    GameDownloader gameDownloader;

    pp::Graphics2D* graphics;
    pp::ImageData* backBuffer;
    std::vector<verge::InputEvent> eventQueue;

    pthread_t thread;
    pthread_mutex_t bbMutex;
    pthread_mutex_t inputMutex;
};

class V1naclModule : public pp::Module {
public:
    V1naclModule() : pp::Module() {}
    virtual ~V1naclModule() {}

    virtual pp::Instance* CreateInstance(PP_Instance instance) {
        return new V1naclInstance(instance, this);
    }
};

namespace pp {
    Module* CreateModule() {
        return new V1naclModule();
    }
}
