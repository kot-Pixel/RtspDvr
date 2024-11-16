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
    interface = new StreamInterface();
    interface->reqLooperInner();

    mRtspUrl = rtspURL;
    mZmqContext = zmq_ctx_new();
    if (mZmqContext == NULL) return;
    LOGI("success create zmq context");
    mZmqSender = zmq_socket(mZmqContext, ZMQ_PUSH);
    if (mZmqSender == NULL) return;
    if (zmq_bind(mZmqSender, "ipc:///sdcard/zmq.sock") != 0) return;
    mZmqSocketSender = true;
}

void KtRtspClient::sendClientSpsPps() {
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

                mCodecParameter = format_ctx->streams[i]->codecpar;

                // 获取 SPS 和 PPS
                extradata = format_ctx->streams[i]->codecpar->extradata;
                extradata_size= format_ctx->streams[i]->codecpar->extradata_size;

                time_base = format_ctx->streams[i]->time_base;
                LOGI("Time base: %d/%d\n", time_base.num, time_base.den);
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


    int ret;

    // 打开输出 MP4 文件
    ret = avformat_alloc_output_context2(&fmtCtx, NULL, "mp4", "/sdcard/output.mp4");
    if (ret < 0) {
        LOGI("Error allocating output context: %s\n", av_err2str(ret));
        return;
    }

    LOGI("Success allocating output context\n");

    // 创建视频流
    videoStream = avformat_new_stream(fmtCtx, nullptr);
    if (!videoStream) {
        LOGE("Error creating new stream\n");
        return;
    }

    LOGE("Success creating new stream\n");

    // 设置视频流的基本参数，这里不使用编码器

    videoStream->codecpar->codec_id = mCodecParameter->codec_id;
    videoStream->codecpar->codec_type = mCodecParameter->codec_type;
    videoStream->codecpar->width = mCodecParameter->width;
    videoStream->codecpar->height = mCodecParameter->height;
    videoStream->codecpar->format = mCodecParameter->format;
    videoStream->codecpar->extradata = extradata;
    videoStream->codecpar->extradata_size = extradata_size;

    // 打开输出文件
    if (!(fmtCtx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&fmtCtx->pb, outputMp4, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOGE("Error opening output file: %s\n", av_err2str(ret));
            return;
        }
    }

    LOGI("Success opening output file\n");

    // 写入文件头
    ret = avformat_write_header(fmtCtx, nullptr);
    if (ret < 0) {
        LOGE("Error writing file header: %s\n", av_err2str(ret));
        return;
    }

    LOGI("Success  writing file header");


    sendClientSpsPps();

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
            if (writeFrameCount < 500) {
                if(judgeFrameIsKeyFrame(packet.data[4])) {
                    if (av_write_frame(fmtCtx, &packet) >= 0) {
                        hasWriteKeyFrame = true;
                        writeFrameCount += 1;
                    }
                } else {
                    if (hasWriteKeyFrame) {
                        if (av_write_frame(fmtCtx, &packet) >= 0) {
                            writeFrameCount += 1;
                        }
                    }
                }
            } else {
                // 写入文件尾
                ret = av_write_trailer(fmtCtx);
                if (ret < 0) {
                    LOGI("Error writing trailer\n");
                }
                LOGI("Success writing trailer\n");
                break;
            }
//            popCachedFrame(packet);
        }
        av_packet_unref(&packet);
    }

    // 释放资源
    avformat_close_input(&format_ctx);
}


void KtRtspClient::popCachedFrame(AVPacket packet) {
    if (packet.data == nullptr || packet.size <= 0 || packet.pts < 0) return;
    int64_t drop = packet.pts - (10 * time_base.den);
    if (drop < 0) return;
    std::shared_ptr<KtRtpFrame> *firstElementPtr = mReaderWriteQueue.peek();
    while (firstElementPtr && *firstElementPtr) {
        std::shared_ptr<KtRtpFrame> firstElement = *firstElementPtr;
        LOGI("first Element pts is %ld and drop is %ld", firstElement->mRtpFramePts, drop);
        if (firstElement->mRtpFramePts < drop) {
            std::shared_ptr<KtRtpFrame> temp;
            mReaderWriteQueue.try_dequeue(temp);
            temp.reset();
            firstElementPtr = mReaderWriteQueue.peek();
        } if (judgeFrameIsKeyFrame(firstElement->mRtpFramePointer[4])) {
            LOGI("cached h264 size over 10s");
            break;
        } else {
            std::shared_ptr<KtRtpFrame> temp;
            mReaderWriteQueue.try_dequeue(temp);
            temp.reset();
            firstElementPtr = mReaderWriteQueue.peek();
        }
    }
    bool sendResult = mReaderWriteQueue.enqueue(
            std::make_shared<KtRtpFrame>(packet.pts, packet.size, packet.data));
    LOGI("send to enqueue result is %d, send packet pts is %ld, send packet size is %d", sendResult,
         packet.pts, packet.size);
    LOGI("the queue first element is queue size approx is %zu", mReaderWriteQueue.size_approx());
}



KtRtspClient::~KtRtspClient() {
}

bool KtRtspClient::judgeFrameIsKeyFrame(uint8_t naulValue) {
    return (naulValue & 0x1F) == 5;
}

void KtRtspClient::startWriteToMp4File() {

}

void KtRtspClient::initWriteFormatContext() {
//
//
//    AVPacket pkt;
//    av_packet_unref(&pkt);
//
//    std::shared_ptr<KtRtpFrame> temp;
//    while(mReaderWriteQueue.try_dequeue(temp)) {
//        pkt.data = temp->mRtpFramePointer;  // 裸数据
//        pkt.size = temp->mRtpFrameSize;             // 数据大小
//        // 设置 PTS 和 DTS，假设帧率为 30 fps
//        pkt.pts = pkt.dts = temp->mRtpFramePts;
//        pkt.duration = 1; // 每帧持续时间，根据帧率计算
//        ret = av_write_frame(fmtCtx, &pkt);
//        if (ret < 0) {
//            LOGI("Error writing frame: %s\n", av_err2str(ret));
//            break;
//        } else {
//            av_packet_unref(&pkt);
//            temp.reset();
//        }
//    }
//    av_packet_unref(&pkt);
//
//    // 写入文件尾并清理资源
//    av_write_trailer(fmtCtx);
//    avformat_free_context(fmtCtx);
}

void KtRtspClient::startRecordLocalAudio() {
    audioRecorder->initKtAudioRecorder();
}

