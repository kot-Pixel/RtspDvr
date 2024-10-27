//
// Created by Tom on 2024/10/11.
//

#ifndef SOCKECTDEMO2_KTRTSPCLIENT_H
#define SOCKECTDEMO2_KTRTSPCLIENT_H

#include "KtStreamClientState.h"
//#include "../../live555/include/UsageEnvironment/UsageEnvironment.hh"
//#include "../../live555/include/liveMedia/RTSPClient.hh"
//#include "../../live555/include/groupsock/NetAddress.hh"
extern "C" {
#include "../../ffmpeg/include/libavformat/avformat.h"
#include "../../ffmpeg/include/libavutil/avutil.h"
}



class KtRtspClient {
public:
    static KtRtspClient *createNew(char const *rtspURL);
    void establishRtsp();
protected:
    KtRtspClient(char const *rtspURL);
    void print_sdp_info();

    virtual ~KtRtspClient();

private :
    char const *mRtspUrl = nullptr;
    AVFormatContext* format_ctx = nullptr;
    int video_stream_index = -1;
};

#endif //SOCKECTDEMO2_KTRTSPCLIENT_H
