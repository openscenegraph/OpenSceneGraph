INCLUDE_DIRECTORIES( ${XINE_INCLUDE_DIR} )

SET(TARGET_SRC 
    video_out_rgb.c
    ReaderWriterXine.cpp
)

SET(TARGET_LIBRARIES_VARS XINE_LIBRARY )

#### end var setup  ###
SETUP_PLUGIN(xine)
