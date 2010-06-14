EPOCROOT = /home/djdron/symbian-sdks/s60_30
CC_INSTALL_PATH = /home/djdron/csl-gcc
ACTIVE_PERL = perl
PATH := $(CC_INSTALL_PATH)/bin:$(PATH)

# Defaults
SRC_PATH = ../..
RSS_PATH = .
SIS_PATH = sis
OBJ_BASE = obj
DIST_BASE = dist
VENDORID = 0
PASS = 

include config.mk
include defines.mk

check:
	@echo 'EPOCROOT is' $(EPOCROOT)
	@echo 'ACTIVE_PERL is ' $(ACTIVE_PERL)
	@echo 'CC_INSTALL_PATH is ' $(CC_INSTALL_PATH)
	@echo 'PATH is ' $(PATH)
	@echo 'CXXSRCS is ' $(CXXSRCS)

preGCCE:
  CC = arm-none-symbianelf-gcc
  CXX = arm-none-symbianelf-g++
  LD = arm-none-symbianelf-ld
  RAU = $(EPOCROOT)/epoc32/release/armv5/urel
  RAL = $(EPOCROOT)/epoc32/release/armv5/lib
  DIST_PATH = $(DIST_BASE)/gcce
  OBJ_PATH = $(OBJ_BASE)/gcce
  TARGET_MACRO = $(GCCE_MACRO) $(GCCE_WARNING_FLAGS)
  CFLAGS += -fexceptions -march=armv5t -mapcs -pipe -nostdinc -c -msoft-float -O3 \
	-MD $(REL_MACRO) -include $(EPOCROOT)/epoc32/include/gcce/gcce.h
  LD_EXE_FLAGS = -L$(CC_INSTALL_PATH)/arm-none-symbianelf/lib \
	-L$(CC_INSTALL_PATH)/lib/gcc/arm-none-symbianelf/4.4.1 --target1-abs \
	--no-undefined -nostdlib -shared -Ttext 0x8000 -Tdata 0x400000 \
	--default-symver -soname $(PROJECT_NAME){000a0000}[$(UID3)].exe \
	--entry _E32Startup -u _E32Startup \
	$(RAU)/eexe.lib \
	-o $(OBJ_PATH)/$(PROJECT_NAME).exe -Map $(BIN_TARGET).map $^ \
	$(patsubst %.lib,$(RAL)/%.dso,$(SYSLIBRARY)) \
	$(RAU)/usrt2_2.lib $(RAL)/dfpaeabi.dso $(RAL)/dfprvct2_2.dso \
	$(RAL)/drtaeabi.dso $(RAL)/scppnwdl.dso $(RAL)/drtrvct2_2.dso \
	-lsupc++ -lgcc

preALL:
  CFLAGS += $(UNICODE_MACRO) $(TARGET_MACRO) $(SYMBIAN_MACRO) $(S60_MACRO) \
	$(EXE_MACRO) $(addprefix -D ,$(PRJ_MACRO)) \
	-D__PRODUCT_INCLUDE__=$(OS_HRH) -I $(VARIANT) \
	$(addprefix -I ,$(USERINCLUDE)) $(addprefix -I ,$(SYSINCLUDE)) \
	-I $(CC_INSTALL_PATH)/lib/gcc/arm-none-symbianelf/4.4.1/include
  CXXFLAGS = $(CFLAGS) -Wno-ctor-dtor-privacy -x c++ $(CPP_MACRO)
  CXX_OBJS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(CXXSRCS))
  CXX_DEPS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.d,$(CXXSRCS))
  C_OBJS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o,$(CSRCS))
  C_DEPS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.d,$(CSRCS))
  BIN_TARGET = $(DIST_PATH)/$(PROJECT_NAME).$(TARGET_TYPE)

mkdirs:
	mkdir -p $(DIST_PATH)
	mkdir -p $(SIS_PATH)
	mkdir -p $(foreach DIR, $(SRCFOLDERS), $(OBJ_PATH)/$(DIR))

icon:
	bmconv $(ICON_TARGET) $(IMG_ARGS)$(ICON_SRC).bmp $(ICON_SRC)_mask.bmp
#	mifconv $@ $(subst /,//,$(IMG_ARGS)) $^

$(RSS_TARGETS): $(DIST_PATH)/%.rsc : $(RSS_PATH)/%.rss
	$(ACTIVE_PERL) -S $(EPOCROOT)/epoc32/tools/epocrc.pl -m045,046,047 \
	$(addprefix -I ,$(RSS_PATH)) $(addprefix -I ,$(USERINCLUDE)) -I- \
	$(addprefix -I ,$(SYSINCLUDE)) $(addprefix -D ,$(LANG_MACRO)) \
	-I $(VARIANT) -t$(OBJ_PATH) -u $< -o$@ \
	-h$(EPOCROOT)/epoc32/include/$(notdir $(basename $@)).rsg
	
$(CXX_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(C_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_TARGET): $(CXX_OBJS) $(C_OBJS) 
	$(LD) $(LD_EXE_FLAGS)
	elf2e32 --sid=0x$(SECUREID) --stack=$(EPOCSTACKSIZE) --heap=$(EPOCHEAPSIZE) \
	--vid=0x$(VENDORID) --uid1=0x1000007a --uid2=0x$(UID2) --uid3=0x$(UID3)\
	--capability=$(CAPABILITY) --fpu=softvfp --targettype=$(TARGET_TYPE) \
	--output=$@ --elfinput=$(OBJ_PATH)/$(PROJECT_NAME).$(TARGET_TYPE) \
	--linkas=$(PROJECT_NAME){000a0000}[$(UID3)].$(TARGET_TYPE) \
	--libpath=$(RAL)

pack: build pack.pkg
	makesis -c -v pack.pkg $(SIS_PATH)/$(PROJECT_NAME).sis
#	$(EPOCROOT)/tools/signsis.exe -v $(PKG_PATH).sis $(PKG_PATH).sisx \
#	$(CERT) $(KEY) $(PASS)

clean: preGCCE preALL
	rm -rf $(IMG_TARGETS)
	rm -rf $(RSS_TARGETS)
	rm -rf $(CXX_OBJS)
	rm -rf $(CXX_DEPS)
	rm -rf $(C_OBJS)
	rm -rf $(C_DEPS)
	rm -rf $(BIN_TARGET)
	rm -rf $(BIN_TARGET).map

build: preGCCE preALL mkdirs icon $(RSS_TARGETS) $(BIN_TARGET)

.PHONY: build clean pack check icon mkdirs preALL preGCCE
