#include <stdio.h>
//C++ source file - Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

// Simple example of use of Producer::RenderSurface
// The myGraphics class is a simple sample of how one would implement
// graphics drawing with Producer::RenderSurface

#include <Producer/Camera>
#include <Producer/CameraConfig>
#include <Producer/OsgCameraGroup>
#include <Producer/OsgSceneHandler>
#include <Producer/InputArea>
#include <Producer/KeyboardMouse>
#include <Producer/Trackball>

#include <osgUtil/Optimizer>

#include <osgDB/ReadFile>

#include "MyKeyboardMouseCallback"

static Producer::CameraConfig *BuildConfig(void)
{
    Producer::RenderSurface *rs1 = new Producer::RenderSurface;
    rs1->setScreenNum(0);
//     rs1->useBorder(false);
//     rs1->setWindowRect(0,0,640,480);
    rs1->setWindowRect(10,10,620,480);

    Producer::Camera *camera1 = new Producer::Camera;
    camera1->setRenderSurface(rs1);
    camera1->setOffset( 1.0, 0.0 );


    Producer::RenderSurface *rs2 = new Producer::RenderSurface;
    rs2->setScreenNum(0);
//     rs2->useBorder(false);
//     rs2->setWindowRect(640,0,640,480);
    rs2->setWindowRect(650,10,620,480);

    Producer::Camera *camera2 = new Producer::Camera;
    camera2->setRenderSurface(rs2);
    camera2->setOffset( -1.0, 0.0 );

    Producer::CameraConfig *cfg = new Producer::CameraConfig;
    cfg->addCamera("Camera 1",camera1);
    cfg->addCamera("Camera 2", camera2);

    Producer::InputArea *ia = new Producer::InputArea;
    ia->addInputRectangle( rs1, Producer::InputRectangle(0.0,0.5,0.0,1.0));
    ia->addInputRectangle( rs2, Producer::InputRectangle(0.5,1.0,0.0,1.0));

    cfg->setInputArea(ia);

    return cfg;
}

int main( int argc, char **argv )
{

    // threading model.
    Producer::CameraGroup::ThreadingModel threadingModel = Producer::CameraGroup::SingleThreaded;
    threadingModel = Producer::CameraGroup::ThreadPerCamera;

    // configuration file.
    std::string configFile; // configFile = "twoWindows.cfg"

    // set up the database files to read.
    std::vector<std::string> filenameList;
    if (argc>1) filenameList.push_back(argv[1]);
    else filenameList.push_back("cow.osg");



    // create the camera group.
    Producer::OsgCameraGroup *cg = 0;

//#define USE_BUILD_CONFIG
#ifdef USE_BUILD_CONFIG

    Producer::CameraConfig *cfg = BuildConfig();
    cg = new Producer::OsgCameraGroup(cfg);
    
#else

    cg = configFile.empty() ?
         (new Producer::OsgCameraGroup()):
         (new Producer::OsgCameraGroup(configFile));

#endif

    // set up the maximum number of graphics contexts, before loading the scene graph
    // to ensure that texture objects and display buffers are configured to the correct size.
    osg::DisplaySettings::instance()->setMaxNumberOfGraphicsContexts( cg->getNumberOfCameras() );


    // read the scene.
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles(filenameList);
    if (!scene) return 1;

    osgUtil::Optimizer optimizer;
    optimizer.optimize(scene.get());


    // set up the keyboard and mouse handling.
    Producer::InputArea *ia = cg->getCameraConfig()->getInputArea();
    Producer::KeyboardMouse *kbm = ia ?
                                   (new Producer::KeyboardMouse(ia)) : 
                                   (new Producer::KeyboardMouse(cg->getCamera(0)->getRenderSurface()));

    bool done = false;
    MyKeyboardMouseCallback kbmcb(done);
    kbm->setCallback( &kbmcb );
    kbm->startThread();

    Producer::Trackball tb;
    tb.setOrientation( Producer::Trackball::Y_UP );



    // set the globa state
    osg::StateSet* globalStateSet = new osg::StateSet;
    globalStateSet->setGlobalDefaults();
    cg->setGlobalStateSet(globalStateSet);
    
    // set the scene to render
    cg->setSceneData(scene.get());

    // set up the pthread stack size to large enough to run into problems.
    cg->setStackSize( 20*1024*1024);

    // create the windows and run the threads.
    cg->realize(threadingModel);

    while( !done )
    {
        tb.input( kbmcb.mx(), kbmcb.my(), kbmcb.mbutton() );

        cg->setView(tb.getMatrix().ptr());
                
        cg->frame();
    }
    return 0;
}

