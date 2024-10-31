//
// Created by Tom on 2024/10/31.
//

#ifndef SOCKECTDEMO2_KTRTPFRAME_H
#define SOCKECTDEMO2_KTRTPFRAME_H

#include <cstdint>
#include <malloc.h>
#include <cstring>

class KtRtpFrame {
public:
    int64_t mRtpFramePts = -1L;
    uint8_t *mRtpFramePointer = nullptr;
    int mRtpFrameSize = 0;

    KtRtpFrame(int64_t rtpFramePts, int rtpFrameSize, uint8_t *rtpFramePointer);
    virtual ~KtRtpFrame();
};
#endif //SOCKECTDEMO2_KTRTPFRAME_H
