IF(ANDROID)
    SET(MODULE_USER_STATIC_OR_DYNAMIC ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC})
ENDIF(ANDROID)

IF(MSVC80 OR MSVC90)
  OPTION(OSG_MSVC_GENERATE_PLUGINS_AND_WRAPPERS_MANIFESTS "Generate or not manifests files under VS8 for dynamically loaded dlls" ON)
ENDIF()

#the old construct SUBDIRS( was substituded by ADD_SUBDIRECTORY that is to be preferred according on CMake docs.
FOREACH( mylibfolder 
        OpenThreads
        osg
        osgDB 
        osgUtil 
        osgGA 
        osgText 
        osgViewer
        osgAnimation 
        osgFX
        osgManipulator
        osgParticle
        osgPresentation
        osgShadow
        osgSim
        osgTerrain
        osgWidget
        osgVolume
        osgWrappers/serializers
        osgWrappers/deprecated-dotosg
        osgPlugins
    )

    ADD_SUBDIRECTORY(${mylibfolder})

ENDFOREACH()

IF ((QT4_FOUND OR Qt5Widgets_FOUND) AND NOT ANDROID)
    ADD_SUBDIRECTORY(osgQt)
ENDIF()

IF(ANDROID)
    configure_file("${OSG_ANDROID_TEMPLATES}/Android.mk.src.in" "${CMAKE_CURRENT_BINARY_DIR}/Android.mk")
ENDIF()

