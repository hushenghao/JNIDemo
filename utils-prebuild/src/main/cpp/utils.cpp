//
// Created by hsh on 4/27/21.
//
#include <jni.h>
#include "utils.h"

char *to_string(JNIEnv *env, jobject that) {
    jclass clazz = env->GetObjectClass(that);
    jmethodID id = env->GetMethodID(clazz, "toString", "()Ljava/lang/String;");
    jstring to_string = jstring(env->CallObjectMethod(that, id));
    char *str = const_cast<char *>(env->GetStringUTFChars(to_string, JNI_FALSE));
    return str;
}