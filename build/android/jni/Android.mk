LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SRC_PATH = ../../..
SRCFOLDERS = . z80 tools 3rdparty/minizip 3rdparty/tinyxml snapshot platform platform/android platform/touch_ui devices devices/fdd devices/input devices/sound
ifdef _PROFILE
SRCFOLDERS += ui platform/custom_ui platform/linux 3rdparty/libpng
endif
CXXSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.cpp))
CSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.c))

LOCAL_MODULE    := usp
LOCAL_SRC_FILES := $(CXXSRCS) $(CSRCS)
LOCAL_CFLAGS	:= -D_ANDROID -DUSE_EXTERN_RESOURCES -DUSE_FILE32API -I$(SRC_PATH)/3rdparty/minizip -I$(SRC_PATH)/3rdparty/tinyxml
LOCAL_LDLIBS	:= -lz

ifndef _PROFILE
LOCAL_CFLAGS += -DUSE_CONFIG -DUSE_ZIP
else
LOCAL_CFLAGS += -DUSE_UI -DUSE_PROFILER -D_LINUX -I$(SRC_PATH)/3rdparty/libpng
endif

include $(BUILD_SHARED_LIBRARY)
