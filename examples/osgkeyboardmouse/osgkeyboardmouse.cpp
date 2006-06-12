// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
//
// Simple example of use of Producer::RenderSurface + KeyboardMouseCallback + SceneView
// example that provides the user with control over view position with basic picking.

#include <Producer/RenderSurface>
#include <Producer/KeyboardMouse>
#include <Producer/Trackball>

#include <osg/Timer>
#include <osg/io_utils>

#include <osgUtil/SceneView>
#include <osgUtil/IntersectVisitor>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgGA/EventQueue>
#include <osgGA/EventVisitor>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>

#include <osgFX/Scribe>

// Begining of glue classes to adapter Producer's keyboard mouse events to osgGA's abstraction events.
class MyKeyboardMouseCallback : public Producer::KeyboardMouseCallback
{
public:

    MyKeyboardMouseCallback(osgGA::EventQueue* eventQueue) :
        _done(false),
        _eventQueue(eventQueue)
    {
    }

    virtual void shutdown()
    {
        _done = true; 
    }

    virtual void specialKeyPress( Producer::KeyCharacter key )
    {
        if (key==Producer::KeyChar_Escape)
                    shutdown();

        _eventQueue->keyPress( (osgGA::GUIEventAdapter::KeySymbol) key );
    }

    virtual void specialKeyRelease( Producer::KeyCharacter key )
    {
        _eventQueue->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) key );
    }

    virtual void keyPress( Producer::KeyCharacter key)
    {
        _eventQueue->keyPress( (osgGA::GUIEventAdapter::KeySymbol) key );
    }

    virtual void keyRelease( Producer::KeyCharacter key)
    {
        _eventQueue->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) key );
    }

    virtual void mouseMotion( float mx, float my ) 
    {
        _eventQueue->mouseMotion( mx, my );
    }

    virtual void buttonPress( float mx, float my, unsigned int mbutton ) 
    {
        _eventQueue->mouseButtonPress(mx, my, mbutton);
    }
    
    virtual void buttonRelease( float mx, float my, unsigned int mbutton ) 
    {
        _eventQueue->mouseButtonRelease(mx, my, mbutton);
    }

    bool done() { return _done; }

private:

    bool                                _done;
    osg::ref_ptr<osgGA::EventQueue>     _eventQueue;
};

class MyActionAdapter : public osgGA::GUIActionAdapter, public osg::Referenced
{
public:
    // Override from GUIActionAdapter
    virtual void requestRedraw() {}

    // Override from GUIActionAdapter
    virtual void requestContinuousUpdate(bool =true) {}

    // Override from GUIActionAdapter
    virtual void requestWarpPointer(float ,float ) {}
    
};

// End of glue classes to adapter Producer's keyboard mouse events to osgGA's abstraction events.


int main( int argc, char **argv )
{
    if (argc<2) 
    {
        std::cout << argv[0] <<": requires filename argument." << std::endl;
        return 1;
    }

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(argv[1]);
    if (!loadedModel) 
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return 1;
    }
    
    // create the window to draw to.
    osg::ref_ptr<Producer::RenderSurface> renderSurface = new Producer::RenderSurface;
    renderSurface->setWindowName("osgkeyboardmouse");
    renderSurface->setWindowRectangle(100,100,800,600);
    renderSurface->useBorder(true);
    renderSurface->realize();
    

    // create the view of the scene.
    osg::ref_ptr<osgUtil::SceneView> sceneView = new osgUtil::SceneView;
    sceneView->setDefaults();
    sceneView->setSceneData(loadedModel.get());
    
    // create the event queue, note that Producer has the y axis increase upwards, like OpenGL, and contary to most Windowing toolkits, so
    // we need to construct the event queue so that it knows about this convention.
    osg::ref_ptr<osgGA::EventQueue> eventQueue = new osgGA::EventQueue(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);

    // set up a KeyboardMouse to manage the events comming in from the RenderSurface
    osg::ref_ptr<Producer::KeyboardMouse>  kbm = new Producer::KeyboardMouse(renderSurface.get());

    // create a KeyboardMouseCallback to handle the mouse events within this applications
    osg::ref_ptr<MyKeyboardMouseCallback> kbmcb = new MyKeyboardMouseCallback(eventQueue.get());

    // create a tracball manipulator to move the camera around in response to keyboard/mouse events
    osg::ref_ptr<osgGA::TrackballManipulator> cameraManipulator = new osgGA::TrackballManipulator;

    // keep a list of event handlers to manipulate the application/scene with in response to keyboard/mouse events
    typedef std::list< osg::ref_ptr<osgGA::GUIEventHandler> > EventHandlers;
    EventHandlers eventHandlers;
    
    osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator;
    statesetManipulator->setStateSet(sceneView->getGlobalStateSet());
    eventHandlers.push_back(statesetManipulator.get());
    
    // create an event visitor to pass the events down to the scene graph nodes
    osg::ref_ptr<osgGA::EventVisitor> eventVisitor = new osgGA::EventVisitor;

    // create an action adapter to allow event handlers to request actions from the GUI.
    osg::ref_ptr<MyActionAdapter> actionAdapter = new MyActionAdapter;

    // record the timer tick at the start of rendering.    
    osg::Timer_t start_tick = osg::Timer::instance()->tick();
    
    unsigned int frameNum = 0;

    eventQueue->setStartTick(start_tick);

    // set the mouse input range (note WindowSize name in appropriate here so osgGA::GUIEventAdapter API really needs looking at, Robert Osfield, June 2006).
    // Producer defaults to using non-dimensional units, so we pass this onto osgGA, most windowing toolkits use pixel coords so use the window size instead.
    eventQueue->getCurrentEventState()->setWindowSize(-1.0, -1.0, 1.0, 1.0);


    // home the manipulator.
    osg::ref_ptr<osgGA::GUIEventAdapter> dummyEvent =  eventQueue->createEvent();
    cameraManipulator->setNode(sceneView->getSceneData());
    cameraManipulator->home(*dummyEvent, *actionAdapter);


    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    while( renderSurface->isRealized() && !kbmcb->done())
    {
        // set up the frame stamp for current frame to record the current time and frame number so that animtion code can advance correctly
        osg::FrameStamp* frameStamp = new osg::FrameStamp;
        frameStamp->setReferenceTime(osg::Timer::instance()->delta_s(start_tick,osg::Timer::instance()->tick()));
        frameStamp->setFrameNumber(frameNum++);
        
        // pass frame stamp to the SceneView so that the update, cull and draw traversals all use the same FrameStamp
        sceneView->setFrameStamp(frameStamp);

        // pass any keyboard mouse events onto the local keyboard mouse callback.
        kbm->update( *kbmcb );
        
        // create an event to signal the new frame.
        eventQueue->frame(frameStamp->getReferenceTime());

        // get the event since the last frame.
        osgGA::EventQueue::Events events;
        eventQueue->takeEvents(events);

        if (eventVisitor.valid())
        {
            eventVisitor->setTraversalNumber(frameStamp->getFrameNumber());
        }

        // dispatch the events in order of arrival.
        for(osgGA::EventQueue::Events::iterator event_itr = events.begin();
            event_itr != events.end();
            ++event_itr)
        {
            bool handled = false;

            if (eventVisitor.valid() && sceneView->getSceneData())
            {
                eventVisitor->reset();
                eventVisitor->addEvent(event_itr->get());
                sceneView->getSceneData()->accept(*eventVisitor);
                if (eventVisitor->getEventHandled())
                    handled = true;
            }

            if (cameraManipulator.valid() && !handled)
            {
                /*handled =*/ cameraManipulator->handle(*(*event_itr), *actionAdapter);
            }

            for(EventHandlers::iterator handler_itr=eventHandlers.begin();
                handler_itr!=eventHandlers.end() && !handled;
                ++handler_itr)
            {   
                handled = (*handler_itr)->handle(*(*event_itr),*actionAdapter,0,0);
            }

            // osg::notify(osg::NOTICE)<<"  Handled event "<<(*event_itr)->getTime()<<" "<< handled<<std::endl;

        }


        // update view matrices
        if (cameraManipulator.valid())
        {
            sceneView->setViewMatrix(cameraManipulator->getInverseMatrix());
        }

        // update the viewport dimensions, incase the window has been resized.
        sceneView->setViewport(0,0,renderSurface->getWindowWidth(),renderSurface->getWindowHeight());

        // do the update traversal the scene graph - such as updating animations
        sceneView->update();
        
        // do the cull traversal, collect all objects in the view frustum into a sorted set of rendering bins
        sceneView->cull();
        
        // draw the rendering bins.
        sceneView->draw();

        // Swap Buffers
        renderSurface->swapBuffers();
    }

    return 0;
}

