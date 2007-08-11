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

#include <stdio.h>

#include <osg/DeleteHandler>
#include <osg/io_utils>

#include <osgUtil/GLObjectsVisitor>
#include <osgDB/Registry>
#include <osgGA/TrackballManipulator>

#include <osgViewer/Viewer>
#include <osgViewer/Renderer>


#include <sstream>

using namespace osgViewer;

Viewer::Viewer()
{
    constructorInit();
}

Viewer::Viewer(osg::ArgumentParser& arguments)
{
    constructorInit();
    
    while (arguments.read("--SingleThreaded")) setThreadingModel(SingleThreaded);
    while (arguments.read("--CullDrawThreadPerContext")) setThreadingModel(CullDrawThreadPerContext);
    while (arguments.read("--DrawThreadPerContext")) setThreadingModel(DrawThreadPerContext);
    while (arguments.read("--CullThreadPerCameraDrawThreadPerContext")) setThreadingModel(CullThreadPerCameraDrawThreadPerContext);

    osg::DisplaySettings::instance()->readCommandLine(arguments);
    osgDB::readCommandLine(arguments);

    std::string colorStr;
    while (arguments.read("--clear-color",colorStr))
    {
        float r, g, b;
        float a = 1.0f;
        int cnt = sscanf( colorStr.c_str(), "%f,%f,%f,%f", &r, &g, &b, &a );
        if( cnt==3 || cnt==4 ) getCamera()->setClearColor( osg::Vec4(r,g,b,a) );
        else osg::notify(osg::WARN)<<"Invalid clear color \""<<colorStr<<"\""<<std::endl;
    }
    
    int screenNum = -1;
    while (arguments.read("--screen",screenNum)) {}
    
    int x = -1, y = -1, width = -1, height = -1;
    while (arguments.read("--window",x,y,width,height)) {}

    if (width>0 && height>0)
    {
        if (screenNum>=0) setUpViewInWindow(x, y, width, height, screenNum);
        else setUpViewInWindow(x,y,width,height);
        
    }
    else if (screenNum>=0)
    {
        setUpViewOnSingleScreen(screenNum);
    }

}

void Viewer::constructorInit()
{
    _firstFrame = true;
    _done = false;
    _keyEventSetsDone = osgGA::GUIEventAdapter::KEY_Escape;
    _quitEventSetsDone = true;
    _threadingModel = AutomaticSelection;
    _threadsRunning = false;
    _useMainThreadForRenderingTraversal = true;
    _endBarrierPosition = AfterSwapBuffers;
    _numWindowsOpenAtLastSetUpThreading = 0;

    _eventVisitor = new osgGA::EventVisitor;
    _eventVisitor->setActionAdapter(this);
    _eventVisitor->setFrameStamp(_frameStamp.get());
    
    _updateVisitor = new osgUtil::UpdateVisitor;
    _updateVisitor->setFrameStamp(_frameStamp.get());

    setStats(new osg::Stats("Viewer"));
}

Viewer::~Viewer()
{
    //osg::notify(osg::NOTICE)<<"Viewer::~Viewer()"<<std::endl;


    Threads threads;
    getAllThreads(threads);

    osg::notify(osg::INFO)<<"Viewer::~Viewer():: start destructor getThreads = "<<threads.size()<<std::endl;


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
    
    getAllThreads(threads);

    osg::notify(osg::INFO)<<"Viewer::~Viewer() end destrcutor getThreads = "<<threads.size()<<std::endl;

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

    if (!getCameraManipulator() && getCamera()->getAllowEventFocus())
    {
        setCameraManipulator(new osgGA::TrackballManipulator());
    }

    if (!isRealized())
    {
        realize();
    }

#if 0
    while (!done())
    {
        frame();
    }
#else

    const char* str = getenv("OSG_RUN_FRAME_COUNT");
    if (str)
    {
        int runTillFrameNumber = atoi(str);
        while (!done() && getFrameStamp()->getFrameNumber()<runTillFrameNumber)
        {
            frame();
        }
    }
    else
    {
        while (!done())
        {
            frame();
        }
    }

#endif    
    return 0;
}

void Viewer::setStartTick(osg::Timer_t tick)
{
    View::setStartTick(tick);
    
    Contexts contexts;
    getContexts(contexts,false);

    getEventQueue()->setStartTick(_startTick);
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


void Viewer::setReferenceTime(double time)
{
    osg::Timer_t tick = osg::Timer::instance()->tick();
    double currentTime = osg::Timer::instance()->delta_s(_startTick, tick);
    double delta_ticks = (time-currentTime)*(osg::Timer::instance()->getSecondsPerTick());
    if (delta_ticks>=0) tick += osg::Timer_t(delta_ticks);
    else tick -= osg::Timer_t(-delta_ticks);

    // assign the new start tick
    setStartTick(tick);
}


void Viewer::setSceneData(osg::Node* node)
{
    setReferenceTime(0.0);

    View::setSceneData(node);
}

GraphicsWindowEmbedded* Viewer::setUpViewerAsEmbeddedInWindow(int x, int y, int width, int height)
{
    setThreadingModel(SingleThreaded);
    osgViewer::GraphicsWindowEmbedded* gw = new osgViewer::GraphicsWindowEmbedded(x,y,width,height);
    getCamera()->setViewport(new osg::Viewport(0,0,width,height));
    getCamera()->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(width)/static_cast<double>(height), 1.0f, 10000.0f);
    getCamera()->setGraphicsContext(gw);
    return gw;
}

void Viewer::setThreadingModel(ThreadingModel threadingModel)
{
    if (_threadingModel == threadingModel) return;
    
    if (_threadsRunning) stopThreading();
    
    _threadingModel = threadingModel;

    if (isRealized() && _threadingModel!=SingleThreaded) startThreading();
}

void Viewer::setUpThreading()
{
    Contexts contexts;
    getContexts(contexts);

    _numWindowsOpenAtLastSetUpThreading = contexts.size();

    if (_threadingModel==SingleThreaded)
    {
        if (_threadsRunning) stopThreading();
        else
        {
            // we'll set processor affinity here to help single threaded apps
            // with multiple processor cores, and using the database pager.
            int numProcessors = OpenThreads::GetNumberOfProcessors();
            bool affinity = numProcessors>1;    
            if (affinity) 
            {
                OpenThreads::SetProcessorAffinityOfCurrentThread(0);
                if (_scene.valid() && _scene->getDatabasePager())
                {
                    _scene->getDatabasePager()->setProcessorAffinity(1);
                }
            }
        }
    }
    else
    {
        if (!_threadsRunning) startThreading();
    }
    
}


void Viewer::setUseMainThreadForRenderingTraversals(bool flag)
{
    if (_useMainThreadForRenderingTraversal==flag) return;

    if (_threadsRunning) stopThreading();
    
    _useMainThreadForRenderingTraversal = flag;
    
    if (_threadingModel!=SingleThreaded) startThreading();
}

void Viewer::setEndBarrierPosition(BarrierPosition bp)
{
    if (_endBarrierPosition == bp) return;
    
    if (_threadsRunning) stopThreading();
    
    _endBarrierPosition = bp;

    if (_threadingModel!=SingleThreaded) startThreading();
}

void Viewer::stopThreading()
{
    if (!_threadsRunning) return;

    osg::notify(osg::INFO)<<"Viewer::stopThreading() - stopping threading"<<std::endl;

    Contexts contexts;
    getContexts(contexts);

    Cameras cameras;
    getCameras(cameras);

    Contexts::iterator gcitr;
    Cameras::iterator citr;

    for(Cameras::iterator camItr = cameras.begin();
        camItr != cameras.end();
        ++camItr)
    {
        osg::Camera* camera = *camItr;
        Renderer* renderer = dynamic_cast<Renderer*>(camera->getRenderer());
        if (renderer) renderer->release();
    }

    // delete all the graphics threads.    
    for(gcitr = contexts.begin();
        gcitr != contexts.end();
        ++gcitr)
    {
        (*gcitr)->setGraphicsThread(0);
    }

    // delete all the camera threads.    
    for(citr = cameras.begin();
        citr != cameras.end();
        ++citr)
    {
        (*citr)->setCameraThread(0);
    }

    for(Cameras::iterator camItr = cameras.begin();
        camItr != cameras.end();
        ++camItr)
    {
        osg::Camera* camera = *camItr;
        Renderer* renderer = dynamic_cast<Renderer*>(camera->getRenderer());
        if (renderer)
        {
            renderer->setGraphicsThreadDoesCull( true );
            renderer->setDone(false);
        }
    }


    int numProcessors = OpenThreads::GetNumberOfProcessors();
    bool affinity = numProcessors>1;    
    if (affinity) 
    {
        OpenThreads::SetProcessorAffinityOfCurrentThread(0);
        if (_scene.valid() && _scene->getDatabasePager())
        {
            _scene->getDatabasePager()->setProcessorAffinity(1);
        }
    }

    _threadsRunning = false;
    _startRenderingBarrier = 0;
    _endRenderingDispatchBarrier = 0;
    _endDynamicDrawBlock = 0;
    _numWindowsOpenAtLastSetUpThreading = contexts.size();

    osg::notify(osg::INFO)<<"Viewer::stopThreading() - stopped threading."<<std::endl;
}

static osg::ApplicationUsageProxy Viewer_e0(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_THREADING <value>","Set the threading model using by Viewer, <value> can be SingleThreaded, CullDrawThreadPerContext, DrawThreadPerContext or CullThreadPerCameraDrawThreadPerContext.");
static osg::ApplicationUsageProxy Viewer_e1(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_SCREEN <value>","Set the default screen that windows should open up on.");
static osg::ApplicationUsageProxy Viewer_e2(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_WINDOW x y width height","Set the default window dimensions that windows should open up on.");

Viewer::ThreadingModel Viewer::suggestBestThreadingModel()
{
    const char* str = getenv("OSG_THREADING");
    if (str)
    {
        if (strcmp(str,"SingleThreaded")==0) return SingleThreaded;
        else if (strcmp(str,"CullDrawThreadPerContext")==0) return CullDrawThreadPerContext;
        else if (strcmp(str,"DrawThreadPerContext")==0) return DrawThreadPerContext;
        else if (strcmp(str,"CullThreadPerCameraDrawThreadPerContext")==0) return CullThreadPerCameraDrawThreadPerContext;
    }

    Contexts contexts;
    getContexts(contexts);
    
    if (contexts.empty()) return SingleThreaded;

#if 0
    // temporary hack to disable multi-threading under Windows till we find good solutions for
    // crashes that users are seeing.
    return SingleThreaded;
#endif

    Cameras cameras;
    getCameras(cameras);

    if (cameras.empty()) return SingleThreaded;


    int numProcessors = OpenThreads::GetNumberOfProcessors();

    if (contexts.size()==1)
    {
        if (numProcessors==1) return SingleThreaded;
        else return DrawThreadPerContext;
    }
    
#if 0
    if (numProcessors >= static_cast<int>(cameras.size()+contexts.size()))
    {
        return CullThreadPerCameraDrawThreadPerContext;
    }
#endif

    return DrawThreadPerContext;
}

void Viewer::startThreading()
{
    if (_threadsRunning) return;
    
    osg::notify(osg::INFO)<<"Viewer::startThreading() - starting threading"<<std::endl;
    
    // release any context held by the main thread.
    releaseContext();

    _threadingModel = _threadingModel==AutomaticSelection ? suggestBestThreadingModel() : _threadingModel;

    Contexts contexts;
    getContexts(contexts);
    
    osg::notify(osg::INFO)<<"Viewer::startThreading() - contexts.size()="<<contexts.size()<<std::endl;

    Cameras cameras;
    getCameras(cameras);
    
    unsigned int numThreadsOnBarrier = 0;
    switch(_threadingModel)
    {
        case(SingleThreaded): 
            numThreadsOnBarrier = 1;
            return;
        case(CullDrawThreadPerContext): 
            numThreadsOnBarrier = contexts.size()+1;
            break;
        case(DrawThreadPerContext): 
            numThreadsOnBarrier = 1;
            break;
        case(CullThreadPerCameraDrawThreadPerContext): 
            numThreadsOnBarrier = _useMainThreadForRenderingTraversal ? cameras.size() : cameras.size()+1;
            break;
        default:
            osg::notify(osg::NOTICE)<<"Error: Threading model not selected"<<std::endl;
            return;
    }

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
    
    int numProcessors = OpenThreads::GetNumberOfProcessors();
    bool affinity = numProcessors>1;    
    
    Contexts::iterator citr;

    unsigned int numViewerDoubleBufferedRenderingOperation = 0;

    bool graphicsThreadsDoesCull = _threadingModel == CullDrawThreadPerContext || _threadingModel==SingleThreaded;

    for(Cameras::iterator camItr = cameras.begin();
        camItr != cameras.end();
        ++camItr)
    {
        osg::Camera* camera = *camItr;
        Renderer* renderer = dynamic_cast<Renderer*>(camera->getRenderer());
        renderer->setGraphicsThreadDoesCull(graphicsThreadsDoesCull);
        renderer->setDone(false);
        ++numViewerDoubleBufferedRenderingOperation;
    }

    if (_threadingModel==CullDrawThreadPerContext)
    {
        _startRenderingBarrier = 0;
        _endRenderingDispatchBarrier = 0;
        _endDynamicDrawBlock = 0;
    }
    else if (_threadingModel==DrawThreadPerContext || 
             _threadingModel==CullThreadPerCameraDrawThreadPerContext)
    {
        _startRenderingBarrier = 0;
        _endRenderingDispatchBarrier = 0;
        _endDynamicDrawBlock = new osg::EndOfDynamicDrawBlock(numViewerDoubleBufferedRenderingOperation);
        
        if (!osg::Referenced::getDeleteHandler()) osg::Referenced::setDeleteHandler(new osg::DeleteHandler(2));
        else osg::Referenced::getDeleteHandler()->setNumFramesToRetainObjects(2);
    }
    
    if (numThreadsOnBarrier>1)
    {
        _startRenderingBarrier = new osg::BarrierOperation(numThreadsOnBarrier, osg::BarrierOperation::NO_OPERATION);
        _endRenderingDispatchBarrier = new osg::BarrierOperation(numThreadsOnBarrier, osg::BarrierOperation::NO_OPERATION);
    }


    osg::ref_ptr<osg::BarrierOperation> swapReadyBarrier = contexts.empty() ? 0 : new osg::BarrierOperation(contexts.size(), osg::BarrierOperation::NO_OPERATION);

    osg::ref_ptr<osg::SwapBuffersOperation> swapOp = new osg::SwapBuffersOperation();

    typedef std::map<OpenThreads::Thread*, int> ThreadAffinityMap;
    ThreadAffinityMap threadAffinityMap;

    unsigned int processNum = 1;
    for(citr = contexts.begin();
        citr != contexts.end();
        ++citr, ++processNum)
    {
        osg::GraphicsContext* gc = (*citr);
        
        gc->getState()->setDynamicObjectRenderingCompletedCallback(_endDynamicDrawBlock.get());

        // create the a graphics thread for this context
        gc->createGraphicsThread();

        if (affinity) gc->getGraphicsThread()->setProcessorAffinity(processNum % numProcessors);
        threadAffinityMap[gc->getGraphicsThread()] = processNum % numProcessors;

        gc->getGraphicsThread()->add(new osgUtil::GLObjectsOperation());

        // add the startRenderingBarrier
        if (_threadingModel==CullDrawThreadPerContext && _startRenderingBarrier.valid()) gc->getGraphicsThread()->add(_startRenderingBarrier.get());

        // add the rendering operation itself.
        gc->getGraphicsThread()->add(new osg::RunOperations());

        if (_threadingModel==CullDrawThreadPerContext && _endBarrierPosition==BeforeSwapBuffers && _endRenderingDispatchBarrier.valid())
        {
            // add the endRenderingDispatchBarrier
            gc->getGraphicsThread()->add(_endRenderingDispatchBarrier.get());
        }

        if (swapReadyBarrier.valid()) gc->getGraphicsThread()->add(swapReadyBarrier.get());

        // add the swap buffers
        gc->getGraphicsThread()->add(swapOp.get());

        if (_threadingModel==CullDrawThreadPerContext && _endBarrierPosition==AfterSwapBuffers && _endRenderingDispatchBarrier.valid())
        {
            // add the endRenderingDispatchBarrier
            gc->getGraphicsThread()->add(_endRenderingDispatchBarrier.get());
        }

    }

    if (_threadingModel==CullThreadPerCameraDrawThreadPerContext && numThreadsOnBarrier>1)
    {
        Cameras::iterator camItr = cameras.begin();
        if (_useMainThreadForRenderingTraversal) ++camItr;

        for(;
            camItr != cameras.end();
            ++camItr, ++processNum)
        {
            osg::Camera* camera = *camItr;
            camera->createCameraThread();

            if (affinity) camera->getCameraThread()->setProcessorAffinity(processNum % numProcessors);
            threadAffinityMap[camera->getCameraThread()] = processNum % numProcessors;

            osg::GraphicsContext* gc = camera->getGraphicsContext();

            // add the startRenderingBarrier
            if (_startRenderingBarrier.valid()) camera->getCameraThread()->add(_startRenderingBarrier.get());

            Renderer* renderer = dynamic_cast<Renderer*>(camera->getRenderer());
            renderer->setGraphicsThreadDoesCull(true);
            camera->getCameraThread()->add(renderer);
            
            if (_endRenderingDispatchBarrier.valid())
            {
                // add the endRenderingDispatchBarrier
                gc->getGraphicsThread()->add(_endRenderingDispatchBarrier.get());
            }

        }

        for(camItr = cameras.begin();
            camItr != cameras.end();
            ++camItr)
        {
            osg::Camera* camera = *camItr;
            if (camera->getCameraThread() && !camera->getCameraThread()->isRunning())
            {
                osg::notify(osg::INFO)<<"  camera->getCameraThread()-> "<<camera->getCameraThread()<<std::endl;
                camera->getCameraThread()->startThread();
            }
        }
    }
    
    if (affinity) 
    {
        OpenThreads::SetProcessorAffinityOfCurrentThread(0);
        if (_scene.valid() && _scene->getDatabasePager())
        {
#if 0        
            _scene->getDatabasePager()->setProcessorAffinity(1);
#else
            _scene->getDatabasePager()->setProcessorAffinity(0);
#endif
        }
    }

#if 0
    if (affinity)
    {
        for(ThreadAffinityMap::iterator titr = threadAffinityMap.begin();
            titr != threadAffinityMap.end();
            ++titr)
        {
            titr->first->setProcessorAffinity(titr->second);
        }
    }
#endif


    for(citr = contexts.begin();
        citr != contexts.end();
        ++citr)
    {
        osg::GraphicsContext* gc = (*citr);
        if (gc->getGraphicsThread() && !gc->getGraphicsThread()->isRunning())
        {
            osg::notify(osg::INFO)<<"  gc->getGraphicsThread()->startThread() "<<gc->getGraphicsThread()<<std::endl;
            gc->getGraphicsThread()->startThread();
            // OpenThreads::Thread::YieldCurrentThread();
        }
    }

    _threadsRunning = true;
    _numWindowsOpenAtLastSetUpThreading = contexts.size();

    osg::notify(osg::INFO)<<"Set up threading"<<std::endl;
}

void Viewer::checkWindowStatus()
{
    Contexts contexts;
    getContexts(contexts);
    
    // osg::notify(osg::NOTICE)<<"Viewer::checkWindowStatus() - "<<contexts.size()<<std::endl;
    
    if (contexts.size()==0)
    {
        _done = true;
        if (areThreadsRunning()) stopThreading();
    }
}


struct LessGraphicsContext
{
    bool operator () (const osg::GraphicsContext* lhs, const osg::GraphicsContext* rhs) const
    {
        int screenLeft = lhs->getTraits()? lhs->getTraits()->screenNum : 0;
        int screenRight = rhs->getTraits()? rhs->getTraits()->screenNum : 0;
        if (screenLeft < screenRight) return true;
        if (screenLeft > screenRight) return false;

        screenLeft = lhs->getTraits()? lhs->getTraits()->x : 0;
        screenRight = rhs->getTraits()? rhs->getTraits()->x : 0;
        if (screenLeft < screenRight) return true;
        if (screenLeft > screenRight) return false;

        screenLeft = lhs->getTraits()? lhs->getTraits()->y : 0;
        screenRight = rhs->getTraits()? rhs->getTraits()->y : 0;
        if (screenLeft < screenRight) return true;
        if (screenLeft > screenRight) return false;
        
        return lhs < rhs;
    } 
};

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

    if (contexts.size()>=2)
    {
        std::sort(contexts.begin(), contexts.end(), LessGraphicsContext()); 
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

void Viewer::getCameras(Cameras& cameras, bool onlyActive)
{
    cameras.clear();
    
    if (!onlyActive || (_camera->getGraphicsContext() && _camera->getGraphicsContext()->valid()) ) cameras.push_back(_camera.get());

    for(Slaves::iterator itr = _slaves.begin();
        itr != _slaves.end();
        ++itr)
    {
        if (!onlyActive || (itr->_camera->getGraphicsContext() && itr->_camera->getGraphicsContext()->valid()) ) cameras.push_back(itr->_camera.get());
    }
        
}

void Viewer::getAllThreads(Threads& threads, bool onlyActive)
{
    OperationThreads operationThreads;
    getOperationThreads(operationThreads);
    
    for(OperationThreads::iterator itr = operationThreads.begin();
        itr != operationThreads.end();
        ++itr)
    {
        threads.push_back(*itr);
    }
    
    if (_scene.valid() && 
        _scene->getDatabasePager() &&
       (!onlyActive || _scene->getDatabasePager()->isRunning())) 
    {
        threads.push_back(_scene->getDatabasePager());
    }
}


void Viewer::getOperationThreads(OperationThreads& threads, bool onlyActive)
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


void Viewer::realize()
{
    //osg::notify(osg::INFO)<<"Viewer::realize()"<<std::endl;
    
    setCameraWithFocus(0);

    Contexts contexts;
    getContexts(contexts);
    
    if (contexts.empty())
    {
        osg::notify(osg::INFO)<<"Viewer::realize() - No valid contexts found, setting up view across all screens."<<std::endl;
    
        // no windows are already set up so set up a default view        
        
        const char* ptr = 0;
        int screenNum = -1;
        if ((ptr = getenv("OSG_SCREEN")) != 0)
        {
            if (strlen(ptr)!=0) screenNum = atoi(ptr);
            else screenNum = -1;
        }
        
        int x = -1, y = -1, width = -1, height = -1;
        if ((ptr = getenv("OSG_WINDOW")) != 0)
        {
            std::istringstream iss(ptr);
            iss >> x >> y >> width >> height;
        }

        if (width>0 && height>0)
        {
            if (screenNum>=0) setUpViewInWindow(x, y, width, height, screenNum);
            else setUpViewInWindow(x,y,width,height);
        }
        else if (screenNum>=0)
        {
            setUpViewOnSingleScreen(screenNum);
        }
        else
        {
            setUpViewAcrossAllScreens();
        }

        getContexts(contexts);
    }

    if (contexts.empty())
    {
        osg::notify(osg::NOTICE)<<"Viewer::realize() - failed to set up any windows"<<std::endl;
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
    
    // initialize the global timer to be relative to the current time.
    osg::Timer::instance()->setStartTick();

    // pass on the start tick to all the associated eventqueues
    setStartTick(osg::Timer::instance()->getStartTick());

    setUpThreading();
    
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


void Viewer::frame(double simulationTime)
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
    advance(simulationTime);
    
    eventTraversal();
    updateTraversal();
    renderingTraversals();
}

void Viewer::advance(double simulationTime)
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
    
    if (getStats() && getStats()->collectStats("frame_rate"))
    {
        // update previous frame stats
        double deltaFrameTime = _frameStamp->getReferenceTime() - prevousReferenceTime;
        getStats()->setAttribute(previousFrameNumber, "Frame duration", deltaFrameTime);
        getStats()->setAttribute(previousFrameNumber, "Frame rate", 1.0/deltaFrameTime);

        // update current frames stats
        getStats()->setAttribute(_frameStamp->getFrameNumber(), "Reference time", _frameStamp->getReferenceTime());
    }

    if (osg::Referenced::getDeleteHandler())
    {
        osg::Referenced::getDeleteHandler()->flush();
        osg::Referenced::getDeleteHandler()->setFrameNumber(_frameStamp->getFrameNumber());
    }

}

void Viewer::eventTraversal()
{
    if (_done) return;

    double beginEventTraversal = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());

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
                                if (camera->getView()==this && 
                                    camera->getAllowEventFocus() &&
                                    camera->getRenderTargetImplementation()==osg::Camera::FRAME_BUFFER)
                                {
                                    osg::Viewport* viewport = camera ? camera->getViewport() : 0;
                                    if (viewport && 
                                        x >= viewport->x() && y >= viewport->y() &&
                                        x <= (viewport->x()+viewport->width()) && y <= (viewport->y()+viewport->height()) )
                                    {
                                        // osg::notify(osg::NOTICE)<<"setCamera with focus "<<camera->getName()<<" x="<<x<<" y="<<y<<std::endl;
                                        setCameraWithFocus(camera);
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

                        // osg::notify(osg::NOTICE)<<"pointer event new_coord.x()="<<new_coord.x()<<" new_coord.y()="<<new_coord.y()<<std::endl;

                        event->setInputRange(eventState->getXmin(), eventState->getYmin(), eventState->getXmax(), eventState->getYmax());
                        event->setX(x);
                        event->setY(y);
                        event->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);

                    }
                    else
                    {
                        x = eventState->getXmin() + (x/double(gw->getTraits()->width))*(eventState->getXmax() - eventState->getXmin());
                        y = eventState->getYmin() + (y/double(gw->getTraits()->height))*(eventState->getYmax() - eventState->getYmin());
                        // osg::notify(osg::NOTICE)<<"new x = "<<x<<" new y = "<<y<<std::endl;

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
                        _currentContext = NULL; 
                        
                        if (wasThreading) startThreading();
                        
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

    for(osgGA::EventQueue::Events::iterator itr = events.begin();
        itr != events.end();
        ++itr)
    {
        osgGA::GUIEventAdapter* event = itr->get();

        for(EventHandlers::iterator hitr = _eventHandlers.begin();
            hitr != _eventHandlers.end();
            ++hitr)
        {
            if ((*hitr)->handle( *event, *this, 0, 0)) event->setHandled(true);
        }

        if (_cameraManipulator.valid())
        {
            if (_cameraManipulator->handle( *event, *this)) event->setHandled(true);
        }
        
    }

    if (_eventVisitor.valid() && _scene.valid())
    {
        _eventVisitor->setFrameStamp(getFrameStamp());
        _eventVisitor->setTraversalNumber(getFrameStamp()->getFrameNumber());

        for(osgGA::EventQueue::Events::iterator itr = events.begin();
            itr != events.end();
            ++itr)
        {
            osgGA::GUIEventAdapter* event = itr->get();

            _eventVisitor->reset();
            _eventVisitor->addEvent( event );

            getSceneData()->accept(*_eventVisitor);

            // call any camera update callbacks, but only traverse that callback, don't traverse its subgraph
            // leave that to the scene update traversal.
            osg::NodeVisitor::TraversalMode tm = _eventVisitor->getTraversalMode();
            _eventVisitor->setTraversalMode(osg::NodeVisitor::TRAVERSE_NONE);

            if (_camera.valid() && _camera->getEventCallback()) _camera->accept(*_eventVisitor);

            for(unsigned int i=0; i<getNumSlaves(); ++i)
            {
                osg::Camera* camera = getSlave(i)._camera.get();
                if (camera && camera->getEventCallback()) camera->accept(*_eventVisitor);
            }

            _eventVisitor->setTraversalMode(tm);

        }
    }

    if (getStats() && getStats()->collectStats("event"))
    {
        double endEventTraversal = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());

        // update current frames stats
        getStats()->setAttribute(_frameStamp->getFrameNumber(), "Event traversal begin time", beginEventTraversal);
        getStats()->setAttribute(_frameStamp->getFrameNumber(), "Event traversal end time", endEventTraversal);
        getStats()->setAttribute(_frameStamp->getFrameNumber(), "Event traversal time taken", endEventTraversal-beginEventTraversal);
    }


}

void Viewer::addUpdateOperation(osg::Operation* operation)
{
    if (!operation) return;

    if (!_updateOperations) _updateOperations = new osg::OperationQueue;
    
    _updateOperations->add(operation);
}

void Viewer::removeUpdateOperation(osg::Operation* operation)
{
    if (!operation) return;

    if (_updateOperations.valid())
    {
        _updateOperations->remove(operation);
    } 
}

void Viewer::updateTraversal()
{
    if (_done) return;

    double beginUpdateTraversal = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());

    if (getSceneData())
    {
        getSceneData()->accept(*_updateVisitor);
    }
    
    if (_scene->getDatabasePager())
    {    
        // syncronize changes required by the DatabasePager thread to the scene graph
        _scene->getDatabasePager()->updateSceneGraph(_frameStamp->getReferenceTime());
    }

    if (_updateOperations.valid())
    {
        _updateOperations->runOperations(this);
    }

    {
        // call any camera update callbacks, but only traverse that callback, don't traverse its subgraph
        // leave that to the scene update traversal.
        osg::NodeVisitor::TraversalMode tm = _updateVisitor->getTraversalMode();
        _updateVisitor->setTraversalMode(osg::NodeVisitor::TRAVERSE_NONE);

        if (_camera.valid() && _camera->getUpdateCallback()) _camera->accept(*_updateVisitor);

        for(unsigned int i=0; i<getNumSlaves(); ++i)
        {
            osg::Camera* camera = getSlave(i)._camera.get();
            if (camera && camera->getUpdateCallback()) camera->accept(*_updateVisitor);
        }

        _updateVisitor->setTraversalMode(tm);
    }

    if (_cameraManipulator.valid())
    {
        setFusionDistance( getCameraManipulator()->getFusionDistanceMode(),
                            getCameraManipulator()->getFusionDistanceValue() );

        _camera->setViewMatrix(_cameraManipulator->getInverseMatrix());
    }

    updateSlaves();

    if (getStats() && getStats()->collectStats("update"))
    {
        double endUpdateTraversal = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());

        // update current frames stats
        getStats()->setAttribute(_frameStamp->getFrameNumber(), "Update traversal begin time", beginUpdateTraversal);
        getStats()->setAttribute(_frameStamp->getFrameNumber(), "Update traversal end time", endUpdateTraversal);
        getStats()->setAttribute(_frameStamp->getFrameNumber(), "Update traversal time taken", endUpdateTraversal-beginUpdateTraversal);
    }
}

void Viewer::renderingTraversals()
{
    // check to see if windows are still valid
    checkWindowStatus();

    if (_done) return;

    double beginRenderingTraversals = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());

    osgDB::DatabasePager* dp = _scene.valid() ? _scene->getDatabasePager() : 0;
    if (dp)
    {
        dp->signalBeginFrame(getFrameStamp());
    }

    // osg::notify(osg::NOTICE)<<std::endl<<"Joing _startRenderingBarrier block"<<std::endl;
    

    Contexts contexts;
    getContexts(contexts);

    Cameras cameras;
    getCameras(cameras);
    
    Contexts::iterator itr;

    // dispatch the the rendering threads
    if (_startRenderingBarrier.valid()) _startRenderingBarrier->block();

    if (_endDynamicDrawBlock.valid())
    {
        _endDynamicDrawBlock->reset();
    }
    
    // reset any double buffer graphics objects
    for(Cameras::iterator camItr = cameras.begin();
        camItr != cameras.end();
        ++camItr)
    {
        osg::Camera* camera = *camItr;
        Renderer* renderer = dynamic_cast<Renderer*>(camera->getRenderer());
        if (renderer)
        {
            if (!renderer->getGraphicsThreadDoesCull() && !(camera->getCameraThread()))
            {
                renderer->cull();
            }
        }
    }

    for(itr = contexts.begin();
        itr != contexts.end();
        ++itr)
    {
        if (_done) return;
        if (!((*itr)->getGraphicsThread()) && (*itr)->valid())
        { 
            makeCurrent(*itr);
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

        if (!((*itr)->getGraphicsThread()) && (*itr)->valid())
        { 
            makeCurrent(*itr);
            (*itr)->swapBuffers();
        }
    }

    if (dp)
    {
        dp->signalEndFrame();
    }

    // wait till the dynamic draw is complete.
    if (_endDynamicDrawBlock.valid()) 
    {
        // osg::Timer_t startTick = osg::Timer::instance()->tick();
        _endDynamicDrawBlock->block();
        // osg::notify(osg::NOTICE)<<"Time waiting "<<osg::Timer::instance()->delta_m(startTick, osg::Timer::instance()->tick())<<std::endl;;
    }


    if (getStats() && getStats()->collectStats("update"))
    {
        double endRenderingTraversals = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());

        // update current frames stats
        getStats()->setAttribute(_frameStamp->getFrameNumber(), "Rendering traversals begin time ", beginRenderingTraversals);
        getStats()->setAttribute(_frameStamp->getFrameNumber(), "Rendering traversals end time ", endRenderingTraversals);
        getStats()->setAttribute(_frameStamp->getFrameNumber(), "Rendering traversals time taken", endRenderingTraversals-beginRenderingTraversals);
    }
}

void Viewer::getUsage(osg::ApplicationUsage& usage) const
{
    if (_cameraManipulator.valid())
    {
        _cameraManipulator->getUsage(usage);
    }

    for(EventHandlers::const_iterator hitr = _eventHandlers.begin();
        hitr != _eventHandlers.end();
        ++hitr)
    {
        (*hitr)->getUsage(usage);
    }

}
