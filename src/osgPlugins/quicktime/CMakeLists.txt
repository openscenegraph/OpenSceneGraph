IF(WIN32)
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT")
ENDIF()

INCLUDE_DIRECTORIES(${QUICKTIME_INCLUDE_DIR})


SET(TARGET_SRC
    MovieData.cpp
    QTUtils.cpp
    QTLiveUtils.cpp
    QTImportExport.cpp
    QuicktimeImageStream.cpp
    QuicktimeLiveImageStream.cpp
    ReaderWriterQT.cpp
)

SET(TARGET_H 
    MovieData.h
    QTUtils.h
    QTLiveUtils.h
    QTImportExport.h
    QuicktimeImageStream.h
    QuicktimeLiveImageStream.h
)

SET(TARGET_ADDED_LIBRARIES)
SET(TARGET_LIBRARIES_VARS QUICKTIME_LIBRARY )
#### end var setup  ###
SETUP_PLUGIN(qt)
