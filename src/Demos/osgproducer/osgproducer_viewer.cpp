//C++ source file - Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

#include <osgUtil/Optimizer>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <osgGA/AnimationPathManipulator>

#include <osgProducer/Viewer>

#include "FrameStatsHandler"


int main( int argc, char **argv )
{

    // create the camera group.

    // create the commandline args.
    std::string pathfile;
    std::string configfile;
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) {
	if( std::string(argv[i]) == "-p" ) {
	    if( (i+1) >= argc ) {
		std::cout << "path argument required for -p option."<<std::endl;
	    	return 1;
	    }
	    else
		pathfile = std::string(argv[++i]);
	}
	else 
	if( std::string(argv[i]) == "-c" ) {
	    if( (i+1) >= argc ) {
		std::cout << "path argument required for -c option."<<std::endl;
	    	return 1;
	    }
	    else
		configfile = std::string(argv[++i]);
	}
	else 
	    commandLine.push_back(argv[i]);
    }

    osgProducer::Viewer* viewer = 0;
    if (!configfile.empty()) viewer = new osgProducer::Viewer(configfile);
    else viewer = new osgProducer::Viewer;

    osg::DisplaySettings::instance()->readCommandLine(commandLine);
    osgDB::readCommandLine(commandLine);


    // read the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(commandLine);
    if (!loadedModel) return 1;

    // optimize it, remove rendundent nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());


    // set up the value with sensible defaults.
    viewer->setUpViewer();

    viewer->enableInstrumentation();

    Producer::FrameStatsHandler* fsh = new Producer::FrameStatsHandler;
    viewer->setStatsHandler(fsh);
    viewer->getCamera(0)->addPostDrawCallback(fsh);
    

    if( !pathfile.empty() ) {
	osg::ref_ptr<osgGA::AnimationPathManipulator> apm = new osgGA::AnimationPathManipulator(pathfile);
	if( apm.valid() && apm->valid() ) 
        {
            unsigned int num = viewer->addCameraManipulator(apm.get());
            viewer->selectCameraManipulator(num);
        }
    }

    // set the scene to render
    viewer->setSceneData(loadedModel.get());

    // set up the pthread stack size to large enough to run into problems.
    viewer->setStackSize( 20*1024*1024);

    // create the windows and run the threads.
    viewer->realize(Producer::CameraGroup::ThreadPerCamera);

    while( !viewer->done() )
    {
        // wait for all cull and draw threads to complete.
        viewer->sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer->update();
         
        // fire off the cull and draw traversals of the scene.
        viewer->frame();
        
    }
    return 0;
}

