#include <jni.h>
#include <string>
#include <android/log.h>
#include "include/utils.h"

// 定义Android LOG方法
#define TAG "CMake-lib"
#define LOGD(TAG, ...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__)
#define LOGI(TAG, ...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__)
#define LOGW(TAG, ...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__)
#define LOGE(TAG, ...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)
#define LOGF(TAG, ...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__)


/**
 * 静态JNI方法
 * @param *evn
 * @param clazz Class对象
 */
extern "C" JNIEXPORT jstring JNICALL
Java_com_dede_cmake_CMakeLib_staticCallJNI(JNIEnv *env, jclass clazz) {
    LOGD(TAG, "staticCallJNI: %s", to_string(env, clazz));
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

/**
 * 成员JNI方法
 * @param *env
 * @param that 对象实例
 */
extern "C" JNIEXPORT jstring JNICALL
Java_com_dede_cmake_CMakeLib_memberCallJNI(JNIEnv *env, jobject that) {
    LOGD(TAG, "memberCallJNI: %s", to_string(env, that));
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}