INCLUDE_DIRECTORIES( ${PERFORMER_INCLUDE_DIR} )

SET(TARGET_SRC 
    ReaderWriterPFB.cpp
    ConvertFromPerformer.cpp
)

SET(TARGET_H 
    ConvertFromPerformer.h
)

SET(TARGET_EXTERNAL_LIBRARIES 
    pfdu pfutil
)

SET(TARGET_LIBRARIES_VARS PERFORMER_LIBRARY )

#### end var setup  ###
SETUP_PLUGIN(pfb)
