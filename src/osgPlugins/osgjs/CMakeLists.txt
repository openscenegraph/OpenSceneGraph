SET(TARGET_SRC
    Animation.cpp
    Base64.cpp
    JSON_Objects.cpp
    ReaderWriterJSON.cpp
    WriteVisitor.cpp)

SET(TARGET_H
    Adaptor
    Animation
    Base64
    CompactBufferVisitor
    JSON_Objects
    json_stream
    utf8_string
    WriteVisitor
)

IF($ENV{SKETCHFAB_BUILD})
    ADD_DEFINITIONS(-DUSE_COMPRESSED_CHANNELS)
ENDIF()

#### end var setup  ###
SET(TARGET_ADDED_LIBRARIES
    osgAnimation
    osgSim)

SETUP_PLUGIN(osgjs)
