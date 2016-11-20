LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SRC_PATH = ../../..
SRCFOLDERS = . z80 tools 3rdparty/minizip 3rdparty/tinyxml2 snapshot platform platform/linux platform/android platform/touch_ui platform/gles2 devices devices/fdd devices/input devices/sound
ifdef _PROFILE
SRCFOLDERS += ui platform/custom_ui
endif
CXXSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.cpp))
CSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.c))

LOCAL_MODULE    := usp
LOCAL_SRC_FILES := $(CXXSRCS) $(CSRCS)
LOCAL_CFLAGS	:= -D_POSIX -D_ANDROID -DUSE_EXTERN_RESOURCES -DUSE_FILE32API -DUSE_CONFIG -DUSE_ZIP -DUSE_GLES2 -I$(SRC_PATH)/3rdparty/minizip -I$(SRC_PATH)/3rdparty/tinyxml2
LOCAL_LDLIBS	:= -lz -lGLESv2

ifdef _PROFILE
LOCAL_CFLAGS += -DUSE_UI -DUSE_PROFILER
endif

include $(BUILD_SHARED_LIBRARY)
