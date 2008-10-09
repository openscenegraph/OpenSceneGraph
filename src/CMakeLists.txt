#the old construct SUBDIRS( was substituded by ADD_SUBDIRECTORY that is to be preferred according on CMake docs.
FOREACH( mylibfolder 
        OpenThreads
        osg
        osgDB 
        osgUtil 
        osgGA 
        osgText 
        osgManipulator
        osgSim
        osgFX
        osgParticle
        osgShadow
        osgTerrain
        osgVolume
        osgViewer
    )

    ADD_SUBDIRECTORY(${mylibfolder})

ENDFOREACH( mylibfolder )

OPTION(BUILD_OSGWIDGET "Enable to build osgWidget" ON)
IF   (BUILD_OSGWIDGET)
    ADD_SUBDIRECTORY(osgWidget)
ENDIF(BUILD_OSGWIDGET)



OPTION(BUILD_OSG_PLUGINS "Enable to build OSG Plugins" ON)
IF   (BUILD_OSG_PLUGINS)
    ADD_SUBDIRECTORY(osgPlugins)
ENDIF(BUILD_OSG_PLUGINS)

OPTION(BUILD_OSG_WRAPPERS "Enable to build Introspection and Wrappers" OFF)
IF   (BUILD_OSG_WRAPPERS)
    ADD_SUBDIRECTORY(osgIntrospection)
    ADD_SUBDIRECTORY(osgWrappers)
ENDIF(BUILD_OSG_WRAPPERS)

IF(MSVC80)
  OPTION(OSG_MSVC_GENERATE_PLUGINS_AND_WRAPPERS_MANIFESTS "Generate or not manifests files under VS8 for dynamically loaded dlls" ON)
ENDIF(MSVC80)

