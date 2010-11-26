SET(TARGET_SRC 
    iphoneViewerAppDelegate.h 
    iphoneViewerAppDelegate.mm
    main.m
    osgPlugins.h
    osgIPhoneViewer-Info.plist
)

#INCLUDE_DIRECTORIES( ${GLUT_INCLUDE_DIR} )
SET(TARGET_LIBRARIES_VARS Foundation.framework QuartzCore.framework UIKit.framework)

#### end var setup  ###
SETUP_EXAMPLE(osgViewerIPhone)
