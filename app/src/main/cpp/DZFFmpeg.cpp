//
// Created by hcDarren on 2019/6/16.
//

#include "DZFFmpeg.h"
#include "DZConstDefine.h"

DZFFmpeg::DZFFmpeg(DZJNICall *pJniCall, const char *url) {
    this->pJniCall = pJniCall;
    // 赋值一份 url ，因为怕外面方法结束销毁了 url
    this->url = (char *) malloc(strlen(url) + 1);
    memcpy(this->url, url, strlen(url) + 1);
}

DZFFmpeg::~DZFFmpeg() {
    release();
}

void DZFFmpeg::play() {
    if (pAudio != NULL) {
        pAudio->play();
    }
}

void DZFFmpeg::callPlayerJniError(ThreadMode threadMode, int code, char *msg) {
    // 释放资源
    release();
    // 回调给 java 层调用
    pJniCall->callPlayerError(threadMode, code, msg);
}

void DZFFmpeg::release() {
    if (pCodecContext != NULL) {
        avcodec_close(pCodecContext);
        avcodec_free_context(&pCodecContext);
        pCodecContext = NULL;
    }

    if (pFormatContext != NULL) {
        avformat_close_input(&pFormatContext);
        avformat_free_context(pFormatContext);
        pFormatContext = NULL;
    }

    if (swrContext != NULL) {
        swr_free(&swrContext);
        free(swrContext);
        swrContext = NULL;
    }

    avformat_network_deinit();

    if (url != NULL) {
        free(url);
        url = NULL;
    }
}

void DZFFmpeg::prepare() {
    prepare(THREAD_MAIN);
}

void *threadPrepare(void *context) {
    DZFFmpeg *pFFmpeg = (DZFFmpeg *) context;
    pFFmpeg->prepare(THREAD_CHILD);
    return 0;
}

void DZFFmpeg::prepareAsync() {
    // 创建一个线程去播放，多线程编解码边播放
    pthread_t prepareThreadT;
    pthread_create(&prepareThreadT, NULL, threadPrepare, this);
    pthread_detach(prepareThreadT);
}

void DZFFmpeg::prepare(ThreadMode threadMode) {
    // 讲的理念的东西，千万要注意
    av_register_all();
    avformat_network_init();
    int formatOpenInputRes = 0;
    int formatFindStreamInfoRes = 0;
    AVCodecParameters *pCodecParameters;
    AVCodec *pCodec = NULL;
    int codecParametersToContextRes = -1;
    int codecOpenRes = -1;

    formatOpenInputRes = avformat_open_input(&pFormatContext, url, NULL, NULL);
    if (formatOpenInputRes != 0) {
        // 第一件事，需要回调给 Java 层(下次课讲)
        // 第二件事，需要释放资源
        LOGE("format open input error: %s", av_err2str(formatOpenInputRes));
        callPlayerJniError(threadMode, formatOpenInputRes, av_err2str(formatOpenInputRes));
        return;
    }

    formatFindStreamInfoRes = avformat_find_stream_info(pFormatContext, NULL);
    if (formatFindStreamInfoRes < 0) {
        LOGE("format find stream info error: %s", av_err2str(formatFindStreamInfoRes));
        // 这种方式一般不推荐这么写，但是的确方便
        callPlayerJniError(threadMode, formatFindStreamInfoRes,
                av_err2str(formatFindStreamInfoRes));
        return;
    }

    // 查找音频流的 index
    int audioStramIndex = av_find_best_stream(pFormatContext, AVMediaType::AVMEDIA_TYPE_AUDIO, -1,
            -1,
            NULL, 0);
    if (audioStramIndex < 0) {
        LOGE("format audio stream error.");
        // 这种方式一般不推荐这么写，但是的确方便
        callPlayerJniError(threadMode, FIND_STREAM_ERROR_CODE, "format audio stream error");
        return;
    }

    // 查找解码
    pCodecParameters = pFormatContext->streams[audioStramIndex]->codecpar;
    pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
    if (pCodec == NULL) {
        LOGE("codec find audio decoder error");
        // 这种方式一般不推荐这么写，但是的确方便
        callPlayerJniError(threadMode, CODEC_FIND_DECODER_ERROR_CODE,
                "codec find audio decoder error");
        return;
    }
    // 打开解码器
    pCodecContext = avcodec_alloc_context3(pCodec);
    if (pCodecContext == NULL) {
        LOGE("codec alloc context error");
        // 这种方式一般不推荐这么写，但是的确方便
        callPlayerJniError(threadMode, CODEC_ALLOC_CONTEXT_ERROR_CODE, "codec alloc context error");
        return;
    }
    codecParametersToContextRes = avcodec_parameters_to_context(pCodecContext, pCodecParameters);
    if (codecParametersToContextRes < 0) {
        LOGE("codec parameters to context error: %s", av_err2str(codecParametersToContextRes));
        callPlayerJniError(threadMode, codecParametersToContextRes,
                av_err2str(codecParametersToContextRes));
        return;
    }

    codecOpenRes = avcodec_open2(pCodecContext, pCodec, NULL);
    if (codecOpenRes != 0) {
        LOGE("codec audio open error: %s", av_err2str(codecOpenRes));
        callPlayerJniError(threadMode, codecOpenRes, av_err2str(codecOpenRes));
        return;
    }

    // ---------- 重采样 start ----------
    int64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    enum AVSampleFormat out_sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S16;
    int out_sample_rate = AUDIO_SAMPLE_RATE;
    int64_t in_ch_layout = pCodecContext->channel_layout;
    enum AVSampleFormat in_sample_fmt = pCodecContext->sample_fmt;
    int in_sample_rate = pCodecContext->sample_rate;
    swrContext = swr_alloc_set_opts(NULL, out_ch_layout, out_sample_fmt,
            out_sample_rate, in_ch_layout, in_sample_fmt, in_sample_rate, 0, NULL);
    if (swrContext == NULL) {
        // 提示错误
        callPlayerJniError(threadMode, SWR_ALLOC_SET_OPTS_ERROR_CODE, "swr alloc set opts error");
        return;
    }
    int swrInitRes = swr_init(swrContext);
    if (swrInitRes < 0) {
        callPlayerJniError(threadMode, SWR_CONTEXT_INIT_ERROR_CODE, "swr context swr init error");
        return;
    }
    pAudio = new DZAudio(audioStramIndex, pJniCall, pCodecContext, pFormatContext);
    // ---------- 重采样 end ----------
    // 回调到 Java 告诉他准备好了
}
