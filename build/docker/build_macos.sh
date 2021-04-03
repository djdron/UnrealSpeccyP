#!/bin/bash

cd $(dirname "$0")
docker build -t build-macos ./macOS
docker run --name build-macos build-macos
docker cp build-macos:/build/UnrealSpeccyPortable.dmg ./unreal-speccy-portable_0.0.90_x86_64_sdl2_gles2_angle_curl.dmg
