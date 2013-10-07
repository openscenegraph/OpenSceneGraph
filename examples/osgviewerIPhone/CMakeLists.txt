SET(TARGET_SRC
    iphoneViewerAppDelegate.h
    iphoneViewerAppDelegate.mm
    main.m
    osgPlugins.h
    osgIPhoneViewer-Info.plist
)

SET(TARGET_ADDED_LIBRARIES osgdb_osg osgdb_imageio osgdb_avfoundation)

#backup setting
SET(TMP_OSG_BUILD_APPLICATION_BUNDLES {$OSG_BUILD_APPLICATION_BUNDLES})
SET(OSG_BUILD_APPLICATION_BUNDLES TRUE)
SETUP_EXAMPLE(osgViewerIPhone)
SET_TARGET_PROPERTIES(example_osgViewerIPhone PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")

#restore setting
SET(OSG_BUILD_APPLICATION_BUNDLES {$TMP_OSG_BUILD_APPLICATION_BUNDLES})
SET(CMAKE_EXE_LINKER_FLAGS "-framework QuartzCore -framework Foundation -framework OpenGLES  -framework UIKit -framework ImageIO -framework CoreImage  -framework MobileCoreServices -framework CoreGraphics")
