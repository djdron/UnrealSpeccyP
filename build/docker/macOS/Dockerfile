FROM nemirtingas/osxcross_vcpkg:SDK10.15
RUN apt install ninja-build
ENV MACOSX_DEPLOYMENT_TARGET=10.9
ENV OSXCROSS_MACPORTS_MIRROR=http://packages.macports.org
RUN osxcross-macports install libsdl2
WORKDIR /build
COPY SDL2.patch .
RUN patch /osxcross/target/macports/pkgs/opt/local/lib/cmake/SDL2/sdl2-config.cmake SDL2.patch
COPY build.sh .
ADD https://github.com/djdron/UnrealSpeccyP/releases/download/angle-chromium84/angle-chromium89.tar.xz .
ENTRYPOINT ./build.sh
