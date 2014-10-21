SET(TARGET_H
    ShaderTerrain.h
)

SET(TARGET_SRC
    ShaderTerrain.cpp
    osgterrain.cpp
)

SET(TARGET_ADDED_LIBRARIES osgTerrain )

#### end var setup  ###
SETUP_EXAMPLE(osgterrain)
