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
#include <../../zeroMq/include/zmq.h>
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
    //视频流index
    int video_stream_index = -1;
    uint8_t *extradata = NULL;
    int extradata_size = -1;

    //ZMQ Context
    void* mZmqContext = NULL;
    void* mZmqSender = NULL;
    bool mZmqSocketSender = false;
    zmq_msg_t message;
};

#endif //SOCKECTDEMO2_KTRTSPCLIENT_H
