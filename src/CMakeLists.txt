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

IF (QT4_FOUND AND QT_QTWEBKIT_FOUND)
    ADD_SUBDIRECTORY(osgQt)
ENDIF()

OPTION(BUILD_OSG_WRAPPERS "Enable to build Introspection and Wrappers" OFF)
IF(BUILD_OSG_WRAPPERS)
    ADD_SUBDIRECTORY(osgIntrospection)
    ADD_SUBDIRECTORY(osgWrappers/introspection)
ENDIF()

IF(MSVC80 OR MSVC90)
  OPTION(OSG_MSVC_GENERATE_PLUGINS_AND_WRAPPERS_MANIFESTS "Generate or not manifests files under VS8 for dynamically loaded dlls" ON)
ENDIF()

