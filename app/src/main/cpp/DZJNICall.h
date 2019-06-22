//
// Created by hcDarren on 2019/6/16.
//

#ifndef MUSICPLAYER_DZJNICALL_H
#define MUSICPLAYER_DZJNICALL_H

#include <jni.h>

enum ThreadMode{
    THREAD_CHILD,THREAD_MAIN
};

class DZJNICall {
public:
    JavaVM *javaVM;
    JNIEnv *jniEnv;
    jmethodID jPlayerErrorMid;
    jobject jPlayerObj;
public:
    DZJNICall(JavaVM *javaVM, JNIEnv *jniEnv, jobject jPlayerObj);
    ~DZJNICall();

public:
    void callPlayerError(ThreadMode threadMode,int code, char *msg);
};


#endif //MUSICPLAYER_DZJNICALL_H
