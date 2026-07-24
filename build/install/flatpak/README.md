# Flatpak packaging for Unreal Speccy Portable

Analog of the Debian (`build/install/linux`) and Fedora (`build/install/fedora`) packaging.

Builds a sandboxed Flatpak using the Freedesktop runtime + SDL2 (from the SDK).

**App ID:** `io.github.djdron.UnrealSpeccyPortable`

## Quick start

From the **root** of the UnrealSpeccyP repository:

```bash
# 1. Install tools + runtime
# Fedora:
sudo dnf install flatpak flatpak-builder rsync
# Debian/Ubuntu:
sudo apt install flatpak flatpak-builder rsync

flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak install -y flathub org.freedesktop.Platform//25.08 org.freedesktop.Sdk//25.08

# 2. Build
./build/install/flatpak/make_flatpak.sh
```

Result:

```
io.github.djdron.UnrealSpeccyPortable.flatpak
```

Install:

```bash
flatpak install --user ./io.github.djdron.UnrealSpeccyPortable.flatpak
flatpak run io.github.djdron.UnrealSpeccyPortable
```

## Icons (optional)

```
build/install/flatpak/icons/
├── 16x16/unreal_speccy_portable.png
├── 22x22/...
└── 128x128/unreal_speccy_portable.png
```

Same icons as for deb/rpm packages. They are installed under the Flatpak app id name.

## Files

```
build/install/flatpak/
├── make_flatpak.sh
├── README.md
├── io.github.djdron.UnrealSpeccyPortable.yml      # manifest
├── io.github.djdron.UnrealSpeccyPortable.metainfo.xml
├── unreal-speccy-portable.desktop
├── unreal_speccy_portable.xml
└── icons/
```

## Permissions (sandbox)

| Permission        | Reason                          |
|-------------------|---------------------------------|
| X11 / Wayland     | Display                         |
| DRI               | GPU / OpenGL                    |
| PulseAudio        | Sound (AY + Beeper)             |
| home filesystem   | Open .tap / .z80 / .trd files   |
| network           | Optional (curl / web features)  |

## Layout inside the sandbox

```
/app/bin/unreal-speccy-portable              # launcher (cd + exec)
/app/lib/unreal-speccy-portable/
├── unreal_speccy_portable                   # real binary
└── res/                                     # ROMs + fonts
```

Same idea as the classic deb/rpm layout so `res/rom/*.rom` is found.

## Notes

- CMake source: `build/cmake/`
- Desktop / MIME / AppStream metainfo included
- Suitable as a base for a future Flathub submission
