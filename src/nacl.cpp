
#include <cstdio>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
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
#include "audiere/audiere.h"
#include "audiere/device_nacl.h"

#include "fs.h"
#include "main.h"
#include "nacl.h"
#include "render.h"
#include "timer.h"
#include "vc.h"
#include "base64.h"

void MiscSetup();
void PutOwnerText();
void initvga();
void InitItems();

#if 0
void __cyg_profile_func_enter (void *func, void *caller) __attribute__((no_instrument_function));
void __cyg_profile_func_exit (void *func, void *caller) __attribute__((no_instrument_function));

__thread int tid;
void __cyg_profile_func_enter(void *this_fn, void *call_site) {
  printf("Thread %p entering %p\n", &tid, this_fn);
}

void __cyg_profile_func_exit(void *this_fn, void *call_site) {
  printf("Thread %p exiting %p\n", &tid, this_fn);
}
#endif

namespace verge {
    verge::IFramebuffer* plugin = 0;

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
}

namespace audiere {
  SampleSource* OpenSource(
    const FilePtr& file,
    const char* filename,
    FileFormat file_format);
}

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
    GameDownloader(pp::Instance* instance)
        : instance(instance)
        , ccFactory(this)
        , game_url("sully/")
    { }

    void start(pp::CompletionCallback oc) {
        onComplete = oc;
        downloader.reset(new Downloader(instance));
        auto cc = ccFactory.NewCallback(&GameDownloader::gotManifest);
        downloader->get(game_url + "manifest.txt", cc);
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
            manifest.push_back(s);
        }
    }

    void getNextFile() {
        if (manifest.size() == 0) {
            onComplete.Run(PP_OK);
            return;
        }

        auto next = manifest.back();
        manifest.pop_back();
        auto url = game_url + next;
        auto cc = ccFactory.NewCallback(&GameDownloader::gotFile, next);
        downloader.reset(new Downloader(instance));
        downloader->get(url, cc);
    }

    void gotFile(int32_t result, std::string currentFile) {
        if (result != PP_OK) {
            printf("Failed ;_;\n");
            return;
        }

        //printf("GameDownloader::gotFile '%s' OK\n", currentFile.c_str());

        auto d = downloader->getData();
        auto l = downloader->getLength();
        verge::vset(currentFile, verge::DataVec(d, d + l));
        getNextFile();
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

        getNextFile();
    }

private:
    pp::Instance* instance;
    pp::CompletionCallbackFactory<GameDownloader> ccFactory;
    std::shared_ptr<Downloader> downloader;
    std::shared_ptr<pp::FileIO> fio;
    std::vector<std::string> manifest;
    pp::CompletionCallback onComplete;
    const std::string game_url;
};

struct V1naclInstance
    : public pp::Instance
    , verge::IFramebuffer
{
    explicit V1naclInstance(PP_Instance instance, pp::Module* module)
        : pp::Instance(instance)
        , module(module)
        , ccfactory(this)
        , fileSystem(this, PP_FILESYSTEMTYPE_LOCALTEMPORARY)
        , gameDownloader(this)
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
        verge::plugin = this;

#ifdef VERGE_AUDIO
        audioDevice = new audiere::NaclAudioDevice(this);
#endif

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
            case PP_INPUTEVENT_TYPE_UNDEFINED:
            case PP_INPUTEVENT_TYPE_MOUSEDOWN:
            case PP_INPUTEVENT_TYPE_MOUSEUP:
            case PP_INPUTEVENT_TYPE_MOUSEMOVE:
            case PP_INPUTEVENT_TYPE_MOUSEENTER:
            case PP_INPUTEVENT_TYPE_MOUSELEAVE:
            case PP_INPUTEVENT_TYPE_WHEEL:
            case PP_INPUTEVENT_TYPE_RAWKEYDOWN:
            case PP_INPUTEVENT_TYPE_CHAR:
            case PP_INPUTEVENT_TYPE_CONTEXTMENU:
            default:
                // Don't care.
                return false;
        }

        pp::KeyboardInputEvent kie(event);
        ie.keyCode = kie.GetKeyCode();

        verge::ScopedLock sl(inputMutex);
        eventQueue.push_back(ie);

        return true;
    }

    void tick(int32_t) {
        incTimerCount();

        auto cb = ccfactory.NewCallback(&V1naclInstance::tick);
        module->core()->CallOnMainThread(10, cb);
    }

    void fileSystemIsOpen(int32_t result) {
        if (result != 0) {
            printf("FileSystem::Open failed %i\n", result);
            return;
        }

        auto cb = ccfactory.NewCallback(&V1naclInstance::downloadComplete);
        gameDownloader.start(cb);
    }

    std::vector<std::string> saveGameBase64;

    // FIXME: This races with engine initialization.
    // We should probably wait on a "start" message before spinning off the game thread
    // to fix this.
    virtual void HandleMessage(const pp::Var& var_message) {
        saveGameBase64.resize(4);

        auto sm(var_message.AsString());
        auto p = sm.find(':');
        auto command = sm.substr(0, p);
        auto index = sm.substr(p + 1)[0] - '0';

        //printf("HandleMessage %s\n", sm.substr(0, 80).c_str());

        if (index < 0 || index > 3) {
            printf("Bad command index %i\n", index);
            return;
        }

        if (command == "clear") {
            saveGameBase64.at(index).clear();
        } else if (command == "append") {
            // sm[p] == ':'
            // sm[p + 1] == 0, 1, 2, or 3
            // sm[p + 2] == ':'
            // sm[p + 3] == first byte of base64'd gunk
            saveGameBase64.at(index).append(sm.substr(p + 3));
        } else if (command == "close") {
            std::string fname("SAVEDAT.00");
            fname += char('0' + index);

            const auto de64 = base64::decode(saveGameBase64.at(index));

            printf("Got savegame %s from localStorage.  %i bytes\n", fname.c_str(), de64.length());
            verge::vset(fname.c_str(), verge::DataVec(de64.begin(), de64.end()));
            saveGameBase64.at(index).clear();
        }
    }

    void downloadComplete(int32_t result) {
       if (result != PP_OK) {
            printf("Download failed :(\n");
            return;
       } else {
           printf("Download complete!  Starting...\n");

           auto result = pthread_create(&thread, 0, &_run, this);
           assert(0 == result && "pthread_create failed");

           tick(0);
       }
    }

    static void* _run(void* ctx) {
        auto self = reinterpret_cast<V1naclInstance*>(ctx);
        return self->run();
    }

    void* run() {
        MiscSetup();
        PutOwnerText();
        initvga();
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
        verge::ScopedLock sl(inputMutex);

        events.swap(eventQueue);
        eventQueue.resize(0);
    }

    struct PersistSaveRequestData {
        V1naclInstance* instance;
        const std::string fileName;
        std::string saveData;

        PersistSaveRequestData(V1naclInstance* instance, const std::string& fileName, const std::string& saveData)
            : instance(instance)
            , fileName(fileName)
            , saveData(saveData)
        { }
    };

    // Caution: Should only be called by the game thread
    virtual void persistSave(const std::string& fileName, const std::string& saveData) {
        auto psr = new PersistSaveRequestData(this, fileName, saveData);
        module->core()->CallOnMainThread(
            0,
            pp::CompletionCallback(&V1naclInstance::_persistSave, psr)
        );
    }

    // Caution: Should only be called by the game thread
    virtual void vgadump(unsigned char* framebuffer, unsigned char* palette) {
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
                src++;
            }
            dst += stride;
        }

        module->core()->CallOnMainThread(0, pp::CompletionCallback(&V1naclInstance::_present, this));
    }

    struct LoadSoundRequest {
        V1naclInstance* self;
        const std::string fileName;
        LoadSoundRequest(V1naclInstance* self, const std::string& fileName)
            : self(self)
            , fileName(fileName)
        {}
    };

    virtual void loadSound(const std::string& fileName) {
        auto lsr = new LoadSoundRequest(this, fileName);
        module->core()->CallOnMainThread(
            0,
            pp::CompletionCallback(&V1naclInstance::_loadSound, lsr)
        );
    }

    static void _loadSound(void* data, int32_t blah) {
        auto lsr = static_cast<LoadSoundRequest*>(data);
        lsr->self->__loadSound(lsr->fileName);
        delete lsr;
    }

    void __loadSound(const std::string& fileName) {
#ifdef VERGE_AUDIO
        printf("V1naclInstance::__loadSound(%s) index %i\n", fileName.c_str(), soundEffects.size());
        soundEffects.push_back(audiere::OpenSound(audioDevice, fileName.c_str(), false));
#endif
    }

    struct PlaySongRequest {
        V1naclInstance* self;
        const std::string songName;

        PlaySongRequest(V1naclInstance* self, const std::string songName)
            : self(self)
            , songName(songName)
            { }
    };

    virtual void playSong(const std::string& songName) {
        auto psr = new PlaySongRequest(this, songName);
        module->core()->CallOnMainThread(
            0,
            pp::CompletionCallback(&V1naclInstance::_playSong, psr)
        );
    }

    static void _playSong(void* data, int32_t bleh) {
        auto psr = static_cast<PlaySongRequest*>(data);
        psr->self->__playSong(psr->songName);
        delete psr;
    }

    void __playSong(const std::string& songName) {
#ifdef VERGE_AUDIO
        printf("V1naclInstance::__playSong(%s)\n", songName.c_str());
        auto f = verge::vopen(songName.c_str(), "r");
        auto file_format = audiere::FF_MOD;
        auto ss = audiere::OpenSource(audiere::FilePtr(f), songName.c_str(), file_format);
        currentMusic = audiere::OpenSound(audioDevice, ss, false);
        if (currentMusic) {
            currentMusic->setRepeat(true);
            currentMusic->play();
        }
        verge::vclose(f);
#else
        printf("V1naclInstance::__playSong(%s) but audio is disabled\n", songName.c_str());
#endif
    }

    struct PlayEffectRequest {
        V1naclInstance* self;
        size_t index;
        PlayEffectRequest (V1naclInstance* self, size_t index)
            : self(self)
            , index(index)
        { }
    };

    virtual void playEffect(size_t index) {
        return;

        auto per = new PlayEffectRequest(this, index);
        module->core()->CallOnMainThread(
            0,
            pp::CompletionCallback(&V1naclInstance::_playEffect, per)
        );
    }

    static void _playEffect(void* data, int32_t) {
        auto per = static_cast<PlayEffectRequest*>(data);
        per->self->__playEffect(per->index);
        delete per;
    }

    void __playEffect(size_t index) {
        printf("V1naclInstance::__playEffect(%i) size=%i\n", index, soundEffects.size());

#ifdef VERGE_AUDIO
        if (0 <= index && index < soundEffects.size()) {
            auto s = soundEffects[index];
            printf("ptr -> %p\n", s.get());
            soundEffects[index]->play();
        }
#endif
    }

    virtual void stopSound() {
        return;
#ifdef VERGE_AUDIO
        for (auto i = 0; i < soundEffects.size(); ++i) {
            soundEffects[i]->stop();
        }
        currentMusic->stop();
#endif
    }

    virtual void setVolume(int volume) {
        return;
#ifdef VERGE_AUDIO
        auto normalizedVolume = float(volume) / 100.0f;
        currentMusic->setVolume(normalizedVolume);
#endif
    }

    virtual void setSongPos(int songPos) {
    }

private:
    static void _persistSave(void* data, int32_t blah) {
        auto psr = static_cast<PersistSaveRequestData*>(data);
        psr->instance->reallyPersistSave(psr->fileName, psr->saveData);
        delete psr;
    }

    void reallyPersistSave(const std::string& fileName, std::string& saveData) {
        // HACK:
        int index = 5;
        auto lastChar = fileName[fileName.length() - 1];
        if (lastChar >= '0' && lastChar <= '3') {
            index = lastChar - '0';
        } else {
            printf("reallyPersistSave bad fname %s index %i\n", fileName.c_str(), index);
            return;
        }

        auto b64(base64::encode(saveData));

        std::stringstream ss;
        ss << "clear:" << index;
        PostMessage(pp::Var(ss.str()));

        const auto SLICE_SIZE = 4096;
        size_t sliceIndex = 0;

        while (sliceIndex < b64.length()) {
            ss.str(std::string());
            ss << "append:" << index << ":" << b64.substr(sliceIndex, SLICE_SIZE);
            sliceIndex += SLICE_SIZE;
            PostMessage(pp::Var(ss.str()));
        }
        printf("Persisted save %s.  %i bytes\n", fileName.c_str(), saveData.length());
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

#ifdef VERGE_AUDIO
    audiere::AudioDevicePtr audioDevice;
    std::vector<audiere::OutputStreamPtr> soundEffects;
    audiere::OutputStreamPtr currentMusic;
#endif
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
