
SUBDIRS(
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
	osgViewer
)

OPTION(BUILD_OSG_PLUGINS "Enable to build OSG Plugins" ON)
IF   (BUILD_OSG_PLUGINS)
	ADD_SUBDIRECTORY(osgPlugins)
ENDIF(BUILD_OSG_PLUGINS)

OPTION(BUILD_OSG_WRAPPERS "Enable to build Introspection and Wrappers" OFF)
IF   (BUILD_OSG_WRAPPERS)
	ADD_SUBDIRECTORY(osgIntrospection)
	ADD_SUBDIRECTORY(osgWrappers)
ENDIF(BUILD_OSG_WRAPPERS)
