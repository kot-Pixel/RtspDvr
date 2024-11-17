//
// Created by Admin on 2024/10/27.
//

#include "../include/KtRtspClient.h"

void doEstablishRtsp(int a, int b) {
    LOGI("mKtRtspClient invoke and %p", mKtRtspClient);
    mKtRtspClient->establishRtsp();
}


int main() {
    mKtRtspClient =  KtRtspClient::createNew("rtsp://192.168.2.104:8554/cam");
    mKtRtspClient->interface->mapper->registerFunction("doEstablishRtsp", [](std::vector<std::any> args) -> std::any {
        if (args.size() != 2) {
            throw std::invalid_argument("Invalid number of arguments");
        }
        int a = std::any_cast<int>(args[0]);
        int b = std::any_cast<int>(args[1]);
        doEstablishRtsp(a, b);
        return "invoked";
    });

    mKtRtspClient->interface->reqLooperInner();
//    mKtRtspClient->establishRtsp();

//    mKtRtspClient->startRecordLocalAudio();
    return 0;
}
