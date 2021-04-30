package com.dede.ndk_build

/**
 * 使用NDK-Build构建的JNI
 * @author hsh
 * @since 4/26/21 5:24 PM
 */
class NDKBuildLib {

    companion object {
        init {
            System.loadLibrary("ndk-build-lib")
        }

        @JvmStatic
        external fun staticCallJNI(): String

        @JvmStatic
        external fun dynamic(): String

        @JvmStatic
        external fun add(a: Int, b: Int): Int
    }

    external fun obj2String(obj: Any): String


    external fun memberCallJNI(): String
}