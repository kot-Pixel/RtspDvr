//
// Created by Tom on 2024/10/31.
//

#include "../include/KtRtpFrame.h"

KtRtpFrame::KtRtpFrame(int64_t rtpFramePts, int rtpFrameSize, uint8_t *rtpFramePointer) {
    mRtpFramePointer = (uint8_t*)malloc(rtpFrameSize);
    memcpy(mRtpFramePointer, rtpFramePointer, rtpFrameSize);
    mRtpFramePts = rtpFramePts;
    mRtpFrameSize = rtpFrameSize;
}

KtRtpFrame::~KtRtpFrame() {
    delete[] mRtpFramePointer;
    mRtpFramePointer = nullptr;
    mRtpFramePts = -1;
    mRtpFrameSize = 0L;
}
