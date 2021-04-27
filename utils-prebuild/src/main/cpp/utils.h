//
// Created by hsh on 4/27/21.
//


#ifndef TOSTRING_H
#define TOSTRING_H

#include <jni.h>


/**
 * 调用Java toString的方法
 * @param env jni上下文环境
 * @param that 目标对象
 * @return 字符串
 */
char *to_string(JNIEnv *env, jobject that);


#endif
