SRC_PATH = ../..
OBJ_BASE = Release

PROJECT_NAME = UnrealSpeccyPortable
SRCFOLDERS = . z80 tools 3rdparty/libpng 3rdparty/zlib 3rdparty/tinyxml2 3rdparty/minizip snapshot platform platform/ios platform/gles2 platform/oal platform/touch_ui platform/custom_ui platform/linux ui devices devices/fdd devices/input devices/sound
CXXSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.cpp))
CSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.c))
MMSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.mm))
MSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.m))

CXX = ios-clang++
C = ios-clang
DIST_PATH = $(OBJ_BASE)
OBJ_PATH = $(OBJ_BASE)
CXX_OBJS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(CXXSRCS))
C_OBJS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o,$(CSRCS))
MM_OBJS = $(patsubst $(SRC_PATH)/%.mm,$(OBJ_PATH)/%.o,$(MMSRCS))
M_OBJS = $(patsubst $(SRC_PATH)/%.m,$(OBJ_PATH)/%.o,$(MSRCS))
BIN_TARGET = $(DIST_PATH)/$(PROJECT_NAME)

BASEFLAGS = -O3 -Wall -c -fmessage-length=0 -objc-arc -fblocks

CXXFLAGS += $(BASEFLAGS) -D_POSIX -DUSE_PNG -DUSE_CONFIG -DUSE_ZIP -DUSE_OAL -D_IOS -DUSE_GLES2 -DUSE_GLES2_SPRITE -DUSE_TUI -DUSE_UI -DUI_REAL_ALPHA -I$(SRC_PATH)/3rdparty/libpng -I$(SRC_PATH)/3rdparty/zlib -I$(SRC_PATH)/3rdparty/minizip -I$(SRC_PATH)/3rdparty/tinyxml2
CFLAGS += $(BASEFLAGS) -DUSE_FILE32API
LFLAGS = -framework Foundation -framework CoreGraphics -framework UIKit -framework GLKit -framework OpenGLES -framework OpenAL

IPHONE_IP:=192.168.0.101
APPFOLDER = $(PROJECT_NAME).app

INFOPLIST:=$(wildcard *Info.plist)
RESOURCES+=$(wildcard ./Images/*)
RESOURCES+=$(wildcard ./Resources/*)
RESOURCES+=$(wildcard ./Localizations/*)

all: build

mkdirs:
	mkdir -p $(DIST_PATH)
	mkdir -p $(foreach dir, $(SRCFOLDERS), $(OBJ_PATH)/$(dir))
	
$(CXX_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(C_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.c
	$(C) $(CFLAGS) -o $@ $<

$(MM_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.mm
	$(CXX) $(CXXFLAGS) -o $@ $<

$(M_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.m
	$(C) $(CFLAGS) -o $@ $<

$(BIN_TARGET): $(CXX_OBJS) $(C_OBJS) $(MM_OBJS) $(M_OBJS)
	$(CXX) $^ $(LFLAGS) -o$(BIN_TARGET)

build: mkdirs $(BIN_TARGET)

clean:
	rm -rf $(CXX_OBJS)
	rm -rf $(C_OBJS)
	rm -rf $(MM_OBJS)
	rm -rf $(M_OBJS)
	rm -rf $(BIN_TARGET)
	
dist: build
	mkdir -p $(APPFOLDER)
	mkdir -p $(APPFOLDER)/res
ifneq ($(RESOURCES),)
	cp -r $(RESOURCES) $(APPFOLDER)
endif
	cp -r ../../res/rom $(APPFOLDER)/res
	cp -r ../../res/font $(APPFOLDER)/res
	rm $(APPFOLDER)/res/rom/test.rom
	cp $(INFOPLIST) $(APPFOLDER)/Info.plist
	cp $(BIN_TARGET) $(APPFOLDER)
	find $(APPFOLDER) -name \*.png|xargs ios-pngcrush -c
	find $(APPFOLDER) -name \*.plist|xargs ios-plutil -c
	find $(APPFOLDER) -name \*.strings|xargs ios-plutil -c

install: dist
ifeq ($(IPHONE_IP),)
	echo "Please set IPHONE_IP"
else
	ssh root@$(IPHONE_IP) 'rm -fr /Applications/$(APPFOLDER)'
	scp -r $(APPFOLDER) root@$(IPHONE_IP):/Applications/$(APPFOLDER)
	echo "Application $(APPFOLDER) installed"
	ssh mobile@$(IPHONE_IP) 'uicache'
endif

.PHONY: mkdirs clean build all dist install

