#include <jni.h>
#include <string>
#include <android/log.h>
#include <utils.h>

// 定义Android LOG方法
#define TAG "NDK-Build-lib"
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
Java_com_dede_ndk_1build_NDKBuildLib_staticCallJNI(JNIEnv *env, jclass clazz) {
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
Java_com_dede_ndk_1build_NDKBuildLib_memberCallJNI(JNIEnv *env, jobject that) {
    LOGD(TAG, "memberCallJNI: %s", to_string(env, that));
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

/**
 * sizeof(array) 数组的sizeof值等于数组所占用的内存字节数
 * sizeof(array[0]) 结构体的sizeof涉及到字节对齐问题，可以简单理解为单个元素所占内存大小
 * 若数组为存储字符串的字符数组（char array），则所求得的长度还需要减一，原因为存储字符串的字符数组末尾有一个'\0'字符，需要去掉它。
 */
#define GET_ARRAY_LEN(array, len){len= (sizeof(array) / sizeof(array[0]));}

/**
 * 动态注册的空参方法
 */
jstring dynamic(JNIEnv *env, jobject that) {
    LOGD(TAG, "dynamic: %s", to_string(env, that));
    return env->NewStringUTF("JNI dynamic");
}

/**
 * 动态注册的jobject 2 string
 */
jstring obj2String(JNIEnv *env, jobject that, jobject obj) {
    LOGD(TAG, "obj2String: %s", to_string(env, that));
    return env->NewStringUTF(to_string(env, obj));
}

/**
 * 加法
 */
jint add(JNIEnv *env, jobject that, jint a, jint b) {
    LOGD(TAG, "add: %s", to_string(env, that));
    return a + b;
}

/**
 * JNI方法动态注册
 * typedef struct {
 *    const char* name;// 方法名
 *    const char* signature;// 方法签名，方法参数和返回值
 *    void*       fnPtr;// 函数指针
 * } JNINativeMethod;
 *
 * 方法签名生成：javac编译后，使用javap -s 全类名，使用输入内容的descriptor:后的内容，例如：
 * public final native java.lang.String obj2String(java.lang.Object);
 *   descriptor: (Ljava/lang/Object;)Ljava/lang/String;
 */
static const JNINativeMethod jniNativeMethod[] = {
        {"dynamic",    "()Ljava/lang/String;",                   (void *) dynamic},
        {"obj2String", "(Ljava/lang/Object;)Ljava/lang/String;", (void *) obj2String},
        {"add",        "(II)I",                                  (void *) add},
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *javaVm, void *pVoid) {
    JNIEnv *jniEnv = nullptr;
    jint result = javaVm->GetEnv(reinterpret_cast<void **>(&jniEnv), JNI_VERSION_1_6);
    if (result != JNI_OK) {
        return -1;
    }
    jclass clazz = jniEnv->FindClass("com/dede/ndk_build/NDKBuildLib");
    int methodCount;
    GET_ARRAY_LEN(jniNativeMethod, methodCount);
    jniEnv->RegisterNatives(clazz, jniNativeMethod, methodCount);
    return JNI_VERSION_1_6;
}