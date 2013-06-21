cmake_minimum_required (VERSION 2.6)
project (USP)

option (USE_WX_WIDGETS "wxWidgets + OpenAL + OpenGL version" ON)
option (USE_BENCHMARK "benchmark mode (console)" OFF)

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
file(GLOB SRCC_ZLIB "../../3rdparty/zlib/*.c")
file(GLOB SRCH_ZLIB "../../3rdparty/zlib/*.h")
file(GLOB SRCC_MINIZIP "../../3rdparty/minizip/*.c")
file(GLOB SRCH_MINIZIP "../../3rdparty/minizip/*.h")
file(GLOB SRCC_LIBPNG "../../3rdparty/libpng/*.c")
file(GLOB SRCH_LIBPNG "../../3rdparty/libpng/*.h")
list(APPEND SRCC ${SRCC_ZLIB} ${SRCC_MINIZIP} ${SRCC_LIBPNG})
list(APPEND SRCH ${SRCH_ZLIB} ${SRCH_MINIZIP} ${SRCH_LIBPNG})
file(GLOB SRCCXX_TINYXML "../../3rdparty/tinyxml/*.cpp")
file(GLOB SRCH_TINYXML "../../3rdparty/tinyxml/*.h")
list(APPEND SRCCXX ${SRCCXX_TINYXML})
list(APPEND SRCH ${SRCH_TINYXML})

source_group("3rdparty\\zlib" FILES ${SRCC_ZLIB} ${SRCH_ZLIB})
source_group("3rdparty\\minizip" FILES ${SRCC_MINIZIP} ${SRCH_MINIZIP})
source_group("3rdparty\\libpng" FILES ${SRCC_LIBPNG} ${SRCH_LIBPNG})
source_group("3rdparty\\tinyxml" FILES ${SRCCXX_TINYXML} ${SRCH_TINYXML})

include_directories("../../3rdparty/zlib")
include_directories("../../3rdparty/minizip")
include_directories("../../3rdparty/libpng")
include_directories("../../3rdparty/tinyxml")

#platform
file(GLOB SRCCXX_PLATFORM "../../platform/*.cpp")
file(GLOB SRCH_PLATFORM "../../platform/*.h")
list(APPEND SRCCXX ${SRCCXX_PLATFORM})
list(APPEND SRCH ${SRCH_PLATFORM})
source_group("platform" FILES ${SRCCXX_PLATFORM} ${SRCH_PLATFORM})

#benchmark mode (console)
if(USE_BENCHMARK)
file(GLOB SRCCXX_PLATFORM_BENCHMARK "../../platform/benchmark/*.cpp")
add_definitions(-DUSE_BENCHMARK)
list(APPEND SRCCXX ${SRCCXX_PLATFORM_BENCHMARK})
source_group("platform\\benchmark" FILES ${SRCCXX_PLATFORM_BENCHMARK})
endif(USE_BENCHMARK)

if(USE_WX_WIDGETS)

#wx widgets
find_package(wxWidgets COMPONENTS core base adv gl REQUIRED)
include("${wxWidgets_USE_FILE}")
file(GLOB SRCCXX_PLATFORM_WX_WIDGETS "../../platform/wxwidgets/*.cpp")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_WX_WIDGETS})
source_group("platform\\wxwidgets" FILES ${SRCCXX_PLATFORM_WX_WIDGETS})

#Open AL
find_package(OpenAL REQUIRED)
include_directories("${OPENAL_INCLUDE_DIR}")
file(GLOB SRCCXX_PLATFORM_OPENAL "../../platform/oal/*.cpp")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_OPENAL})
source_group("platform\\oal" FILES ${SRCCXX_PLATFORM_OAL})

#Open GL
find_package(OpenGL REQUIRED)
include_directories("${OPENGL_INCLUDE_DIR}")
file(GLOB SRCCXX_PLATFORM_OPENGL "../../platform/gl/*.cpp")
list(APPEND SRCCXX ${SRCCXX_PLATFORM_OPENGL})
source_group("platform\\gl" FILES ${SRCCXX_PLATFORM_OPENGL})

endif(USE_WX_WIDGETS)

if(MSVC)
add_definitions("/wd4244 /wd4996")
endif(MSVC)

if(UNIX)
add_definitions(-D_LINUX)
endif(UNIX)

add_executable(unreal_speccy_portable WIN32 ${SRCCXX} ${SRCC} ${SRCH})

if(USE_WX_WIDGETS)
target_link_libraries(unreal_speccy_portable ${wxWidgets_LIBRARIES})
target_link_libraries(unreal_speccy_portable ${OPENAL_LIBRARY})
target_link_libraries(unreal_speccy_portable ${OPENGL_LIBRARIES})
endif(USE_WX_WIDGETS)

