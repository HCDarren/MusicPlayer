//
// Created by hcDarren on 2019/6/16.
//

#include "DZJNICall.h"
#include "DZConstDefine.h"

DZJNICall::DZJNICall(JavaVM *javaVM, JNIEnv *jniEnv) {
    this->javaVM = javaVM;
    this->jniEnv = jniEnv;
    initCrateAudioTrack();
}

void DZJNICall::initCrateAudioTrack() {
    jclass jAudioTrackClass = jniEnv->FindClass("android/media/AudioTrack");
    jmethodID jAudioTackCMid = jniEnv->GetMethodID(jAudioTrackClass, "<init>", "(IIIIII)V");

    int streamType = 3;
    int sampleRateInHz = AUDIO_SAMPLE_RATE;
    int channelConfig = (0x4 | 0x8);
    int audioFormat = 2;
    int mode = 1;

    // int getMinBufferSize(int sampleRateInHz, int channelConfig, int audioFormat)
    jmethodID getMinBufferSizeMid = jniEnv->GetStaticMethodID(jAudioTrackClass, "getMinBufferSize",
            "(III)I");
    int bufferSizeInBytes = jniEnv->CallStaticIntMethod(jAudioTrackClass, getMinBufferSizeMid,
            sampleRateInHz, channelConfig, audioFormat);
    LOGE("bufferSizeInBytes = %d", bufferSizeInBytes);

    jAudioTrackObj = jniEnv->NewObject(jAudioTrackClass, jAudioTackCMid, streamType,
            sampleRateInHz, channelConfig, audioFormat, bufferSizeInBytes, mode);

    // start  method
    jmethodID playMid = jniEnv->GetMethodID(jAudioTrackClass, "play", "()V");
    jniEnv->CallVoidMethod(jAudioTrackObj, playMid);

    // write method
    jAudioTrackWriteMid = jniEnv->GetMethodID(jAudioTrackClass, "write", "([BII)I");
}

void DZJNICall::callAudioTrackWrite(jbyteArray audioData, int offsetInBytes, int sizeInBytes) {
    jniEnv->CallIntMethod(jAudioTrackObj, jAudioTrackWriteMid, audioData, offsetInBytes, sizeInBytes);
}

DZJNICall::~DZJNICall() {
    jniEnv->DeleteLocalRef(jAudioTrackObj);
}
