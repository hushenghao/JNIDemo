package com.dede.cmake;

public class JavaJNI {

    static {
        System.loadLibrary("cmake-lib");
    }

    public static native String callJNI(String str);

}
