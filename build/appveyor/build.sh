#!/bin/bash

git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
cd ../build/cmake

mkdir build_emscripten
cd build_emscripten
emcmake cmake .. -DUSE_SDL=0 -DUSE_SDL2=1 -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j 4
zip unreal-speccy-portable_0.0.90_emscripten_sdl2_gles2.zip unreal_speccy_portable.*

cd ../../docker
./build_macos.sh
