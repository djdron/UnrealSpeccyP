FROM crazymax/osxcross:latest-ubuntu AS osxcross
FROM ubuntu
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y clang lld libc6-dev ninja-build genisoimage zlib1g-dev zip xz-utils bzip2 openssl curl git cmake
COPY --from=osxcross / /
ENV PATH="/osxcross/bin:$PATH"
ENV LD_LIBRARY_PATH="/osxcross/lib:$LD_LIBRARY_PATH"
ENV OSXCROSS_MACPORTS_MIRROR=http://packages.macports.org
WORKDIR /build
ADD https://github.com/djdron/UnrealSpeccyP/releases/download/angle-chromium84/angle-chromium108.tar.xz .
ADD https://github.com/djdron/UnrealSpeccyP/releases/download/angle-chromium84/angle-chromium89.tar.xz .
ADD https://github.com/djdron/UnrealSpeccyP/releases/download/angle-chromium84/dmg-template.tar.xz .
RUN git clone https://github.com/fanquake/libdmg-hfsplus.git && \
    cd libdmg-hfsplus && mkdir build && cd build && cmake .. && cmake --build . -- -j 4
COPY build.sh .
ENTRYPOINT ./build.sh
