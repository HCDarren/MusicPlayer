#include <jni.h>
#include "DZJNICall.h"
#include "DZFFmpeg.h"
#include "DZConstDefine.h"

// 在 c++ 中采用 c 的这种编译方式
extern "C" {
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}

DZJNICall *pJniCall;
DZFFmpeg *pFFmpeg;

JavaVM *pJavaVM = NULL;

// 重写 so 被加载时会调用的一个方法
// 小作业，去了解动态注册
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *javaVM, void *reserved) {
    pJavaVM = javaVM;
    JNIEnv *env;
    if (javaVM->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_4;
}

extern "C" JNIEXPORT void JNICALL
Java_com_darren_media_DarrenPlayer_nPlay(JNIEnv *env, jobject instance) {
    if (pFFmpeg != NULL) {
        pFFmpeg->play();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_darren_media_DarrenPlayer_nPrepare(JNIEnv *env, jobject instance, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    if (pFFmpeg == NULL) {
        pJniCall = new DZJNICall(pJavaVM, env, instance);
        pFFmpeg = new DZFFmpeg(pJniCall, url);
        pFFmpeg->prepare();
    }
    env->ReleaseStringUTFChars(url_, url);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_darren_media_DarrenPlayer_nPrepareAsync(JNIEnv *env, jobject instance, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    if (pFFmpeg == NULL) {
        pJniCall = new DZJNICall(pJavaVM, env, instance);
        pFFmpeg = new DZFFmpeg(pJniCall, url);
        pFFmpeg->prepareAsync();
    }
    env->ReleaseStringUTFChars(url_, url);
}