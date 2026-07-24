# Source-based RPM packaging for Unreal Speccy Portable (Fedora)

Analog of the Debian packaging in `build/install/linux/`.

Builds the emulator **from source** using CMake + SDL2 and produces an RPM.

## Quick start (Fedora / RHEL / Rocky / Alma)

From the **root** of the UnrealSpeccyP repository:

```bash
# 1. Install build dependencies
sudo dnf install rpm-build rpmdevtools cmake gcc-c++ \
    SDL2-devel libcurl-devel zlib-devel libpng-devel \
    desktop-file-utils shared-mime-info libappstream-glib \
    rsync

# Optional: set up standard rpmbuild tree
rpmdev-setuptree

# 2. Build the package
./build/install/fedora/make_rpm.sh
```

The script will:
1. Create a source tarball of the current tree
2. Place it into `~/rpmbuild/SOURCES/`
3. Run `rpmbuild -ba`

Resulting packages appear in:
```
~/rpmbuild/RPMS/<arch>/unreal-speccy-portable-0.0.86.28-1.*.rpm
~/rpmbuild/SRPMS/unreal-speccy-portable-0.0.86.28-1.*.src.rpm
```

Install:
```bash
sudo dnf install ~/rpmbuild/RPMS/*/unreal-speccy-portable-*.rpm
```

## Icons (optional)

Put PNGs into `build/install/fedora/icons/`:

```
build/install/fedora/icons/
├── 16x16/unreal_speccy_portable.png
├── 22x22/unreal_speccy_portable.png
...
└── 128x128/unreal_speccy_portable.png
```

Same icons as for the Debian package.

## Files

```
build/install/fedora/
├── make_rpm.sh
├── README.md
├── unreal-speccy-portable.spec
├── unreal-speccy-portable.desktop
└── unreal_speccy_portable.xml
```

## Notes

- CMake source dir: `build/cmake/`
- Binary renamed: `unreal_speccy_portable` → `unreal-speccy-portable`
- MIME types and `.desktop` match the Debian packaging
- Works on Fedora, RHEL, Rocky, Alma Linux; on openSUSE use `zypper` equivalents
