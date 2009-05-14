SET(TARGET_SRC
    AnimationMaterial.cpp
    Cluster.cpp
    CompileSlideCallback.cpp
    ExportHTML.cpp
    PickEventHandler.cpp
    PointsEventHandler.cpp
    present3D.cpp
    ReaderWriterP3D.cpp
    ReaderWriterPaths.cpp
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

    OPTION(BUILD_PRESENT3D_WITH_SDL "Set to ON to build Present3D with SDL for joystick support." OFF)

    IF (BUILD_PRESENT3D_WITH_SDL)

        SET(TARGET_EXTERNAL_LIBRARIES ${SDL_LIBRARY} )
        INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} )

        SET(TARGET_SRC
           ${TARGET_SRC}
           SDLIntegration.cpp
        )

        SET(TARGET_H
           ${TARGET_H}
           SDLIntegration.h
        )

        ADD_DEFINITIONS(-DUSE_SDL)

    ENDIF()

ENDIF()

SET(TARGET_ADDED_LIBRARIES osgVolume osgFX)

IF   (WIN32)
   SET(TARGET_EXTERNAL_LIBRARIES ${TARGET_EXTERNAL_LIBRARIES} ws2_32)
ENDIF()

SETUP_APPLICATION(present3D-osg)
