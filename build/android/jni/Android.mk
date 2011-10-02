LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SRC_PATH = ../../..
SRCFOLDERS = . z80 tools tools/zlib tools/tinyxml snapshot platform platform/android platform/touch_ui devices devices/fdd devices/input devices/sound
ifdef _PROFILE
SRCFOLDERS += ui platform/custom_ui platform/linux tools/libpng
endif
CXXSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.cpp))
CSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.c))

LOCAL_MODULE    := usp
LOCAL_SRC_FILES := $(CXXSRCS) $(CSRCS)
LOCAL_CFLAGS	:= -D_ANDROID -DUSE_OPTIONS_COMMON -DUSE_EXTERN_RESOURCES

ifndef _PROFILE
LOCAL_CFLAGS += -DUSE_CONFIG -DUSE_ZIP 
else
LOCAL_CFLAGS += -DUSE_UI -DUSE_PROFILER -D_LINUX
endif

include $(BUILD_SHARED_LIBRARY)
