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

#include <osg/Timer>

#include <osgUtil/Optimizer>
#include <osgUtil/UpdateVisitor>

#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchCameraManipulator>
#include <osgGA/StateSetManipulator>

#include <osgProducer/OsgCameraGroup>
#include <osgProducer/OsgSceneHandler>
#include <osgProducer/KeyboardMouseCallback>
#include <osgProducer/ActionAdapter>

#include <list>


int main( int argc, char **argv )
{

    // threading model.
    Producer::CameraGroup::ThreadingModel threadingModel = Producer::CameraGroup::SingleThreaded;
    threadingModel = Producer::CameraGroup::ThreadPerCamera;

    // configuration file.
    std::string configFile;
    //configFile = "twoWindows.cfg";

    // set up the database files to read.
    std::vector<std::string> filenameList;
    if (argc>1) filenameList.push_back(argv[1]);
    else filenameList.push_back("cow.osg");



    // create the camera group.
    osgProducer::OsgCameraGroup *cg = configFile.empty() ?
         (new osgProducer::OsgCameraGroup()):
         (new osgProducer::OsgCameraGroup(configFile));

    // set up the maximum number of graphics contexts, before loading the scene graph
    // to ensure that texture objects and display buffers are configured to the correct size.
    osg::DisplaySettings::instance()->setMaxNumberOfGraphicsContexts( cg->getNumberOfCameras() );


    // read the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(filenameList);
    if (!loadedModel) return 1;

    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());


    // set up the keyboard and mouse handling.
    Producer::InputArea *ia = cg->getCameraConfig()->getInputArea();
    Producer::KeyboardMouse *kbm = ia ?
                                   (new Producer::KeyboardMouse(ia)) : 
                                   (new Producer::KeyboardMouse(cg->getCamera(0)->getRenderSurface()));

    // set up the time and frame counter.
    unsigned int frameNumber = 0;
    osg::Timer timer;
    osg::Timer_t start_tick = timer.tick();

    // set the keyboard mouse callback to catch the events from the windows.
    bool done = false;
    osgProducer::KeyboardMouseCallback kbmcb(kbm,done);
    kbmcb.setStartTick(start_tick);
    
    // register the callback with the keyboard mouse manger.
    kbm->setCallback( &kbmcb );
    //kbm->allowContinuousMouseMotionUpdate(true);
    kbm->startThread();



    // set the globa state
    osg::ref_ptr<osg::StateSet> globalStateSet = new osg::StateSet;
    globalStateSet->setGlobalDefaults();
    cg->setGlobalStateSet(globalStateSet.get());
    
    
    // add either a headlight or sun light to the scene.
    osg::LightSource* lightsource = new osg::LightSource;
    osg::Light* light = new osg::Light;
    lightsource->setLight(light);
    lightsource->setReferenceFrame(osg::LightSource::RELATIVE_TO_ABSOLUTE); // headlight.
    lightsource->setLocalStateSetModes(osg::StateAttribute::ON);

    lightsource->addChild(loadedModel.get());
    
    
    // set the scene to render
//    cg->setSceneData(scene.get());
    cg->setSceneData(lightsource);

    // set up the pthread stack size to large enough to run into problems.
    cg->setStackSize( 20*1024*1024);

    // create the windows and run the threads.
    cg->realize(threadingModel);

    osg::ref_ptr<osg::FrameStamp> frameStamp = cg->getFrameStamp();

    osgUtil::UpdateVisitor update;
    update.setFrameStamp(frameStamp.get());



    // create a camera to use with the manipulators.
    osg::ref_ptr<osg::Camera> old_style_osg_camera = new osg::Camera;

    osg::ref_ptr<osgGA::KeySwitchCameraManipulator> keyswitchManipulator = new osgGA::KeySwitchCameraManipulator;
    keyswitchManipulator->addNumberedCameraManipulator(new osgGA::TrackballManipulator);
    keyswitchManipulator->addNumberedCameraManipulator(new osgGA::FlightManipulator);
    keyswitchManipulator->addNumberedCameraManipulator(new osgGA::DriveManipulator);
    
    keyswitchManipulator->setCamera(old_style_osg_camera.get());
    keyswitchManipulator->setNode(loadedModel.get());


    osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator;
    statesetManipulator->setStateSet(globalStateSet.get());

    // create an event handler list, we'll dispatch our event to these..
    typedef std::list< osg::ref_ptr<osgGA::GUIEventHandler> > EventHandlerList;
    EventHandlerList eventHandlerList;
    eventHandlerList.push_back(keyswitchManipulator.get());
    eventHandlerList.push_back(statesetManipulator.get());

    // create a dummy action adapter right now.
    osgProducer::ActionAdapter actionAdapter;

    osg::ref_ptr<osgProducer::EventAdapter> init_event = new osgProducer::EventAdapter;
    init_event->adaptFrame(0.0);
    keyswitchManipulator->getCurrentCameraManipulator()->home(*init_event,actionAdapter);

    while( !done )
    {
        // syncronize to screen refresh.
        cg->sync();

        // set the frame stamp for the new frame.
        double time_since_start = timer.delta_s(start_tick,timer.tick());
        frameStamp->setFrameNumber(frameNumber);
        frameStamp->setReferenceTime(time_since_start);
        
        
        // get the event since the last frame.
        osgProducer::KeyboardMouseCallback::EventQueue queue;
        kbmcb.getEventQueue(queue);
        
        // create an event to signal the new frame.
        osg::ref_ptr<osgProducer::EventAdapter> frame_event = new osgProducer::EventAdapter;
        frame_event->adaptFrame(frameStamp->getReferenceTime());
        queue.push_back(frame_event);

        // dispatch the events in order of arrival.
        for(osgProducer::KeyboardMouseCallback::EventQueue::iterator event_itr=queue.begin();
            event_itr!=queue.end();
            ++event_itr)
        {
            bool handled = false;
            for(EventHandlerList::iterator handler_itr=eventHandlerList.begin();
                handler_itr!=eventHandlerList.end() && !handled;
                ++handler_itr)
            {   
                handled = (*handler_itr)->handle(*(*event_itr),actionAdapter);
            }
        }

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        cg->getSceneData()->accept(update);


        // update the main producer camera
        cg->setView(old_style_osg_camera->getModelViewMatrix().ptr());
         
        // fire off the cull and draw traversals of the scene.
        cg->frame();
        
        // increment the frame number ready for the next frame
        ++frameNumber;
    }
    return 0;
}

