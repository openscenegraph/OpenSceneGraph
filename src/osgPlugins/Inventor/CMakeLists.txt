INCLUDE(OsgMacroUtils)

INCLUDE_DIRECTORIES( ${INVENTOR_INCLUDE_DIR} )

# Disable the build of a problem section in ConvertToInventor.cpp
ADD_DEFINITIONS(-DDISABLE_PROBLEM_COMPILE_SECTIONS)

SET(TARGET_SRC
    ConvertToInventor.cpp
    ConvertFromInventor.cpp
    GroupSoLOD.cpp
    PendulumCallback.cpp
    ReaderWriterIV.cpp
    ShuttleCallback.cpp
)
SET(TARGET_HDRS
    ConvertToInventor.h
    ConvertFromInventor.h
    GroupSoLOD.h
    PendulumCallback.h
    ReaderWriterIV.h
    ShuttleCallback.h
)

ADD_DEFINITIONS(-DCOIN_DLL)

SET(TARGET_EXTERNAL_LIBRARIES ${INVENTOR_LIBRARY} )

SETUP_PLUGIN(iv)
