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

    SET(TARGET_ADDED_LIBRARIES ${TARGET_ADDED_LIBRARIES}
        osgdb_deprecated_osg osgdb_deprecated_osgparticle osgdb_deprecated_osganimation
        osgdb_deprecated_osgfx osgdb_deprecated_osgsim osgdb_deprecated_osgtext
        osgdb_deprecated_osgviewer osgdb_deprecated_osgshadow osgdb_deprecated_osgterrain
        osgdb_deprecated_osgvolume osgdb_deprecated_osgwidget
    )
    SET(TARGET_ADDED_LIBRARIES ${TARGET_ADDED_LIBRARIES}
        osgdb_serializers_osg osgdb_serializers_osgparticle osgdb_serializers_osgtext
        osgdb_serializers_osgterrain osgdb_serializers_osganimation osgdb_serializers_osgfx
        osgdb_serializers_osgshadow osgdb_serializers_osgmanipulator osgdb_serializers_osgsim
        osgdb_serializers_osgvolume
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
        # ADD_DEFINITIONS(-DUSE_FFMPEG)
        # SET(TARGET_ADDED_LIBRARIES ${TARGET_ADDED_LIBRARIES} osgdb_ffmpeg)
    ENDIF(FFMPEG_FOUND)

    IF(FREETYPE_FOUND)
        ADD_DEFINITIONS(-DUSE_FREETYPE)
        SET(TARGET_ADDED_LIBRARIES ${TARGET_ADDED_LIBRARIES} osgdb_freetype)
    ENDIF(FREETYPE_FOUND)

    IF(POPPLER_FOUND)
        ADD_DEFINITIONS(-DUSE_POPPLER_CAIRO)
        SET(TARGET_ADDED_LIBRARIES ${TARGET_ADDED_LIBRARIES} osgdb_pdf)
    ENDIF(POPPLER_FOUND)
ENDIF()

IF   (WIN32)
    # to support cluster code
    SET(TARGET_EXTERNAL_LIBRARIES ${TARGET_EXTERNAL_LIBRARIES} ws2_32)
ELSE()
   CHECK_LIBRARY_EXISTS("nsl" "gethostbyname" "" LIB_NSL_HAS_GETHOSTBYNAME)
   IF(LIB_NSL_HAS_GETHOSTBYNAME)
      SET(TARGET_EXTERNAL_LIBRARIES ${TARGET_EXTERNAL_LIBRARIES} nsl)
   ENDIF()
   CHECK_LIBRARY_EXISTS("socket" "socket" "" LIB_SOCKET_HAS_SOCKET)
   IF(LIB_SOCKET_HAS_SOCKET)
      SET(TARGET_EXTERNAL_LIBRARIES ${TARGET_EXTERNAL_LIBRARIES} socket)
   ENDIF()
ENDIF()

SETUP_APPLICATION(present3D)
