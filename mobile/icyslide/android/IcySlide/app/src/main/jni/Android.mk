LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := integrity-app
LOCAL_SRC_FILES := app-integrity.c
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := init-app
LOCAL_SRC_FILES := app-init.cpp
LOCAL_CPPFLAGS := -std=c++11
include $(BUILD_SHARED_LIBRARY)