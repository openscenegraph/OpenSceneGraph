//C++ source file - Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgProducer/Viewer>

int main( int argc, char **argv )
{

    // create the camera group.
    osgProducer::Viewer viewer(argc,argv);

    // set up the database files to read.
    std::vector<std::string> filenameList;
    if (argc>1) filenameList.push_back(argv[1]);
    else filenameList.push_back("cow.osg");


    // read the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(filenameList);
    if (!loadedModel) return 1;

    // optimize it, remove rendundent nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());


    // set up the value with sensible defaults.
    viewer.setUpViewer();

    // set the scene to render
    viewer.setSceneData(loadedModel.get());

    // set up the pthread stack size to large enough to run into problems.
    viewer.setStackSize( 20*1024*1024);

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

