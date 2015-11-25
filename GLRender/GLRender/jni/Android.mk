LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := GLRender

LOCAL_SRC_FILES := Interface.cpp IRApplication.cpp                                
LOCAL_C_INCLUDES:=$(LOCAL_PATH) 

LOCAL_LDLIBS    := -llog -landroid  -lEGL -lGLESv3

LOCAL_SHARED_LIBRARIES := assimp

include $(BUILD_SHARED_LIBRARY)

include $(LOCAL_PATH)/Assimp/Android.mk
