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
}

void KtRtspClient::print_sdp_info() {
    // 获取流的 SDP 信息
    if (format_ctx->pb && format_ctx->pb->opaque) {
        LOGI("SDP Information:\n%s", format_ctx->pb->opaque);
    } else {
        LOGE("Failed to retrieve SDP information.");
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
