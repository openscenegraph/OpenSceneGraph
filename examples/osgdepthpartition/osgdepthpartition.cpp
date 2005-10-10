//C++ source file - Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

// Simple example of use of Producer::RenderSurface
// The myGraphics class is a simple sample of how one would implement
// graphics drawing with Producer::RenderSurface

#include <stdio.h>

#include <Producer/Camera>
#include <Producer/CameraConfig>
#include <Producer/InputArea>
#include <Producer/KeyboardMouse>
#include <Producer/Trackball>

#include <osg/Timer>

#include <osgUtil/UpdateVisitor>

#include <osgDB/ReadFile>

#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>

#include <osgProducer/OsgCameraGroup>
#include <osgProducer/OsgSceneHandler>

#include "DepthPartitionNode.h"

class MyKeyboardMouseCallback : public Producer::KeyboardMouseCallback
{
    public:

	MyKeyboardMouseCallback() :
		Producer::KeyboardMouseCallback(),
		_mx(0.0f),_my(0.0f),_mbutton(0),
		_done(false)	
		{}

	virtual void specialKeyPress( Producer::KeyCharacter key )
	{
		if (key==Producer::KeyChar_Escape)
                    shutdown();
	}

        virtual void shutdown()
        {
            _done = true; 
        }

	virtual void keyPress( Producer::KeyCharacter )
	{
	}

	virtual void mouseMotion( float mx, float my ) 
	{
		_mx = mx;
		_my = my;
	}
	virtual void buttonPress( float mx, float my, unsigned int mbutton ) 
	{
		_mx = mx;
		_my = my;
		_mbutton |= (1<<(mbutton-1));
	}
	virtual void buttonRelease( float mx, float my, unsigned int mbutton ) 
	{
		_mx = mx;
		_my = my;
		_mbutton &= ~(1<<(mbutton-1));
	}

	bool done() { return _done; }
	float mx()  { return _mx; }
	float my()  { return _my; }
	unsigned int mbutton()  { return _mbutton; }

    private:

	float _mx, _my;
	unsigned int _mbutton;
	bool _done;
};

const double r_earth = 6378.137;
const double r_sun = 695990.0;
const double AU = 149697900.0;

osg::Node* createScene()
{
	// Create the Earth, in blue
	osg::ShapeDrawable *earth_sd = new osg::ShapeDrawable;
	osg::Sphere* earth_sphere = new osg::Sphere;
	earth_sphere->setRadius(r_earth);
	earth_sd->setShape(earth_sphere);
	earth_sd->setColor(osg::Vec4(0, 0, 1.0, 1.0));

	osg::Geode* earth = new osg::Geode;
	earth->setName("earth");
	earth->addDrawable(earth_sd);

	// Create the Sun, in yellow
	osg::ShapeDrawable *sun_sd = new osg::ShapeDrawable;
	osg::Sphere* sun_sphere = new osg::Sphere;
	sun_sphere->setRadius(r_sun);
	sun_sd->setShape(sun_sphere);
	sun_sd->setColor(osg::Vec4(1.0, 0.0, 0.0, 1.0));

	osg::Geode* sun = new osg::Geode;
	sun->setName("sun");
	sun->addDrawable(sun_sd);

	// Move the sun behind the earth
	osg::PositionAttitudeTransform *pat = new osg::PositionAttitudeTransform;
	pat->setPosition(osg::Vec3d(0.0, AU, 0.0));

	osg::Group* scene = new osg::Group;
	scene->addChild(earth);
	scene->addChild(pat);
	pat->addChild(sun);

	return scene;
}

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard example of using osgProducer::CameraGroup.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");

    // create the camera group.
    osgProducer::OsgCameraGroup cg(arguments);

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    // set up the keyboard and mouse handling.
    Producer::InputArea *ia = cg.getCameraConfig()->getInputArea();
    Producer::KeyboardMouse *kbm = ia ?
                                   (new Producer::KeyboardMouse(ia)) : 
                                   (new Producer::KeyboardMouse(cg.getCamera(0)->getRenderSurface()));


    osg::ref_ptr<MyKeyboardMouseCallback> kbmcb = new MyKeyboardMouseCallback;
    
    // register the callback with the keyboard mouse manger.
    kbm->setCallback( kbmcb.get() );
    kbm->startThread();

    // Create a DepthPartitionNode to manage partitioning of the scene
    DepthPartitionNode *dpn = new DepthPartitionNode;
    osg::Node* scene = createScene();
    dpn->addChild(scene);
    dpn->setActive(true); // Control whether the node analyzes the scene
        
    // set the scene to render
    cg.setSceneData(dpn);

    // Set the view to be looking at the earth
    osg::BoundingSphere bs;
    bs._radius = r_earth;

    osg::ref_ptr<Producer::Trackball> tb = new Producer::Trackball;
    tb->setOrientation( Producer::Trackball::Z_UP );
    tb->setDistance(bs.radius()*10.0f);
    tb->translate(-bs.center().x(),-bs.center().y(),-bs.center().z());

    // create the windows and run the threads.
    cg.realize();

    osgUtil::UpdateVisitor update;

    while( !kbmcb->done() )
    {
        // syncronize to the when cull and draw threads have completed.
        cg.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        cg.getSceneData()->accept(update);

	tb->input( kbmcb->mx(), kbmcb->my(), kbmcb->mbutton() );

        // update the main producer camera
        cg.setViewByMatrix(tb->getMatrix());
         
        // fire off the cull and draw traversals of the scene.
        cg.frame();
    }

    // syncronize to the when cull and draw threads have completed.
    cg.sync();

    return 0;
}

