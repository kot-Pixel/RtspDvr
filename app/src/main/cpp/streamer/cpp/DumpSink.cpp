#include "../include/DummySink.h"

#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 100000

//
// Created by Tom on 2024/10/11.
//
DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId, int socket) {
    return new DummySink(env, subsession, streamId, socket);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId, int socket)
        : MediaSink(env),
          fSubsession(subsession) {
    fStreamId = strDup(streamId);
    fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
    fSocketId = socket;
}

DummySink::~DummySink() {
    delete[] fReceiveBuffer;
    delete[] fStreamId;
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
                                  struct timeval presentationTime, unsigned durationInMicroseconds) {
    DummySink* sink = (DummySink*)clientData;
    sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1

void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                                  struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
    //printf("afterGettingFrame invoke %d - %d", fSocketId, frameSize);
    if (frameSize < 2) return; // 确保有足够的数据



    // 确保 fReceiveBuffer 包含完整的 NALU
    uint8_t nalHeader = fReceiveBuffer[0]; // 第一个字节是 NALU 头
    uint8_t nalHeader2 = fReceiveBuffer[1]; // 第一个字节是 NALU 头
    uint8_t nalHeader3 = fReceiveBuffer[2]; // 第一个字节是 NALU 头
    uint8_t nalHeader4 = fReceiveBuffer[3]; // 第一个字节是 NALU 头

    // 打印 NALU 头信息
    printf("NALU Header: 0x%02X\n", nalHeader);
    printf("sdp decription: %s\n", fSubsession.savedSDPLines());

    //printf("NALU Header: 0x%02X\n", nalHeader2);

    //printf("NALU Header: 0x%02X\n", nalHeader3);

    //printf("NALU Header: 0x%02X\n", nalHeader4);

    // 获取 NAL 单元类型 (低6位)
    uint8_t nalUnitType = nalHeader2 & 0x1F;

    if (nalUnitType == 7) {
        // SPS
        printf("Received SPS NAL Unit\n");
        printf("NALU Header: 0x%02X\n", nalHeader);
        if (spsReceived == 0) {
            spsReceived = 1;
//            memcpy(spsReceiveBuffer, fReceiveBuffer, frameSize);
        }
    } else if (nalUnitType == 8) {
        // PPS
        printf("Received PPS NAL Unit\n");
        printf("NALU Header: 0x%02X\n", nalHeader);
        if (ppsReceived == 0) {
            ppsReceived = 1;
//            memcpy(ppsReceiveBuffer, fReceiveBuffer, frameSize);
        }
    } else {
        // 其他 NAL 单元
        //printf("Received NAL Unit Type: %d\n", nalUnitType);
    }

    if (spsReceived && ppsReceived) {
        //printf("sps and pps buffer has create, now send client sps and pps buffer data");
    } else {
        //do noting.......
    }

    continuePlaying();
}

Boolean DummySink::continuePlaying() {
    if (fSource == NULL) return False; // sanity check (should not happen)

    // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
    fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,
                          afterGettingFrame, this,
                          onSourceClosure, this);
    return True;
}
