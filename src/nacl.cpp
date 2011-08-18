
#include <cstdio>
#include <string>
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/file_system.h"
#include "ppapi/cpp/url_request_info.h"
#include "ppapi/cpp/url_loader.h"
#include "ppapi/cpp/file_io.h"
#include "ppapi/cpp/var.h"

void MiscSetup();
void PutOwnerText();
void initvga();
void InitItems();

struct Downloader {
    explicit Downloader(pp::Instance* instance)
        : instance(instance)
          //, url(url)
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

struct V1naclInstance : public pp::Instance {
    explicit V1naclInstance(PP_Instance instance)
        : pp::Instance(instance)
        , ccfactory(this)
        , downloader(this)
    {}

    virtual ~V1naclInstance() {
    }

    virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]) {
        /*MiscSetup();
        PutOwnerText();
        initvga();
        InitItems();*/

        printf("Init\n");
        auto cb = ccfactory.NewCallback(&V1naclInstance::download);
        downloader.get("http://localhost:5013/v1/sully/SETUP.CFG", cb);
        printf("Dispatched ok\n");

        return true;
    }

    virtual void HandleMessage(const pp::Var& var_message) {
    }

    void download(int32_t result) {
        if (result == PP_OK) {
            printf("OK! :D\n");
            auto d = std::string(downloader.getData(), downloader.getLength());
            printf("Data: %s", d.c_str());
        } else {
            printf("V1naclInstance::download fail :( result=%i\n", result);
        }
    }

private:
    pp::CompletionCallbackFactory<V1naclInstance> ccfactory;
    Downloader downloader;
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
