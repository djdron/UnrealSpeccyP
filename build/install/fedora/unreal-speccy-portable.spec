Name:           unreal-speccy-portable
Version:        0.0.86.28
Release:        1%{?dist}
Summary:        Portable ZX Spectrum emulator
License:        GPL-3.0-or-later
URL:            https://github.com/djdron/UnrealSpeccyP
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.5
BuildRequires:  gcc-c++
BuildRequires:  pkgconfig(sdl2)
BuildRequires:  pkgconfig(libcurl)
BuildRequires:  zlib-devel
BuildRequires:  libpng-devel
BuildRequires:  desktop-file-utils
BuildRequires:  shared-mime-info
BuildRequires:  libappstream-glib

Requires:       SDL2
Requires:       libcurl
Requires:       zlib
Requires:       libpng

%description
Portable ZX-Spectrum emulator based on UnrealSpeccy by SMT.
Supports Z80 128K (Pentagon), AY/YM, Beeper, Beta Disk, Tape,
Kempston Joystick/Mouse, Snapshots and Replays.

Supported formats: sna, z80, szx, rzx, tap, tzx, csw,
trd, scl, fdi, td0, udi, zip.

Built from source using CMake + SDL2.

# Layout (same as classic deb packaging):
#   /usr/bin/unreal-speccy-portable          — launcher script
#   /usr/lib/unreal-speccy-portable/         — binary + res/

%prep
%autosetup -n %{name}-%{version}

%build
%cmake -S build/cmake \
    -DUSE_SDL2=ON \
    -DUSE_SDL=OFF \
    -DUSE_WX_WIDGETS=OFF \
    -DUSE_BENCHMARK=OFF \
    -DUSE_LIBRARY=OFF \
    -DUSE_WEB=ON \
    -DCMAKE_BUILD_TYPE=Release
%cmake_build

%install
%cmake_install

LIBDIR=%{buildroot}%{_libdir}/unreal-speccy-portable
# On multi-arch Fedora _libdir is /usr/lib64; keep classic path for res lookup
LIBDIR=%{buildroot}/usr/lib/unreal-speccy-portable
mkdir -p ${LIBDIR}

# Binary -> /usr/lib/unreal-speccy-portable/unreal_speccy_portable
if [ -f %{buildroot}%{_bindir}/unreal_speccy_portable ]; then
    mv %{buildroot}%{_bindir}/unreal_speccy_portable \
       ${LIBDIR}/unreal_speccy_portable
elif [ -f %{buildroot}%{_bindir}/unreal-speccy-portable ]; then
    mv %{buildroot}%{_bindir}/unreal-speccy-portable \
       ${LIBDIR}/unreal_speccy_portable
elif [ -f %{_vpath_builddir}/unreal_speccy_portable ]; then
    install -m 755 %{_vpath_builddir}/unreal_speccy_portable \
        ${LIBDIR}/unreal_speccy_portable
else
    echo "ERROR: built binary not found" >&2
    exit 1
fi

# res/ (ROMs, fonts) next to the binary
cp -a res ${LIBDIR}/

# Launcher script
install -D -m 755 build/install/fedora/unreal-speccy-portable.launcher \
    %{buildroot}%{_bindir}/unreal-speccy-portable

# Desktop entry
install -D -m 644 build/install/fedora/unreal-speccy-portable.desktop \
    %{buildroot}%{_datadir}/applications/unreal-speccy-portable.desktop

# MIME types
install -D -m 644 build/install/fedora/unreal_speccy_portable.xml \
    %{buildroot}%{_datadir}/mime/packages/unreal_speccy_portable.xml

# Icons (optional) — shared tree build/install/linux/icons/<WxH>/
if [ -d build/install/linux/icons ]; then
    for dir in build/install/linux/icons/*/ ; do
        [ -d "$dir" ] || continue
        size=$(basename "$dir")
        f="$dir/unreal_speccy_portable.png"
        if [ -f "$f" ]; then
            install -D -m 644 "$f" \
                %{buildroot}%{_datadir}/icons/hicolor/$size/apps/unreal_speccy_portable.png
        fi
    done
fi

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/unreal-speccy-portable.desktop || true

%files
%license LICENSE
%doc README.md
%{_bindir}/unreal-speccy-portable
/usr/lib/unreal-speccy-portable/
%{_datadir}/applications/unreal-speccy-portable.desktop
%{_datadir}/mime/packages/unreal_speccy_portable.xml
%{_datadir}/icons/hicolor/*/apps/unreal_speccy_portable.png

%changelog
* Fri Jul 24 2026 djdron <djdron@gmail.com> - 0.0.86.28-1
- Source-based RPM packaging
- Binary + res/ under /usr/lib/unreal-speccy-portable/
- Launcher script in /usr/bin (classic layout)
