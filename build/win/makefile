SRC_PATH = ../..
OBJ_BASE = Release

PROJECT_NAME = unreal_speccy_portable
SRCFOLDERS = . z80 tools tools/zlib tools/tinyxml tools/libpng snapshot ui platform platform/custom_ui platform/gl platform/oal platform/wxwidgets platform/linux platform/sdl platform/benchmark devices devices/fdd devices/input devices/sound
CXXSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.cpp))
CSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.c))

CXX = g++
C = gcc
RES = windres
DIST_PATH = $(OBJ_BASE)
OBJ_PATH = $(OBJ_BASE)
CXX_OBJS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(CXXSRCS))
CXX_DEPS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.d,$(CXXSRCS))
RES_OBJ = $(OBJ_PATH)/$(PROJECT_NAME)_res.o
C_OBJS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o,$(CSRCS))
C_DEPS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.d,$(CSRCS))
BIN_TARGET = $(DIST_PATH)/$(PROJECT_NAME)

CXXFLAGS = -D_WINDOWS -O3 -Wall -c -fmessage-length=0
CFLAGS = -O3 -Wall -c -fmessage-length=0
LFLAGS = -s

all: build

ifdef BENCHMARK
CXXFLAGS := $(CXXFLAGS) -DUSE_BENCHMARK
else
ifdef SDL
CXXFLAGS := $(CXXFLAGS) -DUSE_SDL -DSDL_KEYS_COMMON `sdl-config --cflags`
LFLAGS := $(LFLAGS) `sdl-config --libs`
else
CXXFLAGS := $(CXXFLAGS) -I../../../lib/wxWidgets-2.8.12/include -I../../../lib/wxWidgets-2.8.12/lib/gcc_lib/mswu -I../../../lib/GL/include -I../../../lib/oal/include
LFLAGS := $(LFLAGS) --static -lwxmsw28u_adv -lwxmsw28u_core -lwxmsw28u_gl -lwxbase28u -lopenal32 -lgdi32 -lopengl32 -lcomdlg32 -lole32 -luuid -lcomctl32 -loleaut32 -lwinspool -L../../../lib/wxWidgets-2.8.12/lib/gcc_lib -L../../../lib/oal/libs/Win32
RES_FLAGS = -I../../../lib/wxWidgets-2.8.12/include -DWX_RES
endif
endif

mkdirs:
	-@mkdir $(foreach dir, $(SRCFOLDERS), "$(OBJ_PATH)/$(dir)")
	
$(CXX_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(C_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.c
	$(C) $(CFLAGS) -o $@ $<

$(BIN_TARGET): $(CXX_OBJS) $(C_OBJS) $(RES_OBJ)
	$(CXX) $(RES_OBJ) $^ $(LFLAGS) -o$(BIN_TARGET)

$(RES_OBJ):
	$(RES) $(RES_FLAGS) $(PROJECT_NAME).rc $(RES_OBJ)

build: mkdirs $(BIN_TARGET)

clean:
	-@rmdir /S /Q "$(DIST_PATH)"

.PHONY: mkdirs clean build all
