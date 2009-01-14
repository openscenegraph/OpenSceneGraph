SET(TARGET_SRC 
    Locator.cpp
    ImageLayer.cpp
    Layer.cpp
    Volume.cpp
    VolumeTile.cpp
    ShaderTechnique.cpp
    FixedFunctionTechnique.cpp
)

SET(TARGET_ADDED_LIBRARIES osgVolume )

#### end var setup  ###
SETUP_PLUGIN(osgvolume)


