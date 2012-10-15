SRC_PATH = ../..
OBJ_BASE = Release

PROJECT_NAME = unreal_speccy_portable
SRCFOLDERS = . z80 tools tools/zlib tools/tinyxml snapshot ui platform platform/custom_ui platform/psp devices devices/fdd devices/input devices/sound
CXXSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.cpp))
CSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.c))

CXX = psp-g++
C = psp-gcc

DIST_PATH = $(OBJ_BASE)
OBJ_PATH = $(OBJ_BASE)
CXX_OBJS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(CXXSRCS))
CXX_DEPS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.d,$(CXXSRCS))
C_OBJS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o,$(CSRCS))
C_DEPS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.d,$(CSRCS))
BIN_TARGET = $(DIST_PATH)/$(PROJECT_NAME)

STD_OPTS  = -G0 -Wall -c -fmessage-length=0 -D_PSP_FW_VERSION=150 -I`psp-config --pspsdk-path`/include
CXXFLAGS  = $(STD_OPTS) -fno-exceptions -fno-rtti -D_POSIX -D_PSP -DUSE_CONFIG -DUSE_ZIP -DUSE_UI -DUI_REAL_ALPHA
CFLAGS    = $(STD_OPTS)
LFLAGS    = -L`psp-config --pspsdk-path`/lib -lstdc++ -lc -lpspgu -lpspge -lpspaudiolib -lpspaudio -lpspdisplay -lpspctrl -lpspsdk -lpspuser -lpspkernel 

FIX_TARGET = $(BIN_TARGET).fix
ifdef DEBUG
LFLAGS    := -specs=`psp-config --pspsdk-path`/lib/prxspecs -Wl,-q,-T`psp-config --pspsdk-path`/lib/linkfile.prx $(LFLAGS)
PRX_OBJ   = `psp-config --pspsdk-path`/lib/prxexports.o
PRX_TARGET = $(BIN_TARGET).prx
FIN_TARGET = $(PRX_TARGET)
CXXFLAGS  := -g3 -O3 $(CXXFLAGS)
CFLAGS    := -g3 -O3 $(CFLAGS)
else
CXXFLAGS  := -g0 -O3 $(CXXFLAGS) 
CFLAGS    := -g0 -O3 $(CFLAGS) 
LFLAGS    := -s $(LFLAGS)
FIN_TARGET = $(FIX_TARGET)
endif

ifdef PROFILE
CXXFLAGS  := $(CXXFLAGS) -DUSE_PROFILER
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
	$(C) $^ $(PRX_OBJ) $(LFLAGS) -o$(BIN_TARGET)
	
$(FIX_TARGET): $(BIN_TARGET)
	psp-fixup-imports -o $(FIX_TARGET) $(BIN_TARGET)
	
ifdef DEBUG
$(PRX_TARGET): $(FIX_TARGET)
	psp-prxgen $(FIX_TARGET) $(PRX_TARGET)

$(DIST_PATH)/PARAM.SFO: $(PRX_TARGET)
else
$(DIST_PATH)/PARAM.SFO: $(FIX_TARGET)
endif
	mksfo 'Unreal Speccy Portable' $(DIST_PATH)/PARAM.SFO

$(DIST_PATH)/EBOOT.PBP: $(DIST_PATH)/PARAM.SFO
	pack-pbp $(DIST_PATH)/EBOOT.PBP $(DIST_PATH)/PARAM.SFO icon.png NULL NULL NULL NULL $(FIN_TARGET) NULL

build: mkdirs $(DIST_PATH)/EBOOT.PBP

clean:
	rm -rf $(CXX_OBJS)
	rm -rf $(CXX_DEPS)
	rm -rf $(C_OBJS)
	rm -rf $(C_DEPS)
	rm -rf $(BIN_TARGET)
ifdef DEBUG
	rm -rf $(PRX_TARGET)
endif
	rm -rf $(FIX_TARGET)
	rm -rf $(FIN_TARGET)
	rm -rf $(DIST_PATH)/PARAM.SFO
	rm -rf $(DIST_PATH)/EBOOT.PBP

.PHONY: mkdirs clean build all
