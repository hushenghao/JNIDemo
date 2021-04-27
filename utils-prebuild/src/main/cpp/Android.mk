LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ndk-build-utils-lib

LOCAL_SRC_FILES := utils.cpp

include $(BUILD_SHARED_LIBRARY)
