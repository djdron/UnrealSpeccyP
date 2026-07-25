#!/bin/bash
#
# Build a source-based RPM for Unreal Speccy Portable (Fedora / RHEL / openSUSE)
#
#   ./build/install/fedora/make_rpm.sh
#
# CI:
#   RPMBUILD_DIR="${GITHUB_WORKSPACE}/rpmbuild" \
#   OUTPUT_DIR="${GITHUB_WORKSPACE}/artifacts" \
#   ./build/install/fedora/make_rpm.sh
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
SPEC="$SCRIPT_DIR/unreal-speccy-portable.spec"

NAME="unreal-speccy-portable"
VERSION="0.0.86.28"
TARBALL="${NAME}-${VERSION}.tar.gz"
OUTPUT_DIR="${OUTPUT_DIR:-}"

cd "$REPO_ROOT"

if [ ! -f "$SPEC" ]; then
    echo "ERROR: spec file not found: $SPEC"
    exit 1
fi

RPMBUILD_DIR="${RPMBUILD_DIR:-$HOME/rpmbuild}"
mkdir -p "$RPMBUILD_DIR"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

echo "==> Creating source tarball..."
TMPDIR=$(mktemp -d)
cleanup() { rm -rf "$TMPDIR"; }
trap cleanup EXIT

mkdir -p "$TMPDIR/${NAME}-${VERSION}"
rsync -a \
    --exclude='.git' \
    --exclude='obj-*' \
    --exclude='rpmbuild' \
    --exclude='artifacts' \
    --exclude='build/android' \
    --exclude='build/ios' \
    --exclude='build/win' \
    --exclude='build/uwp' \
    --exclude='build/psp' \
    --exclude='build/dingoo' \
    --exclude='build/dingux' \
    --exclude='build/symbian' \
    --exclude='build/chrome_nacl' \
    --exclude='build/flatpak-build' \
    --exclude='build/flatpak-repo' \
    --exclude='*.deb' \
    --exclude='*.rpm' \
    --exclude='*.flatpak' \
    --exclude='debian' \
    "$REPO_ROOT/" "$TMPDIR/${NAME}-${VERSION}/"

mkdir -p "$TMPDIR/${NAME}-${VERSION}/build/install/fedora"
cp -a "$SCRIPT_DIR"/* "$TMPDIR/${NAME}-${VERSION}/build/install/fedora/" 2>/dev/null || true

tar -C "$TMPDIR" -czf "$RPMBUILD_DIR/SOURCES/$TARBALL" "${NAME}-${VERSION}"
echo "    -> $RPMBUILD_DIR/SOURCES/$TARBALL"

cp "$SPEC" "$RPMBUILD_DIR/SPECS/"

echo "==> Building RPM..."
rpmbuild -ba "$RPMBUILD_DIR/SPECS/unreal-speccy-portable.spec" \
    --define "_topdir $RPMBUILD_DIR"

echo
echo "==> Done."
echo "    RPMs:"
find "$RPMBUILD_DIR/RPMS" -name '*.rpm' 2>/dev/null
find "$RPMBUILD_DIR/SRPMS" -name '*.rpm' 2>/dev/null

if [ -n "$OUTPUT_DIR" ]; then
    mkdir -p "$OUTPUT_DIR"
    find "$RPMBUILD_DIR/RPMS" "$RPMBUILD_DIR/SRPMS" -name '*.rpm' -exec cp -a {} "$OUTPUT_DIR/" \;
    echo "    -> copied to $OUTPUT_DIR/"
fi

echo
echo "    Install with:"
echo "      sudo dnf install $RPMBUILD_DIR/RPMS/*/unreal-speccy-portable-*.rpm"
