//
// Created by Tom on 2024/10/11.
//

#include "../include/KtRtspClient.h"
#include <jni.h>

KtRtspClient* KtRtspClient::createNew(UsageEnvironment& env, char const* rtspURL,
                                       int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) {
    return new KtRtspClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

KtRtspClient::KtRtspClient(UsageEnvironment& env, char const* rtspURL,
                             int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
        : RTSPClient(env,rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1) {
}

KtRtspClient::~KtRtspClient() {
}
