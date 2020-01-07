IF  (DCMTK_FOUND)

    # note, we have to include a '/' in front of the directory string to prevent a CMake bug from ignoring the directory
    INCLUDE_DIRECTORIES(${DCMTK_INCLUDE_DIRS})

    SET(TARGET_SRC ReaderWriterDICOM.cpp )

    LINK_LIBRARIES(${DCMTK_LIBRARIES})

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
