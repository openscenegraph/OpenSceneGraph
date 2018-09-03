SET(TARGET_SRC
    iphoneViewerAppDelegate.h
    iphoneViewerAppDelegate.mm
    main.m
    osgPlugins.h
    shaders.h
    osgIPhoneViewer-Info.plist
)

SET(TARGET_ADDED_LIBRARIES osgdb_osg osgdb_rgb osgdb_imageio osgdb_avfoundation)

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

#backup setting
SET(TMP_OSG_BUILD_APPLICATION_BUNDLES {$OSG_BUILD_APPLICATION_BUNDLES})
SET(OSG_BUILD_APPLICATION_BUNDLES TRUE)
#SET(MACOSX_DEPLOYMENT_TARGET, ${IPHONE_VERSION_MIN})
SETUP_EXAMPLE(osgViewerIPhone)
SET_TARGET_PROPERTIES(example_osgViewerIPhone PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
                                                        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${IPHONE_VERSION_MIN}
                                                        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")

#restore setting
SET(OSG_BUILD_APPLICATION_BUNDLES {$TMP_OSG_BUILD_APPLICATION_BUNDLES})
SET(CMAKE_EXE_LINKER_FLAGS "-framework QuartzCore -framework Foundation -framework UIKit -framework ImageIO -framework CoreImage -framework MobileCoreServices -framework CoreGraphics")
