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
#include <osgDB/Registry>

#include <osg/io_utils>

using namespace osgViewer;

CompositeViewer::CompositeViewer()
{
    constructorInit();
}

CompositeViewer::CompositeViewer(osg::ArgumentParser& arguments)
{
    constructorInit();
    
    while (arguments.read("--SingleThreaded")) setThreadingModel(SingleThreaded);
    while (arguments.read("--ThreadPerContext")) setThreadingModel(ThreadPerContext);

    osg::DisplaySettings::instance()->readCommandLine(arguments);
    osgDB::readCommandLine(arguments);
}

void CompositeViewer::constructorInit()
{
    _firstFrame = true;
    _done = false;
    _keyEventSetsDone = osgGA::GUIEventAdapter::KEY_Escape;
    _quitEventSetsDone = true;
    _threadingModel = ThreadPerContext;
    _threadsRunning = false;
    _endBarrierPosition = AfterSwapBuffers;
    _numThreadsOnBarrier = 0;
    _startTick = 0;

    // make sure View is safe to reference multi-threaded.
    setThreadSafeRefUnref(true);

    _frameStamp = new osg::FrameStamp;
    _frameStamp->setFrameNumber(0);
    _frameStamp->setReferenceTime(0);
    _frameStamp->setSimulationTime(0);

    setEventQueue(new osgGA::EventQueue);

    _eventVisitor = new osgGA::EventVisitor;
}

CompositeViewer::~CompositeViewer()
{
    osg::notify(osg::INFO)<<"CompositeViewer::~CompositeViewer()"<<std::endl;

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

    osg::notify(osg::INFO)<<"finished CompositeViewer::~CompsiteViewer()"<<std::endl;
}


void CompositeViewer::addView(osgViewer::View* view)
{
    bool threadsWereRuinning = _threadsRunning;
    if (threadsWereRuinning) stopThreading();

    _views.push_back(view);
    
    setUpRenderingSupport();
    if (threadsWereRuinning) startThreading();
}

void CompositeViewer::removeView(osgViewer::View* view)
{
    for(Views::iterator itr = _views.begin();
        itr != _views.end();
        ++itr)
    {
        if (*itr == view)
        {
            bool threadsWereRuinning = _threadsRunning;
            if (threadsWereRuinning) stopThreading();

            _views.erase(itr);

            setUpRenderingSupport();
            if (threadsWereRuinning) startThreading();

            return;
        }
    }
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
        if ((view->getCameraManipulator()==0) && view->getCamera()->getAllowEventFocus())
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

    if (isRealized() && _threadingModel!=SingleThreaded) startThreading();
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
    if (!_threadsRunning) return;

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

// Draw operation, that does a draw on the scene graph.
struct CompositeViewerRunOperations : public osg::Operation
{
    CompositeViewerRunOperations():
        osg::Operation("RunOperation",true)
    {
    }
    
    virtual void operator () (osg::Object* object)
    {
        osg::GraphicsContext* context = dynamic_cast<osg::GraphicsContext*>(object);
        if (!context) return;

        context->runOperations();
    }
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
    if (_threadsRunning) return;

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

        gc->getGraphicsThread()->add(new osgUtil::GLObjectsOperation());

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

    _threadsRunning = true;
}

void CompositeViewer::checkWindowStatus()
{
    Contexts contexts;
    getContexts(contexts);
    
    // osg::notify(osg::INFO)<<"Viewer::checkWindowStatus() - "<<contexts.size()<<std::endl;
    
    if (contexts.size()==0)
    {
        _done = true;
        if (areThreadsRunning()) stopThreading();
    }
}



void CompositeViewer::init()
{
    osg::notify(osg::INFO)<<"CompositeViewer::init()"<<std::endl;

    for(Views::iterator itr = _views.begin();
        itr != _views.end();
        ++itr)
    {
        (*itr)->init();
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
struct CompositeViewerRenderingOperation : public osg::Operation
{
    CompositeViewerRenderingOperation(osgUtil::SceneView* sceneView, osgDB::DatabasePager* databasePager):
        osg::Operation("Render",true),
        _sceneView(sceneView),
        _databasePager(databasePager)
    {
        _sceneView->getCullVisitor()->setDatabaseRequestHandler(_databasePager.get());
        
        _flushOperation = new osg::FlushDeletedGLObjectsOperation(0.1);
    }
    
    virtual void operator () (osg::Object*)
    {
        if (!_sceneView) return;
    

        // osg::notify(osg::NOTICE)<<"RenderingOperation"<<std::endl;
        
        // pass on the fusion distance settings from the View to the SceneView
        osgViewer::View* view = dynamic_cast<osgViewer::View*>(_sceneView->getCamera()->getView());
        if (view) _sceneView->setFusionDistance(view->getFusionDistanceMode(), view->getFusionDistanceValue());

        osg::GraphicsContext* compileContext = osg::GraphicsContext::getCompileContext(_sceneView->getState()->getContextID());
        osg::GraphicsThread* compileThread = compileContext ? compileContext->getGraphicsThread() : 0;

        _sceneView->inheritCullSettings(*(_sceneView->getCamera()));
        _sceneView->cull();
        _sceneView->draw();

        double availableTime = 0.004; // 4 ms

        if (_databasePager.valid() && _databasePager->requiresExternalCompileGLObjects(_sceneView->getState()->getContextID()))
        {
            _databasePager->compileGLObjects(*(_sceneView->getState()), availableTime);
        }

        if (compileThread)
        {
            compileThread->add(_flushOperation.get());
        }
        else
        {
            _sceneView->flushDeletedGLObjects(availableTime);
        }
    }
    
    osg::observer_ptr<osgUtil::SceneView>               _sceneView;
    osg::observer_ptr<osgDB::DatabasePager>             _databasePager;
    osg::ref_ptr<osg::FlushDeletedGLObjectsOperation>   _flushOperation;
};

void CompositeViewer::setUpRenderingSupport()
{
#if 1
    _cameraSceneViewMap.clear();

    Contexts contexts;
    getContexts(contexts);
    
    osg::FrameStamp* frameStamp = getFrameStamp();

    for(Contexts::iterator gcitr = contexts.begin();
        gcitr != contexts.end();
        ++gcitr)
    {
        (*gcitr)->removeAllOperations();

        osg::GraphicsContext* gc = *gcitr;
        osg::GraphicsContext::Cameras& cameras = gc->getCameras();
        osg::State* state = gc->getState();
        
        for(osg::GraphicsContext::Cameras::iterator citr = cameras.begin();
            citr != cameras.end();
            ++citr)
        {
            osg::Camera* camera = *citr;
            osgViewer::View* view = dynamic_cast<osgViewer::View*>(camera->getView());
            osgViewer::Scene* scene = view ? view->getScene() : 0;
            
            osg::DisplaySettings* ds = view ? view->getDisplaySettings() : 0;
            if (!ds) ds = osg::DisplaySettings::instance();
        
            osgDB::DatabasePager* dp = scene ? scene->getDatabasePager() : 0;

            camera->setStats(new osg::Stats("Camera"));

            osgUtil::SceneView* sceneView = new osgUtil::SceneView;
            _cameraSceneViewMap[camera] = sceneView;

            sceneView->setGlobalStateSet(view ? view->getCamera()->getStateSet() : 0);
            sceneView->setDefaults();
            sceneView->setDisplaySettings(camera->getDisplaySettings()!=0 ? camera->getDisplaySettings() : ds);
            sceneView->setCamera(camera);
            sceneView->setState(state);
            sceneView->setFrameStamp(frameStamp);

            if (dp) dp->setCompileGLObjectsForContextID(state->getContextID(), true);

            gc->add(new CompositeViewerRenderingOperation(sceneView, dp));
            
        }
    }

#else
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
            sceneView->setDisplaySettings(view->getCamera()->getDisplaySettings()!=0 ? view->getCamera()->getDisplaySettings() : ds);
            sceneView->setCamera(view->getCamera());
            sceneView->setState(view->getCamera()->getGraphicsContext()->getState());
            sceneView->setSceneData(view->getSceneData());
            sceneView->setFrameStamp(frameStamp);

            if (dp) dp->setCompileGLObjectsForContextID(view->getCamera()->getGraphicsContext()->getState()->getContextID(), true);

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
                sceneView->setDisplaySettings(slave._camera->getDisplaySettings()!=0 ? slave._camera->getDisplaySettings() : ds);
                sceneView->setState(slave._camera->getGraphicsContext()->getState());
                sceneView->setSceneData(view->getSceneData());
                sceneView->setFrameStamp(frameStamp);

                if (dp) dp->setCompileGLObjectsForContextID(slave._camera->getGraphicsContext()->getState()->getContextID(), true);

                slave._camera->getGraphicsContext()->add(new CompositeViewerRenderingOperation(sceneView, dp));
            }
        }

    }
#endif
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
        osg::GraphicsContext* gc = *citr;
        gc->realize();
        
        if (_realizeOperation.valid() && gc->valid()) 
        {
            gc->makeCurrent();
            
            (*_realizeOperation)(gc);
            
            gc->releaseContext();
        }
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


void CompositeViewer::frame(double simulationTime)
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
    advance(simulationTime);
    
    eventTraversal();
    updateTraversal();
    renderingTraversals();
}

void CompositeViewer::advance(double simulationTime)
{
    if (_done) return;

    _frameStamp->setFrameNumber(_frameStamp->getFrameNumber()+1);

    _frameStamp->setReferenceTime( osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick()) );

    if (simulationTime==USE_REFERENCE_TIME)
    {
        _frameStamp->setSimulationTime(_frameStamp->getReferenceTime());
    }
    else
    {
        _frameStamp->setSimulationTime(simulationTime);
    }
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
    
    typedef std::map<osgViewer::View*, osgGA::EventQueue::Events> ViewEventsMap;
    ViewEventsMap viewEventsMap;
    
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
                    case(osgGA::GUIEventAdapter::RESIZE):
                        setCameraWithFocus(0);
                        break;
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
                                if (camera->getView() && 
                                    camera->getAllowEventFocus() &&
                                    camera->getRenderTargetImplementation()==osg::Camera::FRAME_BUFFER)
                                {
                                    osg::Viewport* viewport = camera ? camera->getViewport() : 0;
                                    if (viewport && 
                                        x >= viewport->x() && y >= viewport->y() &&
                                        x <= (viewport->x()+viewport->width()) && y <= (viewport->y()+viewport->height()) )
                                    {
                                        setCameraWithFocus(camera);

                                        const osg::GraphicsContext::Traits* traits = gw ? gw->getTraits() : 0;
                                        if (traits) 
                                        {
                                            eventState->setInputRange( 0, 0, traits->width, traits->height);
                                        }
                                        else
                                        {
                                            eventState->setInputRange(-1.0, -1.0, 1.0, 1.0);
                                        }

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
                                            }
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
                        bool wasThreading = areThreadsRunning();
                        if (wasThreading) stopThreading();
                        
                        gw->close();

                        if (wasThreading) startThreading();

                        break;
                    }
                    default:
                        break;
                }
            }

            viewEventsMap[masterView].insert( viewEventsMap[masterView].end(), gw_events.begin(), gw_events.end() );

        }
    }
    

    // osg::notify(osg::NOTICE)<<"mouseEventState Xmin = "<<eventState->getXmin()<<" Ymin="<<eventState->getYmin()<<" xMax="<<eventState->getXmax()<<" Ymax="<<eventState->getYmax()<<std::endl;


    for(Views::iterator vitr = _views.begin();
        vitr != _views.end();
        ++vitr)
    {
        View* view = vitr->get();
        view->getEventQueue()->frame( getFrameStamp()->getReferenceTime() );
        view->getEventQueue()->takeEvents(viewEventsMap[view]);
    }
    
#if 0
    _eventQueue->getCurrentEventState()->setInputRange(eventState->getXmin(), eventState->getYmin(), eventState->getXmax(), eventState->getYmax());
    _eventQueue->getCurrentEventState()->setX(eventState->getX());
    _eventQueue->getCurrentEventState()->setY(eventState->getY());
    _eventQueue->getCurrentEventState()->setButtonMask(eventState->getButtonMask());
    _eventQueue->getCurrentEventState()->setMouseYOrientation(eventState->getMouseYOrientation());

    _eventQueue->frame( getFrameStamp()->getReferenceTime() );
    _eventQueue->takeEvents(events);
#endif


    // osg::notify(osg::NOTICE)<<"Events "<<events.size()<<std::endl;
    
    if ((_keyEventSetsDone!=0) || _quitEventSetsDone)
    {
        for(ViewEventsMap::iterator veitr = viewEventsMap.begin();
            veitr != viewEventsMap.end();
            ++veitr)
        {
            for(osgGA::EventQueue::Events::iterator itr = veitr->second.begin();
                itr != veitr->second.end();
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
    }
        
    if (_done) return;

    for(ViewEventsMap::iterator veitr = viewEventsMap.begin();
        veitr != viewEventsMap.end();
        ++veitr)
    {
        View* view = veitr->first;
        _eventVisitor->setActionAdapter(view);
        
        for(osgGA::EventQueue::Events::iterator itr = veitr->second.begin();
            itr != veitr->second.end();
            ++itr)
        {
            osgGA::GUIEventAdapter* event = itr->get();

            for(View::EventHandlers::iterator hitr = view->getEventHandlers().begin();
                hitr != view->getEventHandlers().end();
                ++hitr)
            {
                if ((*hitr)->handle( *event, *view, 0, 0)) event->setHandled(true);
            }

            if (view->getCameraManipulator())
            {
                if (view->getCameraManipulator()->handle( *event, *view)) event->setHandled(true);
            }
        }
    }

    if (_eventVisitor.valid())
    {
        _eventVisitor->setFrameStamp(getFrameStamp());
        _eventVisitor->setTraversalNumber(getFrameStamp()->getFrameNumber());

        for(ViewEventsMap::iterator veitr = viewEventsMap.begin();
            veitr != viewEventsMap.end();
            ++veitr)
        {
            View* view = veitr->first;
            if (view->getSceneData())
            {            
                for(osgGA::EventQueue::Events::iterator itr = veitr->second.begin();
                    itr != veitr->second.end();
                    ++itr)
                {
                    osgGA::GUIEventAdapter* event = itr->get();

                    _eventVisitor->reset();
                    _eventVisitor->addEvent( event );

                    view->getSceneData()->accept(*_eventVisitor);

                    // call any camera update callbacks, but only traverse that callback, don't traverse its subgraph
                    // leave that to the scene update traversal.
                    osg::NodeVisitor::TraversalMode tm = _eventVisitor->getTraversalMode();
                    _eventVisitor->setTraversalMode(osg::NodeVisitor::TRAVERSE_NONE);

                    if (view->getCamera() && view->getCamera()->getEventCallback()) view->getCamera()->accept(*_eventVisitor);

                    for(unsigned int i=0; i<view->getNumSlaves(); ++i)
                    {
                        osg::Camera* camera = view->getSlave(i)._camera.get();
                        if (camera && camera->getEventCallback()) camera->accept(*_eventVisitor);
                    }

                    _eventVisitor->setTraversalMode(tm);

                }
            }
        }
        
    }
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
        (*sitr)->setFrameStamp(_frameStamp.get());
        (*sitr)->updateTraversal();
    }


    for(Views::iterator vitr = _views.begin();
        vitr != _views.end();
        ++vitr)
    {
        View* view = vitr->get();

        Scene* scene = view->getScene();
        osgUtil::UpdateVisitor* uv = scene ? scene->getUpdateVisitor() : 0;
        if (uv)
        {
            // call any camera update callbacks, but only traverse that callback, don't traverse its subgraph
            // leave that to the scene update traversal.
            osg::NodeVisitor::TraversalMode tm = uv->getTraversalMode();
            uv->setTraversalMode(osg::NodeVisitor::TRAVERSE_NONE);

            if (view->getCamera() && view->getCamera()->getUpdateCallback()) view->getCamera()->accept(*uv);

            for(unsigned int i=0; i<view->getNumSlaves(); ++i)
            {
                osg::Camera* camera = view->getSlave(i)._camera.get();
                if (camera && camera->getUpdateCallback()) camera->accept(*uv);
            }

            uv->setTraversalMode(tm);
        }


        if (view->getCameraManipulator()) 
        {
            view->setFusionDistance( view->getCameraManipulator()->getFusionDistanceMode(),
                                     view->getCameraManipulator()->getFusionDistanceValue() );
                                      
            view->getCamera()->setViewMatrix( view->getCameraManipulator()->getInverseMatrix());
        }
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
        if (!((*itr)->getGraphicsThread())  && (*itr)->valid())
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

        if (!((*itr)->getGraphicsThread()) && (*itr)->valid())
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
