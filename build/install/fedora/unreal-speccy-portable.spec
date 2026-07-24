Name:           unreal-speccy-portable
Version:        0.0.86.28
Release:        1%{?dist}
Summary:        Portable ZX Spectrum emulator
License:        GPL-3.0-or-later
URL:            https://github.com/djdron/UnrealSpeccyP
# Source is the whole repository; build from a checkout / tarball of the project root
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

# Normalize binary name (upstream uses underscore)
if [ -f %{buildroot}%{_bindir}/unreal_speccy_portable ]; then
    mv %{buildroot}%{_bindir}/unreal_speccy_portable \
       %{buildroot}%{_bindir}/unreal-speccy-portable
elif [ -f %{_vpath_builddir}/unreal_speccy_portable ]; then
    install -D -m 755 %{_vpath_builddir}/unreal_speccy_portable \
        %{buildroot}%{_bindir}/unreal-speccy-portable
fi

# Desktop entry
install -D -m 644 build/install/fedora/unreal-speccy-portable.desktop \
    %{buildroot}%{_datadir}/applications/unreal-speccy-portable.desktop

# MIME types
install -D -m 644 build/install/fedora/unreal_speccy_portable.xml \
    %{buildroot}%{_datadir}/mime/packages/unreal_speccy_portable.xml

# Icons (optional)
if [ -d build/install/fedora/icons ]; then
    for size in 16x16 22x22 24x24 32x32 48x48 64x64 128x128; do
        if [ -f build/install/fedora/icons/$size/unreal_speccy_portable.png ]; then
            install -D -m 644 build/install/fedora/icons/$size/unreal_speccy_portable.png \
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
%{_datadir}/applications/unreal-speccy-portable.desktop
%{_datadir}/mime/packages/unreal_speccy_portable.xml
%{_datadir}/icons/hicolor/*/apps/unreal_speccy_portable.png

%changelog
* Fri Jul 24 2026 Andrey Dj <djdron@gmail.com> - 0.0.86.28-1
- Initial source-based RPM packaging
- Built with CMake + SDL2
