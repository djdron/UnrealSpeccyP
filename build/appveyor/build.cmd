cd "%~dp0"

rem Find CMake.

if not defined CMAKE for %%a in (cmake.exe) do (
	set CMAKE_EXE=%%~$PATH:a
	if not [!CMAKE_EXE!] == [] for %%b in ("!CMAKE_EXE!") do (
		set CMAKE_BIN=%%~dpb
		for %%c in ("!CMAKE_BIN:~0,-1!") do set CMAKE=%%~dpc
	)
)

if not defined CMAKE if exist "!ProgramFiles!\CMake" set CMAKE=!ProgramFiles!\CMake
if not defined CMAKE if exist "!ProgramFiles(x86)!\CMake" set CMAKE=!ProgramFiles(x86)!\CMake
if not defined CMAKE if exist "!SystemDrive!\CMake" set CMAKE=!SystemDrive!\CMake
if not defined CMAKE echo Can't find CMake installation - please add it to PATH or specify the path to it in the CMAKE environment variable. & exit /b 1
echo Found CMake at !CMAKE!

cd ../cmake

SET BUILD_DIR=build_win32_benchmark

mkdir %BUILD_DIR%
cd %BUILD_DIR%

cmake .. -DUSE_SDL=0 -DUSE_BENCHMARK=1
cmake --build .
