#!/bin/bash
#
# Build a Flatpak for Unreal Speccy Portable
#
# Packaging stays in build/install/flatpak/
# Script prepares a local "source" tree and runs flatpak-builder.
#
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
BUILD_DIR="${FLATPAK_BUILD_DIR:-$REPO_ROOT/build/flatpak-build}"
REPO_DIR="${FLATPAK_REPO_DIR:-$REPO_ROOT/build/flatpak-repo}"
APP_ID="io.github.djdron.UnrealSpeccyPortable"

cd "$SCRIPT_DIR"

if ! command -v flatpak-builder >/dev/null 2>&1; then
    echo "ERROR: flatpak-builder not found."
    echo "  Fedora:  sudo dnf install flatpak-builder"
    echo "  Debian:  sudo apt install flatpak-builder"
    echo "  Also install the SDK:"
    echo "    flatpak install flathub org.freedesktop.Sdk//24.08 org.freedesktop.Platform//24.08"
    exit 1
fi

echo "==> Preparing source tree for Flatpak..."
rm -rf "$SCRIPT_DIR/source"
mkdir -p "$SCRIPT_DIR/source"

# Copy project sources (exclude heavy / irrelevant platforms)
rsync -a \
    --exclude='.git' \
    --exclude='obj-*' \
    --exclude='build/flatpak-build' \
    --exclude='build/flatpak-repo' \
    --exclude='build/android' \
    --exclude='build/ios' \
    --exclude='build/win' \
    --exclude='build/uwp' \
    --exclude='build/psp' \
    --exclude='build/dingoo' \
    --exclude='build/dingux' \
    --exclude='build/symbian' \
    --exclude='build/chrome_nacl' \
    --exclude='*.deb' \
    --exclude='*.rpm' \
    --exclude='*.flatpak' \
    --exclude='debian' \
    "$REPO_ROOT/" "$SCRIPT_DIR/source/"

# Extra files needed at module root (desktop, mime, metainfo, icons)
cp -a "$SCRIPT_DIR/unreal-speccy-portable.desktop" "$SCRIPT_DIR/source/"
cp -a "$SCRIPT_DIR/unreal_speccy_portable.xml" "$SCRIPT_DIR/source/"
cp -a "$SCRIPT_DIR/io.github.djdron.UnrealSpeccyPortable.metainfo.xml" "$SCRIPT_DIR/source/"
if [ -d "$SCRIPT_DIR/icons" ]; then
    cp -a "$SCRIPT_DIR/icons" "$SCRIPT_DIR/source/"
fi

echo "==> Building Flatpak (this may take a while)..."
mkdir -p "$BUILD_DIR" "$REPO_DIR"

flatpak-builder --force-clean \
    --repo="$REPO_DIR" \
    "$BUILD_DIR" \
    "$SCRIPT_DIR/io.github.djdron.UnrealSpeccyPortable.yml"

echo "==> Exporting .flatpak bundle..."
BUNDLE="$REPO_ROOT/${APP_ID}.flatpak"
flatpak build-bundle "$REPO_DIR" "$BUNDLE" "$APP_ID"

echo
echo "==> Done."
echo "    Bundle: $BUNDLE"
echo
echo "    Install (user):"
echo "      flatpak install --user $BUNDLE"
echo "    Or run without installing:"
echo "      flatpak-builder --run $BUILD_DIR $SCRIPT_DIR/io.github.djdron.UnrealSpeccyPortable.yml unreal-speccy-portable"
echo
echo "    Cleanup temporary source copy:"
echo "      rm -rf $SCRIPT_DIR/source"
