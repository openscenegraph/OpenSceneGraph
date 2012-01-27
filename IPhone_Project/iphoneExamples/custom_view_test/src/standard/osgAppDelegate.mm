//Created by Thomas Hogarth 2009

#import "osgAppDelegate.h"
#include <osgGA/TrackballManipulator>
#include <osg/ShapeDrawable>
#include <osg/Notify>


@implementation osgAppDelegate

//
//Called once app has finished launching, create the viewer then realize. Can't call viewer->run as will 
//block the final inialization of the windowing system
//
- (void)applicationDidFinishLaunching:(UIApplication *)application {

        
    
    osg::setNotifyLevel(osg::INFO);
    
    _sketch = new Sketch();  
    _sketch->prepareLaunch();
	//osg::setNotifyLevel(osg::DEBUG_FP);
	
	[NSTimer scheduledTimerWithTimeInterval:1.0/60.0 target:self selector:@selector(updateScene) userInfo:nil repeats:YES]; 
	
}


//
//Timer called function to update our scene and render the viewer
//
- (void)updateScene {
	_sketch->perFrame();
}



- (void)applicationWillResignActive:(UIApplication *)application {

}


- (void)applicationDidBecomeActive:(UIApplication *)application {

}


-(void)applicationWillTerminate:(UIApplication *)application{
	
} 



- (void)dealloc {
	osg::setNotifyLevel(osg::DEBUG_INFO);
	_sketch->prepareQuit();
	_sketch = NULL;
	[super dealloc];
}

@end
