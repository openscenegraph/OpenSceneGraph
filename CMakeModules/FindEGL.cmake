# Finds EGL header and library
#
# This script defines the following:
#  EGL_FOUND // Set to TRUE if EGL is found
#  EGL_INCLUDE_DIR // Parent directory of directory EGL/egl.h header.
#
# EGL_DIR can be set as an environment variable or a CMake variable,
# to the parent directory of the EGL header.
#


FIND_PATH( EGL_INCLUDE_DIR
    NAMES EGL/egl.h
    HINTS ENV EGL_DIR
)

FIND_LIBRARY(EGL_LIBRARY
    NAMES EGL
    HINTS ENV EGL_DIR
    PATH_SUFFIXES lib
)

# handle the QUIETLY and REQUIRED arguments and set
# EGL_FOUND to TRUE as appropriate
INCLUDE( FindPackageHandleStandardArgs )

FIND_PACKAGE_HANDLE_STANDARD_ARGS(EGL
                                  REQUIRED_VARS EGL_LIBRARY EGL_INCLUDE_DIR)

MARK_AS_ADVANCED(
    EGL_INCLUDE_DIR
    EGL_LIBRARY
)
