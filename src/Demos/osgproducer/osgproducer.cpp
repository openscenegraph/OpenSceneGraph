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

    std::string filename;
    if (argc>1) filename = argv[1];
    else filename = "cow.osg";

    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFile(filename.c_str());
    if (!scene) return 1;
    
    osgUtil::Optimizer optimizer;
    optimizer.optimize(scene.get());

#define USE_BUILD_CONFIG

#ifdef USE_BUILD_CONFIG

    Producer::CameraConfig *cfg = BuildConfig();
    Producer::OsgCameraGroup *cg = new Producer::OsgCameraGroup(cfg);
    
#elif USE_PARSE_CONFIG

    Producer::CameraConfig *cfg = new Producer::CameraConfig;
    cfg->parseFile("twoWindows.cfg");

    Producer::OsgCameraGroup *cg = new Producer::OsgCameraGroup(cfg);
    
#else

    Producer::OsgCameraGroup *cg = new Producer::OsgCameraGroup();
    // ackk we need a CameraConfig to get the InputArea from, as CameraGroup
    // doesn't have its own InputArea...  perhaps there should be a 
    // getOrCreateInputArea on the CameraGroup & CameraConfig classes to help
    // out setting up the input area appropriatly.
    
#endif


    // set up a scene handler for each camera.
    for( int i = 0; i < cg->getNumberOfCameras(); i++ )
        cg->getCamera(i)->setSceneHandler( new Producer::OsgSceneHandler );


    Producer::InputArea *ia = cfg->getInputArea();
    Producer::KeyboardMouse kbm(ia);

    bool done = false;
    MyKeyboardMouseCallback kbmcb(done);
    kbm.setCallback( &kbmcb );
    kbm.startThread();

    Producer::Trackball tb;
    tb.setOrientation( Producer::Trackball::Y_UP );


    cg->setSceneData(scene.get());

//    cg.realize(Producer::CameraGroup::ThreadPerCamera);
    cg->realize(Producer::CameraGroup::SingleThreaded);

    while( !done )
    {
        tb.input( kbmcb.mx(), kbmcb.my(), kbmcb.mbutton() );

        cg->setView(tb.getMatrix().ptr());
                
        cg->frame();
    }
    return 0;
}

