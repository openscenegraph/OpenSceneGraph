IF(ANDROID)
    SET(MODULE_USER_STATIC_OR_DYNAMIC ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC})
ENDIF(ANDROID)

IF(MSVC80 OR MSVC90)
  OPTION(OSG_MSVC_GENERATE_PLUGINS_AND_WRAPPERS_MANIFESTS "Generate or not manifests files under VS8 for dynamically loaded dlls" ON)
ENDIF()

#the old construct SUBDIRS( was substituted by ADD_SUBDIRECTORY that is to be preferred according on CMake docs.
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
        osgUI
        osgVolume
        osgShadow
        osgSim
        osgTerrain
        osgWidget
        osgPresentation
        osgWrappers/serializers
        osgPlugins
    )

    ADD_SUBDIRECTORY(${mylibfolder})

ENDFOREACH()

OPTION(BUILD_OSG_DEPRECATED_SERIALIZERS "Build deprecated .osg serializers" ON)

if(BUILD_OSG_DEPRECATED_SERIALIZERS)
    ADD_SUBDIRECTORY(osgWrappers/deprecated-dotosg)
endif()
