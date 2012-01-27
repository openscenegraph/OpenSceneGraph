//Created by Thomas Hogarth 2009

//
//This exampe shows how to render osg into an existing windw. Apple recommends apps only have one window on IPhone so this
// will be your best bet.
//

#import "iphoneViewerAppDelegate.h"
#include <osgGA/TrackballManipulator>
#include <osg/ShapeDrawable>
//inckude the iphone specific windowing stuff
#include <osgViewer/api/IPhone/GraphicsWindowIPhone> 


#define kAccelerometerFrequency		30.0 // Hz
#define kFilteringFactor			0.1

@implementation iphoneViewerAppDelegate

@synthesize _window;


//
//Called once app has finished launching, create the viewer then realize. Can't call viewer->run as will 
//block the final inialization of the windowing system
//
- (void)applicationDidFinishLaunching:(UIApplication *)application {
    
	std::string test_string;
    test_string = "huhu";
    
    //get the screen size
    CGRect lFrame = [[UIScreen mainScreen] bounds];
    unsigned int w = lFrame.size.width;
    unsigned int h = lFrame.size.height;
	
    // create the main window at screen size
    self._window = [[UIWindow alloc] initWithFrame: lFrame]; 
	
    //show window
    [_window makeKeyAndVisible];

	
//create our graphics context directly so we can pass our own window 
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	
	// Init the Windata Variable that holds the handle for the Window to display OSG in.
	osg::ref_ptr<osg::Referenced> windata = new osgViewer::GraphicsWindowIPhone::WindowData(_window);
	
	// Setup the traits parameters
	traits->x = 0;
	traits->y = 0;
	traits->width = w;
	traits->height = h;
	traits->depth = 16; //keep memory down, default is currently 24
	//traits->alpha = 8;
	//traits->stencil = 8;
	traits->windowDecoration = false;
	traits->doubleBuffer = true;
	traits->sharedContext = 0;
	traits->setInheritedWindowPixelFormat = true;
	//traits->windowName = "osgViewer";

	traits->inheritedWindowData = windata;

	// Create the Graphics Context
	osg::ref_ptr<osg::GraphicsContext> graphicsContext = osg::GraphicsContext::createGraphicsContext(traits.get());

	//create the viewer	
	_viewer = new osgViewer::Viewer();
	//if the context was created then attach to our viewer
	if(graphicsContext)
	{
		_viewer->getCamera()->setGraphicsContext(graphicsContext);
		_viewer->getCamera()->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
	}
		
	//create scene and attch to viewer
	
	//create root
	_root = new osg::MatrixTransform();	
	
	//load and attach scene model
	osg::ref_ptr<osg::Node> model = (osgDB::readNodeFile("hog.osg"));
	_root->addChild(model);
    
    osg::Geode* geode = new osg::Geode();
    osg::ShapeDrawable* drawable = new osg::ShapeDrawable(new osg::Box(osg::Vec3(1,1,1), 1));
    geode->addDrawable(drawable);
    _root->addChild(geode);

	
	//create and attach ortho camera for hud text
	osg::ref_ptr<osg::CameraNode> _hudCamera = new osg::CameraNode;
	
    // set the projection matrix
	_hudCamera->setProjectionMatrix(osg::Matrix::ortho2D(0,w,0,h));
	
    // set the view matrix    
    _hudCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    _hudCamera->setViewMatrix(osg::Matrix::identity());
	
    // only clear the depth buffer
    _hudCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
	
    // draw subgraph after main camera view.
    _hudCamera->setRenderOrder(osg::CameraNode::POST_RENDER);
	_root->addChild(_hudCamera.get());
	
	//attcg text to hud
	osg::ref_ptr<osgText::Text> text = new osgText::Text; 
	osg::ref_ptr<osg::Geode> textGeode = new osg::Geode();
	osg::StateSet* stateset = textGeode->getOrCreateStateSet();
	stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

	textGeode->addDrawable( text );
	_hudCamera->addChild(textGeode.get());
    std::string timesFont("arial.ttf");
	osg::Vec3 position = osg::Vec3(w/2.0f, h/2.0f, 0.0f);
    osg::Vec3 delta(0.0f,-120.0f,0.0f);

        
	text->setFont(timesFont);
	text->setCharacterSize(20.0,1.0);
	text->setColor(osg::Vec4(0.8,0.8,0.8,1.0)); 
	text->setPosition(position);
	text->setMaximumHeight(480);
	text->setMaximumWidth(320);
	text->setAlignment(osgText::Text::CENTER_CENTER );  
	text->setText("It's a Hogs life...");
		
		
	_viewer->setSceneData(_root.get());
	_viewer->setCameraManipulator(new osgGA::TrackballManipulator);
	_viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);//SingleThreaded DrawThreadPerContext
	
	//
	//_viewer->realize();
	
	osg::setNotifyLevel(osg::DEBUG_FP);
	
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