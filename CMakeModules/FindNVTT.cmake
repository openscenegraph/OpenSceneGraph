# Locate nvidia-texture-tools
# This module defines
# NVTT_LIBRARY
# NVTT_FOUND, if false, do not try to link to nvtt
# NVTT_INCLUDE_DIR, where to find the headers
#


FIND_PATH(NVTT_INCLUDE_DIR nvtt/nvtt.h
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES include
)

# NVTT
FIND_LIBRARY(NVTT_LIBRARY_RELEASE
  NAMES nvtt
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)

FIND_LIBRARY(NVTT_LIBRARY_DEBUG
  NAMES nvtt_d
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)

# NVIMAGE
FIND_LIBRARY(NVIMAGE_LIBRARY_RELEASE
  NAMES nvimage
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)

FIND_LIBRARY(NVIMAGE_LIBRARY_DEBUG
  NAMES nvimage_d
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)

# NVMATH
FIND_LIBRARY(NVMATH_LIBRARY_RELEASE
  NAMES nvmath
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)

FIND_LIBRARY(NVMATH_LIBRARY_DEBUG
  NAMES nvmath_d
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)

# NVCORE
FIND_LIBRARY(NVCORE_LIBRARY_RELEASE
  NAMES nvcore
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)
FIND_LIBRARY(NVCORE_LIBRARY_DEBUG
  NAMES nvcore_d
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)

# NVTHREAD
FIND_LIBRARY(NVTHREAD_LIBRARY_RELEASE
  NAMES nvthread
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)
FIND_LIBRARY(NVTHREAD_LIBRARY_DEBUG
  NAMES nvthread_d
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)

# SQUISH
FIND_LIBRARY(NVSQUISH_LIBRARY_RELEASE
  NAMES squish
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)
FIND_LIBRARY(NVSQUISH_LIBRARY_DEBUG
  NAMES squish_d
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)

# BC6H
FIND_LIBRARY(NVBC6H_LIBRARY_RELEASE
  NAMES bc6h
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)
FIND_LIBRARY(NVBC6H_LIBRARY_DEBUG
  NAMES bc6h_d
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)

# BC7
FIND_LIBRARY(NVBC7_LIBRARY_RELEASE
  NAMES bc7
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)
FIND_LIBRARY(NVBC7_LIBRARY_DEBUG
  NAMES bc7_d
  PATHS
  /usr/local
  /usr
  $ENV{NVTT_DIR}
  ${3rdPartyRoot}
  PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static
)

IF (WIN32)
    SET(LIBS_TO_SETUP "NVTT" "NVCORE" "NVMATH" "NVIMAGE" "NVTHREAD" "NVBC7" "NVBC6H" "NVSQUISH")
ELSE()
    SET(LIBS_TO_SETUP "NVTT" "NVCORE" "NVMATH" "NVIMAGE")
ENDIF()


FOREACH(LIB ${LIBS_TO_SETUP})
	IF(${LIB}_LIBRARY_DEBUG)
	   SET(${LIB}_LIBRARIES  optimized ${${LIB}_LIBRARY_RELEASE} debug ${${LIB}_LIBRARY_DEBUG})
	ELSE(${LIB}_LIBRARY_DEBUG)
	   SET(${LIB}_LIBRARY_DEBUG ${${LIB}_LIBRARY_RELEASE})
	   SET(${LIB}_LIBRARIES  optimized  ${${LIB}_LIBRARY_RELEASE} debug ${${LIB}_LIBRARY_DEBUG}) 
	ENDIF(${LIB}_LIBRARY_DEBUG)
ENDFOREACH(LIB ${LIBS_TO_SETUP})

SET(NVTT_LIBRARIES
	${NVTT_LIBRARIES}  
	${NVCORE_LIBRARIES} 
	${NVMATH_LIBRARIES} 
	${NVIMAGE_LIBRARIES} 
	${NVTHREAD_LIBRARIES} 
	${NVBC7_LIBRARIES} 
	${NVBC6H_LIBRARIES} 
	${NVSQUISH_LIBRARIES}
) 

SET(NVTT_FOUND "NO")
IF(NVTT_LIBRARY_RELEASE AND NVTT_INCLUDE_DIR)
   SET(NVTT_FOUND "YES" )
ENDIF(NVTT_LIBRARY_RELEASE AND NVTT_INCLUDE_DIR)
