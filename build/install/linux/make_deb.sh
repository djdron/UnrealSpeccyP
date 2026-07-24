#!/bin/bash
#
# Build a proper source-based .deb package for Unreal Speccy Portable
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
