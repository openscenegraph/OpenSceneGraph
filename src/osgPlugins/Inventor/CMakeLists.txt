INCLUDE(OsgMacroUtils)

INCLUDE_DIRECTORIES( ${INVENTOR_INCLUDE_DIR} )


SET(TARGET_SRC
    ConvertFromInventor.cpp
    GroupSoLOD.cpp
    PendulumCallback.cpp
    ReaderWriterIV.cpp
    ShuttleCallback.cpp
    )
SET(TARGET_HDRS
    ConvertFromInventor.h
    GroupSoLOD.h
    PendulumCallback.h
    ReaderWriterIV.h
    ShuttleCallback.h
    )

SET(TARGET_EXTERNAL_LIBRARIES ${INVENTOR_LIBRARY} )

SETUP_PLUGIN(iv)
