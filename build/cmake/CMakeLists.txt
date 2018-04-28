cmake_minimum_required (VERSION 2.6)

if(POLICY CMP0054)
cmake_policy(SET CMP0054 OLD)
endif()
if(POLICY CMP0004)
cmake_policy(SET CMP0004 OLD)
endif()


set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

option(USE_WX_WIDGETS "wxWidgets + OpenAL + OpenGL version" OFF)
option(USE_SDL "SDL version" ON)
option(USE_SDL2 "SDL2 version" OFF)
option(USE_BENCHMARK "benchmark mode (console)" OFF)
option(USE_LIBRARY "library mode" OFF)
option(USE_WEB "use web sources (SDL or iOS versions)" ON)

project (USP)

set(PROJECT unreal_speccy_portable)

#core
file(GLOB SRCCXX_ROOT "../../*.cpp")
file(GLOB SRCH_ROOT "../../*.h")
file(GLOB SRCCXX_Z80 "../../z80/*.cpp")
file(GLOB SRCH_Z80 "../../z80/*.h")
file(GLOB SRCCXX_TOOLS "../../tools/*.cpp")
file(GLOB SRCH_TOOLS "../../tools/*.h")
file(GLOB SRCCXX_SNAPSHOT "../../snapshot/*.cpp")
file(GLOB SRCH_SNAPSHOT "../../snapshot/*.h")
file(GLOB SRCCXX_DEVICES "../../devices/*.cpp")
file(GLOB SRCH_DEVICES "../../devices/*.h")
file(GLOB SRCCXX_DEVICES_FDD "../../devices/fdd/*.cpp")
file(GLOB SRCH_DEVICES_FDD "../../devices/fdd/*.h")
file(GLOB SRCCXX_DEVICES_INPUT "../../devices/input/*.cpp")
file(GLOB SRCH_DEVICES_INPUT "../../devices/input/*.h")
file(GLOB SRCCXX_DEVICES_SOUND "../../devices/sound/*.cpp")
file(GLOB SRCH_DEVICES_SOUND "../../devices/sound/*.h")
list(APPEND SRCCXX ${SRCCXX_ROOT} ${SRCCXX_Z80} ${SRCCXX_TOOLS} ${SRCCXX_SNAPSHOT} ${SRCCXX_DEVICES} ${SRCCXX_DEVICES_FDD} ${SRCCXX_DEVICES_INPUT} ${SRCCXX_DEVICES_SOUND})
list(APPEND SRCH ${SRCH_ROOT} ${SRCH_Z80} ${SRCH_TOOLS} ${SRCH_SNAPSHOT} ${SRCH_DEVICES} ${SRCH_DEVICES_FDD} ${SRCH_DEVICES_INPUT} ${SRCH_DEVICES_SOUND})

source_group("" FILES ${SRCCXX_ROOT} ${SRCH_ROOT})
source_group("z80" FILES ${SRCCXX_Z80} ${SRCH_Z80})
source_group("tools" FILES ${SRCCXX_TOOLS} ${SRCH_TOOLS})
source_group("snapshot" FILES ${SRCCXX_SNAPSHOT} ${SRCH_SNAPSHOT})
source_group("devices" FILES ${SRCCXX_DEVICES} ${SRCH_DEVICES})
source_group("devices\\fdd" FILES ${SRCCXX_DEVICES_FDD} ${SRCH_DEVICES_FDD})
source_group("devices\\input" FILES ${SRCCXX_DEVICES_INPUT} ${SRCH_DEVICES_INPUT})
source_group("devices\\sound" FILES ${SRCCXX_DEVICES_SOUND} ${SRCH_DEVICES_SOUND})

#3rdparty libs

if(NOT APPLE_IOS AND (USE_SDL OR USE_SDL2) AND USE_WEB AND NOT EMSCRIPTEN)
find_package(CURL REQUIRED)
if(CURL_FOUND)
	include_directories(${CURL_INCLUDE_DIRS})
	list(APPEND THIRDPARTY_LIBRARIES ${CURL_LIBRARIES})
	add_definitions(-DUSE_CURL)
endif(CURL_FOUND)
endif()

if(NOT APPLE_IOS)
find_package(ZLIB)
endif(NOT APPLE_IOS)
if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIRS})
	list(APPEND THIRDPARTY_LIBRARIES ${ZLIB_LIBRARIES})
	add_definitions(-DUSE_FILE32API)
else(ZLIB_FOUND)
	file(GLOB SRCC_ZLIB "../../3rdparty/zlib/*.c")
	file(GLOB SRCH_ZLIB "../../3rdparty/zlib/*.h")
	list(APPEND SRCC ${SRCC_ZLIB})
	list(APPEND SRCH ${SRCH_ZLIB})
	source_group("3rdparty\\zlib" FILES ${SRCC_ZLIB} ${SRCH_ZLIB})
	include_directories("../../3rdparty/zlib")
endif(ZLIB_FOUND)

if(USE_WX_WIDGETS)
if(NOT APPLE_IOS)
find_package(PNG)
endif(NOT APPLE_IOS)
if(PNG_FOUND)
	include_directories(${PNG_INCLUDE_DIRS})
	list(APPEND THIRDPARTY_LIBRARIES ${PNG_LIBRARIES})
else(PNG_FOUND)
	file(GLOB SRCC_LIBPNG "../../3rdparty/libpng/*.c")
	file(GLOB SRCH_LIBPNG "../../3rdparty/libpng/*.h")
	list(APPEND SRCC ${SRCC_LIBPNG})
	list(APPEND SRCH ${SRCH_LIBPNG})
	source_group("3rdparty\\libpng" FILES ${SRCC_LIBPNG} ${SRCH_LIBPNG})
	include_directories("../../3rdparty/libpng")
endif(PNG_FOUND)
endif(USE_WX_WIDGETS)

file(GLOB SRCC_MINIZIP "../../3rdparty/minizip/*.c")
file(GLOB SRCH_MINIZIP "../../3rdparty/minizip/*.h")
list(APPEND SRCC ${SRCC_MINIZIP})
list(APPEND SRCH ${SRCH_MINIZIP})
source_group("3rdparty\\minizip" FILES ${SRCC_MINIZIP} ${SRCH_MINIZIP})
include_directories("../../3rdparty/minizip")

file(GLOB SRCCXX_TINYXML2 "../../3rdparty/tinyxml2/*.cpp")
file(GLOB SRCH_TINYXML2 "../../3rdparty/tinyxml2/*.h")
list(APPEND SRCCXX ${SRCCXX_TINYXML2})
list(APPEND SRCH ${SRCH_TINYXML2})
source_group("3rdparty\\tinyxml2" FILES ${SRCCXX_TINYXML2} ${SRCH_TINYXML2})
include_directories("../../3rdparty/tinyxml2")

if(USE_WEB)
file(GLOB SRCH_JSON "../../3rdparty/json/*.hpp")
list(APPEND SRCH ${SRCH_JSON})
source_group("3rdparty\\json" FILES ${SRCH_JSON})
include_directories("../../3rdparty/json")
endif()

#platform
file(GLOB SRCCXX_PLATFORM "../../platform/*.cpp")
file(GLOB SRCH_PLATFORM "../../platform/*.h")
list(APPEND SRCCXX ${SRCCXX_PLATFORM})
list(APPEND SRCH ${SRCH_PLATFORM})
source_group("platform" FILES ${SRCCXX_PLATFORM} ${SRCH_PLATFORM})

if(USE_WX_WIDGETS OR APPLE_IOS)

#Open AL
find_package(OpenAL REQUIRED)
include_directories("${OPENAL_INCLUDE_DIR}")
file(GLOB SRCCXX_PLATFORM_OPENAL "../../platform/oal/*.cpp")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_OPENAL})
source_group("platform\\oal" FILES ${SRCCXX_PLATFORM_OPENAL})

endif(USE_WX_WIDGETS OR APPLE_IOS)

if(USE_SDL OR USE_SDL2 OR APPLE_IOS)

file(GLOB SRCCXX_UI "../../ui/*.cpp")
file(GLOB SRCH_UI "../../ui/*.h")
list(APPEND SRCCXX ${SRCCXX_UI})
list(APPEND SRCH ${SRCH_UI})
source_group("ui" FILES ${SRCCXX_UI} ${SRCH_UI})

file(GLOB SRCCXX_PLATFORM_CUSTOM_UI "../../platform/custom_ui/*.cpp")
file(GLOB SRCH_PLATFORM_CUSTOM_UI "../../platform/custom_ui/*.h")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_CUSTOM_UI})
list(APPEND SRCH ${SRCH_PLATFORM_CUSTOM_UI})
source_group("platform\\custom_ui" FILES ${SRCCXX_PLATFORM_CUSTOM_UI} ${SRCH_PLATFORM_CUSTOM_UI})

if(USE_WEB)
file(GLOB SRCCXX_PLATFORM_WEB "../../platform/web/*.cpp")
file(GLOB SRCH_PLATFORM_WEB "../../platform/web/*.h")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_WEB})
list(APPEND SRCH ${SRCH_PLATFORM_WEB})
source_group("platform\\web" FILES ${SRCCXX_PLATFORM_WEB} ${SRCH_PLATFORM_WEB})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
endif(USE_WEB)

endif(USE_SDL OR USE_SDL2 OR APPLE_IOS)

if(USE_SDL2 OR APPLE_IOS)

file(GLOB SRCCXX_PLATFORM_GLES2 "../../platform/gles2/*.cpp")
file(GLOB SRCH_PLATFORM_GLES2 "../../platform/gles2/*.h")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_GLES2})
list(APPEND SRCH ${SRCH_PLATFORM_GLES2})
source_group("platform\\gles2" FILES ${SRCCXX_PLATFORM_GLES2} ${SRCH_PLATFORM_GLES2})

endif(USE_SDL2 OR APPLE_IOS)

if(MSVC)
add_definitions(-DWIN32_LEAN_AND_MEAN -DVC_EXTRALEAN -DNOCOLOR)
add_definitions("/wd4244 /wd4996 /wd4800")
set(variables CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
foreach(variable ${variables})
	if(${variable} MATCHES "/MD")
    	string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
	endif()
endforeach()
endif(MSVC)

macro(add_platform_io_stuff)
if(WIN32)
file(GLOB SRCCXX_PLATFORM_WINDOWS "../../platform/win/*.cpp")
file(GLOB SRCH_PLATFORM_WINDOWS "../../platform/win/*.h")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_WINDOWS})
list(APPEND SRCH ${SRCH_PLATFORM_WINDOWS})
source_group("platform\\windows" FILES ${SRCCXX_PLATFORM_WINDOWS} ${SRCH_PLATFORM_WINDOWS})
add_definitions(-D_WINAPI)
elseif(UNIX)
file(GLOB SRCCXX_PLATFORM_LINUX "../../platform/linux/*.cpp")
file(GLOB SRCH_PLATFORM_LINUX "../../platform/linux/*.h")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_LINUX})
list(APPEND SRCH ${SRCH_PLATFORM_LINUX})
source_group("platform\\linux" FILES ${SRCCXX_PLATFORM_LINUX} ${SRCH_PLATFORM_LINUX})
add_definitions(-D_POSIX)
endif(WIN32)
endmacro()

if(APPLE_IOS)

set(PROJECT UnrealSpeccyPortable)

find_library(OPENGLES_FRAMEWORK OpenGLES)
find_library(COREGRAPHICS_FRAMEWORK CoreGraphics)
find_library(UIKIT_FRAMEWORK UIKit)
find_library(GLKIT_FRAMEWORK GLKit)

file(GLOB SRCCXX_PLATFORM_IOS "../../platform/ios/*.cpp")
file(GLOB SRCMM_PLATFORM_IOS "../../platform/ios/*.mm")
file(GLOB SRCH_PLATFORM_IOS "../../platform/ios/*.h")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_IOS})
list(APPEND SRCMM ${SRCMM_PLATFORM_IOS})
list(APPEND SRCH ${SRCH_PLATFORM_IOS})
source_group("platform\\ios" FILES ${SRCCXX_PLATFORM_IOS} ${SRCMM_PLATFORM_IOS} ${SRCH_PLATFORM_IOS})

file(GLOB SRCCXX_PLATFORM_TOUCH_UI "../../platform/touch_ui/*.cpp")
file(GLOB SRCH_PLATFORM_TOUCH_UI "../../platform/touch_ui/*.h")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_TOUCH_UI})
list(APPEND SRCH ${SRCH_PLATFORM_TOUCH_UI})
source_group("platform\\touch_ui" FILES ${SRCCXX_PLATFORM_TOUCH_UI} ${SRCH_PLATFORM_TOUCH_UI})

add_definitions(-DUSE_WEB -DUSE_CONFIG -DUSE_ZIP -DUSE_OAL -D_IOS -DUSE_GLES2 -DUSE_TUI -DUSE_UI -DUI_REAL_ALPHA)

add_platform_io_stuff()

file(GLOB COPY_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../ios/Resources/*)
source_group("Resources" FILES ${COPY_RESOURCES})

file(GLOB COPY_IMAGES ${CMAKE_CURRENT_SOURCE_DIR}/../ios/Images/*)
source_group("Images" FILES ${COPY_IMAGES})

file(GLOB COPY_RES_ROM ${CMAKE_CURRENT_SOURCE_DIR}/../../res/rom/*)
source_group("Resources\\rom" FILES ${COPY_RES_ROM})

file(GLOB COPY_RES_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../res/font/*)
source_group("Resources\\font" FILES ${COPY_RES_FONT})

list(APPEND SRCRES ${COPY_RESOURCES} ${COPY_IMAGES} ${COPY_RES_ROM} ${COPY_RES_FONT})

add_executable(${PROJECT} ${SRCCXX} ${SRCMM} ${SRCC} ${SRCH} ${SRCRES})
target_link_libraries(${PROJECT} ${OPENAL_LIBRARY} ${OPENGLES_FRAMEWORK} ${COREGRAPHICS_FRAMEWORK} ${UIKIT_FRAMEWORK} ${GLKIT_FRAMEWORK})
set_property(TARGET ${PROJECT} PROPERTY MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/../ios/UnrealSpeccyPortable-Info.plist")
set_target_properties(${PROJECT} PROPERTIES XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "iPhone/iPad")
set_target_properties(${PROJECT} PROPERTIES XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES "NO")

foreach(_file ${COPY_RESOURCES})
set_source_files_properties(${_file} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endforeach(_file)

foreach(_file ${COPY_IMAGES})
set_source_files_properties(${_file} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endforeach(_file)

foreach(_file ${COPY_RES_ROM})
set_source_files_properties(${_file} PROPERTIES MACOSX_PACKAGE_LOCATION "res/rom")
endforeach(_file)

foreach(_file ${COPY_RES_FONT})
set_source_files_properties(${_file} PROPERTIES MACOSX_PACKAGE_LOCATION "res/font")
endforeach(_file)

elseif(USE_WX_WIDGETS)

if(APPLE)
add_definitions(-D_MAC)
elseif(UNIX)
add_definitions(-D_LINUX)
endif(UNIX)

#wxWidgets
find_package(wxWidgets COMPONENTS core base adv gl REQUIRED)
include("${wxWidgets_USE_FILE}")
file(GLOB SRCCXX_PLATFORM_WX_WIDGETS "../../platform/wxwidgets/*.cpp")
file(GLOB SRCH_PLATFORM_WX_WIDGETS "../../platform/wxwidgets/*.h")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_WX_WIDGETS})
list(APPEND SRCH ${SRCH_PLATFORM_WX_WIDGETS})
source_group("platform\\wxwidgets" FILES ${SRCCXX_PLATFORM_WX_WIDGETS} ${SRCH_PLATFORM_WX_WIDGETS})

IF(WIN32)
add_definitions(-DWX_RES)
SET(SRCRES "../win/unreal_speccy_portable.rc")
source_group("platform\\win\\res" FILES ${SRCRES})
ENDIF(WIN32)

#Open GL
find_package(OpenGL REQUIRED)
if(NOT WIN32)
include_directories("${OPENGL_INCLUDE_DIR}")
endif(NOT WIN32)
file(GLOB SRCCXX_PLATFORM_OPENGL "../../platform/gl/*.cpp")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_OPENGL})
source_group("platform\\gl" FILES ${SRCCXX_PLATFORM_OPENGL})

add_executable(${PROJECT} WIN32 ${SRCCXX} ${SRCC} ${SRCH} ${SRCRES})
target_link_libraries(${PROJECT} ${wxWidgets_LIBRARIES})
target_link_libraries(${PROJECT} ${OPENAL_LIBRARY})
target_link_libraries(${PROJECT} ${OPENGL_LIBRARIES})

elseif(USE_SDL)

#SDL
find_package(SDL REQUIRED)
include_directories("${SDL_INCLUDE_DIR}")
file(GLOB SRCCXX_PLATFORM_SDL "../../platform/sdl/*.cpp")
add_definitions(-DUSE_SDL -DSDL_USE_JOYSTICK -DSDL_KEYS_COMMON -DUSE_CONFIG -DUSE_ZIP)
if(USE_WEB)
add_definitions(-DUSE_WEB)
endif(USE_WEB)
list(APPEND SRCCXX ${SRCCXX_PLATFORM_SDL})
source_group("platform\\sdl" FILES ${SRCCXX_PLATFORM_SDL})

add_platform_io_stuff()

add_executable(${PROJECT} WIN32 ${SRCCXX} ${SRCC} ${SRCH})
target_link_libraries(${PROJECT} ${SDL_LIBRARY})

elseif(USE_SDL2)

#SDL2
find_package(SDL2 REQUIRED)
if(WIN32)
set(OPENGLES2_INCLUDE_DIR "${ANGLE_INCLUDE_DIR}")
add_definitions(-DGL_GLEXT_PROTOTYPES)
set(OPENGLES2_LIBRARIES "${ANGLE_LIBRARY}")
if(MINGW)
set(SDL2_LINK_FLAGS "-s -static-libgcc -Wl,-Bstatic -lstdc++ -lpthread")
endif(MINGW)
SET(SRCRES "../win/unreal_speccy_portable.rc")
source_group("platform\\win\\res" FILES ${SRCRES})
endif(WIN32)

if(UNIX)
add_definitions(-D_LINUX)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
find_package(OpenGLES2 REQUIRED)
endif(UNIX)

include_directories("${OPENGLES2_INCLUDE_DIR}")

if(NOT EMSCRIPTEN)
include_directories("${SDL2_INCLUDE_DIRS}")
endif(NOT EMSCRIPTEN)
file(GLOB SRCCXX_PLATFORM_SDL2 "../../platform/sdl2/*.cpp")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_SDL2})
source_group("platform\\sdl2" FILES ${SRCCXX_PLATFORM_SDL2})

add_definitions(-DUSE_SDL2 -DSDL_USE_JOYSTICK -DSDL_USE_MOUSE -DUSE_CONFIG -DUSE_ZIP -DUSE_GLES2 -DUSE_UI -DUI_REAL_ALPHA)
if(USE_WEB AND NOT EMSCRIPTEN)
add_definitions(-DUSE_WEB)
endif(USE_WEB AND NOT EMSCRIPTEN)

if(EMSCRIPTEN)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
add_definitions(-DSDL_NO_MAINLOOP -DUSE_EMSCRIPTEN)
file(GLOB SRCCXX_PLATFORM_EMSCRIPTEN "../../platform/emscripten/*.cpp")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_EMSCRIPTEN})
source_group("platform\\emscripten" FILES ${SRCCXX_PLATFORM_EMSCRIPTEN})
set(CMAKE_EXECUTABLE_SUFFIX ".html")
set(EMSCRIPTEN_LINK_FLAGS "--preload-file ../../../res/ -s USE_SDL=2")
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} ${EMSCRIPTEN_LINK_FLAGS})
endif(EMSCRIPTEN)

add_platform_io_stuff()

if(APPLE)

set(PROJECT UnrealSpeccyPortable)
add_definitions(-D_MAC)

file(GLOB SRCCXX_PLATFORM_MAC "../../platform/mac/*.cpp")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_MAC})
source_group("platform\\mac" FILES ${SRCCXX_PLATFORM_MAC})

file(GLOB COPY_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../mac/*.icns)
source_group("Resources" FILES ${COPY_RESOURCES})

file(GLOB COPY_RES_ROM ${CMAKE_CURRENT_SOURCE_DIR}/../../res/rom/*)
source_group("Resources\\rom" FILES ${COPY_RES_ROM})

file(GLOB COPY_RES_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../res/font/*)
source_group("Resources\\font" FILES ${COPY_RES_FONT})

list(APPEND SRCRES ${COPY_RESOURCES} ${COPY_RES_ROM} ${COPY_RES_FONT})

endif(APPLE)

add_executable(${PROJECT} WIN32 MACOSX_BUNDLE ${SRCCXX} ${SRCC} ${SRCH} ${SRCRES})
target_link_libraries(${PROJECT} ${SDL2_LIBRARIES} ${OPENGLES2_LIBRARIES} ${SDL2_LINK_FLAGS})

if(APPLE)

set_property(TARGET ${PROJECT} PROPERTY MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/../mac/UnrealSpeccyPortable-Info.plist")

foreach(_file ${COPY_RESOURCES})
set_source_files_properties(${_file} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endforeach(_file)

foreach(_file ${COPY_RES_ROM})
set_source_files_properties(${_file} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/res/rom")
endforeach(_file)

foreach(_file ${COPY_RES_FONT})
set_source_files_properties(${_file} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/res/font")
endforeach(_file)

endif(APPLE)

elseif(USE_BENCHMARK)

#benchmark mode (console)
file(GLOB SRCCXX_PLATFORM_BENCHMARK "../../platform/benchmark/*.cpp")
add_definitions(-DUSE_BENCHMARK)
list(APPEND SRCCXX ${SRCCXX_PLATFORM_BENCHMARK})
source_group("platform\\benchmark" FILES ${SRCCXX_PLATFORM_BENCHMARK})

add_platform_io_stuff()

add_executable(${PROJECT} ${SRCCXX} ${SRCC} ${SRCH})

elseif(USE_LIBRARY)

#library mode
file(GLOB SRCCXX_PLATFORM_LIBRARY "../../platform/library/*.cpp")
add_definitions(-DUSE_LIBRARY)
list(APPEND SRCCXX ${SRCCXX_PLATFORM_LIBRARY})
source_group("platform\\library" FILES ${SRCCXX_PLATFORM_LIBRARY})

add_platform_io_stuff()

add_library(${PROJECT} SHARED ${SRCCXX} ${SRCC} ${SRCH})

endif(APPLE_IOS)

target_link_libraries(${PROJECT} ${THIRDPARTY_LIBRARIES})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
set_target_properties(${PROJECT} PROPERTIES LINK_FLAGS "/MANIFEST:NO /LARGEADDRESSAWARE")
endif()
