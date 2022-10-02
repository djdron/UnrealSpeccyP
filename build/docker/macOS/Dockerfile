FROM nemirtingas/osxcross_vcpkg:SDK10.15
RUN apt-get install -y ninja-build genisoimage zlib1g-dev
ENV MACOSX_DEPLOYMENT_TARGET=10.9
ENV OSXCROSS_MACPORTS_MIRROR=http://packages.macports.org
RUN osxcross-macports install libsdl2
WORKDIR /build
ADD https://github.com/djdron/UnrealSpeccyP/releases/download/angle-chromium84/angle-chromium89.tar.xz .
ADD https://github.com/djdron/UnrealSpeccyP/releases/download/angle-chromium84/dmg-template.tar.xz .
RUN git clone https://github.com/fanquake/libdmg-hfsplus.git && \
    cd libdmg-hfsplus && mkdir build && cd build && cmake .. && cmake --build . -- -j 4
COPY build.sh .
ENTRYPOINT ./build.sh
