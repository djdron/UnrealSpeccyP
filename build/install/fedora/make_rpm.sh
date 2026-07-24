#!/bin/bash
#
# Build a source-based RPM package for Unreal Speccy Portable (Fedora / RHEL / openSUSE)
#
# Packaging files stay in build/install/fedora/
# This script prepares a source tarball and runs rpmbuild.
#
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
SPEC="$SCRIPT_DIR/unreal-speccy-portable.spec"

NAME="unreal-speccy-portable"
VERSION="0.0.86.28"
TARBALL="${NAME}-${VERSION}.tar.gz"

cd "$REPO_ROOT"

if [ ! -f "$SPEC" ]; then
    echo "ERROR: spec file not found: $SPEC"
    exit 1
fi

# RPM build tree
RPMBUILD_DIR="${RPMBUILD_DIR:-$HOME/rpmbuild}"
mkdir -p "$RPMBUILD_DIR"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

echo "==> Creating source tarball..."
# Create a clean tarball of the current tree (excluding build artifacts)
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

mkdir -p "$TMPDIR/${NAME}-${VERSION}"
# Copy relevant sources (exclude heavy/irrelevant dirs)
rsync -a \
    --exclude='.git' \
    --exclude='obj-*' \
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
    --exclude='debian' \
    "$REPO_ROOT/" "$TMPDIR/${NAME}-${VERSION}/"

# Ensure packaging files are present inside the tarball
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
echo
echo "    Install with:"
echo "      sudo dnf install $RPMBUILD_DIR/RPMS/*/unreal-speccy-portable-*.rpm"
