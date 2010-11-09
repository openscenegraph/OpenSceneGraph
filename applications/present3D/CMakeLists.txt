SET(TARGET_SRC
    Cluster.cpp
    ExportHTML.cpp
    PointsEventHandler.cpp
    present3D.cpp
    ReadShowFile.cpp
    ShowEventHandler.cpp
    SpellChecker.cpp
)

SET(TARGET_H
    Cluster.h
    ExportHTML.h
    PointsEventHandler.h
    ReadShowFile.h
    ShowEventHandler.h
    SpellChecker.h
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
            osgdb_rgb
            osgdb_openflight
            osgdb_obj
    )

    IF(JPEG_FOUND)
        ADD_DEFINITIONS(-DUSE_JPEG)
        SET(TARGET_ADDED_LIBRARIES ${TARGET_ADDED_LIBRARIES} osgdb_jpeg)
    ENDIF(JPEG_FOUND)

    IF(PNG_FOUND)
        ADD_DEFINITIONS(-DUSE_PNG)
        SET(TARGET_ADDED_LIBRARIES ${TARGET_ADDED_LIBRARIES} osgdb_png)
    ENDIF(PNG_FOUND)

    IF(CURL_FOUND)
        ADD_DEFINITIONS(-DUSE_CURL)
        SET(TARGET_ADDED_LIBRARIES ${TARGET_ADDED_LIBRARIES} osgdb_curl)
    ENDIF(CURL_FOUND)

    IF(FFMPEG_FOUND)
        ADD_DEFINITIONS(-DUSE_FFMPEG)
        SET(TARGET_ADDED_LIBRARIES ${TARGET_ADDED_LIBRARIES} osgdb_ffmpeg)
    ENDIF(FFMPEG_FOUND)

    IF(FREETYPE_FOUND)
        ADD_DEFINITIONS(-DUSE_FREETYPE)
        SET(TARGET_ADDED_LIBRARIES ${TARGET_ADDED_LIBRARIES} osgdb_freetype)
    ENDIF(FREETYPE_FOUND)

    IF(POPPLER_FOUND AND CAIRO_FOUND)
        ADD_DEFINITIONS(-DUSE_POPPLER_CAIRO)
        SET(TARGET_ADDED_LIBRARIES ${TARGET_ADDED_LIBRARIES} osgdb_pdf)
    ENDIF(POPPLER_FOUND AND CAIRO_FOUND)
ENDIF()

IF   (WIN32)
    # to support cluster code
    SET(TARGET_EXTERNAL_LIBRARIES ${TARGET_EXTERNAL_LIBRARIES} ws2_32)
ENDIF()


SETUP_APPLICATION(present3D)
