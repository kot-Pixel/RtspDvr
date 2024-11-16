//
// Created by Admin on 2024/11/11.
//

#include "../include/KtAudioRecorder.h"


aaudio_data_callback_result_t dataCallback(AAudioStream *stream, void *userData, void *audioData,
                  int32_t numFrames) {
    LOGI("get local audio record");

    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

KtAudioRecorder::KtAudioRecorder()  = default;

KtAudioRecorder::~KtAudioRecorder() = default;

void KtAudioRecorder::initKtAudioRecorder() {
    aaudio_result_t result = AAudio_createStreamBuilder(&builder);
    if (result != AAUDIO_OK) {
        LOGE("AAudio_createStreamBuilder create failure");
    }
    AAudioStreamBuilder_setDeviceId(builder, 21);
    AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_INPUT);  // 设置为输出模式
    AAudioStreamBuilder_setSharingMode(builder, AAUDIO_UNSPECIFIED);
    AAudioStreamBuilder_setSampleRate(builder, AAUDIO_UNSPECIFIED);
    AAudioStreamBuilder_setChannelCount(builder, AAUDIO_UNSPECIFIED);
    AAudioStreamBuilder_setFormat(builder, AAUDIO_UNSPECIFIED);
    AAudioStreamBuilder_setBufferCapacityInFrames(builder, AAUDIO_UNSPECIFIED);
    AAudioStreamBuilder_setDataCallback(builder,
                                        dataCallback, nullptr); // 设置回调函数

    result = AAudioStreamBuilder_openStream(builder, &stream);

    if (result != AAUDIO_OK) {
        LOGE("无法打开 AAudioStream: ");
        AAudioStreamBuilder_delete(builder);
    }
    LOGI("打开 AAudioStream: ");
    aaudio_stream_state_t state = AAudioStream_getState(stream);
    LOGI("Stream state: %d", state);

    result = AAudioStream_requestStart(stream);
    if (result != AAUDIO_OK) {
        LOGE("无法启动音频流: ");
        AAudioStream_close(stream);
        AAudioStreamBuilder_delete(builder);
    }
    LOGI("启动 AAudioStream: ");
}
