package com.dede.cmake

/**
 * 使用CMake构建的JNI
 *
 * @author hsh
 * @since 4/26/21 4:03 PM
 */
class CMakeLib {

    external fun memberCallJNI(): String

    companion object {
        @JvmStatic
        external fun staticCallJNI(): String

        init {
            System.loadLibrary("cmake-lib")
        }
    }
}