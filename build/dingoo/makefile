MIPSTOOLS = /home/djdron/mipsel-tools/mipsel-4.1.2-nopic
DINGOO_SDK = /home/djdron/dingoo-sdk
PATH := $(MIPSTOOLS)/bin:$(PATH)

CXX       = mipsel-linux-g++
C         = mipsel-linux-gcc
LD        = mipsel-linux-ld
ELF2APP   = $(DINGOO_SDK)/tools/elf2app/elf2app 
BIN2H     = $(DINGOO_SDK)/tools/bin2h/bin2h 

PROJECT_NAME = unreal_speccy_portable
LD_SCRIPT = $(DINGOO_SDK)/lib/dingoo.xn

SRC_PATH = ../..
OBJ_BASE = Release
DIST_PATH = $(OBJ_BASE)
OBJ_PATH = $(OBJ_BASE)
RES_PATH = res

SRCFOLDERS = . z80 tools tools/zlib tools/tinyxml snapshot ui platform platform/custom_ui platform/dingoo devices devices/fdd devices/input devices/sound
LIB_PATH  = $(DINGOO_SDK)/lib
LIBS      = -lc -ljz4740 -lgcc -lstdc++

INCLUDE   = -I$(DINGOO_SDK)/include -I$(MIPSTOOLS)/mipsel-linux/include -I. -I$
STD_OPTS  = -G0 -O3 $(INCLUDE) -Wall -finline-functions -fomit-frame-pointer -msoft-float -fno-builtin -fno-exceptions -mips32 -mno-abicalls -fno-pic -c -DMPU_JZ4740 -DNDEBUG -D_DINGOO -DUSE_UI -DUSE_ZIP -DUSE_CONFIG -DUSE_EMBEDDED_RESOURCES
CPP_OPTS  = $(STD_OPTS) -fno-rtti -fno-threadsafe-statics
C_OPTS    = $(STD_OPTS)
LD_OPTS   = -nodefaultlibs --script $(LD_SCRIPT) -L$(LIB_PATH) $(LIBS)

CXXSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.cpp))
CSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.c))

CXX_OBJS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(CXXSRCS))
CXX_DEPS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.d,$(CXXSRCS))
C_OBJS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o,$(CSRCS))
C_DEPS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.d,$(CSRCS))

BIN_TARGET = $(DIST_PATH)/$(PROJECT_NAME)

ifdef SIM
STD_OPTS := $(STD_OPTS) -DSIM_VERSION
build: mkdirs res $(BIN_TARGET).sim
else
build: mkdirs res $(BIN_TARGET).app
endif

mkdirs:
	mkdir -p $(DIST_PATH)
	mkdir -p $(foreach dir, $(SRCFOLDERS), $(OBJ_PATH)/$(dir))

res:
	mkdir -p $(RES_PATH)/font
	mkdir -p $(RES_PATH)/rom
	$(BIN2H) $(SRC_PATH)/res/font/spxtrm4f.fnt	$(RES_PATH)/font/spxtrm4f
	$(BIN2H) $(SRC_PATH)/res/rom/sos128.rom		$(RES_PATH)/rom/sos128
	$(BIN2H) $(SRC_PATH)/res/rom/sos48.rom		$(RES_PATH)/rom/sos48
	$(BIN2H) $(SRC_PATH)/res/rom/service.rom	$(RES_PATH)/rom/service
	$(BIN2H) $(SRC_PATH)/res/rom/dos513f.rom	$(RES_PATH)/rom/dos513f

$(CXX_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.cpp
	$(CXX) $(CPP_OPTS) -o $@ $<

$(C_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.c
	$(C) $(C_OPTS) -o $@ $<

$(BIN_TARGET).elf: $(CXX_OBJS) $(C_OBJS)
	$(LD) $(CXX_OBJS) $(C_OBJS) $(LD_OPTS) -Map=$(BIN_TARGET).map -o $(BIN_TARGET).elf

$(BIN_TARGET).app: $(BIN_TARGET).elf
	$(ELF2APP) $(BIN_TARGET)

$(BIN_TARGET).sim: $(BIN_TARGET).app
	mv $(BIN_TARGET).app $(BIN_TARGET).sim

clean:
	rm -rf $(RES_PATH)
	rm -rf $(OBJ_BASE)
	rm -rf $(DIST_PATH)

.PHONY: mkdirs res clean build
