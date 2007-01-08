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

#include <osg/GLExtensions>
#include <osgUtil/GLObjectsVisitor>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

#include <osg/io_utils>

using namespace osgViewer;

Viewer::Viewer():
    _firstFrame(true),
    _done(false),
    _keySetsDone(osgGA::GUIEventAdapter::KEY_Escape),
    _threadingModel(ThreadPerContext),
    _numThreadsOnBarrier(0)
{
}

Viewer::~Viewer()
{
    //osg::notify(osg::NOTICE)<<"Viewer::~Viewer()"<<std::endl;

    stopThreading();
    
    if (_scene.valid() && _scene->getDatabasePager())
    {
        _scene->getDatabasePager()->cancel();
        _scene->setDatabasePager(0);
    }

    Contexts contexts;
    getContexts(contexts);

    // clear out all the previously assigned operations
    for(Contexts::iterator citr = contexts.begin();
        citr != contexts.end();
        ++citr)
    {
        (*citr)->close();
    }

    //osg::notify(osg::NOTICE)<<"finish Viewer::~Viewer()"<<std::endl;
    
}

bool Viewer::isRealized() const
{

    Contexts contexts;
    const_cast<Viewer*>(this)->getContexts(contexts);

    unsigned int numRealizedWindows = 0;

    // clear out all the previously assigned operations
    for(Contexts::iterator citr = contexts.begin();
        citr != contexts.end();
        ++citr)
    {
        if ((*citr)->isRealized()) ++numRealizedWindows;
    }
    
    return numRealizedWindows > 0;
}

int Viewer::run()
{
    // if we don't have any scene graph assigned then just return
    if (!getSceneData())
    {
        osg::notify(osg::NOTICE)<<"Warning: Viewer::run() called without a scene graph being assigned to the viewer, cannot run."<<std::endl;
        return 1;
    }

    if (!getCameraManipulator())
    {
        setCameraManipulator(new osgGA::TrackballManipulator());
    }

    if (!isRealized())
    {
        realize();
    }

    while (!done())
    {
        frame();
    }
    
    return 0;
}

osg::FrameStamp*  Viewer::getFrameStamp()
{
    return _scene.valid() ? _scene->getFrameStamp() : 0;
}

const osg::FrameStamp*  Viewer::getFrameStamp() const
{
    return _scene.valid() ? _scene->getFrameStamp() : 0;
}

void Viewer::setThreadingModel(ThreadingModel threadingModel)
{
    if (_threadingModel == threadingModel) return;
    
    if (_threadingModel!=SingleThreaded) stopThreading();
    
    _threadingModel = threadingModel;

    if (_threadingModel!=SingleThreaded) startThreading();
}

void Viewer::stopThreading()
{
    if (_numThreadsOnBarrier==0) return;

    osg::notify(osg::INFO)<<"Viewer::stopThreading() - stopping threading"<<std::endl;

    Contexts contexts;
    getContexts(contexts);

    // delete all the graphics threads.    
    for(Contexts::iterator itr = contexts.begin();
        itr != contexts.end();
        ++itr)
    {
        (*itr)->setGraphicsThread(0);
    }

    _startRenderingBarrier = 0;
    _endRenderingDispatchBarrier = 0;
    _numThreadsOnBarrier = 0;
}

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

unsigned int Viewer::computeNumberOfThreadsIncludingMainRequired()
{
    Contexts contexts;
    getContexts(contexts);

    if (contexts.empty()) return 0;

    if (contexts.size()==1 || _threadingModel==SingleThreaded)
    {
        return 1;
    }

    bool firstContextAsMainThread = _threadingModel == ThreadPerContext;
    
    return firstContextAsMainThread ? contexts.size() : contexts.size()+1;
}


void Viewer::startThreading()
{
    unsigned int numThreadsIncludingMainThread = computeNumberOfThreadsIncludingMainRequired();

    // return if we don't need multiple threads.
    if (numThreadsIncludingMainThread <= 1) return;
    
    // return if threading is already up and running
    if (numThreadsIncludingMainThread == _numThreadsOnBarrier) return;
    
    if (_numThreadsOnBarrier!=0) 
    {
        // we already have threads running but not the right number, so stop them and then create new threads.
        stopThreading();
    }

    osg::notify(osg::INFO)<<"Viewer::startThreading() - starting threading"<<std::endl;

    // using multi-threading so make sure that new objects are allocated with thread safe ref/unref
    osg::Referenced::setThreadSafeReferenceCounting(true);
    
    if (getSceneData())
    {
        osg::notify(osg::INFO)<<"Making scene thread safe"<<std::endl;

        // make sure that existing scene graph objects are allocated with thread safe ref/unref
        getSceneData()->setThreadSafeRefUnref(true);
        
        // update the scene graph so that it has enough GL object buffer memory for the graphics contexts that will be using it.
        getSceneData()->resizeGLObjectBuffers(osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts());
    }


    Contexts contexts;
    getContexts(contexts);

    int numProcessors = OpenThreads::GetNumberOfProcessors();
    bool affinity = true;
    
    bool firstContextAsMainThread = numThreadsIncludingMainThread==contexts.size();

    // osg::notify(osg::NOTICE)<<"numThreadsIncludingMainThread=="<<numThreadsIncludingMainThread<<std::endl;

    _numThreadsOnBarrier = numThreadsIncludingMainThread;
    _startRenderingBarrier = new osg::BarrierOperation(_numThreadsOnBarrier, osg::BarrierOperation::NO_OPERATION);
    _endRenderingDispatchBarrier = new osg::BarrierOperation(_numThreadsOnBarrier, osg::BarrierOperation::NO_OPERATION);
    osg::ref_ptr<osg::SwapBuffersOperation> swapOp = new osg::SwapBuffersOperation();

    Contexts::iterator citr = contexts.begin(); 
    unsigned int processNum = 0;

    if (firstContextAsMainThread)
    {
        if (affinity) OpenThreads::SetProcessorAffinityOfCurrentThread(processNum % numProcessors);
        ++processNum;
        ++citr;
    }

    for(;
        citr != contexts.end();
        ++citr,
        ++processNum)
    {
        osg::GraphicsContext* gc = (*citr);

        // create the a graphics thread for this context
        gc->createGraphicsThread();

        if (affinity) gc->getGraphicsThread()->setProcessorAffinity(processNum % numProcessors);

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


    for(citr = contexts.begin();
        citr != contexts.end();
        ++citr)
    {
        osg::GraphicsContext* gc = (*citr);
        if (gc->getGraphicsThread() && !gc->getGraphicsThread()->isRunning())
        {
            //osg::notify(osg::NOTICE)<<"  gc->getGraphicsThread()->startThread() "<<gc->getGraphicsThread()<<std::endl;
            gc->getGraphicsThread()->startThread();
            // OpenThreads::Thread::YieldCurrentThread();
        }
    }
}

void Viewer::checkWindowStatus()
{
    unsigned int numThreadsIncludingMainThread = computeNumberOfThreadsIncludingMainRequired();
    if (numThreadsIncludingMainThread != _numThreadsOnBarrier)
    {
        stopThreading();
        
        if (numThreadsIncludingMainThread > 1) startThreading();
    }
    
    if (numThreadsIncludingMainThread==0) _done = true;
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

void Viewer::getContexts(Contexts& contexts, bool onlyValid)
{
    typedef std::set<osg::GraphicsContext*> ContextSet;
    ContextSet contextSet;

    if (_camera.valid() && 
        _camera->getGraphicsContext() && 
        (_camera->getGraphicsContext()->valid() || !onlyValid))
    {
        contextSet.insert(_camera->getGraphicsContext());
    }
    
    for(unsigned int i=0; i<getNumSlaves(); ++i)
    {
        Slave& slave = getSlave(i);
        if (slave._camera.valid() && 
            slave._camera->getGraphicsContext() && 
            (slave._camera->getGraphicsContext()->valid() || !onlyValid))
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

void Viewer::getWindows(Windows& windows, bool onlyValid)
{
    windows.clear();

    Contexts contexts;
    getContexts(contexts, onlyValid);
    
    for(Contexts::iterator itr = contexts.begin();
        itr != contexts.end();
        ++itr)
    {
        osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(*itr);
        if (gw) windows.push_back(gw);
    }
}

// Draw operation, that does a draw on the scene graph.
struct RenderingOperation : public osg::GraphicsOperation
{
    RenderingOperation(osgUtil::SceneView* sceneView, osgDB::DatabasePager* databasePager):
        osg::GraphicsOperation("Render",true),
        _sceneView(sceneView),
        _databasePager(databasePager)
    {
        _sceneView->getCullVisitor()->setDatabaseRequestHandler(_databasePager.get());
    }
    
    virtual void operator () (osg::GraphicsContext*)
    {
        if (!_sceneView) return;
    

        // osg::notify(osg::NOTICE)<<"RenderingOperation"<<std::endl;
        
        _sceneView->cull();
        _sceneView->draw();

        if (_databasePager.valid())
        {
            double availableTime = 0.004; // 4 ms
            _databasePager->compileGLObjects(*(_sceneView->getState()), availableTime);
            _sceneView->flushDeletedGLObjects(availableTime);
        }
    }
    
    osg::observer_ptr<osgUtil::SceneView>    _sceneView;
    osg::observer_ptr<osgDB::DatabasePager>  _databasePager;
};

void Viewer::setUpRenderingSupport()
{
    if (!_scene) return;
    
    osg::FrameStamp* frameStamp = _scene->getFrameStamp();

    // what should we do with the old sceneViews?
    _cameraSceneViewMap.clear();

    Contexts contexts;
    getContexts(contexts);

    // clear out all the previously assigned operations
    for(Contexts::iterator citr = contexts.begin();
        citr != contexts.end();
        ++citr)
    {
        (*citr)->removeAllOperations();
    }

    if (_camera.valid() && _camera->getGraphicsContext())
    {
        osgUtil::SceneView* sceneView = new osgUtil::SceneView;
        _cameraSceneViewMap[_camera] = sceneView;

        sceneView->setDefaults();
        sceneView->setCamera(_camera.get());
        sceneView->setState(_camera->getGraphicsContext()->getState());
        sceneView->setSceneData(getSceneData());
        sceneView->setFrameStamp(frameStamp);

        _camera->getGraphicsContext()->add(new RenderingOperation(sceneView, _scene->getDatabasePager()));        
    }
    
    for(unsigned i=0; i<getNumSlaves(); ++i)
    {
        Slave& slave = getSlave(i);
        if (slave._camera.valid() && slave._camera->getGraphicsContext())
        {
            osgUtil::SceneView* sceneView = new osgUtil::SceneView;
            _cameraSceneViewMap[slave._camera] = sceneView;

            sceneView->setDefaults();
            sceneView->setCamera(slave._camera.get());
            sceneView->setState(slave._camera->getGraphicsContext()->getState());
            sceneView->setSceneData(getSceneData());
            sceneView->setFrameStamp(frameStamp);

            slave._camera->getGraphicsContext()->add(new RenderingOperation(sceneView, _scene->getDatabasePager()));
        }
    }
}


void Viewer::realize()
{
    //osg::notify(osg::INFO)<<"Viewer::realize()"<<std::endl;
    
    setCameraWithFocus(0);

    Contexts contexts;
    getContexts(contexts);
    
    if (contexts.empty())
    {
        // no windows are already set up so set up a default view        
        setUpViewAcrossAllScreens();
        getContexts(contexts);
    }

    if (contexts.empty())
    {
        osg::notify(osg::NOTICE)<<"Viewer::realize() - failed to set up any windows"<<std::endl;
        _done = true;
        return;
    }
    
    setUpRenderingSupport();
    
    for(Contexts::iterator citr = contexts.begin();
        citr != contexts.end();
        ++citr)
    {
        (*citr)->realize();
    }
    
    bool grabFocus = true;
    if (grabFocus)
    {
        for(Contexts::iterator citr = contexts.begin();
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
    
    
    startThreading();

    // initialize the global timer to be relative to the current time.
    osg::Timer::instance()->setStartTick();

}


void Viewer::frame()
{
    if (_done) return;

    // osg::notify(osg::NOTICE)<<std::endl<<"Viewer::frame()"<<std::endl<<std::endl;

    if (_firstFrame)
    {
        init();
        
        if (!isRealized())
        {
            realize();
        }
        
        _firstFrame = false;
    }
    advance();
    
    eventTraversal();
    updateTraversal();
    renderingTraversals();
}

void Viewer::advance()
{
    if (_done) return;

    // osg::notify(osg::NOTICE)<<"Viewer::frameAdvance()."<<std::endl;

    _scene->frameAdvance();
}

void Viewer::eventTraversal()
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
            
            osgGA::EventQueue::Events::iterator itr;
            for(itr = gw_events.begin();
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

            for(itr = gw_events.begin();
                itr != gw_events.end();
                ++itr)
            {
                osgGA::GUIEventAdapter* event = itr->get();
                switch(event->getEventType())
                {
                    case(osgGA::GUIEventAdapter::CLOSE_WINDOW):
                    {
                        stopThreading();
                        
                        gw->close();
                        
                        // setThreadingModel(ThreadPerCamera);
                        
                        setThreadingModel(SingleThreaded);
                        
                        startThreading();
                        
                        break;
                    }
                    default:
                        break;
                }
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
    
    if (_keySetsDone!=0)
    {
        for(osgGA::EventQueue::Events::iterator itr = events.begin();
            itr != events.end();
            ++itr)
        {
            osgGA::GUIEventAdapter* event = itr->get();
            switch(event->getEventType())
            {
                case(osgGA::GUIEventAdapter::KEYUP):
                    if (event->getKey()==_keySetsDone) _done = true;
                    else if (event->getKey()=='s') { setThreadingModel(SingleThreaded); }
                    else if (event->getKey()=='c') { setThreadingModel(ThreadPerCamera); }
                    else if (event->getKey()=='w') { setThreadingModel(ThreadPerContext); }
                    break;
                default:
                    break;
            }
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

void Viewer::updateTraversal()
{
    if (_done) return;

    if (_scene.valid()) _scene->frameUpdateTraversal();

    if (_cameraManipulator.valid())
    {
        _camera->setViewMatrix(_cameraManipulator->getInverseMatrix());
    }

    updateSlaves();
}

void Viewer::renderingTraversals()
{
    // check to see if windows are still valid
    checkWindowStatus();

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
            (*itr)->releaseContext();
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
            (*itr)->releaseContext();
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

