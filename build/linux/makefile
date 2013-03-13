SRC_PATH = ../..
OBJ_BASE = Release

PROJECT_NAME = unreal_speccy_portable
SRCFOLDERS = . z80 tools tools/zlib tools/tinyxml tools/libpng snapshot ui platform platform/custom_ui platform/gl platform/oal platform/wxwidgets platform/linux platform/sdl platform/benchmark devices devices/fdd devices/input devices/sound
CXXSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.cpp))
CSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.c))

CXX = g++
C = gcc
DIST_PATH = $(OBJ_BASE)
OBJ_PATH = $(OBJ_BASE)
CXX_OBJS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(CXXSRCS))
CXX_DEPS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.d,$(CXXSRCS))
C_OBJS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o,$(CSRCS))
C_DEPS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.d,$(CSRCS))
BIN_TARGET = $(DIST_PATH)/$(PROJECT_NAME)

CXXFLAGS = -D_LINUX -O3 -Wall -c -fmessage-length=0
CFLAGS = -O3 -Wall -c -fmessage-length=0
LFLAGS = -s

ifdef BENCHMARK
CXXFLAGS := $(CXXFLAGS) -DUSE_BENCHMARK
else
ifdef SDL
CXXFLAGS := $(CXXFLAGS) -DUSE_SDL -DSDL_KEYS_COMMON `sdl-config --cflags`
LFLAGS := $(LFLAGS) `sdl-config --libs`
else
CXXFLAGS := $(CXXFLAGS) `wx-config --cxxflags`
LFLAGS := $(LFLAGS) -lGL -lopenal `wx-config --libs core,base,gl,adv`
endif
endif

all: build

mkdirs:
	mkdir -p $(DIST_PATH)
	mkdir -p $(foreach dir, $(SRCFOLDERS), $(OBJ_PATH)/$(dir))
	
$(CXX_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(C_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.c
	$(C) $(CFLAGS) -o $@ $<

$(BIN_TARGET): $(CXX_OBJS) $(C_OBJS)
	$(CXX) $^ $(LFLAGS) -o$(BIN_TARGET)

build: mkdirs $(BIN_TARGET)

clean:
	rm -rf $(CXX_OBJS)
	rm -rf $(CXX_DEPS)
	rm -rf $(C_OBJS)
	rm -rf $(C_DEPS)
	rm -rf $(BIN_TARGET)

.PHONY: mkdirs clean build all
