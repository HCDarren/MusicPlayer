//
// Created by hcDarren on 2019/6/22.
//

#ifndef MUSICPLAYER_DZAUDIO_H
#define MUSICPLAYER_DZAUDIO_H

#include <pthread.h>
#include "DZJNICall.h"
#include "DZConstDefine.h"
#include "DZPacketQueue.h"
#include "DZPlayerStatus.h"
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
    SwrContext *pSwrContext = NULL;
    uint8_t *resampleOutBuffer = NULL;
    DZJNICall *pJniCall = NULL;
    int audioStreamIndex = -1;
    DZPacketQueue *pPacketQueue = NULL;
    DZPlayerStatus *pPlayerStatus = NULL;
public:
    DZAudio(int audioStreamIndex, DZJNICall *pJniCall, AVFormatContext *pFromatContext);

    ~DZAudio();

    void play();

    void initCrateOpenSLES();

    int resampleAudio();

    void analysisStream(ThreadMode threadMode, AVStream **streams);

    void callPlayerJniError(ThreadMode threadMode, int code, char *msg);

    void release();
};


#endif //MUSICPLAYER_DZAUDIO_H
