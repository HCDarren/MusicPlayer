//
// Created by hcDarren on 2019/6/22.
//

#ifndef MUSICPLAYER_DZAUDIO_H
#define MUSICPLAYER_DZAUDIO_H
#include <pthread.h>
#include "DZJNICall.h"
#include "DZConstDefine.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
};


class DZAudio {
public:
    AVFormatContext *pFormatContext = NULL;
    AVCodecContext *pCodecContext = NULL;
    SwrContext *swrContext = NULL;
    uint8_t *resampleOutBuffer = NULL;
    DZJNICall *pJniCall = NULL;
    int audioStreamIndex = -1;
public:
    DZAudio(int audioStreamIndex, DZJNICall *pJniCall, AVCodecContext *pCodecContext,
            AVFormatContext *pFormatContext);

    void play();

    void initCrateOpenSLES();

    int resampleAudio();
};


#endif //MUSICPLAYER_DZAUDIO_H
