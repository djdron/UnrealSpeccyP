SRC_PATH = ../..
OBJ_BASE = Release

PROJECT_NAME = unreal_speccy_portable
SRCFOLDERS = . z80 tools tools/zlib tools/libpng tools/tinyxml snapshot ui platform platform/custom_ui platform/linux platform/sdl platform/playbook devices devices/fdd devices/input devices/sound 
CXXSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.cpp))
CSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.c))


# THe following is taken from bbndk-env.sh Normally we would have this already
# setup you would typically run it like this ... 
# . /opt/bbndk-2.0/bbndk-env.sh


# Setup QNX toolchain basics ... 
RANLIB=${QNX_HOST}/usr/bin/ntoarmv7-ranlib
CXX=${QNX_HOST}/usr/bin/qcc -V4.4.2,gcc_ntoarmv7le_cpp
C=${QNX_HOST}/usr/bin/qcc -V4.4.2,gcc_ntoarmv7le
LD=${QNX_HOST}/usr/bin/ntoarmv7-ld
CPPFLAGS = -D__PLAYBOOK__ -D__QNXNTO__
CFLAGS = -g
LDFLAGS = -L${QNX_TARGET}/armle-v7/lib -L${PREFIX}/lib -lscreen -lasound -lpps -lm -lpng14 -lbps -lEGL -lGLESv2



DIST_PATH = $(OBJ_BASE)
OBJ_PATH = $(OBJ_BASE)
CXX_OBJS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(CXXSRCS))
CXX_DEPS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.d,$(CXXSRCS))
C_OBJS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o,$(CSRCS))
C_DEPS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.d,$(CSRCS))
BIN_TARGET = $(DIST_PATH)/$(PROJECT_NAME)


PATH_TO_SDL_CODE=$(HOME)/git/SDL
PATH_TO_TCO_CODE=$(HOME)/git/TouchControlOverlay

PBFLAGS = $(CPPFLAGS) -DUSE_SDL -DSDL_KEYS_COMMON -DSDL_AUDIO_SAMPLES=2048 -DSDL_DEFAULT_FOLDER=\"/accounts/1000/shared/misc/roms/zx\"
PBINC = -I$(PATH_TO_SDL_CODE)/include
PBINC += -I$(PATH_TO_TCO_CODE)/inc
PBLIB_DIRS = -L$(PATH_TO_SDL_CODE)/Device-Release -L$(PATH_TO_TCO_CODE)/Device-Release
PBLIBS = -lSDL12 -lTouchControlOverlay -lxml2 -lpng -lbps


# Final flag options ...
CXXFLAGS = $(PBINC) $(PBFLAGS) -D_LINUX -Wall -c -fmessage-length=0
CFLAGS = -O3 -Wall -c -fmessage-length=0
LFLAGS = -s $(LDFLAGS) $(PBLIB_DIRS) $(PBLIBS)


all: build

mkdirs:
	@echo 'CPPFLAGS=' $(CPPFLAGS)
	@echo 'PBFLAGS=' $(PBFLAGS)
	@echo 'CXXFLAGS=' $(CXXFLAGS)
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
