SRC_PATH = ../..

ifdef X86
PLATFORM = _x86_32
else
PLATFORM = _x86_64
endif
OBJ_BASE = Release$(PLATFORM)

PROJECT_NAME = unreal_speccy_portable$(PLATFORM)
SRCFOLDERS = . z80 tools tools/zlib snapshot platform platform/chrome_nacl platform/gles2 devices devices/fdd devices/input devices/sound 
CXXSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.cpp))
CSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.c))

CXX = i686-nacl-g++
C = i686-nacl-gcc
DIST_PATH = $(OBJ_BASE)
OBJ_PATH = $(OBJ_BASE)
CXX_OBJS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(CXXSRCS))
CXX_DEPS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.d,$(CXXSRCS))
C_OBJS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o,$(CSRCS))
C_DEPS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.d,$(CSRCS))
BIN_TARGET = $(PROJECT_NAME).nexe

CXXFLAGS = -D_CHROME_NACL -DUSE_GLES2 -DUSE_ZIP -DUSE_EXTERN_RESOURCES -O3 -Wall -c -Wno-long-long -pthread -fmessage-length=0
CFLAGS = -O3 -Wall -c -Wno-long-long -pthread -fmessage-length=0
LFLAGS = -s -lppapi -lppapi_cpp -lppapi_gles2

ifdef X86
CXXFLAGS := $(CXXFLAGS) -m32
CFLAGS := $(CFLAGS) -m32
LFLAGS := $(LFLAGS) -m32
else
CXXFLAGS := $(CXXFLAGS) -m64
CFLAGS := $(CFLAGS) -m64
LFLAGS := $(LFLAGS) -m64
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
