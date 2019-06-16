#include <jni.h>
#include "DZJNICall.h"

// 在 c++ 中采用 c 的这种编译方式
extern "C" {
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}

DZJNICall *pJniCall;

#include "DZConstDefine.h"
extern "C" JNIEXPORT void JNICALL
Java_com_darren_media_DarrenPlayer_nPlay(JNIEnv *env, jobject instance, jstring url_) {
    pJniCall = new DZJNICall(NULL,env);
    const char *url = env->GetStringUTFChars(url_, 0);
    // 讲的理念的东西，千万要注意
    av_register_all();
    avformat_network_init();
    AVFormatContext *pFormatContext = NULL;
    int formatOpenInputRes = 0;
    int formatFindStreamInfoRes = 0;
    int audioStramIndex = -1;
    AVCodecParameters *pCodecParameters;
    AVCodec *pCodec = NULL;
    AVCodecContext *pCodecContext = NULL;
    int codecParametersToContextRes = -1;
    int codecOpenRes = -1;
    int index = 0;
    AVPacket *pPacket = NULL;
    AVFrame *pFrame = NULL;

    formatOpenInputRes = avformat_open_input(&pFormatContext, url, NULL, NULL);
    if (formatOpenInputRes != 0) {
        // 第一件事，需要回调给 Java 层(下次课讲)
        // 第二件事，需要释放资源
        // return;
        LOGE("format open input error: %s", av_err2str(formatOpenInputRes));
        // goto __av_resources_destroy;
        return;
    }

    formatFindStreamInfoRes = avformat_find_stream_info(pFormatContext, NULL);
    if (formatFindStreamInfoRes < 0) {
        LOGE("format find stream info error: %s", av_err2str(formatFindStreamInfoRes));
        // 这种方式一般不推荐这么写，但是的确方便
        // goto __av_resources_destroy;
        return;
    }

    // 查找音频流的 index
    audioStramIndex = av_find_best_stream(pFormatContext, AVMediaType::AVMEDIA_TYPE_AUDIO, -1, -1,
            NULL, 0);
    if (audioStramIndex < 0) {
        LOGE("format audio stream error: %s");
        // 这种方式一般不推荐这么写，但是的确方便
        // goto __av_resources_destroy;
        return;
    }

    // 查找解码
    pCodecParameters = pFormatContext->streams[audioStramIndex]->codecpar;
    pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
    if (pCodec == NULL) {
        LOGE("codec find audio decoder error");
        // 这种方式一般不推荐这么写，但是的确方便
        // goto __av_resources_destroy;
        return;
    }
    // 打开解码器
    pCodecContext = avcodec_alloc_context3(pCodec);
    if (pCodecContext == NULL) {
        LOGE("codec alloc context error");
        // 这种方式一般不推荐这么写，但是的确方便
        // goto __av_resources_destroy;
        return;
    }
    codecParametersToContextRes = avcodec_parameters_to_context(pCodecContext, pCodecParameters);
    if (codecParametersToContextRes < 0) {
        LOGE("codec parameters to context error: %s", av_err2str(codecParametersToContextRes));
        // 这种方式一般不推荐这么写，但是的确方便
        // goto __av_resources_destroy;
        return;
    }

    codecOpenRes = avcodec_open2(pCodecContext, pCodec, NULL);
    if (codecOpenRes != 0) {
        LOGE("codec audio open error: %s", av_err2str(codecOpenRes));
        // 这种方式一般不推荐这么写，但是的确方便
        // goto __av_resources_destroy;
        return;
    }

    // ---------- 重采样 start ----------
    int64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    enum AVSampleFormat out_sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S16;
    int out_sample_rate = AUDIO_SAMPLE_RATE;
    int64_t in_ch_layout = pCodecContext->channel_layout;
    enum AVSampleFormat in_sample_fmt = pCodecContext->sample_fmt;
    int in_sample_rate = pCodecContext->sample_rate;
    SwrContext *swrContext = swr_alloc_set_opts(NULL, out_ch_layout, out_sample_fmt,
            out_sample_rate, in_ch_layout, in_sample_fmt, in_sample_rate, 0, NULL);
    if (swrContext == NULL) {
        // 提示错误
        return;
    }
    int swrInitRes = swr_init(swrContext);
    if (swrInitRes < 0) {
        return;
    }
    // size 是播放指定的大小，是最终输出的大小
    int outChannels = av_get_channel_layout_nb_channels(out_ch_layout);
    int dataSize = av_samples_get_buffer_size(NULL, outChannels, pCodecParameters->frame_size,
            out_sample_fmt, 0);
    uint8_t *resampleOutBuffer = (uint8_t *) malloc(dataSize);
    // ---------- 重采样 end ----------

    jbyteArray jPcmByteArray = env->NewByteArray(dataSize);
    // native 创建 c 数组
    jbyte *jPcmData = env->GetByteArrayElements(jPcmByteArray, NULL);

    pPacket = av_packet_alloc();
    pFrame = av_frame_alloc();
    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        if (pPacket->stream_index == audioStramIndex) {
            // Packet 包，压缩的数据，解码成 pcm 数据
            int codecSendPacketRes = avcodec_send_packet(pCodecContext, pPacket);
            if (codecSendPacketRes == 0) {
                int codecReceiveFrameRes = avcodec_receive_frame(pCodecContext, pFrame);
                if (codecReceiveFrameRes == 0) {
                    // AVPacket -> AVFrame
                    index++;
                    LOGE("解码第 %d 帧", index);

                    // 调用重采样的方法
                    swr_convert(swrContext, &resampleOutBuffer, pFrame->nb_samples,
                            (const uint8_t **) pFrame->data, pFrame->nb_samples);

                    // write 写到缓冲区 pFrame.data -> javabyte
                    // size 是多大，装 pcm 的数据
                    // 1s 44100 点  2通道 ，2字节    44100*2*2
                    // 1帧不是一秒，pFrame->nb_samples点
                    memcpy(jPcmData, resampleOutBuffer, dataSize);
                    // 0 把 c 的数组的数据同步到 jbyteArray , 然后释放native数组
                    env->ReleaseByteArrayElements(jPcmByteArray, jPcmData, JNI_COMMIT);
                    // TODO
                    pJniCall->callAudioTrackWrite(jPcmByteArray,0,dataSize);
                }
            }
        }
        // 解引用
        av_packet_unref(pPacket);
        av_frame_unref(pFrame);
    }

    // 1. 解引用数据 data ， 2. 销毁 pPacket 结构体内存  3. pPacket = NULL
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    // 解除 jPcmDataArray 的持有，让 javaGC 回收
    env->ReleaseByteArrayElements(jPcmByteArray, jPcmData, 0);
    env->DeleteLocalRef(jPcmByteArray);

    delete pJniCall;

    __av_resources_destroy:
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
    avformat_network_deinit();

    env->ReleaseStringUTFChars(url_, url);
}