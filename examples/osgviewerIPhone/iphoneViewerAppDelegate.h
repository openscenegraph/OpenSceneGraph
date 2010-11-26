//Created by Thomas Hogarth 2009

//force the link to our desired osgPlugins
#include "osgPlugins.h"

#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/CameraNode>
#include <osgText/Text>
#include <osgViewer/Viewer>


#import <UIKit/UIKit.h>


@interface iphoneViewerAppDelegate : NSObject <UIApplicationDelegate, UIAccelerometerDelegate> {
    
    UIWindow* _window; //main application window
    
    UIAccelerationValue        accel[3];
    
    osg::ref_ptr<osgViewer::Viewer> _viewer;
    osg::ref_ptr<osg::MatrixTransform> _root;
    
}

@property (nonatomic, retain) /*IBOutlet*/ UIWindow *_window;

- (void)updateScene;

@end

