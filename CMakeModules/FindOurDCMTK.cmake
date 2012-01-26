# - find DCMTK libraries
#

#  DCMTK_INCLUDE_DIRS  - Directories to include to use DCMTK
#  DCMTK_LIBRARIES     - Files to link against to use DCMTK
#  DCMTK_FOUND         - If false, don't try to use DCMTK
#  DCMTK_DIR           - (optional) Source directory for DCMTK
#
# DCMTK_DIR can be used to make it simpler to find the various include
# directories and compiled libraries if you've just compiled it in the
# source tree. Just set it to the root of the tree where you extracted
# the source.
#
# Written for VXL by Amitha Perera.
# Modified by Robert Osfied to enable support for install placements of DCMTK 3.5.4 versions onwards

FIND_PATH( DCMTK_ROOT_INCLUDE_DIR dcmtk/config/osconfig.h
    ${DCMTK_DIR}/config/include
    ${DCMTK_DIR}/include
    /usr/local/dicom/include
    /usr/local/include/
    /usr/include/
    /usr/local/dicom/include/
)

FIND_PATH( DCMTK_config_INCLUDE_DIR osconfig.h
    ${DCMTK_ROOT_INCLUDE_DIR}/dcmtk/config
    ${DCMTK_DIR}/config/include
    ${DCMTK_DIR}/include
    /usr/local/dicom/include
    /usr/local/include/dcmtk/config
    /usr/include/dcmtk/config
    /usr/local/dicom/include/dcmtk/config
)

FIND_PATH( DCMTK_ofstd_INCLUDE_DIR ofstdinc.h
    ${DCMTK_ROOT_INCLUDE_DIR}/dcmtk/ofstd
    ${DCMTK_DIR}/ofstd/include
    ${DCMTK_DIR}/include/ofstd
    /usr/local/dicom/include/dcmtk/ofstd
    /usr/local/include/dcmtk/ofstd
    /usr/include/dcmtk/ofstd
    /usr/local/dicom/include/dcmtk/ofstd
)

FIND_LIBRARY( DCMTK_ofstd_LIBRARY ofstd
    PATHS
        ${DCMTK_DIR}/ofstd/libsrc
        ${DCMTK_DIR}/ofstd/libsrc/Release
        ${DCMTK_DIR}/ofstd/libsrc/Debug
        ${DCMTK_DIR}/ofstd/Release
        ${DCMTK_DIR}/ofstd/Debug
        ${DCMTK_DIR}/lib
        /usr/local/dicom/lib
        /usr/local/lib64
        /usr/lib64
        /usr/local/lib
        /usr/lib
        /usr/local/dicom/lib
    PATH_SUFFIXES dcmtk
)


FIND_PATH( DCMTK_dcmdata_INCLUDE_DIR dctypes.h
    ${DCMTK_ROOT_INCLUDE_DIR}/dcmtk/dcmdata
    ${DCMTK_DIR}/dcmdata/include
    ${DCMTK_DIR}/include/dcmdata
    /usr/local/dicom/include/dcmtk/dcmdata
    /usr/local/include/dcmtk/dcmdata
    /usr/include/dcmtk/dcmdata
    /usr/local/dicom/include/dcmtk/dcmdata
)

FIND_LIBRARY( DCMTK_dcmdata_LIBRARY dcmdata
    PATHS
        ${DCMTK_DIR}/dcmdata/libsrc
        ${DCMTK_DIR}/dcmdata/libsrc/Release
        ${DCMTK_DIR}/dcmdata/libsrc/Debug
        ${DCMTK_DIR}/dcmdata/Release
        ${DCMTK_DIR}/dcmdata/Debug
        ${DCMTK_DIR}/lib
        /usr/local/dicom/lib
        /usr/local/lib64
        /usr/lib64
        /usr/local/lib
        /usr/lib
        /usr/local/dicom/lib
    PATH_SUFFIXES dcmtk
)


FIND_PATH( DCMTK_dcmimgle_INCLUDE_DIR dcmimage.h
    ${DCMTK_ROOT_INCLUDE_DIR}/dcmtk/dcmimgle
    ${DCMTK_DIR}/dcmimgle/include
    ${DCMTK_DIR}/include/dcmimgle
    /usr/local/dicom/include/dcmtk/dcmimgle
    /usr/local/include/dcmtk/dcmimgle
    /usr/include/dcmtk/dcmimgle
    /usr/local/dicom/include/dcmtk/dcmimgle
)

FIND_LIBRARY( DCMTK_dcmimgle_LIBRARY dcmimgle
    PATHS
        ${DCMTK_DIR}/dcmimgle/libsrc
        ${DCMTK_DIR}/dcmimgle/libsrc/Release
        ${DCMTK_DIR}/dcmimgle/libsrc/Debug
        ${DCMTK_DIR}/dcmimgle/Release
        ${DCMTK_DIR}/dcmimgle/Debug
        ${DCMTK_DIR}/lib
        /usr/local/dicom/lib
        /usr/local/lib64
        /usr/lib64
        /usr/local/lib
        /usr/lib
        /usr/local/dicom/lib
    PATH_SUFFIXES dcmtk
)

FIND_PATH( DCMTK_dcmimage_INCLUDE_DIR diregist.h
    ${DCMTK_ROOT_INCLUDE_DIR}/dcmtk/dcmimage
    ${DCMTK_DIR}/dcmimage/include
    ${DCMTK_DIR}/include/dcmimage
    /usr/local/dicom/include/dcmtk/dcmimage
    /usr/local/include/dcmtk/dcmimage
    /usr/include/dcmtk/dcmimage
    /usr/local/dicom/include/dcmtk/dcmimage
)

FIND_LIBRARY( DCMTK_dcmimage_LIBRARY dcmimage
    PATHS
        ${DCMTK_DIR}/dcmimage/libsrc
        ${DCMTK_DIR}/dcmimage/libsrc/Release
        ${DCMTK_DIR}/dcmimage/libsrc/Debug
        ${DCMTK_DIR}/dcmimage/Release
        ${DCMTK_DIR}/dcmimage/Debug
        ${DCMTK_DIR}/lib
        /usr/local/dicom/lib
        /usr/local/lib64
        /usr/lib64
        /usr/local/lib
        /usr/lib
        /usr/local/dicom/lib
    PATH_SUFFIXES dcmtk
)

FIND_LIBRARY(DCMTK_imagedb_LIBRARY imagedb
    PATHS
        ${DCMTK_DIR}/imagectn/libsrc/Release
        ${DCMTK_DIR}/imagectn/libsrc/
        ${DCMTK_DIR}/imagectn/libsrc/Debug
        ${DCMTK_DIR}/lib
        /usr/local/dicom/lib
        /usr/local/lib64
        /usr/lib64
        /usr/local/lib
        /usr/lib
        /usr/local/dicom/lib
    PATH_SUFFIXES dcmtk
)

FIND_LIBRARY(DCMTK_dcmnet_LIBRARY dcmnet
    PATHS
        ${DCMTK_DIR}/dcmnet/libsrc/Release
        ${DCMTK_DIR}/dcmnet/libsrc/Debug
        ${DCMTK_DIR}/dcmnet/libsrc/
        ${DCMTK_DIR}/lib
        /usr/local/dicom/lib
        /usr/local/lib64
        /usr/lib64
        /usr/local/lib
        /usr/lib
        /usr/local/dicom/lib
    PATH_SUFFIXES dcmtk
)

FIND_LIBRARY(DCMTK_oflog_LIBRARY oflog
    PATHS
        ${DCMTK_DIR}/dcmnet/libsrc/Release
        ${DCMTK_DIR}/dcmnet/libsrc/Debug
        ${DCMTK_DIR}/dcmnet/libsrc/
        ${DCMTK_DIR}/lib
        /usr/local/dicom/lib
        /usr/local/lib64
        /usr/lib64
        /usr/local/lib
        /usr/lib
        /usr/local/dicom/lib
    PATH_SUFFIXES dcmtk
)

FIND_LIBRARY(DCMTK_ofstd_LIBRARY ofstd
    PATHS
        ${DCMTK_DIR}/dcmnet/libsrc/Release
        ${DCMTK_DIR}/dcmnet/libsrc/Debug
        ${DCMTK_DIR}/dcmnet/libsrc/
        ${DCMTK_DIR}/lib
        /usr/local/dicom/lib
        /usr/local/lib64
        /usr/lib64
        /usr/local/lib
        /usr/lib
        /usr/local/dicom/lib
    PATH_SUFFIXES dcmtk
)

IF( DCMTK_config_INCLUDE_DIR
    AND DCMTK_ofstd_INCLUDE_DIR
    AND DCMTK_ofstd_LIBRARY
    AND DCMTK_dcmdata_INCLUDE_DIR
    AND DCMTK_dcmdata_LIBRARY
    AND DCMTK_dcmimgle_INCLUDE_DIR
    AND DCMTK_dcmimgle_LIBRARY
    AND DCMTK_dcmimage_INCLUDE_DIR
    AND DCMTK_dcmimage_LIBRARY )

  SET( DCMTK_FOUND "YES" )
  SET( DCMTK_INCLUDE_DIRS
    ${DCMTK_config_INCLUDE_DIR}
    ${DCMTK_ofstd_INCLUDE_DIR}
    ${DCMTK_dcmdata_INCLUDE_DIR}
    ${DCMTK_dcmimgle_INCLUDE_DIR}
    ${DCMTK_dcmimage_INCLUDE_DIR}
  )

  SET( DCMTK_LIBRARIES
    ${DCMTK_dcmimgle_LIBRARY}
    ${DCMTK_dcmimage_LIBRARY}
    ${DCMTK_dcmdata_LIBRARY}
    ${DCMTK_ofstd_LIBRARY}
    ${DCMTK_config_LIBRARY}
  )

  IF(DCMTK_imagedb_LIBRARY)
   SET( DCMTK_LIBRARIES
   ${DCMTK_LIBRARIES}
   ${DCMTK_imagedb_LIBRARY}
   )
  ENDIF()

  IF(DCMTK_dcmnet_LIBRARY)
   SET( DCMTK_LIBRARIES
   ${DCMTK_LIBRARIES}
   ${DCMTK_dcmnet_LIBRARY}
   )
  ENDIF()

  IF(DCMTK_oflog_LIBRARY)
   SET( DCMTK_LIBRARIES
   ${DCMTK_LIBRARIES}
   ${DCMTK_oflog_LIBRARY}
   )
  ENDIF()

  IF(DCMTK_ofstd_LIBRARY)
   SET( DCMTK_LIBRARIES
   ${DCMTK_LIBRARIES}
   ${DCMTK_ofstd_LIBRARY}
   )
  ENDIF()

  IF( WIN32 )
    SET( DCMTK_LIBRARIES ${DCMTK_LIBRARIES} netapi32 )
  ENDIF()

ENDIF()

IF( NOT DCMTK_FOUND )
  SET( DCMTK_DIR "" CACHE PATH "Root of DCMTK source tree (optional)." )
  MARK_AS_ADVANCED( DCMTK_DIR )
ENDIF()
