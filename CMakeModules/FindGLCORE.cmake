# Finds the OpenGL Core Profile (cp) header file.
# Looks for glcorearb.h
# 
# This script defines the following:
#  GLCORE_FOUND // Set to TRUE if glcorearb.h is found
#  GLCORE_INCLUDE_DIR // Parent directory of directory (gl, GL3, or OpenGL) containing the CP header.
#  GLCORE_GLCOREARB_HEADER // advanced
#
# GLCORE_ROOT can be set as an environment variable or a CMake variable,
# to the parent directory of the gl, GL3, or OpenGL directory containing the CP header.
#


FIND_PATH( GLCORE_GLCOREARB_HEADER
    NAMES GL/glcorearb.h GL3/glcorearb.h OpenGL/glcorearb.h gl/glcorearb.h
    HINTS ${GLCORE_ROOT}
    PATHS ENV GLCORE_ROOT
)

set( GLCORE_INCLUDE_DIR )
if( GLCORE_GLCOREARB_HEADER )
    set( GLCORE_INCLUDE_DIR ${GLCORE_GLCOREARB_HEADER} )
endif()


# handle the QUIETLY and REQUIRED arguments and set
# GLCORE_FOUND to TRUE as appropriate
INCLUDE( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( GLCORE
    "Set GLCORE_ROOT as the parent of the directory containing the OpenGL core profile header."
    GLCORE_INCLUDE_DIR )

MARK_AS_ADVANCED(
    GLCORE_INCLUDE_DIR
    GLCORE_GLCOREARB_HEADER
)
