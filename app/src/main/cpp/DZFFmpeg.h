//
// Created by hcDarren on 2019/6/16.
//

#ifndef MUSICPLAYER_DZFFMPEG_H
#define MUSICPLAYER_DZFFMPEG_H


#include "DZJNICall.h"

extern "C"{
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
};


class DZFFmpeg {
public:
    AVFormatContext *pFormatContext = NULL;
    AVCodecContext *pCodecContext = NULL;
    SwrContext *swrContext = NULL;
    uint8_t *resampleOutBuffer = NULL;
    const char* url = NULL;
    DZJNICall *pJniCall = NULL;
public:
    DZFFmpeg(DZJNICall *pJniCall, const char* url);
    ~DZFFmpeg();

public:
    void play();

    void callPlayerJniError(int code, char* msg);

    void release();

};


#endif //MUSICPLAYER_DZFFMPEG_H
