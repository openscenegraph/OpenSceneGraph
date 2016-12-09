INCLUDE_DIRECTORIES(${OPENCASCADE_INCLUDE_DIR})

SET(TARGET_SRC
    ReaderWriterOpenCASCADE.cpp
)

SET(TARGET_H
    ReaderWriterOpenCASCADE.h
)

SET(TARGET_LIBRARIES_VARS OPENCASCADE_LIBRARY)

# requires CMake 3.1 to do the following:
set (CMAKE_CXX_STANDARD 11)

#### end var setup  ###
SETUP_PLUGIN(opencascade)
