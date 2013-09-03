ifdef ARM

PATH_BIN := $(PATH):$(NACL_PEPPER)/toolchain/linux_arm_newlib/bin/
CXX = arm-nacl-g++
C = arm-nacl-gcc
PLATFORM = _arm

else #ARM

PATH_BIN := $(PATH):$(NACL_PEPPER)/toolchain/linux_x86_newlib/bin/
CXX = i686-nacl-g++
C = i686-nacl-gcc
ifdef X86
PLATFORM = _x86_32
CXXFLAGS = -m32
CFLAGS = -m32
LFLAGS = -m32
else #X86 
PLATFORM = _x86_64
CXXFLAGS = -m64
CFLAGS = -m64
LFLAGS = -m64
endif #X86

endif #ARM

LIB_PATH = $(NACL_PEPPER)/lib/newlib$(PLATFORM)/Release

export PATH = $(PATH_BIN)

SRC_PATH = ../..
OBJ_BASE = Release$(PLATFORM)

PROJECT_NAME = unreal_speccy_portable$(PLATFORM)
SRCFOLDERS = . z80 tools 3rdparty/zlib 3rdparty/minizip snapshot platform platform/chrome_nacl platform/gles2 devices devices/fdd devices/input devices/sound 
CXXSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.cpp))
CSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.c))

DIST_PATH = $(OBJ_BASE)
OBJ_PATH = $(OBJ_BASE)
CXX_OBJS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(CXXSRCS))
CXX_DEPS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.d,$(CXXSRCS))
C_OBJS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o,$(CSRCS))
C_DEPS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.d,$(CSRCS))
BIN_TARGET = $(PROJECT_NAME).nexe

CXXCFLAGS = -O3 -Wall -c -fmessage-length=0 -I$(NACL_PEPPER)/include -I$(SRC_PATH)/3rdparty/zlib

CXXFLAGS += $(CXXCFLAGS) -D_CHROME_NACL -DUSE_GLES2 -DUSE_ZIP -DUSE_EXTERN_RESOURCES -I$(SRC_PATH)/3rdparty/minizip
CFLAGS += $(CXXCFLAGS)
LFLAGS += -s -L$(LIB_PATH) -lppapi -lppapi_cpp -lppapi_gles2

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
