LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SRC_PATH = ../../..
SRCFOLDERS = . z80 tools tools/zlib tools/tinyxml tools/libpng snapshot ui platform platform/custom_ui platform/android/jni platform/linux devices devices/fdd devices/input devices/sound 
CXXSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.cpp))
CSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.c))

LOCAL_MODULE    := usp
LOCAL_SRC_FILES := $(CXXSRCS) $(CSRCS)
LOCAL_CFLAGS	:= -D_ANDROID -DUSE_UI -D_LINUX -DUSE_EXTERN_RESOURCES -O3

include $(BUILD_SHARED_LIBRARY)
