LOCAL_PATH := $(call my-dir)

# Include all subdirectories
include $(LOCAL_PATH)/../../glew/Android.mk
include $(LOCAL_PATH)/../../lib3ds/Android.mk
include $(LOCAL_PATH)/../../lua5/Android.mk
include $(LOCAL_PATH)/../../nebu/Android.mk
include $(LOCAL_PATH)/../../src/Android.mk

# Main application
include $(CLEAR_VARS)

LOCAL_MODULE := gltron
LOCAL_SRC_FILES := ../../src/gltron.c
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2

# Link all libraries
LOCAL_STATIC_LIBRARIES := \
    game \
    input \
    audio \
    video \
    configuration \
    base \
    filesystem \
    nebu_input \
    nebu_audio \
    nebu_video \
    nebu_scripting \
    nebu_filesystem \
    nebu_base \
    lua \
    lualib \
    lib3ds \
    glew

include $(BUILD_SHARED_LIBRARY)