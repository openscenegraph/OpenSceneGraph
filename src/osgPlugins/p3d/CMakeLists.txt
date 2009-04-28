INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} )
SET(TARGET_EXTERNAL_LIBRARIES ${LIBXML2_LIBRARIES})

SET(TARGET_SRC
    SlideShowConstructor.cpp
    ReaderWriterP3D.cpp
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

SET(TARGET_ADDED_LIBRARIES osgVolume osgFX osgViewer)

#### end var setup  ###
SETUP_PLUGIN(p3d)
