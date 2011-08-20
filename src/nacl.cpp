
#include <cstdio>
#include <string>
#include <memory>
#include "ppapi/c/ppb_file_io.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/file_system.h"
#include "ppapi/cpp/file_ref.h"
#include "ppapi/cpp/file_io.h"
#include "ppapi/cpp/url_request_info.h"
#include "ppapi/cpp/url_loader.h"
#include "ppapi/cpp/graphics_2d.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/size.h"
#include "ppapi/cpp/var.h"

#include "main.h"
#include "render.h"
#include "vc.h"
#include "fs.h"

void MiscSetup();
void PutOwnerText();
void initvga(pp::Graphics2D* g2d, pp::ImageData* bb);
void InitItems();

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

struct V1naclInstance : public pp::Instance {
    explicit V1naclInstance(PP_Instance instance)
        : pp::Instance(instance)
        , ccfactory(this)
        , fileSystem(this, PP_FILESYSTEMTYPE_LOCALTEMPORARY)
        , gameDownloader(this, &fileSystem)
        , graphics(0)
        , backBuffer(0)
    {}

    virtual ~V1naclInstance() {
    }

#if 0
    void DidChangeView(const pp::Rect& position, const pp::Rect& clip) {
    }
#endif

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
        }
        printf("Download complete!  Starting...\n");

        MiscSetup();
        PutOwnerText();
        initvga(graphics, backBuffer);
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
    }

private:
    pp::CompletionCallbackFactory<V1naclInstance> ccfactory;
    pp::FileSystem fileSystem;
    GameDownloader gameDownloader;

    pp::Graphics2D* graphics;
    pp::ImageData* backBuffer;
};

class V1naclModule : public pp::Module {
public:
    V1naclModule() : pp::Module() {}
    virtual ~V1naclModule() {}

    virtual pp::Instance* CreateInstance(PP_Instance instance) {
        return new V1naclInstance(instance);
    }
};

namespace pp {
    Module* CreateModule() {
        return new V1naclModule();
    }
}
