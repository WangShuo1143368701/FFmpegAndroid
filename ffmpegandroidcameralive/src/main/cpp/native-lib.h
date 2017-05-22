//
// Created by Shuo.Wang on 2017/5/16.
//




#include <jni.h>


#ifndef FFMPEGANDROID_NATIVE_LIB_H
#define FFMPEGANDROID_NATIVE_LIB_H
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL Java_com_ws_ffmpegandroidcameralive_WSPlayer_initialize
        (JNIEnv *, jobject, jint , jint,jstring );

JNIEXPORT jint JNICALL Java_com_ws_ffmpegandroidcameralive_WSPlayer_start
        (JNIEnv *, jobject , jbyteArray );

JNIEXPORT jint JNICALL Java_com_ws_ffmpegandroidcameralive_WSPlayer_stop
        (JNIEnv *, jobject );

JNIEXPORT jint JNICALL Java_com_ws_ffmpegandroidcameralive_WSPlayer_close
        (JNIEnv *, jobject );

#ifdef __cplusplus
}
#endif
#endif