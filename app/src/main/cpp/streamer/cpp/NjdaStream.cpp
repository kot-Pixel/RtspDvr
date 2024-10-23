#include "../include/KtRtspClient.h"
#include "../include/DummySink.h"
#include "../include/log_utils.h"
#include "../include/sdpUtils.h"
#include <../../zeroMq/include/zmq.h>
#include "../../live555/include/BasicUsageEnvironment/BasicUsageEnvironment.hh"

#include <thread>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <media/NdkMediaCodec.h>


static AMediaCodec* codec = nullptr;

int configureAndStartMediaCodec() {
    LOGI("start configure and start media codec");
    codec = AMediaCodec_createDecoderByType("video/avc");
    if (!codec) {
        printf("Failed to create codec");
        return -1;
    }
    AMediaFormat* format = AMediaFormat_new();
    AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/avc");
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, 1280);
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, 720);
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, 0x7F420888);
    media_status_t configure_status = AMediaCodec_configure(codec, format, nullptr, nullptr, 0);
    if (configure_status != AMEDIA_OK) {
        printf("Failed to configure codec");
        AMediaCodec_delete(codec);
        AMediaFormat_delete(format);
        return -1;
    }
    printf("MediaCodec start success");
    media_status_t start_status = AMediaCodec_start(codec);
    if (start_status != AMEDIA_OK) {
        printf("Failed to start codec");
        AMediaCodec_delete(codec);
        AMediaFormat_delete(format);
        return -1;
    }
    return 0;
}

void decodeH264ThreadFunction() {
    if (codec == nullptr) {
        return;
    }
    while (true) {
        AMediaCodecBufferInfo info;
        ssize_t outIdx = AMediaCodec_dequeueOutputBuffer(codec, &info, 2000);  // 等待2秒
        LOGI("result decodeH264ThreadFunction, size: %d",outIdx);
        if (outIdx >= 0) {
            uint8_t* outputBuf = AMediaCodec_getOutputBuffer(codec, outIdx, nullptr);
            // 释放输出缓冲区
            AMediaCodec_releaseOutputBuffer(codec, outIdx, false);
        } else if (outIdx == -1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            LOGI("decodeH264ThreadFunction out of buffer, size: %d",outIdx);
        }
    }
}

int main() {

//    int result = configureAndStartMediaCodec();
//
//    LOGI("success configure and start mediacodec");
//
//    std::thread decodeH264Thread(decodeH264ThreadFunction);
//
//    decodeH264Thread.detach();

//    int server_fd, client_id;
//    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
//
//    struct sockaddr_un address;
//    int addrlen = sizeof(address);
//
//    if (server_fd < 0) {
//        LOGE("socket server error");
//        return -1;
//    }
//
//    memset(&address, 0, sizeof(address));
//
//    address.sun_family = AF_UNIX;
//    strncpy(address.sun_path, RTSP_PREVIEW_SOCKET_PATH, sizeof(address.sun_path) - 1);
//
//    unlink(RTSP_PREVIEW_SOCKET_PATH);  // 确保之前的 socket 被删除
//    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
//        LOGE("socket bind error");
//        return -1;
//    }
//
//    LOGI("socket bind success");
//
//    // 监听连接
//    if (listen(server_fd, 3) < 0) {
//        LOGE("socket listen failure");
//        close(server_fd);
//        return 0;
//    }
//    LOGI("Waiting for client connection...");

    // 创建 ZMQ 上下文
    void* context = zmq_ctx_new();

    // 创建 ZMQ 套接字 (ZMQ_REP 用于响应请求)
    void* responder = zmq_socket(context, ZMQ_PUSH);

    // 绑定到 Unix Domain Socket 地址
    int rc = zmq_bind(responder, "ipc:///sdcard/zmq.sock");

//    int timeout = 1; // 超时设置为 16 毫秒
//    zmq_setsockopt(responder, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));

    if (rc != 0) {
        LOGE("Error binding to socket:");
        return -1;
    }

    // Begin by setting up our usage environment:
    TaskScheduler *scheduler = BasicTaskScheduler::createNew();
    UsageEnvironment *env = BasicUsageEnvironment::createNew(*scheduler);
//
    // 接受客户端连接
//    if ((client_id = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
//        perror("Accept failed");
//        close(server_fd);
//        exit(EXIT_FAILURE);
//    }

//    LOGI("success accept a socket.... %d", client_id);

    openURL(*env, "xxxx", "rtsp://192.168.2.104:8554/cam", responder);

    env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
    return 0;
}


void openURL(UsageEnvironment &env, char const *progName, char const *rtspURL, void* responder) {
    LOGI("openURL invoke");
    // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
    // to receive (even if more than stream uses the same "rtsp://" URL).
    RTSPClient *rtspClient = KtRtspClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL,
                                                     progName);
    KtStreamClientState &scs = ((KtRtspClient *) rtspClient)->scs; // alias

    scs.responder = responder;

    if (rtspClient == NULL) {
        LOGE("Failed to create a RTSP client for URL %s", env.getResultMsg());
        return;
    }
    LOGI("success create a RTSP client");
    ++rtspClientCount;

    // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
    // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
    // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
    rtspClient->sendDescribeCommand(continueAfterDESCRIBE);
}


// Implementation of the RTSP 'response handlers':

void continueAfterDESCRIBE(RTSPClient *rtspClient, int resultCode, char *resultString) {
    do {

        LOGI("continueAfterDESCRIBE invoke");

        UsageEnvironment &env = rtspClient->envir(); // alias
        KtStreamClientState &scs = ((KtRtspClient *) rtspClient)->scs; // alias

        if (resultCode != 0) {
            LOGE("Failed to get a SDP description");
            delete[] resultString;
            break;
        }

        char *const sdpDescription = resultString;
        env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";

        // Create a media session object from this SDP description:
        scs.session = MediaSession::createNew(env, sdpDescription);
        delete[] sdpDescription; // because we don't need it anymore
        if (scs.session == NULL) {
            env << *rtspClient
                << "Failed to create a MediaSession object from the SDP description: "
                << env.getResultMsg() << "\n";
            break;
        } else if (!scs.session->hasSubsessions()) {
            env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
            break;
        }

        // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
        // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
        // (Each 'subsession' will have its own data source.)
        scs.iter = new MediaSubsessionIterator(*scs.session);
        setupNextSubsession(rtspClient);
        return;
    } while (0);

    // An unrecoverable error occurred with this stream.
    shutdownStream(rtspClient);
}


void setupNextSubsession(RTSPClient *rtspClient) {
    UsageEnvironment &env = rtspClient->envir(); // alias
    KtStreamClientState &scs = ((KtRtspClient *) rtspClient)->scs; // alias

    scs.subsession = scs.iter->next();
    if (scs.subsession != NULL) {
        if (!scs.subsession->initiate()) {
            env << *rtspClient << "Failed to initiate the \"" << *scs.subsession
                << "\" subsession: " << env.getResultMsg() << "\n";
            setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
        } else {
            env << *rtspClient << "Initiated the \"" << *scs.subsession << "\" subsession (";
            if (scs.subsession->rtcpIsMuxed()) {
                env << "client port " << scs.subsession->clientPortNum();
            } else {
                env << "client ports " << scs.subsession->clientPortNum() << "-"
                    << scs.subsession->clientPortNum() + 1;
            }
            env << ")\n";

            // Continue setting up this subsession, by sending a RTSP "SETUP" command:
            rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False,
                                         REQUEST_STREAMING_OVER_TCP);
        }
        return;
    }

    // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
    if (scs.session->absStartTime() != NULL) {
        // Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
        rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(),
                                    scs.session->absEndTime());
    } else {
        scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
        rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
    }
}


void printHex(uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        printf("%02x ", data[i]);
    }
    printf("\n");
}

const std::string filename = "/sdcard/header.264";
zmq_msg_t message;


void continueAfterSETUP(RTSPClient *rtspClient, int resultCode, char *resultString) {
    do {
        UsageEnvironment &env = rtspClient->envir(); // alias
        KtStreamClientState &scs = ((KtRtspClient *) rtspClient)->scs; // alias

        if (resultCode != 0) {
            env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: "
                << resultString << "\n";
            break;
        }

        env << *rtspClient << "Set up the \"" << *scs.subsession << "\" subsession (";
        if (scs.subsession->rtcpIsMuxed()) {
            env << "client port " << scs.subsession->clientPortNum();
        } else {
            env << "client ports " << scs.subsession->clientPortNum() << "-"
                << scs.subsession->clientPortNum() + 1;
        }
        env << ")\n";

        // 查找 fmtp 行
        std::string sdpLines(scs.subsession->savedSDPLines());
        std::string fmtpLine;
        std::istringstream sdpStream(sdpLines);
        std::string line;

        while (std::getline(sdpStream, line)) {
            if (line.find("a=fmtp:") != std::string::npos) {
                fmtpLine = line;
                break; // 找到 fmtp 行后退出循环
            }
        }

        if (!fmtpLine.empty()) {
            std::string xx = extractSpropParameterSets(fmtpLine);
            std::pair<std::string, std::string> spsPPs = subByDelimiter(xx, ',');

            LOGI("fmtp Line info is111 %s", xx.c_str());
            LOGI("fmtp Line info is222 %s", spsPPs.first.c_str());
            LOGI("fmtp Line info is3333 %s", spsPPs.second.c_str());

            //结论是相同的

//            std::string original = "Z/QAKpGWgHgCJ+JwEQAAAwABAAADAHiPGDKg,aM4PGSA=";

            std::vector<uint8_t> sps_base64Char = stpStringBase64Decode(spsPPs.first);
            std::vector<uint8_t> pps_base64Char = stpStringBase64Decode(spsPPs.second);
//
//            // 将两个 char 数组拼接
            std::vector<unsigned char> combined;

            std::vector<unsigned char> spsPrefix = {
                    0x00, 0x00, 0x00, 0x01
            };

            std::vector<unsigned char> ppsPrefix = {
                    0x00, 0x00, 0x00, 0x01
            };

            combined.insert(combined.end(), sps_base64Char.begin(), sps_base64Char.end());
            combined.insert(combined.end(), ppsPrefix.begin(), ppsPrefix.end());
            combined.insert(combined.end(), pps_base64Char.begin(), pps_base64Char.end());

            zmq_msg_init_data(&message, combined.data(), combined.size(), nullptr, nullptr);
            // 发送消息
            zmq_msg_send(&message, scs.responder, 0);
            // 清理
            zmq_msg_close(&message);

            int  timeout = 0;
            zmq_setsockopt(scs.responder, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));

//            send(scs.socketId, combined.data(), combined.size(), 0);
//
            //std::vector<uint8_t> originalChar = stpStringBase64Decode(spsPPs);

            LOGI("originalChar size %d", combined.size());

//            // 使用范围 for 循环打印每个元素
//            for (unsigned char byte : combined) {
//                // 打印为十六进制格式
//                std::cout << "0x" << std::hex << static_cast<int>(byte) << " ";
//            }
//
//            // 打开文件进行二进制写入
//            std::ofstream outputFile(filename, std::ios::binary);
//
//            // 检查文件是否成功打开
//            if (!outputFile) {
//               LOGI("Error: Could not open the file for writing.");
//            }
//
//            // 写入 vector 的数据到文件
//            outputFile.write(reinterpret_cast<const char*>(combined.data()), combined.size());
//
//            // 检查写入是否成功
//            if (!outputFile) {
//                LOGI("Error: Could not write data to the file.");
//            }
//
//            // 关闭文件
//            outputFile.close();

//            ssize_t bufIdx = AMediaCodec_dequeueInputBuffer(codec, 0);
//
//            LOGI("bufIdx %d\n", bufIdx);
//
//            if (bufIdx >= 0) {
//                size_t bufSize;
//                uint8_t* inputBuf = AMediaCodec_getInputBuffer(codec, bufIdx, &bufSize);
//                memcpy(inputBuf, combined.data(), combined.size());
//                bufIdx = AMediaCodec_queueInputBuffer(codec, bufIdx, 0, combined.size(), 0, 2);
//                if (bufIdx == -2) {
//                    LOGI("AMediaCodec bufIdx format changed\n");
//                    break;
//                } else if (bufIdx == -1) {
//                    LOGI("AMediaCodec input try later");
//                } else {
//                    LOGI("AMediaCodec_queueInputBuffer %d\n", bufIdx);
//                }
//            }

//            // 比较两个结果是否相等
//            if (combined.size() == originalChar.size() && memcmp(combined.data(), originalChar.data(), combined.size()) == 0) {
//                LOGI("xxxxxxxxxxxx1");
//            } else {
//                LOGI("xxxxxxxxxxxx2");
//            }

//            LOGI("fmtp Line info is %s", xx.c_str());
//            LOGI("fmtp Line info is %s", spsPPs.c_str());
//            LOGI("fmtp Line info is %s", fmtpLine.c_str());

//                std::for_each(originalChar.begin(), originalChar.end(), [](char x) {
//                    LOGI("xxxxxxxxxxxx1  %02x", x);
//                });
        }


        scs.subsession->sink = DummySink::createNew(env, *scs.subsession, rtspClient->url(), scs.responder, codec);
        // perhaps use your own custom "MediaSink" subclass instead
        if (scs.subsession->sink == NULL) {
            env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
                << "\" subsession: " << env.getResultMsg() << "\n";
            break;
        }

        env << *rtspClient << "Created a data sink for the \"" << *scs.subsession
            << "\" subsession\n";
        scs.subsession->miscPtr = rtspClient; // a hack to let subsession handler functions get the "RTSPClient" from the subsession
        scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
                                           subsessionAfterPlaying, scs.subsession);
        // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
        if (scs.subsession->rtcpInstance() != NULL) {
            scs.subsession->rtcpInstance()->setByeWithReasonHandler(subsessionByeHandler,
                                                                    scs.subsession);
        }
    } while (0);
    delete[] resultString;

    // Set up the next subsession, if any:
    setupNextSubsession(rtspClient);
}

void continueAfterPLAY(RTSPClient *rtspClient, int resultCode, char *resultString) {
    Boolean success = False;

    do {
        UsageEnvironment &env = rtspClient->envir(); // alias
        KtStreamClientState &scs = ((KtRtspClient *) rtspClient)->scs; // alias

        if (resultCode != 0) {
            env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
            break;
        }

        // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
        // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
        // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
        // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
        if (scs.duration > 0) {
            unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
            scs.duration += delaySlop;
            unsigned uSecsToDelay = (unsigned) (scs.duration * 1000000);
            scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay,
                                                                          (TaskFunc *) streamTimerHandler,
                                                                          rtspClient);
        }

        env << *rtspClient << "Started playing session";
        if (scs.duration > 0) {
            env << " (for up to " << scs.duration << " seconds)";
        }
        env << "...\n";

        success = True;
    } while (0);
    delete[] resultString;

    if (!success) {
        // An unrecoverable error occurred with this stream.
        shutdownStream(rtspClient);
    }
}


// Implementation of the other event handlers:

void subsessionAfterPlaying(void *clientData) {
    MediaSubsession *subsession = (MediaSubsession *) clientData;
    RTSPClient *rtspClient = (RTSPClient *) (subsession->miscPtr);

    // Begin by closing this subsession's stream:
    Medium::close(subsession->sink);
    subsession->sink = NULL;

    // Next, check whether *all* subsessions' streams have now been closed:
    MediaSession &session = subsession->parentSession();
    MediaSubsessionIterator iter(session);
    while ((subsession = iter.next()) != NULL) {
        if (subsession->sink != NULL) return; // this subsession is still active
    }

    // All subsessions' streams have now been closed, so shutdown the client:
    shutdownStream(rtspClient);
}

void subsessionByeHandler(void *clientData, char const *reason) {
    MediaSubsession *subsession = (MediaSubsession *) clientData;
    RTSPClient *rtspClient = (RTSPClient *) subsession->miscPtr;
    UsageEnvironment &env = rtspClient->envir(); // alias

    env << *rtspClient << "Received RTCP \"BYE\"";
    if (reason != NULL) {
        env << " (reason:\"" << reason << "\")";
        delete[] (char *) reason;
    }
    env << " on \"" << *subsession << "\" subsession\n";

    // Now act as if the subsession had closed:
    subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void *clientData) {
    KtRtspClient *rtspClient = (KtRtspClient *) clientData;
    KtStreamClientState &scs = rtspClient->scs; // alias

    scs.streamTimerTask = NULL;

    // Shut down the stream:
    shutdownStream(rtspClient);
}

void shutdownStream(RTSPClient *rtspClient, int exitCode) {
    UsageEnvironment &env = rtspClient->envir(); // alias
    KtStreamClientState &scs = ((KtRtspClient *) rtspClient)->scs; // alias

    // First, check whether any subsessions have still to be closed:
    if (scs.session != NULL) {
        Boolean someSubsessionsWereActive = False;
        MediaSubsessionIterator iter(*scs.session);
        MediaSubsession *subsession;

        while ((subsession = iter.next()) != NULL) {
            if (subsession->sink != NULL) {
                Medium::close(subsession->sink);
                subsession->sink = NULL;

                if (subsession->rtcpInstance() != NULL) {
                    subsession->rtcpInstance()->setByeHandler(NULL,
                                                              NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
                }

                someSubsessionsWereActive = True;
            }
        }

        if (someSubsessionsWereActive) {
            // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
            // Don't bother handling the response to the "TEARDOWN".
            rtspClient->sendTeardownCommand(*scs.session, NULL);
        }
    }

    env << *rtspClient << "Closing the stream.\n";
    Medium::close(rtspClient);
    // Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.

    if (--rtspClientCount == 0) {
        // The final stream has ended, so exit the application now.
        // (Of course, if you're embedding this code into your own application, you might want to comment this out,
        // and replace it with "eventLoopWatchVariable = 1;", so that we leave the LIVE555 event loop, and continue running "main()".)
        exit(exitCode);
    }
}