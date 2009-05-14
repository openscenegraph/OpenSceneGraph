SET(TARGET_SRC
    SlideShowConstructor.cpp
    ReaderWriterP3D.cpp
    ReaderWriterPaths.cpp
    PickEventHandler.cpp
    AnimationMaterial.cpp
    SlideEventHandler.cpp
    CompileSlideCallback.cpp
)
SET(TARGET_H
    SlideShowConstructor.h
    PickEventHandler.h
    AnimationMaterial.h
    SlideEventHandler.h
    CompileSlideCallback.h
)

SET(TARGET_ADDED_LIBRARIES osgGA osgText osgVolume osgFX osgViewer)

#### end var setup  ###
SETUP_PLUGIN(p3d)
