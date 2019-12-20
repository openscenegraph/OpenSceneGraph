IF  (DCMTK_FOUND)

    IF (DCMTK_ROOT_INCLUDE_DIR)
        # osg used to have a FindOurDCMTK.cmake until 2017/7/26 (3.5.6); it defined this DCMTK_ROOT_INCLUDE_DIR
        # keep using this if it's provided on command line
        
        # note, we have to include a '/' in front of the directory string to prevent a CMake bug from ignoring the directory
        INCLUDE_DIRECTORIES(${DCMTK_ROOT_INCLUDE_DIR})
    ELSE (DCMTK_ROOT_INCLUDE_DIR)
        IF (DCMTK_INCLUDE_DIRS)
            #CMake 3.4.1 and up define this in FindDCMTK.cmake
            INCLUDE_DIRECTORIES(${DCMTK_INCLUDE_DIRS})
        ELSE (DCMTK_INCLUDE_DIRS)
            #final fall-back:
            #CMake 2.1.0 22/4/2004 until now (2019) have DCMTK_DIR
            INCLUDE_DIRECTORIES(${DCMTK_DIR}/include)
        ENDIF (DCMTK_INCLUDE_DIRS)
    ENDIF (DCMTK_ROOT_INCLUDE_DIR)
    

    SET(TARGET_SRC ReaderWriterDICOM.cpp )

    LINK_LIBRARIES(${DCMTK_LIBRARIES} ${ZLIB_LIBRARIES})

    ADD_DEFINITIONS(-DUSE_DCMTK)

    IF (WIN32)
        SET(TARGET_EXTERNAL_LIBRARIES wsock32.lib ws2_32.lib)
    ENDIF()

ELSEIF(ITK_FOUND)

    INCLUDE(${ITK_USE_FILE})

    SET(TARGET_SRC ReaderWriterDICOM.cpp )

    LINK_LIBRARIES(ITKCommon ITKIO)

    ADD_DEFINITIONS(-DUSE_ITK)
    ADD_DEFINITIONS(-DVCL_CAN_STATIC_CONST_INIT_FLOAT=0)

ENDIF()

SET(TARGET_ADDED_LIBRARIES osgVolume )

IF(CMAKE_COMPILER_IS_GNUCXX)
    # Remove -pedantic flag as it barfs on ffmpeg headers
   STRING(REGEX REPLACE "-pedantic" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
ENDIF()

#### end var setup  ###
SETUP_PLUGIN(dicom dicom)
