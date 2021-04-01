#/bin/bash

docker build -t build-macos ./macOS
docker run --name build-macos -it build-macos
docker cp build-macos:/build/UnrealSpeccyPortable.app.zip .
