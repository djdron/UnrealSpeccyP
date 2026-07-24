# Source-based Debian packaging for Unreal Speccy Portable

This directory replaces the old **binary-only** deb packaging.

The new packaging builds the emulator **from source** using CMake + SDL2
and produces a proper `.deb` package.

`debian/` stays here (`build/install/linux/debian/`).  
The build script automatically creates a temporary symlink from the
repository root, so you don't need to copy anything.

## Quick start

From the **root** of the UnrealSpeccyP repository:

```bash
# 1. Install build dependencies
sudo apt update
sudo apt install build-essential debhelper cmake \
    libsdl2-dev libcurl4-openssl-dev zlib1g-dev libpng-dev pkg-config

# 2. Build the package
./build/install/linux/make_deb.sh
```

The script will:
1. Create a temporary symlink `debian` → `build/install/linux/debian`
2. Run `debuild` / `dpkg-buildpackage`
3. Remove the symlink when finished

The resulting package appears one level above the repository:

```
../unreal-speccy-portable_0.0.86.28-1_amd64.deb
```

## Icons (optional but recommended)

Copy PNG icons into `build/install/linux/debian/icons/`:

```
build/install/linux/debian/icons/
├── 16x16/unreal_speccy_portable.png
├── 22x22/unreal_speccy_portable.png
├── 24x24/unreal_speccy_portable.png
├── 32x32/unreal_speccy_portable.png
├── 48x48/unreal_speccy_portable.png
├── 64x64/unreal_speccy_portable.png
└── 128x128/unreal_speccy_portable.png
```

You can take them from the previous  
`build/install/linux/debian/usr/share/icons/hicolor/` tree.

## What changed compared to the old packaging

| Old (binary)                     | New (source-based)                     |
|----------------------------------|----------------------------------------|
| Pre-built binary only            | Builds from source with CMake          |
| Hard-coded amd64                 | Architecture: any                      |
| libcurl3 / old deps              | Modern libcurl4 + libsdl2-dev          |
| Simple `dpkg -b`                 | Full debhelper + dpkg-buildpackage     |
| Version 0.0.83                   | 0.0.86.28-1 (matches recent upstream)  |

## Files in this packaging

```
build/install/linux/
├── make_deb.sh              ← creates symlink + builds
├── README.md
└── debian/
    ├── changelog
    ├── control
    ├── copyright
    ├── rules                ← CMake + SDL2
    ├── postinst / postrm
    ├── source/format
    ├── unreal-speccy-portable.desktop
    ├── unreal_speccy_portable.xml
    └── unreal-speccy-portable.install
```

## Notes

- The CMake source directory is `build/cmake/` relative to the repository root.
- Binary is renamed from `unreal_speccy_portable` → `unreal-speccy-portable`.
- MIME types and `.desktop` file are preserved from the previous packaging.
- The temporary symlink is automatically removed after the build (even on error).
