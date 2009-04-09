INCLUDE_DIRECTORIES( ${PERFORMER_INCLUDE_DIR} ${PERFORMER_INCLUDE_DIR}/Performer )

SET(TARGET_SRC 
    ReaderWriterPFB.cpp
    ConvertFromPerformer.cpp
)

SET(TARGET_H 
    ConvertFromPerformer.h
)

IF(MSVC)
    SET(TARGET_EXTERNAL_LIBRARIES 
            libpfdu-util
    )
ELSE()
    SET(TARGET_EXTERNAL_LIBRARIES 
            pfdu pfutil
    )
ENDIF()

SET(TARGET_LIBRARIES_VARS PERFORMER_LIBRARY )

#### end var setup  ###
SETUP_PLUGIN(pfb)
