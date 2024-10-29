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
    if (extradata == NULL || extradata_size < 0) return;
    zmq_msg_init_data(&message, (void*)extradata, extradata_size, nullptr, nullptr);
    zmq_msg_send(&message, mZmqSender, 0);
    zmq_msg_close(&message);
}

void custom_free(void *data, void *hint) {
    free(data);  // 释放数据
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
            // 确保是 H.264 视频流
            if (format_ctx->streams[i]->codecpar->codec_id == AV_CODEC_ID_H264) {

                // 获取 SPS 和 PPS
                extradata = format_ctx->streams[i]->codecpar->extradata;
                extradata_size= format_ctx->streams[i]->codecpar->extradata_size;
            }
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
            auto* packet_data_copy = (uint8_t*)malloc(packet.size);
            memcpy(packet_data_copy, packet.data, packet.size);
            LOGI("NAL data %02X", packet_data_copy[4]);
            zmq_msg_init_data(&message, packet_data_copy, packet.size, custom_free, nullptr);
            zmq_msg_send(&message, mZmqSender, ZMQ_DONTWAIT);
            zmq_msg_close(&message);
        }
        av_packet_unref(&packet);
    }

    // 释放资源
    avformat_close_input(&format_ctx);
}

KtRtspClient::~KtRtspClient() {
}
