//
// Created by Admin on 2024/10/27.
//

#include "../include/KtRtspClient.h"

static KtRtspClient* mKtRtspClient = nullptr;

int main() {
    mKtRtspClient =  KtRtspClient::createNew("rtsp://192.168.2.104:8554/cam");
    mKtRtspClient->establishRtsp();
//    mKtRtspClient->startRecordLocalAudio();
    return 0;
}
