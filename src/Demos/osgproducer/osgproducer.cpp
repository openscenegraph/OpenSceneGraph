#include <stdio.h>
//C++ source file - Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

// Simple example of use of Producer::RenderSurface
// The myGraphics class is a simple sample of how one would implement
// graphics drawing with Producer::RenderSurface

#include <Producer/Camera>
#include <Producer/OsgCameraGroup>
#include <Producer/OsgSceneHandler>
#include <Producer/InputArea>
#include <Producer/KeyboardMouse>
#include <Producer/Trackball>

#include <osgUtil/Optimizer>

#include <osgDB/ReadFile>

#include "MyKeyboardMouseCallback"

int main( int argc, char **argv )
{

    std::string filename;
    if (argc>1) filename = argv[1];
    else filename = "cow.osg";

    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFile(filename.c_str());
    if (!scene) return 1;
    
    osgUtil::Optimizer optimizer;
    optimizer.optimize(scene.get());


    Producer::Camera camera1;
    Producer::RenderSurface *rs1 = camera1.getRenderSurface();
    rs1->setWindowRect(10,10,620,480);
    camera1.setOffset( 1.0, 0.0 );
    camera1.setSceneHandler(new Producer::OsgSceneHandler);

    Producer::Camera camera2;
    Producer::RenderSurface *rs2 = camera2.getRenderSurface();
    rs2->setWindowRect(650,10,620,480);
    camera2.setOffset( -1.0, 0.0 );
    camera2.setSceneHandler(new Producer::OsgSceneHandler);
    
    Producer::InputArea *ia = new Producer::InputArea;
    ia->addInputRectangle( rs1, Producer::InputRectangle(0.0,0.5,0.0,1.0));
    ia->addInputRectangle( rs2, Producer::InputRectangle(0.5,1.0,0.0,1.0));

    Producer::CameraConfig cfg;
    cfg.addCamera("Camera 1", &camera1);
    cfg.addCamera("Camera 2", &camera2);

    Producer::OsgCameraGroup cg(&cfg);
        
    cg.setSceneData(scene.get());

    Producer::KeyboardMouse kbm(ia);

    bool done = false;
    MyKeyboardMouseCallback kbmcb(done);
    kbm.setCallback( &kbmcb );
    kbm.startThread();

    Producer::Trackball tb;
    tb.setOrientation( Producer::Trackball::Y_UP );

//    cg.realize(Producer::CameraGroup::ThreadPerCamera);
    cg.realize(Producer::CameraGroup::SingleThreaded);

    while( !done )
    {
        tb.input( kbmcb.mx(), kbmcb.my(), kbmcb.mbutton() );

        cg.setView(tb.getMatrix().ptr());
                
        cg.frame();
    }
    return 0;
}

