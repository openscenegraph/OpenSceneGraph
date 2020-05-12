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
    NAMES egl.h
    PATH_SUFFIXES EGL
    PATHS ENV EGL_DIR
)

FIND_LIBRARY(EGL_LIBRARY
    NAMES EGL libEGL.dll.lib
    PATHS ENV EGL_DIR
)

# handle the QUIETLY and REQUIRED arguments and set
# EGL_FOUND to TRUE as appropriate
INCLUDE( FindPackageHandleStandardArgs )

find_package_handle_standard_args(EGL
	FOUND_VAR 
	  EGL_FOUND
	REQUIRED_VARS
	  EGL_LIBRARY
	  EGL_INCLUDE_DIR
	FAIL_MESSAGE
	  "Could NOT find EGL, try to set the path to EGL root folder in the system variable EGL_DIR"
)
