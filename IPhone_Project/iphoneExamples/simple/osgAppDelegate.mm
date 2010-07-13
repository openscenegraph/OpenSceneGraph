//Created by Thomas Hogarth 2009

#import "osgAppDelegate.h"
#include <osgGA/TrackballManipulator>
#include <osg/ShapeDrawable>

#define kAccelerometerFrequency		30.0 // Hz
#define kFilteringFactor			0.1

@implementation osgAppDelegate

//
//Called once app has finished launching, create the viewer then realize. Can't call viewer->run as will 
//block the final inialization of the windowing system
//
- (void)applicationDidFinishLaunching:(UIApplication *)application {
    
    osg::setNotifyLevel(osg::INFO);
    
	_root = new osg::MatrixTransform();	
	osg::ref_ptr<osg::Node> model = (osgDB::readNodeFile("hog.osg"));
	_root->addChild(model);
	
    osg::Geode* geode = new osg::Geode();
    osg::ShapeDrawable* drawable = new osg::ShapeDrawable(new osg::Box(osg::Vec3(1,1,1), 1));
    geode->addDrawable(drawable);
    _root->addChild(geode);
    
	_viewer = new osgViewer::Viewer();
	_viewer->setSceneData(_root.get());
	_viewer->setCameraManipulator(new osgGA::TrackballManipulator);
	_viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);//SingleThreaded DrawThreadPerContext
	_viewer->realize();
	
	//osg::setNotifyLevel(osg::DEBUG_FP);
	
	[NSTimer scheduledTimerWithTimeInterval:1.0/30.0 target:self selector:@selector(updateScene) userInfo:nil repeats:YES]; 
	
	//Configure and start accelerometer
	[[UIAccelerometer sharedAccelerometer] setUpdateInterval:(1.0 / kAccelerometerFrequency)];
	[[UIAccelerometer sharedAccelerometer] setDelegate:self];
}


//
//Timer called function to update our scene and render the viewer
//
- (void)updateScene {
	_viewer->frame();
}


- (void)applicationWillResignActive:(UIApplication *)application {

}


- (void)applicationDidBecomeActive:(UIApplication *)application {

}


-(void)applicationWillTerminate:(UIApplication *)application{
	_root = NULL;
	_viewer = NULL;
} 

//
//Accelorometer
//
- (void)accelerometer:(UIAccelerometer*)accelerometer didAccelerate:(UIAcceleration*)acceleration
{
	//Use a basic low-pass filter to only keep the gravity in the accelerometer values
	accel[0] = acceleration.x * kFilteringFactor + accel[0] * (1.0 - kFilteringFactor);
	accel[1] = acceleration.y * kFilteringFactor + accel[1] * (1.0 - kFilteringFactor);
	accel[2] = acceleration.z * kFilteringFactor + accel[2] * (1.0 - kFilteringFactor);
}


- (void)dealloc {
	_root = NULL;
	_viewer = NULL;
	[super dealloc];
}

@end
