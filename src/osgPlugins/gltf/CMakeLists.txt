SET(TARGET_SRC  ReaderWriterGLTF.cpp)

SET(TARGET_ADDED_LIBRARIES osgAnimation)

SET(TARGET_H
    GLTFReader
    Helpers
    utf8_string
    stb_image.h
    tiny_gltf.h
)

ADD_DEFINITIONS("-Werror")

#### end var setup  ###
SETUP_PLUGIN(gltf)
