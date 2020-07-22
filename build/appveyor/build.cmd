@echo off
setlocal enabledelayedexpansion

cd "%~dp0"

rem Find CMake.

if not defined CMAKE for %%a in (cmake.exe) do (
	set CMAKE_EXE=%%~$PATH:a
	if not [!CMAKE_EXE!] == [] for %%b in ("!CMAKE_EXE!") do (
		set CMAKE_BIN=%%~dpb
		for %%c in ("!CMAKE_BIN:~0,-1!") do set CMAKE="%%~dpc"
	)
)

if not defined CMAKE if exist "!ProgramFiles!\CMake" set CMAKE="!ProgramFiles!\CMake"
if not defined CMAKE if exist "!ProgramFiles(x86)!\CMake" set CMAKE="!ProgramFiles(x86)!\CMake"
if not defined CMAKE if exist "!SystemDrive!\CMake" set CMAKE="!SystemDrive!\CMake"
if not defined CMAKE echo Can't find CMake installation - please add it to PATH or specify the path to it in the CMAKE environment variable. & exit /b 1
echo Found CMake at !CMAKE!

rem Find 7-Zip.

if not defined 7ZIP for %%a in (7z.exe) do if not [%%~$PATH:a] == [] set 7ZIP="%%~$PATH:a"
if not defined 7ZIP if exist "!ProgramFiles!\7-Zip\7z.exe" set 7ZIP="!ProgramFiles!\7-Zip\7z.exe"
if not defined 7ZIP if exist "!ProgramFiles(x86)!\7-Zip\7z.exe" set 7ZIP="!ProgramFiles(x86)!\7-Zip\7z.exe"
if not defined 7ZIP if exist "!SystemDrive!\7-Zip\7z.exe" set 7ZIP="!SystemDrive!\7-Zip\7z.exe"
if not defined 7ZIP echo drmingw_build: Can't find 7-Zip installation - please add it to PATH or specify the full path to 7z.exe in the 7ZIP environment variable. & exit /b 1
echo Found 7-Zip at !7ZIP!

cd ../cmake

for %%I in (.) do set CUR_DIR=%%~dpnI

rem build library---------------------------------------------------------------

set BUILD_DIR=build_win32_library
set BUILD_ARGS=-DUSE_SDL=0 -DUSE_LIBRARY=1
call :build


rem build benchmark-------------------------------------------------------------

set BUILD_DIR=build_win32_benchmark
set BUILD_ARGS=-DUSE_SDL=0 -DUSE_BENCHMARK=1
call :build


rem build sdl2 gles2 angle curl-------------------------------------------------

set SDL2LIB=SDL2-devel-2.0.12-VC.zip
set URL=https://www.libsdl.org/release/%SDL2LIB%
set DEST=%SDL2LIB%
call :download
if errorlevel 1 exit /b 1
set SDL2LIB_DIR=%CUR_DIR%\SDL2-2.0.12
del %SDL2LIB_DIR% /S /Q /F>nul
!7ZIP! x %SDL2LIB%

set ANGLELIB=angle-chromium84.7z
set URL=https://github.com/djdron/UnrealSpeccyP/releases/download/angle-chromium84/%ANGLELIB%
set DEST=%ANGLELIB%
call :download
if errorlevel 1 exit /b 1
set ANGLELIB_DIR=%CUR_DIR%\angle-chromium84
del %ANGLELIB_DIR% /S /Q /F>nul
!7ZIP! x %ANGLELIB%

set CURLLIB=curl-7.61.1.7z
set URL=https://github.com/djdron/UnrealSpeccyP/releases/download/angle-chromium84/%CURLLIB%
set DEST=%CURLLIB%
call :download
if errorlevel 1 exit /b 1
set CURLLIB_DIR=%CUR_DIR%\curl-7.61.1
del %CURLLIB_DIR% /S /Q /F>nul
!7ZIP! x %CURLLIB%

set BUILD_DIR=build_win32_sdl2
set ANGLE_ARGS=-DANGLE_INCLUDE_DIR=%ANGLELIB_DIR%/include -DANGLE_LIBRARY=%ANGLELIB_DIR%/lib/x64/libGLESv2.lib;%ANGLELIB_DIR%/lib/x64/libEGL.lib
set SDL2_ARGS=-DSDL2_INCLUDE_DIRS=%SDL2LIB_DIR%/include -DSDL2_LIBRARIES=%SDL2LIB_DIR%/lib/x64/SDL2main.lib;%SDL2LIB_DIR%/lib/x64/SDL2.lib
set CURL_ARGS=-DCURL_INCLUDE_DIR=%CURLLIB_DIR%/include -DCURL_LIBRARY=%CURLLIB_DIR%/lib/x64/libcurl.lib
set BUILD_ARGS=-DUSE_SDL=0 -DUSE_SDL2=1 %ANGLE_ARGS% %SDL2_ARGS% %CURL_ARGS%
call :build


rem build wxWidgets OpenAL Soft-------------------------------------------------

set OPENALLIB=openal-soft-1.20.1-bin.zip
set URL=https://openal-soft.org/openal-binaries/%OPENALLIB%
set DEST=%OPENALLIB%
call :download
if errorlevel 1 exit /b 1
set OPENALLIB_DIR=%CUR_DIR%\openal-soft-1.20.1-bin
del %OPENALLIB_DIR% /S /Q /F>nul
!7ZIP! x %OPENALLIB%

set OPENALDIR=%OPENALLIB_DIR%
set BUILD_DIR=build_win32_wxwidgets
set BUILD_ARGS=-DUSE_SDL=0 -DUSE_WX_WIDGETS=1
call :build

exit /b


:build

rem CMake build

del %BUILD_DIR% /S /Q /F>nul
mkdir %BUILD_DIR%
pushd %BUILD_DIR%
cmake .. -A x64 %BUILD_ARGS%
cmake --build . --config Release
popd
exit /b


:download

rem Download !URL! to !DEST!.
rem Note: Positional parameters don't work here, as the percent signs in URLs are eagerly interpreted despite quoting.

echo Downloading !URL!

for %%a in (powershell.exe) do if not [%%~$PATH:a] == [] powershell -Command "(New-Object Net.WebClient).DownloadFile('!URL!', '!DEST!')" & goto :eof
for %%a in (curl.exe) do if not [%%~$PATH:a] == [] curl "!URL!" -O "!DEST!" --location & goto :eof
for %%a in (wget.exe) do if not [%%~$PATH:a] == [] wget "!URL!" -o "!DEST!" --max-redirect=5 & goto :eof
for %%a in (bitsadmin.exe) do if not [%%~$PATH:a] == [] start /wait "MinGW download" bitsadmin /transfer "MinGW" "!URL!" "!DEST!" & goto :eof
echo No download utilities available - please download file at !URL! and save to !DEST! manually
exit /b 1
