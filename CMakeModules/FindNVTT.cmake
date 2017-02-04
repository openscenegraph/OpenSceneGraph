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

# NVTHREAD
FIND_LIBRARY(NVTHREAD_LIBRARY
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
FIND_LIBRARY(NVSQUISH_LIBRARY
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
FIND_LIBRARY(NVBC6H_LIBRARY
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
FIND_LIBRARY(NVBC7_LIBRARY
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



SET(NVTT_FOUND "NO")
IF(NVTT_LIBRARY AND NVTT_INCLUDE_DIR)
  SET(NVTT_FOUND "YES")
ENDIF(NVTT_LIBRARY AND NVTT_INCLUDE_DIR)
