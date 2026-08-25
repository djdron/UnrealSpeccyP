#!/bin/bash
#
# Build a source-based .deb for Unreal Speccy Portable
#
# debian/ stays in build/install/linux/debian/
# Creates a temporary symlink from the repository root (removed on exit).
#
#   ./build/install/linux/make_deb.sh
#
# CI: collect packages into a directory
#   OUTPUT_DIR="${GITHUB_WORKSPACE}/artifacts" ./build/install/linux/make_deb.sh
#
# Version from git:
#   git describe --tags --always --dirty="-dev"
#   tag 0.0.86.30           → 0.0.86.30-1
#   0.0.86.30-3-g1abebfd    → 0.0.86.30+3.g1abebfd-1
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
DEBIAN_SRC="$SCRIPT_DIR/debian"
DEBIAN_LINK="$REPO_ROOT/debian"
OUTPUT_DIR="${OUTPUT_DIR:-}"
CHANGELOG_BAK=""

cd "$REPO_ROOT"

cleanup() {
    if [ -n "$CHANGELOG_BAK" ] && [ -f "$CHANGELOG_BAK" ]; then
        mv -f "$CHANGELOG_BAK" "$DEBIAN_SRC/changelog"
    fi
    if [ -L "$DEBIAN_LINK" ]; then
        echo "==> Removing temporary symlink $DEBIAN_LINK"
        rm -f "$DEBIAN_LINK"
    fi
}
trap cleanup EXIT

# --- version from git --------------------------------------------------------
DESC=$(git -C "$REPO_ROOT" describe --tags --always --dirty="-dev" 2>/dev/null || echo "unknown")
DESC="${DESC#v}"

if [[ "$DESC" =~ ^([0-9]+(\.[0-9]+)*)-(.+)$ ]]; then
    VER="${BASH_REMATCH[1]}"
    SUFFIX="${BASH_REMATCH[3]}"
    SUFFIX="${SUFFIX//-/.}"
    UPSTREAM="${VER}+${SUFFIX}"
elif [[ "$DESC" =~ ^[0-9]+(\.[0-9]+)*$ ]]; then
    UPSTREAM="$DESC"
else
    UPSTREAM="${DESC//-/.}"
fi
DEB_VERSION="${UPSTREAM}-1"

echo "==> git describe: $DESC"
echo "==> deb version:  $DEB_VERSION"

if [ ! -d "$DEBIAN_SRC" ]; then
    echo "ERROR: packaging directory not found: $DEBIAN_SRC"
    exit 1
fi

if [ -e "$DEBIAN_LINK" ] && [ ! -L "$DEBIAN_LINK" ]; then
    echo "ERROR: $DEBIAN_LINK already exists and is not a symlink."
    echo "       Remove or rename it first."
    exit 1
fi

if [ -L "$DEBIAN_LINK" ]; then
    rm -f "$DEBIAN_LINK"
fi

# Inject version into changelog for this build (restored on exit)
CHANGELOG_BAK=$(mktemp)
cp -a "$DEBIAN_SRC/changelog" "$CHANGELOG_BAK"
{
    echo "unreal-speccy-portable (${DEB_VERSION}) unstable; urgency=medium"
    echo
    echo "  * Build from git describe: ${DESC}."
    echo
    echo " -- djdron <djdron@gmail.com>  $(date -R)"
    echo
    cat "$CHANGELOG_BAK"
} > "$DEBIAN_SRC/changelog"

echo "==> Creating temporary symlink:"
echo "    $DEBIAN_LINK  ->  $DEBIAN_SRC"
ln -s "$DEBIAN_SRC" "$DEBIAN_LINK"

chmod +x "$DEBIAN_SRC/rules" "$DEBIAN_SRC/postinst" "$DEBIAN_SRC/postrm" 2>/dev/null || true

echo "==> Building source-based .deb ..."
echo "    Working directory: $REPO_ROOT"

if command -v debuild >/dev/null 2>&1; then
    debuild -us -uc -b
elif command -v dpkg-buildpackage >/dev/null 2>&1; then
    dpkg-buildpackage -us -uc -b
else
    echo "ERROR: neither debuild nor dpkg-buildpackage found."
    echo "  sudo apt install debhelper devscripts build-essential"
    exit 1
fi

echo
echo "==> Done."

PARENT="$(cd "$REPO_ROOT/.." && pwd)"
DEBS=()
while IFS= read -r f; do
    DEBS+=("$f")
done < <(find "$PARENT" -maxdepth 1 -name "unreal-speccy-portable_${DEB_VERSION}_*.deb" 2>/dev/null | sort)

if [ ${#DEBS[@]} -eq 0 ]; then
    while IFS= read -r f; do
        DEBS+=("$f")
    done < <(find "$PARENT" -maxdepth 1 -name 'unreal-speccy-portable_*.deb' 2>/dev/null | sort)
fi

if [ ${#DEBS[@]} -eq 0 ]; then
    while IFS= read -r f; do
        DEBS+=("$f")
    done < <(find "$REPO_ROOT" -maxdepth 1 -name 'unreal-speccy-portable_*.deb' 2>/dev/null | sort)
fi

if [ ${#DEBS[@]} -eq 0 ]; then
    echo "    WARNING: no .deb found"
else
    for f in "${DEBS[@]}"; do
        echo "    $f"
        if [ -n "$OUTPUT_DIR" ]; then
            mkdir -p "$OUTPUT_DIR"
            cp -a "$f" "$OUTPUT_DIR/"
            echo "    -> copied to $OUTPUT_DIR/"
        fi
    done
fi
