/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY
{
}
 without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osgViewer/Viewer>
#include <osgUtil/GLObjectsVisitor>
#include <osg/GLExtensions>

#include <osg/io_utils>

using namespace osgViewer;

Viewer::Viewer():
    _firstFrame(true),
    _done(false),
    _threadingModel(ThreadPerContext)
{
}

Viewer::~Viewer()
{
#if 0
    Contexts contexts;
    getContexts(contexts);

    // cancel any graphics threads.
    for(Contexts::iterator citr = contexts.begin();
        citr != contexts.end();
        ++citr)
    {
        osg::GraphicsContext* gc = (*citr);
        gc->setGraphicsThread(0);
    }
    
    if (_scene.valid() && _scene->getDatabasePager())
    {
        _scene->getDatabasePager()->cancel();
        _scene->setDatabasePager(0);
    }
#endif    
}

void Viewer::setThreadingModel(ThreadingModel threadingModel)
{
    _threadingModel = threadingModel;
}

void Viewer::init()
{
    osg::notify(osg::INFO)<<"Viewer::init()"<<std::endl;
    
    osg::ref_ptr<osgGA::GUIEventAdapter> initEvent = _eventQueue->createEvent();
    initEvent->setEventType(osgGA::GUIEventAdapter::FRAME);
    
    if (_cameraManipulator.valid())
    {
        _cameraManipulator->init(*initEvent, *this);
    }
}

void Viewer::getContexts(Contexts& contexts)
{
    typedef std::set<osg::GraphicsContext*> ContextSet;
    ContextSet contextSet;

    if (_camera.valid() && _camera->getGraphicsContext())
    {
        contextSet.insert(_camera->getGraphicsContext());
    }
    
    for(unsigned int i=0; i<getNumSlaves(); ++i)
    {
        Slave& slave = getSlave(i);
        if (slave._camera.valid() && slave._camera->getGraphicsContext())
        {
            contextSet.insert(slave._camera->getGraphicsContext());
        }
    }

    contexts.clear();
    contexts.reserve(contextSet.size());

    for(ContextSet::iterator itr = contextSet.begin();
        itr != contextSet.end();
        ++itr)
    {
        contexts.push_back(const_cast<osg::GraphicsContext*>(*itr));
    }
}

void Viewer::getWindows(Windows& windows)
{
    typedef std::set<osgViewer::GraphicsWindow*> WindowSet;
    WindowSet windowSet;

    if (_camera.valid() && _camera->getGraphicsContext())
    {
        osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(_camera->getGraphicsContext());
        if (gw) windowSet.insert(gw);
    }
    
    for(unsigned int i=0; i<getNumSlaves(); ++i)
    {
        Slave& slave = getSlave(i);
        if (slave._camera.valid() && slave._camera->getGraphicsContext())
        {
            osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(slave._camera->getGraphicsContext());
            if (gw) windowSet.insert(gw);
        }
    }

    windows.clear();
    windows.reserve(windowSet.size());

    for(WindowSet::iterator itr = windowSet.begin();
        itr != windowSet.end();
        ++itr)
    {
        windows.push_back(const_cast<osgViewer::GraphicsWindow*>(*itr));
    }
}


OpenThreads::Mutex mutex;

// Compile operation, that compile OpenGL objects.
struct CompileOperation : public osg::GraphicsOperation
{
    CompileOperation(osg::Node* scene):
        osg::GraphicsOperation("Compile",false),
        _scene(scene)
    {
    }
    
    virtual void operator () (osg::GraphicsContext* context)
    {
        // OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);
        // osg::notify(osg::NOTICE)<<"Compile "<<context<<" "<<OpenThreads::Thread::CurrentThread()<<std::endl;

        // context->makeCurrentImplementation();

        osgUtil::GLObjectsVisitor compileVisitor;
        compileVisitor.setState(context->getState());

        // do the compile traversal
        _scene->accept(compileVisitor);

        // osg::notify(osg::NOTICE)<<"Done Compile "<<context<<" "<<OpenThreads::Thread::CurrentThread()<<std::endl;
    }
    
    osg::ref_ptr<osg::Node> _scene;
};


// Draw operation, that does a draw on the scene graph.
struct RunOperations : public osg::GraphicsOperation
{
    RunOperations(osg::GraphicsContext* gc):
        osg::GraphicsOperation("RunOperation",true),
        _originalContext(gc)
    {
    }
    
    virtual void operator () (osg::GraphicsContext* gc)
    {
        gc->runOperations();
    }
    
    osg::GraphicsContext* _originalContext;
};


void Viewer::realize()
{
    osg::notify(osg::INFO)<<"Viewer::realize()"<<std::endl;

    setCameraWithFocus(0);

    Contexts::iterator citr;

    Contexts contexts;
    getContexts(contexts);

    bool multiThreaded = contexts.size() > 1 && _threadingModel>=ThreadPerContext;
    
    if (multiThreaded)
    {
        bool firstContextAsMainThread = _threadingModel==ThreadPerContext;
        unsigned int numThreadsIncludingMainThread = firstContextAsMainThread ? contexts.size() : contexts.size()+1;
    
        osg::notify(osg::NOTICE)<<"numThreadsIncludingMainThread=="<<numThreadsIncludingMainThread<<std::endl;
    
        _startRenderingBarrier = new osg::BarrierOperation(numThreadsIncludingMainThread, osg::BarrierOperation::NO_OPERATION);
        _endRenderingDispatchBarrier = new osg::BarrierOperation(numThreadsIncludingMainThread, osg::BarrierOperation::NO_OPERATION);
        osg::ref_ptr<osg::SwapBuffersOperation> swapOp = new osg::SwapBuffersOperation();

        citr = contexts.begin(); 
        if (firstContextAsMainThread) ++citr;

        for(;
            citr != contexts.end();
            ++citr)
        {
            osg::GraphicsContext* gc = (*citr);
                        
            // create the a graphics thread for this context
            gc->createGraphicsThread();
            
            gc->getGraphicsThread()->add(new CompileOperation(getSceneData()));
            
            // add the startRenderingBarrier
            gc->getGraphicsThread()->add(_startRenderingBarrier.get());

            // add the rendering operation itself.
            gc->getGraphicsThread()->add(new RunOperations(gc));
            
            // add the endRenderingDispatchBarrier
            gc->getGraphicsThread()->add(_endRenderingDispatchBarrier.get());

            // add the swap buffers
            gc->getGraphicsThread()->add(swapOp.get());

        }
        
    }

    for(citr = contexts.begin();
        citr != contexts.end();
        ++citr)
    {
        (*citr)->realize();
        OpenThreads::Thread::YieldCurrentThread();
    }
    
    bool grabFocus = true;
    if (grabFocus)
    {
        for(citr = contexts.begin();
            citr != contexts.end();
            ++citr)
        {
            osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(*citr);
            if (gw)
            {
                gw->grabFocusIfPointerInWindow();    
            }
        }
    }            

    // initialize the global timer to be relative to the current time.
    osg::Timer::instance()->setStartTick();

    if (multiThreaded)
    {
        for(citr = contexts.begin();
            citr != contexts.end();
            ++citr)
        {
            osg::GraphicsContext* gc = (*citr);
            if (gc->getGraphicsThread() && !gc->getGraphicsThread()->isRunning())
            {
                gc->getGraphicsThread()->startThread();
                OpenThreads::Thread::YieldCurrentThread();
            }
        }
    }
}


void Viewer::frame()
{
    if (_done) return;

    // osg::notify(osg::NOTICE)<<std::endl<<"Viewer::frame()"<<std::endl<<std::endl;

    if (_firstFrame)
    {
        init();
        _firstFrame = false;
    }
    frameAdvance();
    
    frameEventTraversal();
    frameUpdateTraversal();
    frameRenderingTraversals();
}

void Viewer::frameAdvance()
{
    if (_done) return;

    // osg::notify(osg::NOTICE)<<"Viewer::frameAdvance()."<<std::endl;

    _scene->frameAdvance();
}

void Viewer::frameEventTraversal()
{
    if (_done) return;

    // osg::notify(osg::NOTICE)<<"Viewer::frameEventTraversal()."<<std::endl;
    
    // need to copy events from the GraphicsWindow's into local EventQueue;
    osgGA::EventQueue::Events events;

    Contexts contexts;
    getContexts(contexts);

    osgGA::GUIEventAdapter* eventState = getEventQueue()->getCurrentEventState(); 
    osg::Matrix masterCameraVPW = getCamera()->getViewMatrix() * getCamera()->getProjectionMatrix();
    if (getCamera()->getViewport()) 
    {
        osg::Viewport* viewport = getCamera()->getViewport();
        masterCameraVPW *= viewport->computeWindowMatrix();
        eventState->setInputRange( viewport->x(), viewport->y(), viewport->x() + viewport->width(), viewport->y() + viewport->height());
    }
    else
    {
        eventState->setInputRange(-1.0, -1.0, 1.0, 1.0);
    }


    for(Contexts::iterator citr = contexts.begin();
        citr != contexts.end();
        ++citr)
    {
        osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(*citr);
        if (gw)
        {
            gw->checkEvents();
            
            osgGA::EventQueue::Events gw_events;
            gw->getEventQueue()->takeEvents(gw_events);
            
            for(osgGA::EventQueue::Events::iterator itr = gw_events.begin();
                itr != gw_events.end();
                ++itr)
            {
                osgGA::GUIEventAdapter* event = itr->get();
                
                bool pointerEvent = false;

                float x = event->getX();
                float y = event->getY();
                
                bool invert_y = event->getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS;
                if (invert_y) y = gw->getTraits()->height - y;
                
                switch(event->getEventType())
                {
                    case(osgGA::GUIEventAdapter::PUSH):
                    case(osgGA::GUIEventAdapter::RELEASE):
                    case(osgGA::GUIEventAdapter::DRAG):
                    case(osgGA::GUIEventAdapter::MOVE):
                    {
                        pointerEvent = true;
                        
                        if (event->getEventType()!=osgGA::GUIEventAdapter::DRAG || !getCameraWithFocus())
                        {
                            osg::GraphicsContext::Cameras& cameras = gw->getCameras();
                            for(osg::GraphicsContext::Cameras::iterator citr = cameras.begin();
                                citr != cameras.end();
                                ++citr)
                            {
                                osg::Camera* camera = *citr;
                                osg::Viewport* viewport = camera ? camera->getViewport() : 0;
                                if (viewport && 
                                    x >= viewport->x() && y >= viewport->y() &&
                                    x <= (viewport->x()+viewport->width()) && y <= (viewport->y()+viewport->height()) )
                                {
                                    setCameraWithFocus(camera);
                                }
                            }
                        }
                        
                        break;
                    }
                    default:
                        break;
                }
                
                if (pointerEvent)
                {
                    if (getCameraWithFocus())
                    {
                        osg::Viewport* viewport = getCameraWithFocus()->getViewport();
                        osg::Matrix localCameraVPW = getCameraWithFocus()->getViewMatrix() * getCameraWithFocus()->getProjectionMatrix();
                        if (viewport) localCameraVPW *= viewport->computeWindowMatrix();

                        osg::Matrix matrix( osg::Matrix::inverse(localCameraVPW) * masterCameraVPW );

                        osg::Vec3d new_coord = osg::Vec3d(x,y,0.0) * matrix;

                        x = new_coord.x();
                        y = new_coord.y();                                

                        event->setInputRange(eventState->getXmin(), eventState->getYmin(), eventState->getXmax(), eventState->getYmax());
                        event->setX(x);
                        event->setY(y);
                        event->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);

                    }
                    // pass along the new pointer events details to the eventState of the viewer 
                    eventState->setX(x);
                    eventState->setY(y);
                    eventState->setButtonMask(event->getButtonMask());
                    eventState->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);

                }
                else
                {
                    event->setInputRange(eventState->getXmin(), eventState->getYmin(), eventState->getXmax(), eventState->getYmax());
                    event->setX(eventState->getX());
                    event->setY(eventState->getY());
                    event->setButtonMask(eventState->getButtonMask());
                    event->setMouseYOrientation(eventState->getMouseYOrientation());
                }
                //osg::notify(osg::NOTICE)<<"   mouse x = "<<event->getX()<<" y="<<event->getY()<<std::endl;
                // osg::notify(osg::NOTICE)<<"   mouse Xmin = "<<event->getXmin()<<" Ymin="<<event->getYmin()<<" xMax="<<event->getXmax()<<" Ymax="<<event->getYmax()<<std::endl;
            }

            events.insert(events.end(), gw_events.begin(), gw_events.end());

        }
    }
    
#if 0    
    // pointer coordinate transform
    for(osgGA::EventQueue::Events::iterator itr = events.begin();
        itr != events.end();
        ++itr)
    {
        osgGA::GUIEventAdapter* event = itr->get();
        
        if (getEventQueue()->getUseFixedMouseInputRange())
        {
            event->setInputRange(eventState->getXmin(), eventState->getYmin(), eventState->getXmax(), eventState->getYmax());
        }
        
        switch(event->getEventType())
        {
            case(osgGA::GUIEventAdapter::PUSH):
            case(osgGA::GUIEventAdapter::RELEASE):
            case(osgGA::GUIEventAdapter::DRAG):
            case(osgGA::GUIEventAdapter::MOVE):
                eventState->setX(event->getX());
                eventState->setY(event->getY());
                eventState->setButtonMask(event->getButtonMask());
                // osg::notify(osg::NOTICE)<<"   mouse x = "<<event->getX()<<" y="<<event->getY()<<std::endl;
                // osg::notify(osg::NOTICE)<<"   mouse Xmin = "<<event->getXmin()<<" Ymin="<<event->getYmin()<<" xMax="<<event->getXmax()<<" Ymax="<<event->getYmax()<<std::endl;
                break;
            default:
                break;
        }
    }
#else
#endif    

    // osg::notify(osg::NOTICE)<<"mouseEventState Xmin = "<<eventState->getXmin()<<" Ymin="<<eventState->getYmin()<<" xMax="<<eventState->getXmax()<<" Ymax="<<eventState->getYmax()<<std::endl;


    _eventQueue->frame( _scene->getFrameStamp()->getReferenceTime() );
    _eventQueue->takeEvents(events);


#if 0
    // osg::notify(osg::NOTICE)<<"Events "<<events.size()<<std::endl;
    for(osgGA::EventQueue::Events::iterator itr = events.begin();
        itr != events.end();
        ++itr)
    {
        osgGA::GUIEventAdapter* event = itr->get();
        switch(event->getEventType())
        {
            case(osgGA::GUIEventAdapter::PUSH):
                osg::notify(osg::NOTICE)<<"  PUSH "<<event->getButton()<<" x="<<event->getX()<<" y="<<event->getY()<<std::endl;
                break;
            case(osgGA::GUIEventAdapter::RELEASE):
                osg::notify(osg::NOTICE)<<"  RELEASE "<<event->getButton()<<" x="<<event->getX()<<" y="<<event->getY()<<std::endl;
                break;
            case(osgGA::GUIEventAdapter::DRAG):
                osg::notify(osg::NOTICE)<<"  DRAG "<<event->getButtonMask()<<" x="<<event->getX()<<" y="<<event->getY()<<std::endl;
                break;
            case(osgGA::GUIEventAdapter::MOVE):
                osg::notify(osg::NOTICE)<<"  MOVE "<<event->getButtonMask()<<" x="<<event->getX()<<" y="<<event->getY()<<std::endl;
                break;
            case(osgGA::GUIEventAdapter::SCROLL):
                osg::notify(osg::NOTICE)<<"  SCROLL "<<event->getScrollingMotion()<<std::endl;
                break;
            case(osgGA::GUIEventAdapter::KEYDOWN):
                osg::notify(osg::NOTICE)<<"  KEYDOWN '"<<(char)event->getKey()<<"'"<<std::endl;
                break;
            case(osgGA::GUIEventAdapter::KEYUP):
                osg::notify(osg::NOTICE)<<"  KEYUP '"<<(char)event->getKey()<<"'"<<std::endl;
                break;
            case(osgGA::GUIEventAdapter::RESIZE):
                osg::notify(osg::NOTICE)<<"  RESIZE "<<std::endl;
                break;
            case(osgGA::GUIEventAdapter::FRAME):
                // osg::notify(osg::NOTICE)<<"  FRAME "<<std::endl;
                break;
            default:
                // osg::notify(osg::NOTICE)<<"  Event not handled"<<std::endl;
                break;
        }
    }
#endif

    // osg::notify(osg::NOTICE)<<"Events "<<events.size()<<std::endl;
    for(osgGA::EventQueue::Events::iterator itr = events.begin();
        itr != events.end();
        ++itr)
    {
        osgGA::GUIEventAdapter* event = itr->get();
        switch(event->getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYUP):
                if (event->getKey()==osgGA::GUIEventAdapter::KEY_Escape) _done = true;
                break;
            default:
                break;
        }
    }
    
    if (_done) return;

    for(osgGA::EventQueue::Events::iterator itr = events.begin();
        itr != events.end();
        ++itr)
    {
        osgGA::GUIEventAdapter* event = itr->get();

        bool handled = false;
        
        if (_cameraManipulator.valid())
        {
            _cameraManipulator->handle( *event, *this);
        }
        
        for(EventHandlers::iterator hitr = _eventHandlers.begin();
            hitr != _eventHandlers.end() && !handled;
            ++hitr)
        {
            handled = (*hitr)->handle( *event, *this, 0, 0);
        }
    }

}

void Viewer::frameUpdateTraversal()
{
    if (_done) return;

    if (_scene.valid()) _scene->frameUpdateTraversal();

    if (_cameraManipulator.valid())
    {
        _camera->setViewMatrix(_cameraManipulator->getInverseMatrix());
    }

    updateSlaves();
}

void Viewer::frameRenderingTraversals()
{
    if (_done) return;

    osgDB::DatabasePager* dp = _scene->getDatabasePager();
    if (dp)
    {
        dp->signalBeginFrame(_scene->getFrameStamp());
    }

    // osg::notify(osg::NOTICE)<<std::endl<<"Joing _startRenderingBarrier block"<<std::endl;

    Contexts contexts;
    getContexts(contexts);

    // dispatch the the rendering threads
    if (_startRenderingBarrier.valid()) _startRenderingBarrier->block();

    Contexts::iterator itr;
    for(itr = contexts.begin();
        itr != contexts.end();
        ++itr)
    {
        if (_done) return;
        if (!((*itr)->getGraphicsThread()))
        { 
            (*itr)->makeCurrent();
            (*itr)->runOperations();
        }
    }

    // osg::notify(osg::NOTICE)<<"Joing _endRenderingDispatchBarrier block"<<std::endl;

    // wait till the rendering dispatch is done.
    if (_endRenderingDispatchBarrier.valid()) _endRenderingDispatchBarrier->block();

    for(itr = contexts.begin();
        itr != contexts.end();
        ++itr)
    {
        if (_done) return;
        if (!((*itr)->getGraphicsThread()))
        { 
            (*itr)->makeCurrent();
            (*itr)->swapBuffers();
        }
    }

    if (dp)
    {
        dp->signalEndFrame();
    }
}

void Viewer::releaseAllGLObjects()
{
    osg::notify(osg::NOTICE)<<"Viewer::releaseAllGLObjects() not implemented yet."<<std::endl;
}

void Viewer::cleanup()
{
    osg::notify(osg::NOTICE)<<"Viewer::cleanup() not implemented yet."<<std::endl;
}

