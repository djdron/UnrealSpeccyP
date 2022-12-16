#!/bin/bash

tar -xJf ./angle-chromium108.tar.xz
tar -xJf ./dmg-template.tar.xz

#workaround ripemd160 in openssl
sed -i 's/\bopenssl rmd160\b/openssl rmd160 -provider legacy/g' $(command -v osxcross-macports)
sed -i 's/\bopenssl dgst -ripemd160 -verify\b/openssl dgst -provider default -provider legacy -ripemd160 -verify/g' $(command -v osxcross-macports)

#workaround universal binaries with arm64, not i386
sed -i 's/ARCH="i386-x86_64"/ARCH="arm64-x86_64"/g' $(command -v osxcross-macports)

export MACOSX_DEPLOYMENT_TARGET=12.0

osxcross-macports install libsdl2 -universal

git clone https://github.com/djdron/UnrealSpeccyP.git usp
cd usp/build/cmake
mkdir build_macos
cd build_macos
export OSXCROSS_HOST=x86_64-apple-darwin21.4
export GLES2_PATH=/build/angle-chromium108
export GLES2_INCLUDE_PATH=$GLES2_PATH/include
export GLES2_LIB_PATH=$GLES2_PATH/lib/macOS
eval `osxcross-conf`
cmake .. -G Ninja -DUSE_SDL2=1 -DUSE_SDL=0 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/osxcross/toolchain.cmake -DOPENGLES2_INCLUDE_DIR=$GLES2_INCLUDE_PATH -DOPENGLES2_gl_LIBRARY=$GLES2_LIB_PATH/libGLESv2.dylib
cmake --build . -- -j 4

cd UnrealSpeccyPortable.app/Contents
mkdir lib
cd lib
cp $GLES2_LIB_PATH/* .
cp /osxcross/macports/pkgs/opt/local/lib/libSDL2-2.0.0.dylib .
export OTOOL=$OSXCROSS_HOST-otool
export INTOOL=$OSXCROSS_HOST-install_name_tool
$INTOOL -id @rpath/libEGL.dylib ./libEGL.dylib
$INTOOL -id @rpath/libGLESv2.dylib ./libGLESv2.dylib
$INTOOL -id @rpath/libSDL2-2.0.0.dylib ./libSDL2-2.0.0.dylib
cd ../MacOS
$INTOOL -change ./libGLESv2.dylib @rpath/libGLESv2.dylib ./UnrealSpeccyPortable
$INTOOL -change /opt/local/lib/libSDL2-2.0.0.dylib @rpath/libSDL2-2.0.0.dylib ./UnrealSpeccyPortable
$INTOOL -add_rpath @executable_path/../lib/ ./UnrealSpeccyPortable
cd ../../..

zip /build/UnrealSpeccyPortable.app.zip ./UnrealSpeccyPortable.app -r
cd /build
unzip ./UnrealSpeccyPortable.app.zip -d ./dmg-template
export PATH=/build/libdmg-hfsplus/build/dmg/:$PATH
mkisofs -R -V UnrealSpeccyPortable -o ./UnrealSpeccyPortable.iso ./dmg-template
dmg ./UnrealSpeccyPortable.iso ./UnrealSpeccyPortable.dmg
