package com.dede.cmake

/**
 * @author hsh
 * @since 4/28/21 3:54 PM
 */
class KotlinJNI {
    external fun memberCallJNI(): String

    companion object {
        @JvmStatic
        external fun staticCallJNI(): String
    }
}