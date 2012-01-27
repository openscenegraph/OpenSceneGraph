//Created by Thomas Hogarth 2009

//force the link to our desired osgPlugins
#include "osgPlugins.h"

#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osgViewer/Viewer>


#import <UIKit/UIKit.h>


@interface osgAppDelegate : NSObject <UIApplicationDelegate, UIAccelerometerDelegate> {

	UIAccelerationValue		accel[3];
	
	osg::ref_ptr<osgViewer::Viewer> _viewer;
	osg::ref_ptr<osg::MatrixTransform> _root;

}

- (void)updateScene;

@end

