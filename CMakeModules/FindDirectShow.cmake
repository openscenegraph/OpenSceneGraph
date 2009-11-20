# Locate directshow
# This module defines
# DIRECTSHOW_LIBRARIES
# DIRECTSHOW_FOUND, if false, do not try to link to directshow
# DIRECTSHOW_INCLUDE_DIR, where to find the headers
#
# $DIRECTSHOW_DIR is an environment variable that would
# point to the this path in the plateform devkit (Samples\Multimedia\DirectShow)
#
# Created by Cedric Pinson.
#


SET(DIRECTSHOW_FOUND "NO")
SET(DIRECTSHOW_SAMPLE_ROOT "$ENV{DIRECTSHOW_DIR}" CACHE PATH "Location of DirectShow sample in devkit")

IF(WIN32)
    FIND_PATH(DIRECTSHOW_STRMBASE_INCLUDE_DIRS renbase.h
    PATHS
    ${DIRECTSHOW_SAMPLE_ROOT}/BaseClasses/
    $ENV{DIRECTSHOW_SAMPLE_ROOT}/BaseClasses/
    DOC "Location of DirectShow Base include on the windows devkit"
    )
    
    FIND_LIBRARY(DIRECTSHOW_STRMBASE_LIBRARY_RELEASE strmbase
    PATHS
    ${DIRECTSHOW_SAMPLE_ROOT}/BaseClasses/Release_MBCS/ # sdk 6.1
    $ENV{DIRECTSHOW_SAMPLE_ROOT}/BaseClasses/Release_MBCS/  # sdk 6.1
    ${DIRECTSHOW_SAMPLE_ROOT}/BaseClasses/Release/  # sdk 2003
    $ENV{DIRECTSHOW_SAMPLE_ROOT}/BaseClasses/Release/ # sdk 2003
    DOC "Location of DirectShow Base library on the windows devkit"
    )

    FIND_LIBRARY(DIRECTSHOW_STRMBASE_LIBRARY_DEBUG strmbasd
    PATHS
    ${DIRECTSHOW_SAMPLE_ROOT}/BaseClasses/Debug_MBCS/  # sdk 6.1
    $ENV{DIRECTSHOW_SAMPLE_ROOT}/BaseClasses/Debug_MBCS/  # sdk 6.1
    ${DIRECTSHOW_SAMPLE_ROOT}/BaseClasses/Debug/  # sdk 2003
    $ENV{DIRECTSHOW_SAMPLE_ROOT}/BaseClasses/Debug/  # sdk 2003
    DOC "Location of DirectShow Base library on the windows devkit"
    )

    IF   (DIRECTSHOW_STRMBASE_INCLUDE_DIRS AND DIRECTSHOW_STRMBASE_LIBRARY_RELEASE)
        SET(WIN_LIBS winmm d3d9 d3dx9 kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32)
        SET(DIRECTSHOW_FOUND "YES")
        SET(DIRECTSHOW_LIBRARY_DEBUG
            ${DIRECTSHOW_STRMBASE_LIBRARY_DEBUG}
        )
        SET(DIRECTSHOW_LIBRARY
            ${DIRECTSHOW_STRMBASE_LIBRARY_RELEASE}
        )
        SET(DIRECTSHOW_INLUDE_DIRS
            ${DIRECTSHOW_STRMBASE_INCLUDE_DIRS}
        )
        
    ENDIF()
    
ENDIF()
