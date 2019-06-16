#include <jni.h>
#include "DZJNICall.h"
#include "DZFFmpeg.h"

// 在 c++ 中采用 c 的这种编译方式
extern "C" {
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}

DZJNICall *pJniCall;
DZFFmpeg *pFFmpeg;

#include "DZConstDefine.h"
extern "C" JNIEXPORT void JNICALL
Java_com_darren_media_DarrenPlayer_nPlay(JNIEnv *env, jobject instance, jstring url_) {
    pJniCall = new DZJNICall(NULL,env);
    const char *url = env->GetStringUTFChars(url_, 0);
    pFFmpeg = new DZFFmpeg(pJniCall,url);
    pFFmpeg->play();
    delete pJniCall;
    delete pFFmpeg;
    env->ReleaseStringUTFChars(url_, url);
}