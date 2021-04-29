#include <jni.h>
#include <string>
#include <android/log.h>
#include "include/utils.h"
#include "com_dede_cmake_CMakeLib.h"
#include "md5.h"

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

std::string upper_cast_md5(std::string str) {
    std::string s = md5(str);
    // return s;
    // 转大写
    char *buf = const_cast<char *>(s.c_str());
    int i = 0;
    char c;
    while (buf[i]) {
        c = buf[i];
        buf[i] = static_cast<char>(toupper(c));
        i++;
    }
    return buf;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_dede_cmake_CMakeLib_md5(JNIEnv *env, jclass clazz, jstring string) {
    std::string input = (env->GetStringUTFChars(string, JNI_FALSE));
    return (env)->NewStringUTF(upper_cast_md5(input).c_str());
}