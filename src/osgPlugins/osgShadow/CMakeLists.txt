SET(TARGET_NAME osgShadow )
SET(TARGET_SRC 
    ParallelSplitShadowMap.cpp
    ReaderWriterOsgShadow.cpp
    ShadowedScene.cpp
    ShadowMap.cpp
    ShadowTechnique.cpp
    ShadowTexture.cpp
    ShadowVolume.cpp
)
SET(TARGET_ADDED_LIBRARIES osgShadow )

#### end var setup  ###
SETUP_PLUGIN()
