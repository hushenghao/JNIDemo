# 基于 AS 4.1.x 的 Android JNI 项目总结

一个Android JNI 开发的项目Demo，演示了在 Android Studio上 CMake 和 ndk-build 的配置和使用，包含了**预编译库**的编译导入和使用。

![](https://assets.che300.com/wiki/2021-04-28/16196034480785026.png)

**app**：jni演示demo；
**cmake**：cmake构建的JNI项目；
**ndk-build**：ndk-build构建的JNI项目；
**utils-prebuild**：预编译库生成，不直接参与项目编译。

## 背景
近期收到了应用商店发送的提醒邮件，由于公司目前项目使用的是 `armeabi` 兼容模式，都没有针对`arm64-v8a`架构兼容，而且如果不能及时适配还面临下架风险，所以arm64适配任务刻不容缓。
![邮件截图](https://assets.che300.com/wiki/2021-04-28/16195775177886608.png)
统计了一下项目中需要适配的功能：
* 已适配：
    * ijkPlayer
    * MMKV

* 需要升级SDK：
    * UMeng推送、统计
    * 百度定位、地图
    * 腾讯IM
    * 一键登录
    * OCR

* 需要自编译：
    * 接口加密
    * 签名校验

整理后发现项目中公共依赖和SDK这些还好，都已经提供了全架构的so文件，只需要根据最新版本的文档升级即可。重点是后面的App接口加密、签名校验功能，由于使用的是预构建静态库，源码已经不知去向，而且已经渐渐不能满足所需的加密的场景，所以打算重新实现现有的加密验签逻辑，同时整理出来一份Android NDK开发时一些基础知识。

## 方案选择
目前Android Studio 直接支持两种 C 代码编译工具，分别是：

> [CMake](https://developer.android.google.cn/ndk/guides/cmake)
一个跨平台的C代码编译工具，配置更加灵活，社区支持更丰富，应用更广范。

> [ndk-build](https://developer.android.com/ndk/guides/ndk-build)
一个速度比 CMake 快但仅支持 Android 的编译脚本，有官方支持并使用的构建脚本，源码中大量模块使用 `Android.mk`构建。

可以根据实际情况，选择合适的方案。当前项目基于 `Android Studio 4.1.3`，`CMake 3.10.2`，`NDK 22.1.7171670` 构建。

## 环境配置

1. [下载所需的 NDK 和 CMake，LLDB 可选](https://developer.android.google.cn/studio/projects/install-ndk)
    假如只用 ndk-build，CMake 也可以不下载
    ![下载 NDK、CMake](https://assets.che300.com/wiki/2021-04-28/16195889338345077.png)

2. [关联到项目](https://developer.android.google.cn/studio/projects/gradle-external-native-builds)

    配置 Gradle 以关联到原生库，需要将 `externalNativeBuild` 块添加到模块级 `build.gradle` 文件中，并使用 `cmake` 或 `ndkBuild` 块对其进行配置。目前不支持在同一模块中同时使用 CMake 和 ndk-build。

3. 编写代码

    所有相应c代码路径没有明确限制，只需要在 build.gradle 所指向的 `Android.mk` 或 `CMakeLists.txt` 文件中引用代码即可。但是为了易于管理，放在了路径 `src/main/cpp` 或者 `src/main/jni` 下。

### CMake配置

build.gradle
```groovy
android {
    defaultConfig {
        // 编译配置项，不需要可以不添加
        externalNativeBuild {
            cmake {
                // 自定义参数
                arguments "-DANDROID_ARM_NEON=TRUE", "-DANDROID_TOOLCHAIN=clang", "-DANDROID_PLATFORM=android-19", "-DANDROID_STL=c++_static"
                // c flag
                cFlags "-D__STDC_FORMAT_MACROS"
                // cpp flag
                cppFlags "-fexceptions", "-frtti"
            }
        }

        ndk {
            // 指定需要的ABI 可以不配置，默认全部架构
            abiFilters 'x86', 'x86_64', 'armeabi-v7a', 'arm64-v8a'
        }
    }

    externalNativeBuild {
        cmake {
            // 指定CMakeLists文件路径
            path "src/main/cpp/CMakeLists.txt"
            // 指定CMake版本，也可以在CMakeLists.txt内配置
            version "3.10.2"
        }
    }
}
```

[CMakeLists.txt](https://developer.android.google.cn/ndk/guides/cmake)

```Makefile
# CMake版本
cmake_minimum_required(VERSION 3.10.2)
# 项目名称，单项目可以不配置
project("cmake-jni")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

# 创建lib库名称，并设置为共享库
add_library( # 库名称不用包含lib开头，将生成libcmake-lib.so文件
        cmake-lib
        # 使用共享库，静态库为 STATIC
        SHARED
        # 源码文件，如果多个用空格隔开，这里为CMakeLists.txt所在位置的相对路径
        cmake_lib.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

# 查找导入预编译库
find_library( # 设置log-lib名称变量
        log-lib
        # 指定需要的库名称
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # 指定链接到的目标库，必须与add_library()名称相同
        cmake-lib
        # 需要链接的库 多个用空格分割
        ${log-lib})
```

### ndk-build配置

build.gradle
```groovy
android {
    defaultConfig {
        ndk {
            // 导入log库
            ldLibs "log"
            // 指定需要的ABI 可以不配置，默认全部架构
            abiFilters 'x86', 'x86_64', 'armeabi-v7a', 'arm64-v8a'
        }
    }

    // 指定ndk版本
    ndkVersion '22.1.7171670'

    externalNativeBuild {
        ndkBuild {
            // 指定Android.mk文件
            path 'src/main/jni/Android.mk'
        }
    }
}
```
[Android.mk](https://developer.android.google.cn/ndk/guides/android_mk)
```Makefile
# 声明LOCAL_PATH路径为当前路径
LOCAL_PATH := $(call my-dir)
# 清除其他模块设置的一些flag等配置
include $(CLEAR_VARS)
# 模块名称，同样不需要包含lib开头
# 会生成libndk-build-lib.so
LOCAL_MODULE := ndk-build-lib
# 源文件，多个用空格分割
LOCAL_SRC_FILES := ndk_build_lib.cpp
# 导入log库，需要在gradle声明
LOCAL_LDLIBS := -llog
# 这里编译为共享库，同样支持使用 BUILD_STATIC_LIBRARY 生成静态库
include $(BUILD_SHARED_LIBRARY)
```
[Application.mk](https://developer.android.google.cn/ndk/guides/application_mk)
```Makefile
# https://developer.android.com/ndk/guides/application_mk.html
# 只生成指定架构的so库
# APP_ABI := all armeabi-v7a arm64-v8a x86
# 类似于CMake "-DANDROID_PLATFORM=android-19", "-DANDROID_STL=c++_static"
APP_PLATFORM := android-19 # 目标版本
# APP_STL := c++_static # 运行时库配置
```

## 生成头文件

实际上简单的JNI开发，头文件是可选的，但是为了防止拼写错误，这边演示一下如何用使用命令行生成Class对应的头文件。

### Kotlin
使用Kotlin声明JNI交互方法，Kotlin的JNI关键字是`external`，不同于Java的 `native`。
```kotlin
package com.dede.cmake
// 也可以直接声明为object CMakeLib {...}
// 可以避免伴生对象声明与创建，这里是为了演示成员JNI方法
class CMakeLib {

    external fun memberCallJNI(): String

    companion object {
        @JvmStatic
        external fun staticCallJNI(): String

        init {
            // 改为声明生成的so文件，不用包含lib前缀和.so后缀
            System.loadLibrary("cmake-lib")
        }
    }
}
```

重新编译模块 **Build -> Make Module 'XXX.xxx'**，编译生成.class文件。
class路径为：`mudule/build/tmp/kotlin-classes/debug/com/dede/cmake/CMakeLib.class`，在 `mudule/build/tmp/kotlin-classes/debug`路径下运行：
```shell
// javah 全类名
javah com.dede.cmake.CMakeLib 
```
运行完会在`module/../debug`下生成对应的头文件：`com_dede_cmake_CMakeLib.h`：
```c++
/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_dede_cmake_CMakeLib */

#ifndef _Included_com_dede_cmake_CMakeLib
#define _Included_com_dede_cmake_CMakeLib
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_dede_cmake_CMakeLib
 * Method:    memberCallJNI
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_dede_cmake_CMakeLib_memberCallJNI
  (JNIEnv *, jobject);

/*
 * Class:     com_dede_cmake_CMakeLib
 * Method:    staticCallJNI
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_dede_cmake_CMakeLib_staticCallJNI
  (JNIEnv *, jclass);

#ifdef __cplusplus
}
#endif
#endif
```

### Java
Java声明JNI交互的Class和方法，**这里的Java生成方法只做为演示，没有实现相应的代码**
```Java
public class JavaJNI {

    static {
        System.loadLibrary("cmake-lib");
    }
    
    public static native String callJNI(String str);
}
```
重新编译当前模块 **Build -> Make Module 'XXX.xxx'**，编译生成.class文件。
class路为：`mudule/build/intermediates/javac/debug/classes/com/dede/cmake/JavaJNI.class`，在 `mudule/build/intermediates/javac/debug/classes`路径下运行：`javah 全类名`，
运行完会在`module/../classes`下生成对应的头文件：`com_dede_cmake_JavaJNI.h`
```c++
/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_dede_cmake_JavaJNI */

#ifndef _Included_com_dede_cmake_JavaJNI
#define _Included_com_dede_cmake_JavaJNI
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_dede_cmake_JavaJNI
 * Method:    callJNI
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_dede_cmake_JavaJNI_callJNI
  (JNIEnv *, jclass, jstring);

#ifdef __cplusplus
}
#endif
#endif
```

## 编写C代码
将成功的头文件复制到c源码路径下即可，声明相应的cpp文件，文件名可以不和头文件一样，例如：`cmake_lib.cpp`

```C++
#include <jni.h>
#include <string>
// 导入Android log头文件
#include <android/log.h>
// 1. 导入目标头文件
#include "com_dede_cmake_CMakeLib.h"

// Android LOG打印，调用会在Logcat打印日志，不需要可以不定义
#define TAG "CMake-lib"
#define LOGD(TAG, ...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__)
#define LOGI(TAG, ...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__)
#define LOGW(TAG, ...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__)
#define LOGE(TAG, ...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)
#define LOGF(TAG, ...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__)

// 2. 直接输入方法名联想自动生成空方法，实现相应逻辑
extern "C" JNIEXPORT jstring JNICALL
Java_com_dede_cmake_CMakeLib_staticCallJNI(JNIEnv *env, jclass clazz) {
    LOGD(TAG, "staticCallJNI");// 打印log
    std::string hello = "Hello from C++";// 声明字符串
    return env->NewStringUTF(hello.c_str());// 转换为jstring返回
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_dede_cmake_CMakeLib_memberCallJNI(JNIEnv *env, jobject that) {
    LOGD(TAG, "memberCallJNI");
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
```

调用JNI方法，在Activity内调用

```kotlin
Log.i(TAG, CMakeLib.staticCallJNI())
Log.i(TAG, CMakeLib().memberCallJNI())
Log.i(TAG, NDKBuildLib.staticCallJNI())
Log.i(TAG, NDKBuildLib().memberCallJNI())
```

编写完成，连接手机运行，一个简单的jni demo就可以跑起来了。
![](https://assets.che300.com/wiki/2021-04-28/16196045974754046.png)

但是目前只是一个简单的demo，我们要把我们的一些安全加密代码添加进去。
例如：添加一个简单的md5字符串加密代码：

添加新的md5方法，这里使用静态方法实现
```kotlin
class CMakeLib {

    external fun memberCallJNI(): String

    companion object {
        // 添加 md5 jni方法
        @JvmStatic
        external fun md5(string: String): String

        @JvmStatic
        external fun staticCallJNI(): String

        init {
            System.loadLibrary("cmake-lib")
        }
    }
}
```

`javah` 重新生成头文件。
声明C实现的md5 jni方法，md5实现代码就不贴了，可以在[JNIDemo](https://github.com/hushenghao/JNIDemo)里查看：
```C++
// 导入头文件
#include "md5.h"

// 字符串md5加密并转大写
std::string upper_cast_md5(std::string str) {
    std::string s = md5(str);// 调用md5方法
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
Java_com_dede_cmake_CMakeLib_md5(JNIEnv *env, jobject thiz, jstring string) {
    // 转成string类型
    std::string input = (env->GetStringUTFChars(string, JNI_FALSE));
    // 转换为jstring
    return (env)->NewStringUTF(upper_cast_md5(input).c_str());
}
```

不要忘记把`md5.cpp`添加到编译文件内，根据你选择的构建工具，添加到CMake或者ndk-build文件内：

```Makefile
# CMake
add_library(cmake-lib

        SHARED

        cmake_lib.cpp
        md5.cpp)

# ndk-build
LOCAL_SRC_FILES := ndk_build_lib.cpp md5.cpp
```

调用新添加的md5方法：
```kotlin
findViewById<TextView>(R.id.sample_text).apply {
    // 加密Hello World!
    text = CMakeLib.md5(text.toString())
}
```

至此一个简单的C实现的md5功能就实现了，当然项目中的加密不可能仅仅依靠一个md5加密，这里只做为演示。可以根据需要添加我们自己的加密和校验逻辑。

两种方案使用后发现 CMake 会比 ndk-build 包含多余代码，生成的so文件会比 ndk-build 大，应该是包含了`C++ 运行时库`的问题，但是也不能去掉，可能是哪里配置错了，有了解的望指正。

## 预编译库使用

大部分时候我们不会在我们的项目中从0编写C代码，通常会调用一些已经预编译好的第三方库，比如`OpenCV`、`ffmpeg`等等，他们可能是共享库`.so`文件，或者是静态库`.a`、`.o`。下面给简单演示预编译库我们应该怎么用，这里使用共享库做演示：

### 生成预编译库

这里直接生成一个简单的预编译库`utils-lib`，由于是预编译库，我们需要对外提供头文件以供集成方调用，例如：
utils.h
```C++
#ifndef TOSTRING_H
#define TOSTRING_H

#include <jni.h>

/**
 * 提供一个调用Java toString的方法
 * @param env jni上下文环境
 * @param that 目标对象
 * @return 字符串
 */
char *to_string(JNIEnv *env, jobject that);

#endif
```

utils.cpp
```C++
#include <jni.h>
#include "utils.h"

char *to_string(JNIEnv *env, jobject that) {
    // 获取jclass
    jclass clazz = env->GetObjectClass(that);
    // 获取toString方法id
    jmethodID id = env->GetMethodID(clazz, "toString", "()Ljava/lang/String;");
    // 调用toString方法
    jstring to_string = jstring(env->CallObjectMethod(that, id));
    // 不要忘了jtring转换为string
    char *str = const_cast<char *>(env->GetStringUTFChars(to_string, JNI_FALSE));
    return str;
}
```

这里使用CMake构建，ndk-build构建同理：
```Makefile
cmake_minimum_required(VERSION 3.10.2)
# 编译utils.cpp
add_library(cmake-utils-lib SHARED utils.cpp)
# 生成libcmake-utils-lib.so
target_link_libraries(cmake-utils-lib)
```

编译生成共享库所需的`.so`文件
```bash
// 可以直接使用assembleRelease命令编译
./gradlew :utils-prebuild:assembleRelease

// 也可以使用externalNativeBuildRelease直接编译
./gradlew :utils-prebuild:externalNativeBuildRelease
```
编译产物在module的以下路径：

* CMake：`module/build/intermediates/ndkBuild/release/obj/local`
* ndk-build：`module/build/intermediates/cmake/release/obj`

只需要使用`.so`文件即可。
这时编译的多个架构的文件和头文件就可以提供给集成方了。

### 导入预编译库

将所有架构的so文件导入，同样这个路径也是不限制的，这里复制到了 `src/main/cpp` 或者 `src/main/jni` 下，同时将头文件也复制到了当前路径。

**CMake**
![](https://assets.che300.com/wiki/2021-04-28/16196033785774102.png)

```Makefile
cmake_minimum_required(VERSION 3.10.2)

project("cmake-jni")

add_library(cmake-lib

        SHARED

        cmake_lib.cpp
        md5.cpp)

# 引入预编译库utils-lib
add_library(utils-lib STATIC IMPORTED)
# 设置预编译库路径，不可以用相对路径
# ${CMAKE_SOURCE_DIR}表示当前文件所在路径
set_target_properties(
        utils-lib
        PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libcmake-utils-lib.so)
# AS4.0 .so不需要放到默认jniLibs文件夹了
# 导入预编译库的头文件
include_directories(include)

find_library(log-lib log)

target_link_libraries(
        cmake-lib

        # 链接utils-lib
        ${log-lib} utils-lib)
```

[**ndk-build**](https://developer.android.google.cn/ndk/guides/prebuilts)
![](https://assets.che300.com/wiki/2021-04-28/16196032941938022.png)

```Makefile
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
# 声明utils模块
LOCAL_MODULE := utils-lib
# 导入依赖so文件，使用 TARGET_ARCH_ABI 寻找目标api的so文件
LOCAL_SRC_FILES := utils/$(TARGET_ARCH_ABI)/libndk-build-utils-lib.so
# 导出头文件 同时会将路径附加到 LOCAL_C_INCLUDES
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/utils
# 导出共享库
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
# 导入共享库
LOCAL_SHARED_LIBRARIES := utils-lib
# 声明了LOCAL_EXPORT_C_INCLUDES 可以不声明LOCAL_C_INCLUDES
# LOCAL_C_INCLUDES := $(LOCAL_PATH)/utils

# 以下都一样
LOCAL_MODULE := ndk-build-lib

LOCAL_SRC_FILES := ndk_build_lib.cpp
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)
```
**Sync Now** 后即可在C代码中引用到预编译库

### 调用预编译库
以CMake做演示，ndk-build导入头文件声明方式稍有不同。
```C++
// 导入头文件
#include "include/utils.h"

// ...

extern "C" JNIEXPORT jstring JNICALL
Java_com_dede_ndk_1build_NDKBuildLib_staticCallJNI(JNIEnv *env, jclass clazz) {
    // 调用 utils.to_string方法
    LOGD(TAG, "staticCallJNI: %s", to_string(env, clazz));
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
```

这时就可以运行代码了，可以在logcat看到如下打印，已经看到输出toString方法打印的内容了。
![](https://assets.che300.com/wiki/2021-04-28/16196044206449473.png)

这里演示了共享库的导入，静态库导入方式类似。[查看官方文档](https://developer.android.google.cn/ndk/guides/prebuilts)

## Todo

* so文件瘦身
* 预编译静态库

## 相关

[源码 JNIDemo](https://github.com/hushenghao/JNIDemo)

[向您的添加C和C++代码](https://developer.android.google.cn/studio/projects/add-native-code)

[Android ndk-build 文档](https://developer.android.google.cn/ndk/guides/ndk-build)
[Android.mk 文档](https://developer.android.google.cn/ndk/guides/android_mk)
[Application.mk 文档](https://developer.android.google.cn/ndk/guides/application_mk)

[Android CMake 文档](https://developer.android.google.cn/ndk/guides/cmake)
[CMake 工具链参数](https://developer.android.google.cn/ndk/guides/cmake#variables)


[ReLinker](https://github.com/KeepSafe/ReLinker) 
A robust native library loader for Android.
用于lib.so加载，可以解决部分so未被正确加载引起的问题。

[CMake 官网](https://cmake.org/)
