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
FIND_LIBRARY(NVTT_LIBRARY
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
FIND_LIBRARY(NVIMAGE_LIBRARY
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
FIND_LIBRARY(NVMATH_LIBRARY
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
FIND_LIBRARY(NVCORE_LIBRARY
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




SET(NVTT_FOUND "NO")
IF(NVTT_LIBRARY AND NVTT_INCLUDE_DIR)
  SET(NVTT_FOUND "YES")
ENDIF(NVTT_LIBRARY AND NVTT_INCLUDE_DIR)
