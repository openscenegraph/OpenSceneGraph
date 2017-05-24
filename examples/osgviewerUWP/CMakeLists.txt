
SET(PLATFORM STORE)

SET(TARGET_SRC
    App.h
    App.cpp
    pch.h
	osgPlugins.h
	shaders.h
)
SET(TARGET_ADDED_LIBRARIES osgdb_osg osgdb_rgb osgdb_dds)

SET(TARGET_ADDED_LIBRARIES ${TARGET_ADDED_LIBRARIES}
    osgdb_deprecated_osg osgdb_deprecated_osgparticle osgdb_deprecated_osganimation
    osgdb_deprecated_osgfx osgdb_deprecated_osgsim osgdb_deprecated_osgtext
    osgdb_deprecated_osgviewer osgdb_deprecated_osgshadow osgdb_deprecated_osgterrain
    osgdb_deprecated_osgvolume
)
SET(TARGET_ADDED_LIBRARIES ${TARGET_ADDED_LIBRARIES}
    osgdb_serializers_osg osgdb_serializers_osgparticle osgdb_serializers_osgtext
    osgdb_serializers_osgterrain osgdb_serializers_osganimation osgdb_serializers_osgfx
    osgdb_serializers_osgshadow osgdb_serializers_osgmanipulator osgdb_serializers_osgsim
    osgdb_serializers_osgvolume
)

SETUP_EXAMPLE(osgviewerUWP)

SET_PROPERTY(TARGET example_osgviewerUWP PROPERTY VS_WINRT_COMPONENT TRUE)
