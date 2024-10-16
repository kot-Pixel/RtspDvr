#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <android/log.h>
#include <thread>
#include "include/log_utils.h"

#define SOCKET_PATH "/data/local/tmp/source.sockect"

AMediaCodec* codec = nullptr;

void decodeH264ThreadFunction() {
    if (codec == nullptr) {
        return;
    }
    while (true) {
        AMediaCodecBufferInfo info;
        ssize_t outIdx = AMediaCodec_dequeueOutputBuffer(codec, &info, 2000);  // 等待2秒
        if (outIdx >= 0) {
            uint8_t* outputBuf = AMediaCodec_getOutputBuffer(codec, outIdx, nullptr);
            if (outputBuf) {
                LOGI("Decoded frame, size: %d", info.size);
            }
            // 释放输出缓冲区
            AMediaCodec_releaseOutputBuffer(codec, outIdx, true);
        } else if (outIdx == -1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 休眠1秒
        }
    }
}

int configureAndStartMediaCodec() {
    LOGI("start configure and start media codec");
    codec = AMediaCodec_createDecoderByType("video/avc");
    if (!codec) {
        printf("Failed to create codec");
        return -1;
    }
    AMediaFormat* format = AMediaFormat_new();
    AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/avc");
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, 1920);
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, 1060);
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

int main() {

    int result = configureAndStartMediaCodec();

    LOGI("success configure and start mediacodec");

    std::thread decodeH264Thread(decodeH264ThreadFunction);
    decodeH264Thread.detach();

    int client_fd;
    struct sockaddr_un server_addr;

    // 创建客户端套接字
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd < 0) {
        std::cerr << "Socket creation error: " << strerror(errno) << std::endl;
        return -1;
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // 尝试连接服务器
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed: " << strerror(errno) << std::endl;
        close(client_fd);
        return -1;
    }

    // 接收服务器的响应
    char buffer[1024] = {0};

    while (true) {
        // 清空缓冲区
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            ssize_t bufIdx = AMediaCodec_dequeueInputBuffer(codec, 0);
            if (bufIdx >= 0) {
                size_t bufSize;
                uint8_t* inputBuf = AMediaCodec_getInputBuffer(codec, bufIdx, &bufSize);
                if (bytes_received <= bufSize) {
                    memcpy(inputBuf, buffer, bytes_received);
                    AMediaCodec_queueInputBuffer(codec, bufIdx, 0, bytes_received, 0, 0);
                } else {
                    LOGI("Received data is too large for the input buffer.");
                }
            } else if (bufIdx == -2) {
                LOGI("AMediaCodec bufIdx format changed");
            } else if (bufIdx == -1) {
                LOGI("AMediaCodec input try later");
            }
        }
    }
    return 0;
}