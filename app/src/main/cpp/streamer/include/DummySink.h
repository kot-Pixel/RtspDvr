//
// Created by Tom on 2024/10/11.
//

#ifndef SOCKECTDEMO2_DUMMYSINK_H
#define SOCKECTDEMO2_DUMMYSINK_H


#include <media/NdkMediaCodec.h>
#include "../../live555/include/liveMedia/MediaSink.hh"
#include "../../live555/include/UsageEnvironment/UsageEnvironment.hh"
#include "../../live555/include/liveMedia/MediaSession.hh"


class DummySink : public MediaSink {
public:
    static DummySink *createNew(UsageEnvironment &env,
                                MediaSubsession &subsession, // identifies the kind of data that's being received
                                char const *streamId, void *responder,
                                AMediaCodec *codec); // identifies the stream itself (optional)

private:
    DummySink(UsageEnvironment &env, MediaSubsession &subsession, char const *streamId,
              void *responderd, AMediaCodec *codec);

    // called only by "createNew()"
    virtual ~DummySink();

    static void afterGettingFrame(void *clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds);

    void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                           struct timeval presentationTime, unsigned durationInMicroseconds);

private:
    // redefined virtual functions:
    virtual Boolean continuePlaying();

private:
    u_int8_t *fReceiveBuffer;
    MediaSubsession &fSubsession;
    char *fStreamId;
    //int fSocketId;
    void *socket;
    int spsReceived = 0;
    u_int8_t *spsReceiveBuffer;
    int ppsReceived = 0;
    u_int8_t *ppsReceiveBuffer;
    AMediaCodec *fcodec;
    const char *filename = "/sdcard/header.264";
};

#endif //SOCKECTDEMO2_DUMMYSINK_H
