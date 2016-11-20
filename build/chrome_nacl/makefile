SHELL := /bin/bash

PATH_BIN := $(PATH):$(NACL_PEPPER)/toolchain/win_pnacl/bin/
CXX = pnacl-clang++
C = pnacl-clang
FINALIZE = pnacl-finalize

LIB_PATH = $(NACL_PEPPER)/lib/pnacl/Release

export PATH = $(PATH_BIN)

SRC_PATH = ../..
OBJ_BASE = Release

PROJECT_NAME = unreal_speccy_portable
SRCFOLDERS = . z80 tools 3rdparty/zlib 3rdparty/minizip snapshot platform platform/chrome_nacl platform/gles2 devices devices/fdd devices/input devices/sound 
CXXSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.cpp))
CSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.c))

DIST_PATH = $(OBJ_BASE)
OBJ_PATH = $(OBJ_BASE)
CXX_OBJS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(CXXSRCS))
CXX_DEPS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.d,$(CXXSRCS))
C_OBJS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o,$(CSRCS))
C_DEPS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.d,$(CSRCS))
BIN_TARGET = $(PROJECT_NAME)_not_final.pexe
BIN_TARGET_FINAL = $(PROJECT_NAME).pexe

CXXCFLAGS = -O2 -Wall -c -fmessage-length=0 -I$(NACL_PEPPER)/include -I$(SRC_PATH)/3rdparty/zlib

CXXFLAGS += $(CXXCFLAGS) -D_CHROME_NACL -DUSE_GLES2 -DUSE_ZIP -DUSE_EXTERN_RESOURCES -I$(SRC_PATH)/3rdparty/minizip
CFLAGS += $(CXXCFLAGS)
LFLAGS += -O2 -L$(LIB_PATH) -lppapi -lppapi_cpp -lppapi_gles2

OBJFOLDERS = $(addprefix $(OBJ_PATH)/,$(SRCFOLDERS))

ifeq ($(shell echo "check_quotes"),"check_quotes")
	mkdir = mkdir $(subst /,\,$(1))
else
	mkdir = mkdir -p $(1)
endif


all: build

$(OBJFOLDERS):
	$(call mkdir,$@)
	
$(CXX_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(C_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.c
	$(C) $(CFLAGS) -o $@ $<

$(BIN_TARGET): $(CXX_OBJS) $(C_OBJS)
	$(CXX) $^ $(LFLAGS) -o$(BIN_TARGET)

$(BIN_TARGET_FINAL): $(BIN_TARGET)
	$(FINALIZE) $(BIN_TARGET) -o $(BIN_TARGET_FINAL) --compress

build: $(OBJFOLDERS) $(BIN_TARGET_FINAL)

clean:
	rm -rf $(CXX_OBJS)
	rm -rf $(CXX_DEPS)
	rm -rf $(C_OBJS)
	rm -rf $(C_DEPS)
	rm -rf $(BIN_TARGET)

.PHONY: mkdirs clean build all
