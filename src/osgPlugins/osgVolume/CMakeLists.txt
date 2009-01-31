SET(TARGET_SRC 
    Locator.cpp
    ImageLayer.cpp
    Layer.cpp
    Volume.cpp
    VolumeTile.cpp
    RayTracedTechnique.cpp
    FixedFunctionTechnique.cpp
    Property.cpp
    CompositeProperty.cpp
    SwitchProperty.cpp
    ScalarProperty.cpp
    TransferFunctionProperty.cpp
    PropertyAdjustmentCallback.cpp
)

SET(TARGET_ADDED_LIBRARIES osgVolume )

#### end var setup  ###
SETUP_PLUGIN(osgvolume)


