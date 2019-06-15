#include <jni.h>

// 在 c++ 中采用 c 的这种编译方式
extern "C" {
#include "libavformat/avformat.h"
}

#include "DZConstDefine.h"

jobject initCreateAudioTrack(JNIEnv *env) {
    /*AudioTrack(int streamType, int sampleRateInHz, int channelConfig, int audioFormat,
            int bufferSizeInBytes, int mode)*/
    jclass jAudioTrackClass = env->FindClass("android/media/AudioTrack");
    jmethodID jAudioTackCMid = env->GetMethodID(jAudioTrackClass, "<init>", "(IIIIII)V");

    int streamType = 3;
    int sampleRateInHz = AUDIO_SAMPLE_RATE;
    int channelConfig = (0x4 | 0x8);
    int audioFormat = 2;
    int mode = 1;

    // int getMinBufferSize(int sampleRateInHz, int channelConfig, int audioFormat)
    jmethodID getMinBufferSizeMid = env->GetStaticMethodID(jAudioTrackClass, "getMinBufferSize",
            "(III)I");
    int bufferSizeInBytes = env->CallStaticIntMethod(jAudioTrackClass, getMinBufferSizeMid,
            sampleRateInHz, channelConfig, audioFormat);
    LOGE("bufferSizeInBytes = %d",bufferSizeInBytes);

    jobject jAudioTrackObj = env->NewObject(jAudioTrackClass, jAudioTackCMid, streamType,
            sampleRateInHz, channelConfig, audioFormat, bufferSizeInBytes, mode);

    // play
    jmethodID playMid = env->GetMethodID(jAudioTrackClass, "play", "()V");
    env->CallVoidMethod(jAudioTrackObj, playMid);

    return jAudioTrackObj;
}

extern "C" JNIEXPORT void JNICALL
Java_com_darren_media_DarrenPlayer_nPlay(JNIEnv *env, jobject instance, jstring url_) {
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
    jobject jAudioTrackObj;
    jmethodID jWriteMid;
    jclass jAudioTrackClass;

    formatOpenInputRes = avformat_open_input(&pFormatContext, url, NULL, NULL);
    if (formatOpenInputRes != 0) {
        // 第一件事，需要回调给 Java 层(下次课讲)
        // 第二件事，需要释放资源
        // return;
        LOGE("format open input error: %s", av_err2str(formatOpenInputRes));
        goto __av_resources_destroy;
    }

    formatFindStreamInfoRes = avformat_find_stream_info(pFormatContext, NULL);
    if (formatFindStreamInfoRes < 0) {
        LOGE("format find stream info error: %s", av_err2str(formatFindStreamInfoRes));
        // 这种方式一般不推荐这么写，但是的确方便
        goto __av_resources_destroy;
    }

    // 查找音频流的 index
    audioStramIndex = av_find_best_stream(pFormatContext, AVMediaType::AVMEDIA_TYPE_AUDIO, -1, -1,
            NULL, 0);
    if (audioStramIndex < 0) {
        LOGE("format audio stream error: %s");
        // 这种方式一般不推荐这么写，但是的确方便
        goto __av_resources_destroy;
    }

    // 查找解码
    pCodecParameters = pFormatContext->streams[audioStramIndex]->codecpar;
    pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
    if (pCodec == NULL) {
        LOGE("codec find audio decoder error");
        // 这种方式一般不推荐这么写，但是的确方便
        goto __av_resources_destroy;
    }
    // 打开解码器
    pCodecContext = avcodec_alloc_context3(pCodec);
    if (pCodecContext == NULL) {
        LOGE("codec alloc context error");
        // 这种方式一般不推荐这么写，但是的确方便
        goto __av_resources_destroy;
    }
    codecParametersToContextRes = avcodec_parameters_to_context(pCodecContext, pCodecParameters);
    if (codecParametersToContextRes < 0) {
        LOGE("codec parameters to context error: %s", av_err2str(codecParametersToContextRes));
        // 这种方式一般不推荐这么写，但是的确方便
        goto __av_resources_destroy;
    }

    codecOpenRes = avcodec_open2(pCodecContext, pCodec, NULL);
    if (codecOpenRes != 0) {
        LOGE("codec audio open error: %s", av_err2str(codecOpenRes));
        // 这种方式一般不推荐这么写，但是的确方便
        goto __av_resources_destroy;
    }

    jAudioTrackClass = env->FindClass("android/media/AudioTrack");
    jWriteMid = env->GetMethodID(jAudioTrackClass, "write", "([BII)I");
    jAudioTrackObj = initCreateAudioTrack(env);

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

                    // write 写到缓冲区 pFrame.data -> javabyte
                    // size 是多大，装 pcm 的数据
                    // 1s 44100 点  2通道 ，2字节    44100*2*2
                    // 1帧不是一秒，pFrame->nb_samples点
                    int dataSize = av_samples_get_buffer_size(NULL, pFrame->channels,
                            pFrame->nb_samples, pCodecContext->sample_fmt, 0);
                    jbyteArray jPcmByteArray = env->NewByteArray(dataSize);
                    // native 创建 c 数组
                    jbyte *jPcmData = env->GetByteArrayElements(jPcmByteArray, NULL);
                    memcpy(jPcmData, pFrame->data, dataSize);
                    // 0 把 c 的数组的数据同步到 jbyteArray , 然后释放native数组
                    env->ReleaseByteArrayElements(jPcmByteArray, jPcmData, 0);
                    env->CallIntMethod(jAudioTrackObj, jWriteMid, jPcmByteArray, 0, dataSize);
                    // 解除 jPcmDataArray 的持有，让 javaGC 回收
                    env->DeleteLocalRef(jPcmByteArray);
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
    env->DeleteLocalRef(jAudioTrackObj);

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