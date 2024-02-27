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
option(USE_HAIKU "Haiku version" OFF)
option(USE_BENCHMARK "benchmark mode (console)" OFF)
option(USE_LIBRARY "library mode" OFF)
option(USE_WEB "use web sources (SDL or iOS versions)" ON)
option(USE_IMGUI "use Dear ImGui (only with SDL2/GLESv2)" ON)

project (USP)

set(PROJECT unreal_speccy_portable)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS")
set(APPLE_IOS 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
set(ANDROID 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
set(UWP 1)
endif()

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

if(NOT APPLE_IOS AND NOT ANDROID AND (USE_SDL OR USE_SDL2 OR USE_HAIKU) AND USE_WEB AND NOT EMSCRIPTEN)
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
if(NOT EMSCRIPTEN)
	file(GLOB SRCC_ZLIB "../../3rdparty/zlib/*.c")
	file(GLOB SRCH_ZLIB "../../3rdparty/zlib/*.h")
	list(APPEND SRCC ${SRCC_ZLIB})
	list(APPEND SRCH ${SRCH_ZLIB})
	source_group("3rdparty\\zlib" FILES ${SRCC_ZLIB} ${SRCH_ZLIB})
	include_directories("../../3rdparty/zlib")
endif(NOT EMSCRIPTEN)
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
list(APPEND THIRDPARTY_LIBRARIES ${OPENAL_LIBRARY})
file(GLOB SRCCXX_PLATFORM_OPENAL "../../platform/oal/*.cpp")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_OPENAL})
source_group("platform\\oal" FILES ${SRCCXX_PLATFORM_OPENAL})

endif(USE_WX_WIDGETS OR APPLE_IOS)

if(NOT ANDROID AND (USE_SDL OR USE_SDL2 OR APPLE_IOS OR USE_HAIKU))

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

if(USE_SDL2 AND USE_IMGUI)
file(GLOB SRCCXX_IMGUI "../../3rdparty/imgui/*.cpp")
file(GLOB SRCH_IMGUI "../../3rdparty/imgui/*.h")
list(APPEND SRCCXX ${SRCCXX_IMGUI})
list(APPEND SRCH ${SRCH_IMGUI})
source_group("imgui" FILES ${SRCCXX_IMGUI} ${SRCH_IMGUI})
add_definitions(-DIMGUI_IMPL_OPENGL_ES2 -DUSE_IMGUI)

include_directories("../../3rdparty/imgui/")
file(GLOB SRCCXX_IMGUI_BACKENDS "../../3rdparty/imgui/backends/imgui_impl_sdl2.cpp" "../../3rdparty/imgui/backends/imgui_impl_opengl3.cpp")
file(GLOB SRCH_IMGUI_BACKENDS "../../3rdparty/imgui/backends/imgui_impl_sdl2.h" "../../3rdparty/imgui/backends/imgui_impl_opengl3.h")
list(APPEND SRCCXX ${SRCCXX_IMGUI_BACKENDS})
list(APPEND SRCH ${SRCH_IMGUI_BACKENDS})
source_group("imgui\\backends" FILES ${SRCCXX_IMGUI_BACKENDS} ${SRCH_IMGUI_BACKENDS})
endif(USE_SDL2 AND USE_IMGUI)

if(USE_WEB)
file(GLOB SRCCXX_PLATFORM_WEB "../../platform/web/*.cpp")
file(GLOB SRCH_PLATFORM_WEB "../../platform/web/*.h")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_WEB})
list(APPEND SRCH ${SRCH_PLATFORM_WEB})
source_group("platform\\web" FILES ${SRCCXX_PLATFORM_WEB} ${SRCH_PLATFORM_WEB})
if(NOT MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
endif(NOT MSVC)
endif(USE_WEB)

endif(NOT ANDROID AND (USE_SDL OR USE_SDL2 OR APPLE_IOS OR USE_HAIKU))

if(USE_SDL2 OR APPLE_IOS OR ANDROID)

file(GLOB SRCCXX_PLATFORM_GLES2 "../../platform/gles2/*.cpp")
file(GLOB SRCH_PLATFORM_GLES2 "../../platform/gles2/*.h")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_GLES2})
list(APPEND SRCH ${SRCH_PLATFORM_GLES2})
source_group("platform\\gles2" FILES ${SRCCXX_PLATFORM_GLES2} ${SRCH_PLATFORM_GLES2})

endif(USE_SDL2 OR APPLE_IOS OR ANDROID)

if(MSVC)
if(NOT USE_WX_WIDGETS)
add_definitions(-DWIN32_LEAN_AND_MEAN -DVC_EXTRALEAN -DNOCOLOR)
endif(NOT USE_WX_WIDGETS)
add_definitions("/wd4244 /wd4996 /wd4800 /wd4267")
if(NOT UWP)
set(variables CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
foreach(variable ${variables})
	if(${variable} MATCHES "/MD")
    	string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
	endif()
endforeach()
endif(NOT UWP)
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

elseif(ANDROID)

set(PROJECT usp)

file(GLOB SRCCXX_PLATFORM_ANDROID "../../platform/android/*.cpp")
file(GLOB SRCH_PLATFORM_ANDROID "../../platform/android/*.h")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_ANDROID})
list(APPEND SRCH ${SRCH_PLATFORM_ANDROID})
source_group("platform\\android" FILES ${SRCCXX_PLATFORM_ANDROID} ${SRCH_PLATFORM_ANDROID})

file(GLOB SRCCXX_PLATFORM_TOUCH_UI "../../platform/touch_ui/*.cpp")
file(GLOB SRCH_PLATFORM_TOUCH_UI "../../platform/touch_ui/*.h")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_TOUCH_UI})
list(APPEND SRCH ${SRCH_PLATFORM_TOUCH_UI})
source_group("platform\\touch_ui" FILES ${SRCCXX_PLATFORM_TOUCH_UI} ${SRCH_PLATFORM_TOUCH_UI})

add_definitions(-DUSE_EXTERN_RESOURCES -DUSE_CONFIG -DUSE_ZIP -D_ANDROID -DUSE_GLES2 -DUSE_TUI)

add_platform_io_stuff()

add_library(${PROJECT} SHARED ${SRCCXX} ${SRCC} ${SRCH})
target_link_libraries(${PROJECT} GLESv2)

elseif(USE_WX_WIDGETS)

if(APPLE)
add_definitions(-D_MAC)
elseif(UNIX)
add_definitions(-D_LINUX)
endif(APPLE)

#wxWidgets
set(wxUSE_STC 0)
set(wxUSE_WXHTML_HELP 0)
set(wxUSE_HTML 0)
set(wxUSE_XRC 0)
set(wxUSE_MDI_ARCHITECTURE 0)
set(wxUSE_MEDIACTRL 0)
set(wxUSE_PROPGRID 0)
set(wxUSE_RIBBON 0)
set(wxUSE_RICHTEXT 0)
set(wxUSE_WEBVIEW 0)
set(wxUSE_URL 0)
set(wxUSE_PROTOCOL 0)
set(wxUSE_SOCKETS 0)
set(wxUSE_FS_INET 0)
set(wxUSE_DEBUGREPORT 0)
set(wxBUILD_SHARED 0)
set(wxBUILD_USE_STATIC_RUNTIME 1)
set(wxBUILD_INSTALL 0)
add_subdirectory("../../3rdparty/wxWidgets" "wxWidgets")

file(GLOB SRCCXX_PLATFORM_WX_WIDGETS "../../platform/wxwidgets/*.cpp")
file(GLOB SRCH_PLATFORM_WX_WIDGETS "../../platform/wxwidgets/*.h")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_WX_WIDGETS})
list(APPEND SRCH ${SRCH_PLATFORM_WX_WIDGETS})
source_group("platform\\wxwidgets" FILES ${SRCCXX_PLATFORM_WX_WIDGETS} ${SRCH_PLATFORM_WX_WIDGETS})

IF(WIN32)
add_definitions(-DWX_RES)
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
add_definitions(-DWX_CPU_AMD64)
else()
add_definitions(-DWX_CPU_X86)
endif()
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
target_link_libraries(${PROJECT} wx::core wx::base wx::adv wx::aui wx::gl)
target_link_libraries(${PROJECT} ${OPENGL_LIBRARIES})

elseif(USE_HAIKU)

#Haiku
file(GLOB SRCCXX_PLATFORM_HAIKU "../../platform/haiku/*.cpp")
add_definitions(-DAPP_SIGNATURE="application/x-vnd.unreal-speccy-portable" -DUSE_HAIKU -DUSE_PNG -DUSE_UI -DUSE_CONFIG -DUSE_ZIP)
if(USE_WEB)
add_definitions(-DUSE_WEB)
endif(USE_WEB)
list(APPEND SRCCXX ${SRCCXX_PLATFORM_HAIKU})
source_group("platform\\haiku" FILES ${SRCCXX_PLATFORM_HAIKU})

add_platform_io_stuff()

add_executable(${PROJECT} WIN32 ${SRCCXX} ${SRCC} ${SRCH})
target_link_libraries(${PROJECT} -lbe -lmedia -ldevice -lgame -ltracker -llocalestub -lpng)

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
if(NOT EMSCRIPTEN AND NOT WIN32)
find_package(SDL2 REQUIRED)
endif(NOT EMSCRIPTEN AND NOT WIN32)
list(APPEND THIRDPARTY_LIBRARIES ${SDL2_LIBRARIES})

if(UWP)
find_package(SDL2 REQUIRED)
find_package(unofficial-angle REQUIRED)
list(APPEND THIRDPARTY_LIBRARIES SDL2::SDL2 unofficial::angle::libEGL unofficial::angle::libANGLE unofficial::angle::libGLESv2 CURL::libcurl)
file(GLOB SRCCXX_PLATFORM_UWP "../uwp/*")
file(GLOB ASSETS_PLATFORM_UWP "../uwp/assets/*")
file(GLOB RES_ROM_PLATFORM_UWP "../../res/rom/*")
file(GLOB RES_FONT_PLATFORM_UWP "../../res/font/*")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_UWP} ${ASSETS_PLATFORM_UWP} ${RES_ROM_PLATFORM_UWP} ${RES_FONT_PLATFORM_UWP})
set_property(SOURCE ${ASSETS_PLATFORM_UWP} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${ASSETS_PLATFORM_UWP} PROPERTY VS_DEPLOYMENT_LOCATION "assets")
set_property(SOURCE ${RES_ROM_PLATFORM_UWP} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${RES_ROM_PLATFORM_UWP} PROPERTY VS_DEPLOYMENT_LOCATION "res/rom")
set_property(SOURCE ${RES_FONT_PLATFORM_UWP} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${RES_FONT_PLATFORM_UWP} PROPERTY VS_DEPLOYMENT_LOCATION "res/font")
set_source_files_properties(../uwp/SDL_winrt_main_NonXAML.cpp PROPERTIES COMPILE_FLAGS /ZW)
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /nodefaultlib:vccorlibd /nodefaultlib:msvcrtd vccorlibd.lib msvcrtd.lib")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /nodefaultlib:vccorlib /nodefaultlib:msvcrt vccorlib.lib msvcrt.lib")
source_group("platform\\uwp" FILES ${SRCCXX_PLATFORM_UWP})
source_group("platform\\uwp\\assets" FILES ${ASSETS_PLATFORM_UWP})
source_group("platform\\uwp\\res\\rom" FILES ${RES_ROM_PLATFORM_UWP})
source_group("platform\\uwp\\res\\font" FILES ${RES_FONT_PLATFORM_UWP})
add_definitions(-D_UWP -DSDL_USE_PREF_PATH)
set(DEPLOY_LIB_PATH "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
file(GLOB LIB_PLATFORM_UWP "${DEPLOY_LIB_PATH}/bin/*.dll")
file(GLOB LIB_DBG_PLATFORM_UWP "${DEPLOY_LIB_PATH}/debug/bin/*.dll")
list(APPEND SRCCXX ${LIB_PLATFORM_UWP} ${LIB_DBG_PLATFORM_UWP})
set_property(SOURCE ${LIB_PLATFORM_UWP} PROPERTY VS_DEPLOYMENT_CONTENT $<NOT:$<CONFIG:Debug>>)
set_property(SOURCE ${LIB_PLATFORM_UWP} PROPERTY VS_DEPLOYMENT_LOCATION ".")
set_property(SOURCE ${LIB_DBG_PLATFORM_UWP} PROPERTY VS_DEPLOYMENT_CONTENT $<CONFIG:Debug>)
set_property(SOURCE ${LIB_DBG_PLATFORM_UWP} PROPERTY VS_DEPLOYMENT_LOCATION ".")
source_group("platform\\uwp\\lib" FILES ${LIB_PLATFORM_UWP})
source_group("platform\\uwp\\lib\\dbg" FILES ${LIB_DBG_PLATFORM_UWP})
endif(UWP)

if(WIN32)
set(OPENGLES2_INCLUDE_DIR "${ANGLE_INCLUDE_DIR}")
add_definitions(-DGL_GLEXT_PROTOTYPES)
list(APPEND THIRDPARTY_LIBRARIES ${ANGLE_LIBRARY})
if(MINGW)
set(SDL2_LINK_FLAGS "-s -static-libgcc -Wl,-Bstatic -lstdc++ -lpthread")
endif(MINGW)
if(NOT UWP)
SET(SRCRES "../win/unreal_speccy_portable.rc")
source_group("platform\\win\\res" FILES ${SRCRES})
endif(NOT UWP)
endif(WIN32)

if(UNIX)
add_definitions(-D_LINUX)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
if(NOT EMSCRIPTEN)
find_package(OpenGLES2 REQUIRED)
list(APPEND THIRDPARTY_LIBRARIES ${OPENGLES2_LIBRARIES})
endif(NOT EMSCRIPTEN)
endif(UNIX)

if(NOT EMSCRIPTEN AND NOT UWP)
include_directories("${OPENGLES2_INCLUDE_DIR}")
include_directories("${SDL2_INCLUDE_DIRS}")
endif(NOT EMSCRIPTEN AND NOT UWP)
file(GLOB SRCCXX_PLATFORM_SDL2 "../../platform/sdl2/*.cpp")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_SDL2})
source_group("platform\\sdl2" FILES ${SRCCXX_PLATFORM_SDL2})

add_definitions(-DUSE_SDL2 -DSDL_USE_JOYSTICK -DSDL_USE_MOUSE -DUSE_CONFIG -DUSE_ZIP -DUSE_GLES2 -DUSE_UI -DUI_REAL_ALPHA)
if(USE_WEB AND NOT EMSCRIPTEN)
add_definitions(-DUSE_WEB)
endif(USE_WEB AND NOT EMSCRIPTEN)

if(EMSCRIPTEN)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sUSE_SDL=2 -sUSE_ZLIB=1")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sUSE_ZLIB=1")
add_definitions(-DSDL_NO_MAIN -DUSE_EMSCRIPTEN -DSDL_DEFAULT_FOLDER="/cache/")
file(GLOB SRCCXX_PLATFORM_EMSCRIPTEN "../../platform/emscripten/*.cpp")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_EMSCRIPTEN})
source_group("platform\\emscripten" FILES ${SRCCXX_PLATFORM_EMSCRIPTEN})
set(CMAKE_EXECUTABLE_SUFFIX ".html")
set(EMSCRIPTEN_LINK_FLAGS "--preload-file ../../../res/@res/ -sUSE_SDL=2 -sUSE_ZLIB=1 -sALLOW_MEMORY_GROWTH=1 -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -lidbfs.js")
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
target_link_libraries(${PROJECT} ${SDL2_LINK_FLAGS})

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

if(HAIKU)
add_custom_target(Resources ALL
	DEPENDS ${PROJECT}
	SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../../platform/haiku/resources.rdef)
add_custom_command(TARGET Resources
	COMMAND rc -o resources.rsrc ${CMAKE_CURRENT_SOURCE_DIR}/../../platform/haiku/resources.rdef
	COMMENT "${P} Compiling resource file."
	VERBATIM)
add_custom_command(TARGET Resources
	COMMAND xres -o ${PROJECT} resources.rsrc
	COMMENT "${P} Adding compiled resources to '${PROJECT}' executable."
	VERBATIM)
add_custom_command(TARGET Resources
	COMMAND mimeset --all ${PROJECT}
	COMMENT "${P} Updating MIME types for '${PROJECT}' executable."
	VERBATIM)
endif(HAIKU)

if(MSVC AND NOT UWP)
set_target_properties(${PROJECT} PROPERTIES LINK_FLAGS "/MANIFEST:NO /LARGEADDRESSAWARE")
endif(MSVC AND NOT UWP)

if(WIN32 AND (USE_WX_WIDGETS OR USE_SDL2))
install(TARGETS ${PROJECT} RUNTIME DESTINATION ".")
set(CPACK_PACKAGE_EXECUTABLES ${PROJECT} "Unreal Speccy Portable")
foreach(_lib ${THIRDPARTY_LIBRARIES})
	string(REPLACE "\\" "/" _lib2 ${_lib})
	string(REPLACE ".lib" ".dll" _dll ${_lib2})
	file(GLOB _dll_exist ${_dll})
	list(APPEND THIRDPARTY_LIBRARIES_INSTALL ${_dll_exist})
endforeach(_lib)
install(FILES ${THIRDPARTY_LIBRARIES_INSTALL} DESTINATION ".")
install(DIRECTORY "../../res" DESTINATION ".")
set(CPACK_PACKAGE_NAME "Unreal Speccy Portable")
set(CPACK_PACKAGE_VENDOR djdron)
set(CPACK_PACKAGE_VERSION "0.0.90")
set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/../../build/win/unreal_speccy_portable.ico")
set(CPACK_WIX_PRODUCT_ICON ${CPACK_PACKAGE_ICON})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Unreal Speccy Portable")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ZX Spectrum emulator")
set(CPACK_PACKAGE_FILE_NAME "unreal-speccy-portable_${CPACK_PACKAGE_VERSION}_win32")
if(USE_WX_WIDGETS)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}_wxwidgets_openal")
elseif(USE_SDL2)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}_sdl2_gles2_angle_curl")
endif()
set(CPACK_GENERATOR WIX)
set(CPACK_WIX_UPGRADE_GUID "039D5BD5-4261-4D6F-9291-F38BD84EF706")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../../gpl-3.0.rtf")
include(CPack)
endif(WIN32 AND (USE_WX_WIDGETS OR USE_SDL2))
