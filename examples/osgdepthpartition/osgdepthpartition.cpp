//C++ source file - Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

// Simple example of use of Producer::RenderSurface
// The myGraphics class is a simple sample of how one would implement
// graphics drawing with Producer::RenderSurface

#include <osgUtil/UpdateVisitor>

#include <osgDB/ReadFile>

#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>

#include <osgProducer/Viewer>

#include "DepthPartitionNode.h"

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

    osg::Geode* sun_geode = new osg::Geode;
    sun_geode->setName("sun");
    sun_geode->addDrawable(sun_sd);

    // Move the sun behind the earth
    osg::PositionAttitudeTransform *pat = new osg::PositionAttitudeTransform;
    pat->setPosition(osg::Vec3d(0.0, AU, 0.0));

    osg::Group* scene = new osg::Group;
    scene->addChild(earth);
    scene->addChild(pat);
    pat->addChild(sun_geode);

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

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

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


    bool needToSetHomePosition = false;

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles(arguments);

    // if one hasn't been loaded create an earth and sun test model.
    if (!scene) 
    {
        scene = createScene(); 
        needToSetHomePosition = true;
    }
    
    // Create a DepthPartitionNode to manage partitioning of the scene
    osg::ref_ptr<DepthPartitionNode> dpn = new DepthPartitionNode;
    dpn->addChild(scene.get());
    dpn->setActive(true); // Control whether the node analyzes the scene
        
    // pass the loaded scene graph to the viewer.
    viewer.setSceneData(dpn.get());

    if (needToSetHomePosition)
    {
        viewer.getKeySwitchMatrixManipulator()->setHomePosition(osg::Vec3d(0.0,-5.0*r_earth,0.0),osg::Vec3d(0.0,0.0,0.0),osg::Vec3d(0.0,0.0,1.0));
    }

    // create the windows and run the threads.
    viewer.realize();

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return 0;
}

