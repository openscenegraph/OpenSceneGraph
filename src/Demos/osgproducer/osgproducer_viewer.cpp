//C++ source file - Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>


#include <osgUtil/Optimizer>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <osgGA/AnimationPathManipulator>

#include <osgProducer/Viewer>


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    osg::ApplicationUsage::instance()->setCommandLineUsage(arguments.getProgramName()+" [options] filename ...");
    osg::ApplicationUsage::instance()->addCommandLineOption("-h or --help","Display this information");
    osg::ApplicationUsage::instance()->addCommandLineOption("-p <filename>","Specify camera path file to animate the camera through the loaded scene");

    
    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer();

    // if a pathfile has been specified on command line use it to animate the camera via an AnimationPathManipulator.
    std::string pathfile;
    while (arguments.read("-p",pathfile))
    {
	osg::ref_ptr<osgGA::AnimationPathManipulator> apm = new osgGA::AnimationPathManipulator(pathfile);
	if( apm.valid() && apm->valid() ) 
        {
            unsigned int num = viewer.addCameraManipulator(apm.get());
            viewer.selectCameraManipulator(num);
        }
    }


    // if user request help pritn it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        osg::ApplicationUsage::instance()->write(cout);
        return 1;
    }


    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // any option left unread a converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(cout);
        return 1;
    }


    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        std::cout << arguments.getProgramName() <<": No input files" << std::endl;
        return 1;
    }


    // optimize the scene graph, remove rendundent nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());

    // set the scene to render
    viewer.setSceneData(loadedModel.get());

    // create the windows and run the threads.
    viewer.realize(Producer::CameraGroup::ThreadPerCamera);

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
    return 0;
}

