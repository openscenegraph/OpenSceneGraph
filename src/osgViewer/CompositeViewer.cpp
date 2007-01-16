/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osg/GLExtensions>
#include <osgUtil/GLObjectsVisitor>
#include <osgGA/TrackballManipulator>
#include <osgViewer/CompositeViewer>

#include <osg/io_utils>

using namespace osgViewer;

CompositeViewer::CompositeViewer():
    _firstFrame(true),
    _done(false),
    _keyEventSetsDone(osgGA::GUIEventAdapter::KEY_Escape),
    _quitEventSetsDone(true),
    _threadingModel(ThreadPerContext),
    _endBarrierPosition(AfterSwapBuffers),
    _numThreadsOnBarrier(0)
{
    // make sure View is safe to reference multi-threaded.
    setThreadSafeRefUnref(true);

    _frameStamp = new osg::FrameStamp;
    _frameStamp->setFrameNumber(0);
    _frameStamp->setReferenceTime(0);

    setEventQueue(new osgGA::EventQueue);

    _eventVisitor = new osgGA::EventVisitor;
    _eventVisitor->setActionAdapter(this);
}

CompositeViewer::~CompositeViewer()
{
    osg::notify(osg::NOTICE)<<"CompositeViewer::~CompositeViewer()"<<std::endl;

    stopThreading();
    
    Scenes scenes;
    getScenes(scenes);
    
    for(Scenes::iterator sitr = scenes.begin();
        sitr != scenes.end();
        ++sitr)
    {
        Scene* scene = *sitr;
        if (scene->getDatabasePager())
        {
            scene->getDatabasePager()->cancel();
            scene->setDatabasePager(0);
        }
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

    osg::notify(osg::NOTICE)<<"finish CompositeViewer::~CompsiteViewer()"<<std::endl;
}


void CompositeViewer::addView(osgViewer::View* view)
{
    _views.push_back(view);
    
    setUpRenderingSupport();
}

bool CompositeViewer::isRealized() const
{
    Contexts contexts;
    const_cast<CompositeViewer*>(this)->getContexts(contexts);

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

int CompositeViewer::run()
{
    for(Views::iterator itr = _views.begin();
        itr != _views.end();
        ++itr)
    {
        osgViewer::View* view = itr->get();
        if (view->getCameraManipulator()==0)
        {
            view->setCameraManipulator(new osgGA::TrackballManipulator());
        }
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

void CompositeViewer::setStartTick(osg::Timer_t tick)
{
    _startTick = tick;
    
    Contexts contexts;
    getContexts(contexts,false);

    for(Contexts::iterator citr = contexts.begin();
        citr != contexts.end();
        ++citr)
    {
        osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(*citr);
        if (gw)
        {
            gw->getEventQueue()->setStartTick(_startTick);
        }
    }
}


void CompositeViewer::setReferenceTime(double time)
{
    osg::Timer_t tick = osg::Timer::instance()->tick();
    double currentTime = osg::Timer::instance()->delta_s(_startTick, tick);
    double delta_ticks = (time-currentTime)*(osg::Timer::instance()->getSecondsPerTick());
    if (delta_ticks>=0) tick += osg::Timer_t(delta_ticks);
    else tick -= osg::Timer_t(-delta_ticks);

    // assign the new start tick
    setStartTick(tick);
}


void CompositeViewer::setThreadingModel(ThreadingModel threadingModel)
{
    if (_threadingModel == threadingModel) return;
    
    if (_threadingModel!=SingleThreaded) stopThreading();
    
    _threadingModel = threadingModel;

    if (_threadingModel!=SingleThreaded) startThreading();
}


void CompositeViewer::setEndBarrierPosition(BarrierPosition bp)
{
    if (_endBarrierPosition == bp) return;
    
    if (_threadingModel!=SingleThreaded) stopThreading();
    
    _endBarrierPosition = bp;

    if (_threadingModel!=SingleThreaded) startThreading();
}

void CompositeViewer::stopThreading()
{
    if (_numThreadsOnBarrier==0) return;

    osg::notify(osg::INFO)<<"CompositeViewer::stopThreading() - stopping threading"<<std::endl;

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
struct CompositeViewerCompileOperation : public osg::GraphicsOperation
{
    CompositeViewerCompileOperation():
        osg::GraphicsOperation("Compile",false)
    {
        osg::notify(osg::NOTICE)<<"Constructed CompileOperation"<<std::endl;
    }
    
    virtual void operator () (osg::GraphicsContext* context)
    {
        // OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);
        // osg::notify(osg::NOTICE)<<"Compile "<<context<<" "<<OpenThreads::Thread::CurrentThread()<<std::endl;

        // context->makeCurrentImplementation();

        osgUtil::GLObjectsVisitor compileVisitor;
        compileVisitor.setState(context->getState());
        
        for(osg::GraphicsContext::Cameras::iterator itr = context->getCameras().begin();
            itr != context->getCameras().end();
            ++itr)
        {
            (*itr)->accept(compileVisitor);
        }

        // osg::notify(osg::NOTICE)<<"Done Compile "<<context<<" "<<OpenThreads::Thread::CurrentThread()<<std::endl;
    }
};


// Draw operation, that does a draw on the scene graph.
struct CompositeViewerRunOperations : public osg::GraphicsOperation
{
    CompositeViewerRunOperations():
        osg::GraphicsOperation("RunOperation",true)
    {
    }
    
    virtual void operator () (osg::GraphicsContext* gc)
    {
        gc->runOperations();
    }
    
    osg::GraphicsContext* _originalContext;
};

unsigned int CompositeViewer::computeNumberOfThreadsIncludingMainRequired()
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


void CompositeViewer::startThreading()
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

    osg::notify(osg::INFO)<<"CompositeViewer::startThreading() - starting threading"<<std::endl;

    // using multi-threading so make sure that new objects are allocated with thread safe ref/unref
    osg::Referenced::setThreadSafeReferenceCounting(true);
    
    Scenes scenes;
    getScenes(scenes);
    for(Scenes::iterator sitr = scenes.begin();
        sitr != scenes.end();
        ++sitr)
    {
        osg::Node* sceneData = (*sitr)->getSceneData();
        if (sceneData)
        {
            osg::notify(osg::INFO)<<"Making scene thread safe"<<std::endl;

            // make sure that existing scene graph objects are allocated with thread safe ref/unref
            sceneData->setThreadSafeRefUnref(true);

            // update the scene graph so that it has enough GL object buffer memory for the graphics contexts that will be using it.
            sceneData->resizeGLObjectBuffers(osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts());
        }
    }

    Contexts contexts;
    getContexts(contexts);

    int numProcessors = OpenThreads::GetNumberOfProcessors();
    bool affinity = false;
    
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

        gc->getGraphicsThread()->add(new CompositeViewerCompileOperation());

        // add the startRenderingBarrier
        gc->getGraphicsThread()->add(_startRenderingBarrier.get());

        // add the rendering operation itself.
        gc->getGraphicsThread()->add(new CompositeViewerRunOperations());

        if (_endBarrierPosition==BeforeSwapBuffers)
        {
            // add the endRenderingDispatchBarrier
            gc->getGraphicsThread()->add(_endRenderingDispatchBarrier.get());
        }

        // add the swap buffers
        gc->getGraphicsThread()->add(swapOp.get());

        if (_endBarrierPosition==AfterSwapBuffers)
        {
            // add the endRenderingDispatchBarrier
            gc->getGraphicsThread()->add(_endRenderingDispatchBarrier.get());
        }

    }

    if (firstContextAsMainThread)
    {
        if (affinity) OpenThreads::SetProcessorAffinityOfCurrentThread(0);
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

void CompositeViewer::checkWindowStatus()
{
    unsigned int numThreadsIncludingMainThread = computeNumberOfThreadsIncludingMainRequired();
    if (numThreadsIncludingMainThread != _numThreadsOnBarrier)
    {
        stopThreading();
        
        if (numThreadsIncludingMainThread > 1) startThreading();
    }
    
    if (numThreadsIncludingMainThread==0) _done = true;
}



void CompositeViewer::init()
{
    osg::notify(osg::INFO)<<"CompositeViewer::init()"<<std::endl;

    for(Views::iterator itr = _views.begin();
        itr != _views.end();
        ++itr)
    {
        // (*itr)->init();
    }
}

void CompositeViewer::getContexts(Contexts& contexts, bool onlyValid)
{
    typedef std::set<osg::GraphicsContext*> ContextSet;
    ContextSet contextSet;

    for(Views::iterator vitr = _views.begin();
        vitr != _views.end();
        ++vitr)
    {
        osgViewer::View* view = vitr->get();
        if (view->getCamera() && 
            view->getCamera()->getGraphicsContext() && 
            (view->getCamera()->getGraphicsContext()->valid() || !onlyValid))
        {
            contextSet.insert(view->getCamera()->getGraphicsContext());
        }

        for(unsigned int i=0; i<view->getNumSlaves(); ++i)
        {
            View::Slave& slave = view->getSlave(i);
            if (slave._camera.valid() && 
                slave._camera->getGraphicsContext() && 
                (slave._camera->getGraphicsContext()->valid() || !onlyValid))
            {
                contextSet.insert(slave._camera->getGraphicsContext());
            }
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

void CompositeViewer::getWindows(Windows& windows, bool onlyValid)
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

void CompositeViewer::getScenes(Scenes& scenes, bool onlyValid)
{
    typedef std::set<osgViewer::Scene*> SceneSet;
    SceneSet sceneSet;

    for(Views::iterator vitr = _views.begin();
        vitr != _views.end();
        ++vitr)
    {
        osgViewer::View* view = vitr->get();
        if (view->getScene() && (!onlyValid || view->getScene()->getSceneData()))
        {
            sceneSet.insert(view->getScene());
        }
    }

    for(SceneSet::iterator sitr = sceneSet.begin();
        sitr != sceneSet.end();
        ++sitr)
    {
        scenes.push_back(const_cast<osgViewer::Scene*>(*sitr));
    }
}


// Draw operation, that does a draw on the scene graph.
struct CompositeViewerRenderingOperation : public osg::GraphicsOperation
{
    CompositeViewerRenderingOperation(osgUtil::SceneView* sceneView, osgDB::DatabasePager* databasePager):
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

void CompositeViewer::setUpRenderingSupport()
{
    osg::FrameStamp* frameStamp = getFrameStamp();

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



    for(Views::iterator itr = _views.begin();
        itr != _views.end();
        ++itr)
    {
        osgViewer::View* view = itr->get();
        osg::DisplaySettings* ds = view->getDisplaySettings() ? view->getDisplaySettings() : osg::DisplaySettings::instance();
        
        osgDB::DatabasePager* dp = view->getScene() ? view->getScene()->getDatabasePager() : 0;

        if (view->getCamera() && view->getCamera()->getGraphicsContext())
        {
            osgUtil::SceneView* sceneView = new osgUtil::SceneView;
            _cameraSceneViewMap[view->getCamera()] = sceneView;

            sceneView->setGlobalStateSet(view->getCamera()->getStateSet());
            sceneView->setDefaults();
            sceneView->setDisplaySettings(ds);
            sceneView->setCamera(view->getCamera());
            sceneView->setState(view->getCamera()->getGraphicsContext()->getState());
            sceneView->setSceneData(view->getSceneData());
            sceneView->setFrameStamp(frameStamp);

            view->getCamera()->getGraphicsContext()->add(new CompositeViewerRenderingOperation(sceneView, dp));
        }

        for(unsigned i=0; i<view->getNumSlaves(); ++i)
        {
            View::Slave& slave = view->getSlave(i);
            if (slave._camera.valid() && slave._camera->getGraphicsContext())
            {
                osgUtil::SceneView* sceneView = new osgUtil::SceneView;
                _cameraSceneViewMap[slave._camera] = sceneView;

                sceneView->setGlobalStateSet(view->getCamera()->getStateSet());
                sceneView->setDefaults();
                sceneView->setCamera(slave._camera.get());
                sceneView->setDisplaySettings(ds);
                sceneView->setState(slave._camera->getGraphicsContext()->getState());
                sceneView->setSceneData(view->getSceneData());
                sceneView->setFrameStamp(frameStamp);

                slave._camera->getGraphicsContext()->add(new CompositeViewerRenderingOperation(sceneView, dp));
            }
        }

    }

}


void CompositeViewer::realize()
{
    //osg::notify(osg::INFO)<<"CompositeViewer::realize()"<<std::endl;
    
    setCameraWithFocus(0);

    if (_views.empty())
    {
        osg::notify(osg::NOTICE)<<"CompositeViewer::realize() - not views to realize."<<std::endl;
        _done = true;
        return;
    }

    Contexts contexts;
    getContexts(contexts);
    
    if (contexts.empty())
    {
        osg::notify(osg::INFO)<<"CompositeViewer::realize() - No valid contexts found, setting up view across all screens."<<std::endl;
    
        // no windows are already set up so set up a default view        
        _views[0]->setUpViewAcrossAllScreens();
        
        getContexts(contexts);
    }

    if (contexts.empty())
    {
        osg::notify(osg::NOTICE)<<"CompositeViewer::realize() - failed to set up any windows"<<std::endl;
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

    // pass on the start tick to all the associated eventqueues
    setStartTick(osg::Timer::instance()->getStartTick());
}


void CompositeViewer::frame()
{
    if (_done) return;

    // osg::notify(osg::NOTICE)<<std::endl<<"CompositeViewer::frame()"<<std::endl<<std::endl;

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

void CompositeViewer::advance()
{
    if (_done) return;

    _frameStamp->setReferenceTime( osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick()) );
    _frameStamp->setFrameNumber(_frameStamp->getFrameNumber()+1);
}

void CompositeViewer::setCameraWithFocus(osg::Camera* camera)
{
    _cameraWithFocus = camera;

    if (camera)
    {
        for(Views::iterator vitr = _views.begin();
            vitr != _views.end();
            ++vitr)
        {
            View* view = vitr->get();
            if (view->containsCamera(camera)) 
            {
                _viewWithFocus = view;
                return;
            }
        }
    }

    _viewWithFocus = 0;
}

void CompositeViewer::eventTraversal()
{
    if (_done) return;
    
    if (_views.empty()) return;

    // osg::notify(osg::NOTICE)<<"CompositeViewer::frameEventTraversal()."<<std::endl;
    
    // need to copy events from the GraphicsWindow's into local EventQueue;
    osgGA::EventQueue::Events events;

    Contexts contexts;
    getContexts(contexts);

    Scenes scenes;
    getScenes(scenes);

    osgViewer::View* masterView = getViewWithFocus() ? getViewWithFocus() : _views[0].get();
    
    osg::Camera* masterCamera = masterView->getCamera();
    osgGA::GUIEventAdapter* eventState = masterView->getEventQueue()->getCurrentEventState(); 
    osg::Matrix masterCameraVPW = masterCamera->getViewMatrix() * masterCamera->getProjectionMatrix();
    if (masterCamera->getViewport()) 
    {
        osg::Viewport* viewport = masterCamera->getViewport();
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
                if (invert_y && gw->getTraits()) y = gw->getTraits()->height - y;
                
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
                                    if (getViewWithFocus()!=masterView)
                                    {
                                        // need to reset the masterView
                                        masterView = getViewWithFocus();
                                        masterCamera = masterView->getCamera();
                                        eventState = masterView->getEventQueue()->getCurrentEventState(); 
                                        masterCameraVPW = masterCamera->getViewMatrix() * masterCamera->getProjectionMatrix();
                                        if (masterCamera->getViewport()) 
                                        {
                                            osg::Viewport* viewport = masterCamera->getViewport();
                                            masterCameraVPW *= viewport->computeWindowMatrix();
                                            eventState->setInputRange( viewport->x(), viewport->y(), viewport->x() + viewport->width(), viewport->y() + viewport->height());
                                        }
                                        else
                                        {
                                            eventState->setInputRange(-1.0, -1.0, 1.0, 1.0);
                                        }
                                    }
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
                        gw->close();
                        break;
                    }
                    default:
                        break;
                }
            }

            events.insert(events.end(), gw_events.begin(), gw_events.end());

        }
    }
    

    // osg::notify(osg::NOTICE)<<"mouseEventState Xmin = "<<eventState->getXmin()<<" Ymin="<<eventState->getYmin()<<" xMax="<<eventState->getXmax()<<" Ymax="<<eventState->getYmax()<<std::endl;


    _eventQueue->getCurrentEventState()->setInputRange(eventState->getXmin(), eventState->getYmin(), eventState->getXmax(), eventState->getYmax());
    _eventQueue->getCurrentEventState()->setX(eventState->getX());
    _eventQueue->getCurrentEventState()->setY(eventState->getY());
    _eventQueue->getCurrentEventState()->setButtonMask(eventState->getButtonMask());
    _eventQueue->getCurrentEventState()->setMouseYOrientation(eventState->getMouseYOrientation());

    _eventQueue->frame( getFrameStamp()->getReferenceTime() );
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
                osg::notify(osg::NOTICE)<<"  RESIZE "<<event->getWindowX()<<"/"<<event->getWindowY()<<" x "<<event->getWindowWidth()<<"/"<<event->getWindowHeight() << std::endl;
                break;
            case(osgGA::GUIEventAdapter::QUIT_APPLICATION):
                osg::notify(osg::NOTICE)<<"  QUIT_APPLICATION " << std::endl;
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
    
    if ((_keyEventSetsDone!=0) || _quitEventSetsDone)
    {
        for(osgGA::EventQueue::Events::iterator itr = events.begin();
            itr != events.end();
            ++itr)
        {
            osgGA::GUIEventAdapter* event = itr->get();
            switch(event->getEventType())
            {
                case(osgGA::GUIEventAdapter::KEYUP):
                    if (_keyEventSetsDone && event->getKey()==_keyEventSetsDone) _done = true;
                    break;
                
                case(osgGA::GUIEventAdapter::QUIT_APPLICATION):
                    if (_quitEventSetsDone) _done = true;
                    break;
                    
                default:
                    break;
            }
        }
    }
        
    if (_done) return;

    if (getViewWithFocus())
    {
        View* view = getViewWithFocus();
        for(osgGA::EventQueue::Events::iterator itr = events.begin();
            itr != events.end();
            ++itr)
        {
            osgGA::GUIEventAdapter* event = itr->get();

            if (event->getEventType()!=osgGA::GUIEventAdapter::FRAME)
            {
                bool handled = false;

                if (view->getCameraManipulator())
                {
                    view->getCameraManipulator()->handle( *event, *this);
                }
#if 0
                for(EventHandlers::iterator hitr = view->getEventHandlers().begin();
                    hitr != view->getEventHandlers().end() && !handled;
                    ++hitr)
                {
                    handled = (*hitr)->handle( *event, *this, 0, 0);
                }
#endif
            }
        }
    }
    
    for(Views::iterator vitr = _views.begin();
        vitr != _views.end();
        ++vitr)
    {
        View* view = vitr->get();
        for(osgGA::EventQueue::Events::iterator itr = events.begin();
            itr != events.end();
            ++itr)
        {
            osgGA::GUIEventAdapter* event = itr->get();

            if (event->getEventType()==osgGA::GUIEventAdapter::FRAME)
            {
                bool handled = false;

                if (view->getCameraManipulator())
                {
                    view->getCameraManipulator()->handle( *event, *this);
                }
#if 0
                for(EventHandlers::iterator hitr = view->getEventHandlers().begin();
                    hitr != view->getEventHandlers().end() && !handled;
                    ++hitr)
                {
                    handled = (*hitr)->handle( *event, *this, 0, 0);
                }
#endif
            }
        }
    }

#if 0
    if (_eventVisitor.valid() && _scene.valid())
    {
        _eventVisitor->setFrameStamp(getFrameStamp());
        _eventVisitor->setTraversalNumber(getFrameStamp()->getFrameNumber());

        for(osgGA::EventQueue::Events::iterator itr = events.begin();
            itr != events.end();
            ++itr)
        {
            osgGA::GUIEventAdapter* event = itr->get();

            bool handled = false;

            _eventVisitor->reset();
            _eventVisitor->addEvent( event );

            getSceneData()->accept(*_eventVisitor);

            if (_eventVisitor->getEventHandled())  handled = true;
        }
    }
#endif

}

void CompositeViewer::updateTraversal()
{
    if (_done) return;

    Scenes scenes;
    getScenes(scenes);
    
    for(Scenes::iterator sitr = scenes.begin();
        sitr != scenes.end();
        ++sitr)
    {
        (*sitr)->frameUpdateTraversal();
    }


    for(Views::iterator vitr = _views.begin();
        vitr != _views.end();
        ++vitr)
    {
        View* view = vitr->get();
        view->getCamera()->setViewMatrix( view->getCameraManipulator()->getInverseMatrix());
        view->updateSlaves();
    }

}

void CompositeViewer::renderingTraversals()
{
    // check to see if windows are still valid
    checkWindowStatus();

    if (_done) return;

    Scenes scenes;
    getScenes(scenes);
    
    for(Scenes::iterator sitr = scenes.begin();
        sitr != scenes.end();
        ++sitr)
    {
        Scene* scene = *sitr;
        osgDB::DatabasePager* dp = scene ? scene->getDatabasePager() : 0;
        if (dp)
        {
            dp->signalBeginFrame(getFrameStamp());
        }
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

    for(Scenes::iterator sitr = scenes.begin();
        sitr != scenes.end();
        ++sitr)
    {
        Scene* scene = *sitr;
        osgDB::DatabasePager* dp = scene ? scene->getDatabasePager() : 0;
        if (dp)
        {
            dp->signalEndFrame();
        }
    }
}

void CompositeViewer::requestRedraw()
{
}

void CompositeViewer::requestContinuousUpdate(bool)
{
}

void CompositeViewer::requestWarpPointer(float x,float y)
{
}
