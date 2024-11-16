//
// Created by Admin on 2024/11/11.
//

#ifndef SOCKECTDEMO2_KTAUDIORECORDER_H
#define SOCKECTDEMO2_KTAUDIORECORDER_H

#include <aaudio/AAudio.h>
#include "../include/log_utils.h"

class KtAudioRecorder {
public:
    KtAudioRecorder();
    virtual ~KtAudioRecorder();
    void initKtAudioRecorder();
private:
    AAudioStream *stream = nullptr;
    AAudioStreamBuilder *builder = nullptr;
};


#endif //SOCKECTDEMO2_KTAUDIORECORDER_H
