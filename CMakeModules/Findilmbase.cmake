# Locate ILMBASE
# This module defines
# ILMBASE_LIBRARY
# ILMBASE_FOUND, if false, do not try to link to ILMBASE 
# ILMBASE_INCLUDE_DIR, where to find the headers
#
# $ILMBASE_DIR is an environment variable that would
# correspond to the ./configure --prefix=$ILMBASE_DIR
#
# Created by Robert Osfield. 


FIND_PATH(ILMBASE_INCLUDE_DIR OpenEXR/ImathVec.h
    $ENV{ILMBASE_DIR}/include
    $ENV{ILMBASE_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    /usr/freeware/include
)

# Macro to find ilmbase libraries
# example: ILMBASE_FIND_VAR(OPENEXR_IlmThread_LIBRARY IlmThread)
MACRO(ILMBASE_FIND_VAR varname libname)
    FIND_LIBRARY( ${varname}
        NAMES ${libname} ${libname}-2_1 ${libname}-2_2
        PATHS
        $ENV{ILMBASE_DIR}/lib
        $ENV{ILMBASE_DIR}
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        /usr/freeware/lib64
    )
ENDMACRO(ILMBASE_FIND_VAR)

# Macro to find exr libraries (and debug versions)
# example: ILMBASE_FIND(OPENEXR_IlmThread_LIBRARY IlmThread)
MACRO(ILMBASE_FIND libname)
    ILMBASE_FIND_VAR(ILMBASE_${libname}_LIBRARY ${libname})
    ILMBASE_FIND_VAR(ILMBASE_${libname}_LIBRARY_DEBUG ${libname}d)
ENDMACRO(ILMBASE_FIND)

ILMBASE_FIND(IlmThread)
ILMBASE_FIND(Iex)
ILMBASE_FIND(Half)

SET(ILMBASE_FOUND "NO")
IF(ILMBASE_INCLUDE_DIR AND ILMBASE_IlmThread_LIBRARY AND ILMBASE_Iex_LIBRARY AND ILMBASE_Half_LIBRARY)
    SET(ILMBASE_LIBRARIES ${ILMBASE_IlmThread_LIBRARY} ${ILMBASE_Half_LIBRARY} ${ILMBASE_Iex_LIBRARY} )
    SET(ILMBASE_LIBRARIES_VARS ILMBASE_IlmThread_LIBRARY ILMBASE_Half_LIBRARY ILMBASE_Iex_LIBRARY )
    SET(ILMBASE_FOUND "YES")
ENDIF(ILMBASE_INCLUDE_DIR AND ILMBASE_IlmThread_LIBRARY AND ILMBASE_Iex_LIBRARY AND ILMBASE_Half_LIBRARY)
