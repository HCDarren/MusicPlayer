//
// Created by hcDarren on 2019/6/16.
//

#ifndef MUSICPLAYER_DZFFMPEG_H
#define MUSICPLAYER_DZFFMPEG_H
#include "DZJNICall.h"
#include "DZAudio.h"
#include <pthread.h>

extern "C"{
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
};

class DZFFmpeg {
public:
    AVFormatContext *pFormatContext = NULL;
    AVCodecContext *pCodecContext = NULL;
    SwrContext *swrContext = NULL;
    char* url = NULL;
    DZJNICall *pJniCall = NULL;
    DZAudio *pAudio = NULL;

public:
    DZFFmpeg(DZJNICall *pJniCall, const char* url);
    ~DZFFmpeg();

public:
    void play();

    void prepare();

    void prepareAsync();

    void prepare(ThreadMode threadMode);

    void callPlayerJniError(ThreadMode threadMode, int code, char* msg);

    void release();
};


#endif //MUSICPLAYER_DZFFMPEG_H
