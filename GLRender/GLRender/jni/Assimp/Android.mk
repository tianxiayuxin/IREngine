#
#3rd party prebuilt assimp library
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := assimp
LOCAL_SRC_FILES := libs/libassimp.so

include $(PREBUILT_SHARED_LIBRARY)