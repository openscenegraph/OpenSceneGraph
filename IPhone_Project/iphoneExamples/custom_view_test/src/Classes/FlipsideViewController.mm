//
//  FlipsideViewController.m
//  custom_view_test
//
//  Created by Stephan Huber on 04.05.11.
//  Copyright 2011 Stephan Maximilian Huber, digital mind. All rights reserved.
//

#import "FlipsideViewController.h"
#include <iostream>
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osgGA/MultiTouchTrackballManipulator>
#include <osgViewer/api/IOS/GraphicsWindowIOS>


@implementation FlipsideViewController

@synthesize delegate;

-(void)initOsg {
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	CGRect lFrame = [self.view bounds];
	unsigned int w = lFrame.size.width;
	unsigned int h = lFrame.size.height;
	
	osg::ref_ptr<osg::Referenced> windata = new osgViewer::GraphicsWindowIOS::WindowData(self.view, osgViewer::GraphicsWindowIOS::WindowData::IGNORE_ORIENTATION);
	
	// Setup the traits parameters
	traits->x = 0;
	traits->y = 40;
	traits->width = w;
	traits->height = h - traits->y;
	traits->depth = 16; //keep memory down, default is currently 24
	//traits->alpha = 8;
	//traits->stencil = 8;
	traits->windowDecoration = false;
	traits->doubleBuffer = true;
	traits->sharedContext = 0;
	traits->setInheritedWindowPixelFormat = true;
	
	traits->inheritedWindowData = windata;

	// Create the Graphics Context
	osg::ref_ptr<osg::GraphicsContext> graphicsContext = osg::GraphicsContext::createGraphicsContext(traits.get());
	
	osg::ShapeDrawable* drawable = new osg::ShapeDrawable(new osg::Box());
	osg::Geode* geode = new osg::Geode();
	geode->addDrawable(drawable);
	
	_viewer = new osgViewer::Viewer();
	
	if(graphicsContext)
	{
		_viewer->getCamera()->setGraphicsContext(graphicsContext);
		_viewer->getCamera()->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
	}
	
	_viewer->setSceneData(geode);
	_viewer->setCameraManipulator(new osgGA::MultiTouchTrackballManipulator());
	
	_viewer->realize();
	
	// get the created view
	osgViewer::GraphicsWindowIOS* window_ios = dynamic_cast<osgViewer::GraphicsWindowIOS*>(graphicsContext.get());
	
	NSLog(@"view: %@ bound: %@", window_ios->getView() , [window_ios->getView() bounds]);
	
	// draw a frame
	_viewer->frame();
}

- (id)initWithNibName:(NSString *)nibName bundle:(NSBundle *)nibBundle
{
	std::cout << "initWithNibName" << std::endl;
	
	if (self = [super initWithNibName:nibName bundle: nibBundle]) 
	{
		
	}
	
	return self;
}


- (void)viewDidLoad {
	std::cout << "viewDidLoad" << std::endl;
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor viewFlipsideBackgroundColor];
    
	[self initOsg];
	
	_displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(updateScene:)];
	[_displayLink setFrameInterval:1];
	[_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];  
} 

//
//Timer called function to update our scene and render the viewer
//
- (void)updateScene: (CADisplayLink *)sender {
	//std::cout << "updateScene" << std::endl;
	_viewer->frame();
}

- (IBAction)done:(id)sender {
	std::cout << "done" << std::endl;
	[self.delegate flipsideViewControllerDidFinish:self];
	
	
	[_displayLink invalidate];	
}


- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}


- (void)viewDidUnload {
	std::cout << "viewDidUnload" << std::endl;
    
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	// Return YES for supported orientations
	return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/


- (void)dealloc {
	
	std::cout << "dealloc" << std::endl;
	
	_displayLink = nil;
    _viewer = NULL;
	
    [super dealloc];
}


@end

USE_GRAPICSWINDOW_IMPLEMENTATION(IOS)
