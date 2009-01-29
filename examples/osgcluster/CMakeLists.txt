SET(TARGET_SRC
    broadcaster.cpp
    receiver.cpp
    osgcluster.cpp
)

IF   (WIN32)
   SET(TARGET_EXTERNAL_LIBRARIES ws2_32)
ENDIF(WIN32)

#### end var setup  ###
SETUP_EXAMPLE(osgcluster)
