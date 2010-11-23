SET(TARGET_SRC
    TextNode.cpp
    osgtext3D_orig.cpp
    osgtext3D_test.cpp
    osgtext3D.cpp
)

SET(TARGET_H
    TextNode.h
)

SET(TARGET_ADDED_LIBRARIES osgText )
#### end var setup  ###
SETUP_EXAMPLE(osgtext3D)
