//
// Created by hcDarren on 2019/6/16.
//

#include "DZJNICall.h"
#include "DZConstDefine.h"

DZJNICall::DZJNICall(JavaVM *javaVM, JNIEnv *jniEnv, jobject jPlayerObj) {
    this->javaVM = javaVM;
    this->jniEnv = jniEnv;
    this->jPlayerObj = jniEnv->NewGlobalRef(jPlayerObj);

    jclass jPlayerClass = jniEnv->GetObjectClass(jPlayerObj);
    jPlayerErrorMid = jniEnv->GetMethodID(jPlayerClass, "onError", "(ILjava/lang/String;)V");
    jPlayerPreparedMid = jniEnv->GetMethodID(jPlayerClass, "onPrepared", "()V");
}

DZJNICall::~DZJNICall() {
    jniEnv->DeleteGlobalRef(jPlayerObj);
}

void DZJNICall::callPlayerError(ThreadMode threadMode, int code, char *msg) {
    // 子线程用不了主线程 jniEnv （native 线程）
    // 子线程是不共享 jniEnv ，他们有自己所独有的
    if (threadMode == THREAD_MAIN) {
        jstring jMsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jPlayerObj, jPlayerErrorMid, code, jMsg);
        jniEnv->DeleteLocalRef(jMsg);
    } else if (threadMode == THREAD_CHILD) {
        // 获取当前线程的 JNIEnv， 通过 JavaVM
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
            LOGE("get child thread jniEnv error!");
            return;
        }

        jstring jMsg = env->NewStringUTF(msg);
        env->CallVoidMethod(jPlayerObj, jPlayerErrorMid, code, jMsg);
        env->DeleteLocalRef(jMsg);

        javaVM->DetachCurrentThread();
    }
}

/**
 * 回调到 java 层告诉准备好了
 * @param threadMode
 */
void DZJNICall::callPlayerPrepared(ThreadMode threadMode) {
    // 子线程用不了主线程 jniEnv （native 线程）
    // 子线程是不共享 jniEnv ，他们有自己所独有的
    if (threadMode == THREAD_MAIN) {
        jniEnv->CallVoidMethod(jPlayerObj, jPlayerPreparedMid);
    } else if (threadMode == THREAD_CHILD) {
        // 获取当前线程的 JNIEnv， 通过 JavaVM
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
            LOGE("get child thread jniEnv error!");
            return;
        }
        env->CallVoidMethod(jPlayerObj, jPlayerPreparedMid);
        javaVM->DetachCurrentThread();
    }
}
