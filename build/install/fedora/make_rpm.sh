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
# Version from git only:
#   git describe --tags --always --dirty="-dev"
#
# Tag commit  → Version=<tag>  Release=1%{?dist}          (final release)
# Later commits → Version=<tag>  Release=1.<n>.g…%{?dist}  (newer than release 1)
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
SPEC="$SCRIPT_DIR/unreal-speccy-portable.spec"

NAME="unreal-speccy-portable"
OUTPUT_DIR="${OUTPUT_DIR:-}"

cd "$REPO_ROOT"

# --- version from git --------------------------------------------------------
# v0.0.86.28              → Version 0.0.86.28   Release 1%{?dist}
# v0.0.86.28-5-g1a2b3c4   → Version 0.0.86.28   Release 1.5.g1a2b3c4%{?dist}
# v0.0.86.28-dev          → Version 0.0.86.28   Release 1.dev%{?dist}
# abcdef1 (no tags)       → Version abcdef1     Release 0%{?dist}
DESC=$(git -C "$REPO_ROOT" describe --tags --always --dirty="-dev" 2>/dev/null || echo "unknown")
DESC="${DESC#v}"

if [[ "$DESC" =~ ^([0-9]+(\.[0-9]+)*)-(.+)$ ]]; then
    # commits (and optional dirty) after the last tag — still same Version, Release > 1
    VERSION="${BASH_REMATCH[1]}"
    SUFFIX="${BASH_REMATCH[3]}"
    SUFFIX="${SUFFIX//-/.}"          # 5-g1a2b3c4-dev → 5.g1a2b3c4.dev
    RELEASE="1.${SUFFIX}%{?dist}"
elif [[ "$DESC" =~ ^[0-9]+(\.[0-9]+)*$ ]]; then
    # exact tag — final release of this Version
    VERSION="$DESC"
    RELEASE="1%{?dist}"
else
    # bare hash / unknown — no numeric tag yet
    VERSION="${DESC//-/.}"
    RELEASE="0%{?dist}"
fi

echo "==> git describe: $DESC"
echo "==> RPM Version=$VERSION  Release=$RELEASE"

TARBALL="${NAME}-${VERSION}.tar.gz"

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

sed -e "s/^Version:.*/Version:        ${VERSION}/" \
    -e "s/^Release:.*/Release:        ${RELEASE}/" \
    -e "s/^Source0:.*/Source0:        %{name}-%{version}.tar.gz/" \
    "$SPEC" > "$RPMBUILD_DIR/SPECS/unreal-speccy-portable.spec"

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
