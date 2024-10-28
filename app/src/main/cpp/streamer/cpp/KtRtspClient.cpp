//
// Created by Tom on 2024/10/11.
//

#include "../include/KtRtspClient.h"
#include "../include/log_utils.h"
#include <jni.h>

KtRtspClient* KtRtspClient::createNew(char const* rtspURL) {
    return new KtRtspClient(rtspURL);
}

KtRtspClient::KtRtspClient(char const* rtspURL) {
    mRtspUrl = rtspURL;
    mZmqContext = zmq_ctx_new();
    if (mZmqContext == NULL) return;
    LOGI("success create zmq context");
    mZmqSender = zmq_socket(mZmqContext, ZMQ_PUSH);
    if (mZmqSender == NULL) return;
    if (zmq_bind(mZmqSender, "ipc:///sdcard/zmq.sock") != 0) return;
    mZmqSocketSender = true;
}

void KtRtspClient::print_sdp_info() {
    char sdp_buffer[2048];
    if (av_sdp_create(&format_ctx, 1, sdp_buffer, sizeof(sdp_buffer)) == 0) {
        sdp_info = sdp_buffer;
        LOGI("success get sdp information: %s", sdp_info.c_str());
        if (mZmqSocketSender) {
            zmq_msg_init_data(&message, (void*)sdp_info.data(), sdp_info.size(), nullptr, nullptr);
            zmq_msg_send(&message, mZmqSender, 0);
            zmq_msg_close(&message);
        }
    } else {
        LOGE("failed to retrieve sdp information.");
        sdp_info.clear();
    }
}

void KtRtspClient::establishRtsp() {
    if (mRtspUrl == nullptr) {
        LOGI("mRtspUrl is null pointer");
        return;
    }

    LOGI("ktRtspClient start establishRtsp....");

    // 打开 RTSP 流
    if (avformat_open_input(&format_ctx, mRtspUrl, nullptr, nullptr) < 0) {
        LOGE("ktRtspClient can't open input");
        return;  // 无法打开流
    }

    // 查找流信息
    if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
        avformat_close_input(&format_ctx);
        return;
    }

    for (unsigned int i = 0; i < format_ctx->nb_streams; i++) {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index == -1) {
        avformat_close_input(&format_ctx);
        LOGE("ktRtspClient can't find video stream");
        return;
    }

    LOGI("ktRtspClient find video stream and video stream index is %d", video_stream_index);

    print_sdp_info();

    AVPacket packet;

    // 循环读取帧数据
    while (av_read_frame(format_ctx, &packet) >= 0) {
        if (packet.stream_index == video_stream_index) {
            LOGI("read frame and pack size is %d", packet.size);
            LOGI("NAL data %02X", packet.data[4]);
        }
        av_packet_unref(&packet);  // 释放当前包
    }

    // 释放资源
    avformat_close_input(&format_ctx);
}

KtRtspClient::~KtRtspClient() {
}
