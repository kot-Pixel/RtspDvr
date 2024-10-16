//
// Created by Tom on 2024/10/11.
//

#ifndef SOCKECTDEMO2_KTRTSPCLIENT_H
#define SOCKECTDEMO2_KTRTSPCLIENT_H

#include "KtStreamClientState.h"
#include "../../live555/include/UsageEnvironment/UsageEnvironment.hh"
#include "../../live555/include/liveMedia/RTSPClient.hh"
#include "../../live555/include/groupsock/NetAddress.hh"


class KtRtspClient: public RTSPClient {
public:
    static KtRtspClient* createNew(UsageEnvironment& env, char const* rtspURL,
                                    int verbosityLevel = 0,
                                    char const* applicationName = NULL,
                                    portNumBits tunnelOverHTTPPortNum = 0);

protected:
    KtRtspClient(UsageEnvironment& env, char const* rtspURL,
                  int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);

    virtual ~KtRtspClient();

public:
    KtStreamClientState scs;
};

#endif //SOCKECTDEMO2_KTRTSPCLIENT_H
