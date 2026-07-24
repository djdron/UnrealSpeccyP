#!/bin/bash
#
# Build a proper source-based .deb package for Unreal Speccy Portable
#
# Usage (from the root of UnrealSpeccyP repository):
#
#   1. Copy the packaging files into place:
#        cp -a build/install/linux/debian .
#
#   2. (Optional) Copy icons from the old binary packaging if you still have them:
#        mkdir -p debian/icons
#        # put 16x16, 22x22, ... 128x128 PNGs named unreal_speccy_portable.png
#
#   3. Install build dependencies:
#        sudo apt install build-essential debhelper cmake \
#            libsdl2-dev libcurl4-openssl-dev zlib1g-dev libpng-dev pkg-config
#
#   4. Build the package:
#        ./build/install/linux/make_deb.sh
#      or simply:
#        dpkg-buildpackage -us -uc -b
#        debuild -us -uc -b
#
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

cd "$REPO_ROOT"

if [ ! -d debian ]; then
    echo "==> Copying debian/ packaging into repository root..."
    cp -a "$SCRIPT_DIR/debian" .
fi

if [ ! -f debian/rules ]; then
    echo "ERROR: debian/rules not found. Something went wrong with packaging files."
    exit 1
fi

chmod +x debian/rules debian/postinst debian/postrm 2>/dev/null || true

echo "==> Building source-based .deb (this may take a while)..."
echo "    Working directory: $REPO_ROOT"

# Prefer debuild if available, otherwise plain dpkg-buildpackage
if command -v debuild >/dev/null 2>&1; then
    debuild -us -uc -b
else
    dpkg-buildpackage -us -uc -b
fi

echo
echo "==> Done."
echo "    The resulting .deb should be in the parent directory:"
ls -1 ../*.deb 2>/dev/null || ls -1 *.deb 2>/dev/null || echo "    (look for unreal-speccy-portable_*.deb)"
