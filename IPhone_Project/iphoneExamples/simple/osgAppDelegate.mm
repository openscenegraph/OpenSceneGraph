//Created by Thomas Hogarth 2009

#import "osgAppDelegate.h"
#include <osgGA/TrackballManipulator>
#include <osgGA/MultiTouchTrackballManipulator>
#include <osg/ShapeDrawable>
#include <osg/DisplaySettings>

#include "DebugTouchPointsEventHandler.h"

#define kAccelerometerFrequency		30.0 // Hz
#define kFilteringFactor			0.1

@implementation osgAppDelegate

//
//Called once app has finished launching, create the viewer then realize. Can't call viewer->run as will 
//block the final inialization of the windowing system
//
- (void)applicationDidFinishLaunching:(UIApplication *)application {
    
    osg::setNotifyLevel(osg::DEBUG_INFO);
    
	_root = new osg::MatrixTransform();	
	osg::ref_ptr<osg::Node> model = (osgDB::readNodeFile("skydome.ive"));
	_root->addChild(model);
	
    /*
	osg::Geode* geode = new osg::Geode();
    osg::ShapeDrawable* drawable = new osg::ShapeDrawable(new osg::Box(osg::Vec3(1,1,1), 1));
    geode->addDrawable(drawable);
    _root->addChild(geode);
    */
    
    // try msaa. available for iOS >= 4.0
    osg::DisplaySettings* settings = osg::DisplaySettings::instance();
    settings->setNumMultiSamples(4);
	
    osg::ref_ptr<osg::Camera> hudCamera = new osg::Camera();
    
	DebugTouchPointsEventHandler* touch_handler = new DebugTouchPointsEventHandler(hudCamera);
	{
		unsigned int w(640);
		unsigned int h(480);
		
        
		// set the projection matrix
		hudCamera->setProjectionMatrix(osg::Matrix::ortho2D(0,w,0,h));
		
		// set the view matrix    
		hudCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		hudCamera->setViewMatrix(osg::Matrix::identity());
		
		// only clear the depth buffer
		hudCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
		
		// draw subgraph after main camera view.
		hudCamera->setRenderOrder(osg::Camera::POST_RENDER);
		
		_root->addChild(hudCamera.get());
		hudCamera->addChild(touch_handler->getDebugNode());
		
	}
	_viewer = new osgViewer::Viewer();
	_viewer->addEventHandler(touch_handler);
	_viewer->setSceneData(_root.get());
	_viewer->setCameraManipulator(new osgGA::MultiTouchTrackballManipulator);
	_viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);//SingleThreaded DrawThreadPerContext
	_viewer->realize();
    
    osgViewer::GraphicsWindow* win = dynamic_cast<osgViewer::GraphicsWindow*>(_viewer->getCamera()->getGraphicsContext());
    if (win && hudCamera)
    {
        int l, t, w, h;
        win->getWindowRectangle(l, t, w, h);
        hudCamera->setProjectionMatrix(osg::Matrix::ortho2D(0,w,0,h));
    }
    // render first frame to prevent black frame
	_viewer->frame();
	osg::setNotifyLevel(osg::INFO);
	
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
