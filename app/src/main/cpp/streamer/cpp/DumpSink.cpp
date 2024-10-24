#include "../include/DummySink.h"
#include "../include/sdpUtils.h"
#include "zmq.h"
#include "../include/log_utils.h"
#include <fstream>

#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 100000

const u_int8_t naulHeader[] = {
        0x0, 0x0, 0x0, 0x1
};

//
// Created by Tom on 2024/10/11.
//
DummySink *
DummySink::createNew(UsageEnvironment &env, MediaSubsession &subsession, char const *streamId,
                     void *responder, AMediaCodec *codec) {
    return new DummySink(env, subsession, streamId, responder, codec);
}

DummySink::DummySink(UsageEnvironment &env, MediaSubsession &subsession, char const *streamId,
                     void *responder, AMediaCodec *codec)
        : MediaSink(env),
          fSubsession(subsession) {
    fStreamId = strDup(streamId);
    socket = responder;
    fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
    fcodec = codec;
}

DummySink::~DummySink() {
    delete[] fReceiveBuffer;
    delete[] fStreamId;
}

void DummySink::afterGettingFrame(void *clientData, unsigned frameSize, unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds) {
    DummySink *sink = (DummySink *) clientData;
    sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1

zmq_msg_t message2;

void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned /*durationInMicroseconds*/) {
    //printf("afterGettingFrame invoke %d - %d", fSocketId, frameSize);
    if (frameSize < 2) return; // 确保有足够的数据

//    std::ofstream outputFile(filename, std::ios::binary | std::ios::app);
//    if (!outputFile) {
//        std::cerr << "Error: Could not open the file for writing." << std::endl;
//    }
//
//
//    // 写入 vector 的数据到文件
//    outputFile.write(reinterpret_cast<const char*>(naulHeader), 4);
//    outputFile.write(reinterpret_cast<const char*>(fReceiveBuffer), frameSize);
//    if (!outputFile) {
//        std::cerr << "Error: Could not write data to the file." << std::endl;
//    }
//
//    outputFile.close();

    // 确保 fReceiveBuffer 包含完整的 NALU
    uint8_t nalHeader = fReceiveBuffer[0]; // 第一个字节是 NALU 头
//    uint8_t nalHeader2 = fReceiveBuffer[1]; // 第一个字节是 NALU 头
//    uint8_t nalHeader3 = fReceiveBuffer[2]; // 第一个字节是 NALU 头
//    uint8_t nalHeader4 = fReceiveBuffer[3]; // 第一个字节是 NALU 头

    // 打印 NALU 头信息
    //printf("NALU Header: 0x%02X\n", nalHeader);
//    printf("sdp decription: %s\n", fSubsession.savedSDPLines());


//    ssize_t bufIdx = AMediaCodec_dequeueInputBuffer(fcodec, 0);
//    if (bufIdx >= 0) {
//        size_t bufSize;
//        uint8_t* inputBuf = AMediaCodec_getInputBuffer(fcodec, bufIdx, &bufSize);
//        memcpy(inputBuf, naulHeader, 4);
//        memcpy(inputBuf + 4, fReceiveBuffer, frameSize);

//        if (inputBuf != NULL) {
//            printf("inputBuf data (all bytes): ");
//            for (size_t i = 0; i < bufSize; i++) {
//                printf("%02x ", inputBuf[i]);
//            }
//            printf("\n");
//        } else {
//            printf("inputBuf is NULL\n");
//        }

//        bufIdx = AMediaCodec_queueInputBuffer(fcodec, bufIdx, 0, frameSize, 0, 0);
//        if (bufIdx == -2) {
//            printf("AMediaCodec bufIdx format changed\n");
//        } else if (bufIdx == -1) {
//            printf("AMediaCodec input try later\n");
//        } else {
//            printf("AMediaCodec bufIdx index\n", bufIdx);
//        }
//    }

//    // 查找 fmtp 行
//    std::string sdpLines(fSubsession.savedSDPLines());
//    std::string fmtpLine;
//    std::istringstream sdpStream(sdpLines);
//    std::string line;
//
//    while (std::getline(sdpStream, line)) {
//        if (line.find("a=fmtp:") != std::string::npos) {
//            fmtpLine = line;
//            break; // 找到 fmtp 行后退出循环
//        }
//    }
//
//    if (!fmtpLine.empty()) {
//        extractSpropParameterSets(fmtpLine);
//    }
//
//    const char* sps_base64 = "your_sps_base64_string_here";
//    const char* pps_base64 = "your_pps_base64_string_here";
//
//    std::vector<unsigned char> sps = stpStringBase64Decode(sps_base64);
//    std::vector<unsigned char> pps = stpStringBase64Decode(pps_base64);
//

    zmq_msg_init_data(&message2, fReceiveBuffer, frameSize, nullptr, nullptr);
    // 发送消息,非阻塞信息发送。
    int bytes_sent = zmq_msg_send(&message2, socket, ZMQ_DONTWAIT);

    // 查询当前接收缓冲区大小
    int rcv_buffer_size;
    size_t rcv_buffer_size_len = sizeof(rcv_buffer_size);
    std::cout << "Receive buffer size: " << rcv_buffer_size << std::endl;

    if (bytes_sent == -1) {
        // 发送失败，处理错误
        int errnum = zmq_errno();
        printf("Error sending message: %s\n", zmq_strerror(errnum));
    } else {
        // 发送成功，bytes_sent 包含发送的字节数
        printf("Sent %d bytes\n", bytes_sent);
    }
    // 清理
    zmq_msg_close(&message2);

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
