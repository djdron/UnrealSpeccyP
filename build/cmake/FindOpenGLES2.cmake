# - Find OpenGLES2
# Find the native OpenGLES2 includes and libraries
#
#  OPENGLES2_INCLUDE_DIR - where to find GLES2/gl2.h, etc.
#  OPENGLES2_LIBRARIES   - List of libraries when using OpenGLES2.
#  OPENGLES2_FOUND       - True if OpenGLES2 found.

# Downloaded from
# https://sourceforge.net/p/alleg/allegro/ci/5.1/tree/cmake/FindOpenGLES.cmake
# Modified to use the v2 library rather than the v1 library.

if(OPENGLES2_INCLUDE_DIR)
    # Already in cache, be silent
    set(OPENGLES2_FIND_QUIETLY TRUE)
endif(OPENGLES2_INCLUDE_DIR)

find_path(OPENGLES2_INCLUDE_DIR GLES2/gl2.h)

find_library(OPENGLES2_gl_LIBRARY NAMES GLESv2)

# Handle the QUIETLY and REQUIRED arguments and set OPENGLES_FOUND
# to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OPENGLES2 DEFAULT_MSG
    OPENGLES2_INCLUDE_DIR OPENGLES2_gl_LIBRARY)

set(OPENGLES2_LIBRARIES ${OPENGLES2_gl_LIBRARY})

mark_as_advanced(OPENGLES2_INCLUDE_DIR)
mark_as_advanced(OPENGLES2_gl_LIBRARY)
