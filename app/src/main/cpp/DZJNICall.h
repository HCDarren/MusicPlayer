//
// Created by hcDarren on 2019/6/16.
//

#ifndef MUSICPLAYER_DZJNICALL_H
#define MUSICPLAYER_DZJNICALL_H


#include <jni.h>

class DZJNICall {
public:
    jobject jAudioTrackObj;
    jmethodID jAudioTrackWriteMid;
    JavaVM *javaVM;
    JNIEnv *jniEnv;
public:
    DZJNICall(JavaVM *javaVM, JNIEnv *jniEnv);
    ~DZJNICall();

private:
    void initCrateAudioTrack();

public:
    void callAudioTrackWrite(jbyteArray audioData, int offsetInBytes, int sizeInBytes);
};


#endif //MUSICPLAYER_DZJNICALL_H
