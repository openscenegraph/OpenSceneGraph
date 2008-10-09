SET(TARGET_SRC
    IO_ShapeAttribute.cpp
    IO_BlinkSequence.cpp
    IO_DOFTransform.cpp
    IO_Impostor.cpp
    IO_LightPoint.cpp
    IO_LightPointNode.cpp
    IO_MultiSwitch.cpp
    IO_OverlayNode.cpp
    IO_Sector.cpp
    IO_VisibilityGroup.cpp
    IO_ObjectRecordData.cpp
)
SET(TARGET_H IO_LightPoint.h )
SET(TARGET_ADDED_LIBRARIES osgSim )
#### end var setup  ###
SETUP_PLUGIN(osgsim)
