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
#include <osgViewer/Renderer>
#include <osgDB/Registry>

#include <osg/io_utils>

using namespace osgViewer;

CompositeViewer::CompositeViewer()
{
    constructorInit();
}

CompositeViewer::CompositeViewer(const CompositeViewer& cv,const osg::CopyOp& copyop):
    ViewerBase()
{
    constructorInit();
}

CompositeViewer::CompositeViewer(osg::ArgumentParser& arguments)
{
    constructorInit();
    
    std::string filename;
    bool readConfig = false;
    while (arguments.read("-c",filename))
    {
        readConfig = readConfiguration(filename) || readConfig;
    }

    while (arguments.read("--SingleThreaded")) setThreadingModel(SingleThreaded);
    while (arguments.read("--CullDrawThreadPerContext")) setThreadingModel(CullDrawThreadPerContext);
    while (arguments.read("--DrawThreadPerContext")) setThreadingModel(DrawThreadPerContext);
    while (arguments.read("--CullThreadPerCameraDrawThreadPerContext")) setThreadingModel(CullThreadPerCameraDrawThreadPerContext);

    osg::DisplaySettings::instance()->readCommandLine(arguments);
    osgDB::readCommandLine(arguments);
}

void CompositeViewer::constructorInit()
{
    _endBarrierPosition = AfterSwapBuffers;
    _startTick = 0;

    // make sure View is safe to reference multi-threaded.
    setThreadSafeRefUnref(true);

    _frameStamp = new osg::FrameStamp;
    _frameStamp->setFrameNumber(0);
    _frameStamp->setReferenceTime(0);
    _frameStamp->setSimulationTime(0);
    
    _eventVisitor = new osgGA::EventVisitor;
    _eventVisitor->setFrameStamp(_frameStamp.get());
    
    _updateVisitor = new osgUtil::UpdateVisitor;
    _updateVisitor->setFrameStamp(_frameStamp.get());

    setViewerStats(new osg::Stats("CompsiteViewer"));
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

bool CompositeViewer::readConfiguration(const std::string& filename)
{
    osg::notify(osg::NOTICE)<<"CompositeViewer::readConfiguration("<<filename<<")"<<std::endl;
    return false;
}


void CompositeViewer::addView(osgViewer::View* view)
{
    if (!view) return;

    bool alreadyRealized = isRealized();
    
    bool threadsWereRuinning = _threadsRunning;
    if (threadsWereRuinning) stopThreading();

    _views.push_back(view);
    
    view->_viewerBase = this;
    
    if (view->getSceneData())
    {        
        // make sure that existing scene graph objects are allocated with thread safe ref/unref
        if (getThreadingModel()!=ViewerBase::SingleThreaded) 
        {
            view->getSceneData()->setThreadSafeRefUnref(true);
        }
        
        // update the scene graph so that it has enough GL object buffer memory for the graphics contexts that will be using it.
        view->getSceneData()->resizeGLObjectBuffers(osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts());
    }

    view->setFrameStamp(_frameStamp.get());
    
    if (alreadyRealized)
    {
        Contexts contexts;
        if (view->getCamera()->getGraphicsContext())
        {
            contexts.push_back(view->getCamera()->getGraphicsContext());
        }
        for(unsigned int i=0; i<view->getNumSlaves(); ++i)
        {
            if (view->getSlave(i)._camera->getGraphicsContext())
            {
                contexts.push_back(view->getSlave(i)._camera->getGraphicsContext());
            }
        }

        for(Contexts::iterator itr = contexts.begin();
            itr != contexts.end();
            ++itr)
        {
            if (!((*itr)->isRealized()))
            {
                (*itr)->realize();
            }
        }

    }
    
    if (threadsWereRuinning) startThreading();
}

void CompositeViewer::removeView(osgViewer::View* view)
{
    for(RefViews::iterator itr = _views.begin();
        itr != _views.end();
        ++itr)
    {
        if (*itr == view)
        {
            bool threadsWereRuinning = _threadsRunning;
            if (threadsWereRuinning) stopThreading();

            view->_viewerBase = 0;

            _views.erase(itr);

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
    for(RefViews::iterator itr = _views.begin();
        itr != _views.end();
        ++itr)
    {
        osgViewer::View* view = itr->get();
        if ((view->getCameraManipulator()==0) && view->getCamera()->getAllowEventFocus())
        {
            view->setCameraManipulator(new osgGA::TrackballManipulator());
        }
    }
        
    setReleaseContextAtEndOfFrameHint(false);

    return ViewerBase::run();
}

void CompositeViewer::setStartTick(osg::Timer_t tick)
{
    _startTick = tick;
    
    for(RefViews::iterator vitr = _views.begin();
        vitr != _views.end();
        ++vitr)
    {
        (*vitr)->setStartTick(tick);
    }
    
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



void CompositeViewer::viewerInit()
{
    osg::notify(osg::INFO)<<"CompositeViewer::init()"<<std::endl;

    for(RefViews::iterator itr = _views.begin();
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

    contexts.clear();

    for(RefViews::iterator vitr = _views.begin();
        vitr != _views.end();
        ++vitr)
    {
        osgViewer::View* view = vitr->get();
        osg::GraphicsContext* gc = view->getCamera() ? view->getCamera()->getGraphicsContext() : 0;
        if (gc && (gc->valid() || !onlyValid))
        {
            if (contextSet.count(gc)==0)
            {
                contextSet.insert(gc);
                contexts.push_back(gc);
            }
        }

        for(unsigned int i=0; i<view->getNumSlaves(); ++i)
        {
            View::Slave& slave = view->getSlave(i);
            osg::GraphicsContext* sgc = slave._camera.valid() ? slave._camera->getGraphicsContext() : 0;
            if (sgc && (sgc->valid() || !onlyValid))
            {
                if (contextSet.count(sgc)==0)
                {
                    contextSet.insert(sgc);
                    contexts.push_back(sgc);
                }
            }
        }
    }
}

void CompositeViewer::getCameras(Cameras& cameras, bool onlyActive)
{
    cameras.clear();
    
    for(RefViews::iterator vitr = _views.begin();
        vitr != _views.end();
        ++vitr)
    {
        View* view = vitr->get();

        if (view->getCamera() && 
            (!onlyActive || (view->getCamera()->getGraphicsContext() && view->getCamera()->getGraphicsContext()->valid())) ) cameras.push_back(view->getCamera());

        for(View::Slaves::iterator itr = view->_slaves.begin();
            itr != view->_slaves.end();
            ++itr)
        {
            if (itr->_camera.valid() &&
                (!onlyActive || (itr->_camera->getGraphicsContext() && itr->_camera->getGraphicsContext()->valid())) ) cameras.push_back(itr->_camera.get());
        }
    }
}
 
void CompositeViewer::getScenes(Scenes& scenes, bool onlyValid)
{
    scenes.clear();

    typedef std::set<osgViewer::Scene*> SceneSet;
    SceneSet sceneSet;

    for(RefViews::iterator vitr = _views.begin();
        vitr != _views.end();
        ++vitr)
    {
        osgViewer::View* view = vitr->get();
        if (view->getScene() && (!onlyValid || view->getScene()->getSceneData()))
        {
            if (sceneSet.count(view->getScene())==0)
            {
                sceneSet.insert(view->getScene());
                scenes.push_back(view->getScene());
            }
        }
    }
}

void CompositeViewer::getViews(Views& views, bool onlyValid)
{
    views.clear();

    for(RefViews::iterator vitr = _views.begin();
        vitr != _views.end();
        ++vitr)
    {
        views.push_back(vitr->get());
    }
}

void CompositeViewer::getAllThreads(Threads& threads, bool onlyActive)
{
    threads.clear();

    OperationThreads operationThreads;
    getOperationThreads(operationThreads);
    
    for(OperationThreads::iterator itr = operationThreads.begin();
        itr != operationThreads.end();
        ++itr)
    {
        threads.push_back(*itr);
    }

    Scenes scenes;
    getScenes(scenes);
    
    for(Scenes::iterator sitr = scenes.begin();
        sitr != scenes.end();
        ++sitr)
    {
        Scene* scene = *sitr;
        osgDB::DatabasePager* dp = scene->getDatabasePager();
        if (dp)
        {
            for(unsigned int i=0; i<dp->getNumDatabaseThreads(); ++i)
            {
                osgDB::DatabasePager::DatabaseThread* dt = dp->getDatabaseThread(i);
                if (!onlyActive || dt->isRunning())
                {
                    threads.push_back(dt);
                }
            }
        }
    }
}


void CompositeViewer::getOperationThreads(OperationThreads& threads, bool onlyActive)
{
    threads.clear();
    
    Contexts contexts;
    getContexts(contexts);
    for(Contexts::iterator gcitr = contexts.begin();
        gcitr != contexts.end();
        ++gcitr)
    {
        osg::GraphicsContext* gc = *gcitr;
        if (gc->getGraphicsThread() && 
            (!onlyActive || gc->getGraphicsThread()->isRunning()) )
        {
            threads.push_back(gc->getGraphicsThread());
        }
    }
    
    Cameras cameras;
    getCameras(cameras);
    for(Cameras::iterator citr = cameras.begin();
        citr != cameras.end();
        ++citr)
    {
        osg::Camera* camera = *citr;
        if (camera->getCameraThread() && 
            (!onlyActive || camera->getCameraThread()->isRunning()) )
        {
            threads.push_back(camera->getCameraThread());
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

    if (osg::DisplaySettings::instance()->getCompileContextsHint())
    {
        int numProcessors = OpenThreads::GetNumberOfProcessors();
        int processNum = 0;

        for(unsigned int i=0; i<= osg::GraphicsContext::getMaxContextID(); ++i)
        {
            osg::GraphicsContext* gc = osg::GraphicsContext::getOrCreateCompileContext(i);

            if (gc)
            {
                gc->createGraphicsThread();
                gc->getGraphicsThread()->setProcessorAffinity(processNum % numProcessors);
                gc->getGraphicsThread()->startThread();
                
                ++processNum;
            }
        }
    }

}

void CompositeViewer::advance(double simulationTime)
{
    if (_done) return;
    
    double prevousReferenceTime = _frameStamp->getReferenceTime();
    int previousFrameNumber = _frameStamp->getFrameNumber();


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
    
    if (getViewerStats() && getViewerStats()->collectStats("frame_rate"))
    {
        // update previous frame stats
        double deltaFrameTime = _frameStamp->getReferenceTime() - prevousReferenceTime;
        getViewerStats()->setAttribute(previousFrameNumber, "Frame duration", deltaFrameTime);
        getViewerStats()->setAttribute(previousFrameNumber, "Frame rate", 1.0/deltaFrameTime);

        // update current frames stats
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(), "Reference time", _frameStamp->getReferenceTime());
    }

}

void CompositeViewer::setCameraWithFocus(osg::Camera* camera)
{
    _cameraWithFocus = camera;

    if (camera)
    {
        for(RefViews::iterator vitr = _views.begin();
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
    
    double beginEventTraversal = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());

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
                
                //osg::notify(osg::NOTICE)<<"event->getGraphicsContext()="<<event->getGraphicsContext()<<std::endl;
                
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

                                        // If this camera is not a slave camera
                                        if (camera->getView()->getCamera() == camera)
                                        {
                                            eventState->setGraphicsContext(gw);
                                            eventState->setInputRange( viewport->x(), viewport->y(), 
                                                                       viewport->x()+viewport->width(),
                                                                       viewport->y()+viewport->height());

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

                                        // If this camera is not a slave camera
                                        if (camera->getView()->getCamera() == camera)
                                        {
                                            eventState->setGraphicsContext(gw);
                                            eventState->setInputRange( viewport->x(), viewport->y(), 
                                                                       viewport->x()+viewport->width(),
                                                                       viewport->y()+viewport->height());

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


    for(RefViews::iterator vitr = _views.begin();
        vitr != _views.end();
        ++vitr)
    {
        View* view = vitr->get();
        view->getEventQueue()->frame( getFrameStamp()->getReferenceTime() );
        view->getEventQueue()->takeEvents(viewEventsMap[view]);
    }
    

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

    if (_eventVisitor.valid())
    {
        _eventVisitor->setFrameStamp(getFrameStamp());
        _eventVisitor->setTraversalNumber(getFrameStamp()->getFrameNumber());

        for(ViewEventsMap::iterator veitr = viewEventsMap.begin();
            veitr != viewEventsMap.end();
            ++veitr)
        {
            View* view = veitr->first;
            _eventVisitor->setActionAdapter(view);
            
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

    for(ViewEventsMap::iterator veitr = viewEventsMap.begin();
        veitr != viewEventsMap.end();
        ++veitr)
    {
        View* view = veitr->first;
        
        for(osgGA::EventQueue::Events::iterator itr = veitr->second.begin();
            itr != veitr->second.end();
            ++itr)
        {
            osgGA::GUIEventAdapter* event = itr->get();

            for(View::EventHandlers::iterator hitr = view->getEventHandlers().begin();
                hitr != view->getEventHandlers().end();
                ++hitr)
            {
                (*hitr)->handleWithCheckAgainstIgnoreHandledEventsMask( *event, *view, 0, 0);
            }
        }
    }

    for(ViewEventsMap::iterator veitr = viewEventsMap.begin();
        veitr != viewEventsMap.end();
        ++veitr)
    {
        View* view = veitr->first;
        
        for(osgGA::EventQueue::Events::iterator itr = veitr->second.begin();
            itr != veitr->second.end();
            ++itr)
        {
            osgGA::GUIEventAdapter* event = itr->get();

            if (view->getCameraManipulator())
            {
                view->getCameraManipulator()->handleWithCheckAgainstIgnoreHandledEventsMask( *event, *view);
            }
        }
    }

    

    if (getViewerStats() && getViewerStats()->collectStats("event"))
    {
        double endEventTraversal = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());

        // update current frames stats
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(), "Event traversal begin time", beginEventTraversal);
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(), "Event traversal end time", endEventTraversal);
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(), "Event traversal time taken", endEventTraversal-beginEventTraversal);
    }
}


void CompositeViewer::updateTraversal()
{
    if (_done) return;
    
    double beginUpdateTraversal = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());

    _updateVisitor->reset();
    _updateVisitor->setFrameStamp(getFrameStamp());
    _updateVisitor->setTraversalNumber(getFrameStamp()->getFrameNumber());

    Scenes scenes;
    getScenes(scenes);
    for(Scenes::iterator sitr = scenes.begin();
        sitr != scenes.end();
        ++sitr)
    {
        Scene* scene = *sitr;
        scene->updateSceneGraph(*_updateVisitor);
    }

    // if we have a shared state manager prune any unused entries
    if (osgDB::Registry::instance()->getSharedStateManager())
        osgDB::Registry::instance()->getSharedStateManager()->prune();

    // update the Registry object cache.
    osgDB::Registry::instance()->updateTimeStampOfObjectsInCacheWithExternalReferences(*getFrameStamp());
    osgDB::Registry::instance()->removeExpiredObjectsInCache(*getFrameStamp());


    if (_updateOperations.valid())
    {
        _updateOperations->runOperations(this);
    }

    for(RefViews::iterator vitr = _views.begin();
        vitr != _views.end();
        ++vitr)
    {
        View* view = vitr->get();

        {
            // call any camera update callbacks, but only traverse that callback, don't traverse its subgraph
            // leave that to the scene update traversal.
            osg::NodeVisitor::TraversalMode tm = _updateVisitor->getTraversalMode();
            _updateVisitor->setTraversalMode(osg::NodeVisitor::TRAVERSE_NONE);

            if (view->getCamera() && view->getCamera()->getUpdateCallback()) view->getCamera()->accept(*_updateVisitor);

            for(unsigned int i=0; i<view->getNumSlaves(); ++i)
            {
                osg::Camera* camera = view->getSlave(i)._camera.get();
                if (camera && camera->getUpdateCallback()) camera->accept(*_updateVisitor);
            }

            _updateVisitor->setTraversalMode(tm);
        }


        if (view->getCameraManipulator()) 
        {
            view->setFusionDistance( view->getCameraManipulator()->getFusionDistanceMode(),
                                     view->getCameraManipulator()->getFusionDistanceValue() );
                                      
            view->getCamera()->setViewMatrix( view->getCameraManipulator()->getInverseMatrix());
        }
        view->updateSlaves();

    }
    
    if (getViewerStats() && getViewerStats()->collectStats("update"))
    {
        double endUpdateTraversal = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());

        // update current frames stats
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(), "Update traversal begin time", beginUpdateTraversal);
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(), "Update traversal end time", endUpdateTraversal);
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(), "Update traversal time taken", endUpdateTraversal-beginUpdateTraversal);
    }

}

double CompositeViewer::elapsedTime()
{
    return osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());
}

void CompositeViewer::getUsage(osg::ApplicationUsage& usage) const
{
    for(RefViews::const_iterator vitr = _views.begin();
        vitr != _views.end();
        ++vitr)
    {
        const View* view = vitr->get();
        if (view->getCameraManipulator())
        {
            view->getCameraManipulator()->getUsage(usage);
        }

        for(View::EventHandlers::const_iterator hitr = view->_eventHandlers.begin();
            hitr != view->_eventHandlers.end();
            ++hitr)
        {
            (*hitr)->getUsage(usage);
        }
    }
}
