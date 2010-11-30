SET(TARGET_SRC 
    iphoneViewerAppDelegate.h 
    iphoneViewerAppDelegate.mm
    main.m
    osgPlugins.h
    osgIPhoneViewer-Info.plist
)

#backup setting
SET(TMP_OSG_BUILD_APPLICATION_BUNDLES {$OSG_BUILD_APPLICATION_BUNDLES})
SET(OSG_BUILD_APPLICATION_BUNDLES TRUE)
SETUP_EXAMPLE(osgViewerIPhone)
#restore setting
SET(OSG_BUILD_APPLICATION_BUNDLES {$TMP_OSG_BUILD_APPLICATION_BUNDLES})

SET(CMAKE_EXE_LINKER_FLAGS "-framework Foundation -framework OpenGLES  -framework UIKit")

