SRC_PATH = ../..
OBJ_BASE = Release

PROJECT_NAME = unreal_speccy_portable
CXXFOLDERS = . z80 tools platform platform/gl platform/oal platform/wxwidgets devices devices/fdd devices/input devices/sound 
CXXSRCS = $(foreach dir, $(CXXFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.cpp))

CXX = g++
DIST_PATH = $(OBJ_BASE)
OBJ_PATH = $(OBJ_BASE)
CXX_OBJS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(CXXSRCS))
CXX_DEPS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.d,$(CXXSRCS))
BIN_TARGET = $(DIST_PATH)/$(PROJECT_NAME)

CXXFLAGS = -D_LINUX -O3 -Wall -c -fmessage-length=0 `wx-config --cxxflags`
LDFLAGS = -s -lGL -lopenal `wx-config --libs core,base,gl,adv` $^ -o"$(BIN_TARGET)"

all: build

mkdirs:
	mkdir -p $(DIST_PATH)
	mkdir -p $(foreach dir, $(CXXFOLDERS), $(OBJ_PATH)/$(dir))
	
$(CXX_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BIN_TARGET): $(CXX_OBJS)
	$(CXX) $(LDFLAGS)

clean:
	rm -rf $(CXX_OBJS)
	rm -rf $(CXX_DEPS)
	rm -rf $(BIN_TARGET)

build: mkdirs $(BIN_TARGET)

.PHONY: mkdirs clean build all
