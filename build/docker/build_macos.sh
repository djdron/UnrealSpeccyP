#!/bin/bash

VERSION=$(git describe --tags --always --dirty="-dev" 2>/dev/null || echo "unknown")
VERSION="${VERSION#v}"

cd $(dirname "$0")
docker build --progress=plain -t build-macos-x64 ./macOS-x64
docker run --name build-macos-x64 build-macos-x64
docker cp build-macos-x64:/build/UnrealSpeccyPortable.dmg ./unreal-speccy-portable-${VERSION}_x86_64_sdl2_gles2_angle_curl.dmg

docker build --progress=plain -t build-macos-arm64 ./macOS-arm64
docker run --name build-macos-arm64 build-macos-arm64
docker cp build-macos-arm64:/build/UnrealSpeccyPortable.dmg ./unreal-speccy-portable-${VERSION}_arm64_sdl2_gles2_angle_curl.dmg
