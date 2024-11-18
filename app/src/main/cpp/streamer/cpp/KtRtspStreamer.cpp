//
// Created by Admin on 2024/10/27.
//

#include "../include/KtRtspClient.h"

int doEstablishRtsp(int a, int b) {
    LOGI("mKtRtspClient invoke and %p", mKtRtspClient);
    std::thread t([&]() {
        if (mKtRtspClient) {
            mKtRtspClient->establishRtsp();
        }
    });
    t.detach(); // 分离线程，主线程无需等待
    return 1;
}

int startRecord(int start) {
    mKtRtspClient->mWriteFlag.store(true,std::memory_order_release);
//    LOGI("mKtRtspClient startRecord invoke %d", mKtRtspClient->mWriteFlag.store(true));
    return 1;
}

int stopRecord(int end) {
    mKtRtspClient->mWriteFlag.store(false,std::memory_order_release);
    return 1;
}


int main() {
    mKtRtspClient =  KtRtspClient::createNew("rtsp://192.168.2.104:8554/cam");
    mKtRtspClient->interface->mapper->registerFunction("doEstablishRtsp", [](std::vector<std::any> args) -> std::any {
        if (args.size() != 2) {
            throw std::invalid_argument("Invalid number of arguments");
        }
        int a = std::any_cast<int>(args[0]);
        int b = std::any_cast<int>(args[1]);
        return doEstablishRtsp(a, b);
    });

    mKtRtspClient->interface->mapper->registerFunction("startRecord", [](std::vector<std::any> args) -> std::any {
        int a = std::any_cast<int>(args[0]);
        return startRecord(a);
    });

    mKtRtspClient->interface->mapper->registerFunction("stopRecord", [](std::vector<std::any> args) -> std::any {
        int a = std::any_cast<int>(args[0]);
        return stopRecord(a);
    });

    mKtRtspClient->interface->reqLooperInner();
//    mKtRtspClient->establishRtsp();

//    mKtRtspClient->startRecordLocalAudio();
    return 0;
}
