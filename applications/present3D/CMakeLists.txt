SET(TARGET_SRC
    Cluster.cpp
    ExportHTML.cpp
    PointsEventHandler.cpp
    present3D.cpp
    ReadShowFile.cpp
    ShowEventHandler.cpp
)

SET(TARGET_H
    Cluster.h
    ExportHTML.h
    PointsEventHandler.h
    ReadShowFile.h
    ShowEventHandler.h
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

SET(TARGET_ADDED_LIBRARIES osgVolume osgFX osgPresentation)

IF (NOT DYNAMIC_OPENSCENEGRAPH)
    SET(TARGET_ADDED_LIBRARIES
        ${TARGET_ADDED_LIBRARIES}
            osgdb_ive
            osgdb_osg
            osgdb_p3d

            osgdb_freetype
            osgdb_pdf
            osgdb_rgb
            osgdb_jpeg
            osgdb_png

            osgdb_ffmpeg

            osgdb_openflight
            osgdb_obj

            osgdb_curl
    )
ENDIF()

IF   (WIN32)
    # to support cluster code
    SET(TARGET_EXTERNAL_LIBRARIES ${TARGET_EXTERNAL_LIBRARIES} ws2_32)
ENDIF()


SETUP_APPLICATION(present3D)
