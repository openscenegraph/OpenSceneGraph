INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} )
SET(TARGET_EXTERNAL_LIBRARIES ${LIBXML2_LIBRARIES})

SET(TARGET_SRC
    AnimationMaterial.cpp
    Cluster.cpp
    CompileSlideCallback.cpp
    ExportHTML.cpp
    PickEventHandler.cpp
    PointsEventHandler.cpp
    present3D.cpp
    ReaderWriterP3D.cpp
    ReadShowFile.cpp
    ShowEventHandler.cpp
    SlideEventHandler.cpp
    SlideShowConstructor.cpp
)

SET(TARGET_H
    AnimationMaterial.h
    Cluster.h
    CompileSlideCallback.h
    ExportHTML.h
    PickEventHandler.h
    PointsEventHandler.h
    ReadShowFile.h
    ShowEventHandler.h
    SlideEventHandler.h
    SlideShowConstructor.h
)

IF (SDL_FOUND)

    OPTION(BUILD_WITH_SDL "Set to ON to build with SDL for joystick support." OFF)

    IF (BUILD_WITH_SDL)

        SET(TARGET_ADDED_LIBRARIES ${TARGET_ADDED_LIBRARIES} ${SDL_LIBRARY})
        SET(EXTERNAL_INCLUDE_DIRS ${EXTERNAL_INCLUDE_DIRS} ${SDL_INCLUDE_DIR})

        SET(TARGET_SRC
           ${TARGET_SRC}
           SDLIntegration.cpp
        )

        SET(TARGET_H
           ${TARGET_H}
           SDLIntegration.h
        )

        ADD_DEFINITIONS(-DUSE_SDL)

    ENDIF(BUILD_WITH_SDL)

ENDIF(SDL_FOUND)

SET(TARGET_ADDED_LIBRARIES osgVolume osgFX)

SETUP_APPLICATION(present3D-osg)
