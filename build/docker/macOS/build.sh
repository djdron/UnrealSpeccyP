#!/bin/bash

tar -xJf ./angle-chromium89.tar.xz
git clone https://bitbucket.org/djdron/unrealspeccyp.git usp
cd usp/build/cmake
mkdir build_macos
cd build_macos
export OSXCROSS_HOST=x86_64-apple-darwin19
export GLES2_PATH=/build/angle-chromium89
export GLES2_INCLUDE_PATH=$GLES2_PATH/include
export GLES2_LIB_PATH=$GLES2_PATH/lib/macOS/x64
osxcross-conf
cmake .. -G Ninja -DUSE_SDL2=1 -DUSE_SDL=0 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/osxcross/target/toolchain.cmake -DOPENGLES2_INCLUDE_DIR=$GLES2_INCLUDE_PATH -DOPENGLES2_gl_LIBRARY=$GLES2_LIB_PATH/libGLESv2.dylib
cmake --build . -- -j 4
cd UnrealSpeccyPortable.app/Contents
mkdir lib
cd lib
cp $GLES2_LIB_PATH/* .
cp /osxcross/target/macports/pkgs/opt/local/lib/libSDL2-2.0.0.dylib .
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
