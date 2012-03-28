
BBTOOLS=${QNX_HOST}/usr/bin/blackberry-
BBPKG=${BBTOOLS}nativepackager


BAR_MEDIA=sdl-controls.xml
BAR_MEDIA+= media/zxspectrum48.png

BAR_NAME=unreal_speccy_portable.bar
BAR_XML=bar-descriptor.xml

BIN_TARGET=../Release/unreal_speccy_portable

pkg: $(BIN_TARGET)
	$(BBPKG) -devMode -package $(BAR_NAME) $(BAR_XML)  $(BIN_TARGET) 
