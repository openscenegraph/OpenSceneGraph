IF(WIN32)
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT")
ENDIF(WIN32)

INCLUDE_DIRECTORIES(${QUICKTIME_INCLUDE_DIR})


SET(TARGET_SRC
    MovieData.cpp
    QTUtils.cpp
    QTtexture.cpp
    QuicktimeImageStream.cpp
    ReaderWriterQT.cpp
)

SET(TARGET_H 
    MovieData.h
    QTUtils.h
    QTtexture.h
    QuicktimeImageStream.h
)

SET(TARGET_ADDED_LIBRARIES)
SET(TARGET_LIBRARIES_VARS QUICKTIME_LIBRARY )
#### end var setup  ###
SETUP_PLUGIN(qt)
