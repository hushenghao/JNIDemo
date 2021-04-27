#https://developer.android.com/ndk/guides/android_mk.html
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := utils-lib

LOCAL_SRC_FILES := utils/$(TARGET_ARCH_ABI)/libndk-build-utils-lib.so
# 导出头文件 同时会将路径附加到 LOCAL_C_INCLUDES
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/utils

include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)

# 导入共享库
LOCAL_SHARED_LIBRARIES := utils-lib
LOCAL_C_INCLUDES := $(LOCAL_PATH)/utils

LOCAL_MODULE := ndk-build-lib

LOCAL_SRC_FILES := ndk_build_lib.cpp
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)
