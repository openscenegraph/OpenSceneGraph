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
#include <osg/TextureRectangle>
#include <osg/TextureCubeMap>

#include <osgGA/TrackballManipulator>
#include <osgViewer/CompositeViewer>
#include <osgViewer/Renderer>
#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osg/io_utils>

using namespace osgViewer;

CompositeViewer::CompositeViewer()
{
    constructorInit();
}

CompositeViewer::CompositeViewer(const CompositeViewer& cv,const osg::CopyOp& /*copyop*/):
    osg::Object(true),
    ViewerBase(cv)
{
    constructorInit();
}

CompositeViewer::CompositeViewer(osg::ArgumentParser& arguments)
{
    constructorInit();

    arguments.getApplicationUsage()->addCommandLineOption("--SingleThreaded","Select SingleThreaded threading model for viewer.");
    arguments.getApplicationUsage()->addCommandLineOption("--CullDrawThreadPerContext","Select CullDrawThreadPerContext threading model for viewer.");
    arguments.getApplicationUsage()->addCommandLineOption("--DrawThreadPerContext","Select DrawThreadPerContext threading model for viewer.");
    arguments.getApplicationUsage()->addCommandLineOption("--CullThreadPerCameraDrawThreadPerContext","Select CullThreadPerCameraDrawThreadPerContext threading model for viewer.");

    arguments.getApplicationUsage()->addCommandLineOption("--run-on-demand","Set the run methods frame rate management to only rendering frames when required.");
    arguments.getApplicationUsage()->addCommandLineOption("--run-continuous","Set the run methods frame rate management to rendering frames continuously.");
    arguments.getApplicationUsage()->addCommandLineOption("--run-max-frame-rate","Set the run methods maximum permissible frame rate, 0.0 is default and switching off frame rate capping.");


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


    while(arguments.read("--run-on-demand")) { setRunFrameScheme(ON_DEMAND); }
    while(arguments.read("--run-continuous")) { setRunFrameScheme(CONTINUOUS); }

    double runMaxFrameRate;
    while(arguments.read("--run-max-frame-rate", runMaxFrameRate)) { setRunMaxFrameRate(runMaxFrameRate); }


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
    OSG_INFO<<"CompositeViewer::~CompositeViewer()"<<std::endl;

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

    OSG_INFO<<"finished CompositeViewer::~CompositeViewer()"<<std::endl;
}

bool CompositeViewer::readConfiguration(const std::string& filename)
{
    OSG_NOTICE<<"CompositeViewer::readConfiguration("<<filename<<")"<<std::endl;
    osg::ref_ptr<osg::Object> obj = osgDB::readObjectFile(filename);
    osgViewer::View * view = dynamic_cast<osgViewer::View *>(obj.get());
    if (view)
    {
        addView(view);
        return true;
    }
    return false;
}


void CompositeViewer::addView(osgViewer::View* view)
{
    if (!view) return;

    bool alreadyRealized = isRealized();

    bool threadsWereRunning = _threadsRunning;
    if (threadsWereRunning) stopThreading();

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

    if (threadsWereRunning) startThreading();
}

void CompositeViewer::removeView(osgViewer::View* view)
{
    for(RefViews::iterator itr = _views.begin();
        itr != _views.end();
        ++itr)
    {
        if (*itr == view)
        {
            bool threadsWereRunning = _threadsRunning;
            if (threadsWereRunning) stopThreading();

            view->_viewerBase = 0;

            _views.erase(itr);

            if (threadsWereRunning) startThreading();

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

bool CompositeViewer::checkNeedToDoFrame()
{
    if (_requestRedraw) return true;
    if (_requestContinousUpdate) return true;

    for(RefViews::iterator itr = _views.begin();
        itr != _views.end();
        ++itr)
    {
        osgViewer::View* view = itr->get();
        if (view)
        {
            // If the database pager is going to update the scene the render flag is
            // set so that the updates show up
            if (view->getDatabasePager()->requiresUpdateSceneGraph() ||
                view->getDatabasePager()->getRequestsInProgress()) return true;

            // if there update callbacks then we need to do frame.
            if (view->getCamera()->getUpdateCallback()) return true;
            if (view->getSceneData()!=0 && view->getSceneData()->getNumChildrenRequiringUpdateTraversal()>0) return true;
        }
    }

    // check if events are available and need processing
    if (checkEvents()) return true;

    if (_requestRedraw) return true;
    if (_requestContinousUpdate) return true;

    return false;
}


bool CompositeViewer::checkEvents()
{
    for(RefViews::iterator itr = _views.begin();
        itr != _views.end();
        ++itr)
    {
        osgViewer::View* view = itr->get();
        if (view)
        {
            // check events from any attached sources
            for(View::Devices::iterator eitr = view->getDevices().begin();
                eitr != view->getDevices().end();
                ++eitr)
            {
                osgGA::Device* es = eitr->get();
                if (es->getCapabilities() & osgGA::Device::RECEIVE_EVENTS)
                {
                    if (es->checkEvents()) return true;
                }

            }
        }
    }

    // get events from all windows attached to Viewer.
    Windows windows;
    getWindows(windows);
    for(Windows::iterator witr = windows.begin();
        witr != windows.end();
        ++witr)
    {
        if ((*witr)->checkEvents()) return true;
    }

    return false;
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
    double delta_ticks = (time-currentTime)/(osg::Timer::instance()->getSecondsPerTick());
    if (delta_ticks>=0) tick += osg::Timer_t(delta_ticks);
    else tick -= osg::Timer_t(-delta_ticks);

    // assign the new start tick
    setStartTick(tick);
}



void CompositeViewer::viewerInit()
{
    OSG_INFO<<"CompositeViewer::init()"<<std::endl;

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

void CompositeViewer::getViews(Views& views, bool /*onlyValid*/)
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
    //OSG_INFO<<"CompositeViewer::realize()"<<std::endl;

    setCameraWithFocus(0);

    if (_views.empty())
    {
        OSG_NOTICE<<"CompositeViewer::realize() - No views to realize."<<std::endl;
        _done = true;
        return;
    }

    Contexts contexts;
    getContexts(contexts);

    if (contexts.empty())
    {
        OSG_INFO<<"CompositeViewer::realize() - No valid contexts found, setting up view across all screens."<<std::endl;

        // no windows are already set up so set up a default view
        _views[0]->setUpViewAcrossAllScreens();

        getContexts(contexts);
    }

    if (contexts.empty())
    {
        OSG_NOTICE<<"CompositeViewer::realize() - failed to set up any windows"<<std::endl;
        _done = true;
        return;
    }

    // get the display settings that will be active for this viewer
    osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();

    // pass on the display settings to the WindowSystemInterface.
    if (wsi && wsi->getDisplaySettings()==0) wsi->setDisplaySettings(ds);

    unsigned int maxTexturePoolSize = ds->getMaxTexturePoolSize();
    unsigned int maxBufferObjectPoolSize = ds->getMaxBufferObjectPoolSize();

    for(Contexts::iterator citr = contexts.begin();
        citr != contexts.end();
        ++citr)
    {
        osg::GraphicsContext* gc = *citr;

        if (ds->getSyncSwapBuffers()) gc->setSwapCallback(new osg::SyncSwapBuffersCallback);

        // set the pool sizes, 0 the default will result in no GL object pools.
        gc->getState()->setMaxTexturePoolSize(maxTexturePoolSize);
        gc->getState()->setMaxBufferObjectPoolSize(maxBufferObjectPoolSize);

        gc->realize();

        if (_realizeOperation.valid() && gc->valid())
        {
            gc->makeCurrent();

            (*_realizeOperation)(gc);

            gc->releaseContext();
        }
    }

    // attach contexts to _incrementalCompileOperation if attached.
    if (_incrementalCompileOperation) _incrementalCompileOperation->assignContexts(contexts);


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

    // configure threading.
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

void CompositeViewer::advance(double simulationTime)
{
    if (_done) return;

    double previousReferenceTime = _frameStamp->getReferenceTime();
    unsigned int previousFrameNumber = _frameStamp->getFrameNumber();


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
        double deltaFrameTime = _frameStamp->getReferenceTime() - previousReferenceTime;
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


void CompositeViewer::generateSlavePointerData(osg::Camera* camera, osgGA::GUIEventAdapter& event)
{
    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(event.getGraphicsContext());
    if (!gw) return;

    // What type of Camera is it?
    // 1) Master Camera : do nothin extra
    // 2) Slave Camera, Relative RF, Same scene graph as master : transform coords into Master Camera and add to PointerData list
    // 3) Slave Camera, Relative RF, Different scene graph from master : do nothing extra?
    // 4) Slave Camera, Absolute RF, Same scene graph as master : do nothing extra?
    // 5) Slave Camera, Absolute RF, Different scene graph : do nothing extra?
    // 6) Slave Camera, Absolute RF, Different scene graph but a distortion correction subgraph depending upon RTT Camera (slave or master)
    //                              : project ray into RTT Camera's clip space, and RTT Camera's is Relative RF and sharing same scene graph as master then transform coords.

    // if camera isn't the master it must be a slave and could need reprojecting.


    osgViewer::View* view = dynamic_cast<osgViewer::View*>(camera->getView());
    if (!view) return;

    osg::Camera* view_masterCamera = view->getCamera();
    if (camera!=view_masterCamera)
    {
        float x = event.getX();
        float y = event.getY();

        bool invert_y = event.getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS;
        if (invert_y && gw->getTraits()) y = gw->getTraits()->height - y;

        double master_min_x = -1.0;
        double master_max_x = 1.0;
        double master_min_y = -1.0;
        double master_max_y = 1.0;

        osg::Matrix masterCameraVPW = view_masterCamera->getViewMatrix() * view_masterCamera->getProjectionMatrix();
        if (view_masterCamera->getViewport())
        {
            osg::Viewport* viewport = view_masterCamera->getViewport();
            master_min_x = viewport->x();
            master_min_y = viewport->y();
            master_max_x = viewport->x()+viewport->width();
            master_max_y = viewport->y()+viewport->height();
            masterCameraVPW *= viewport->computeWindowMatrix();
        }

        // slave Camera tahnks to sharing the same View
        osg::View::Slave* slave = view ? view->findSlaveForCamera(camera) : 0;
        if (slave)
        {
            if (camera->getReferenceFrame()==osg::Camera::RELATIVE_RF && slave->_useMastersSceneData)
            {
                osg::Viewport* viewport = camera->getViewport();
                osg::Matrix localCameraVPW = camera->getViewMatrix() * camera->getProjectionMatrix();
                if (viewport) localCameraVPW *= viewport->computeWindowMatrix();

                osg::Matrix matrix( osg::Matrix::inverse(localCameraVPW) * masterCameraVPW );
                osg::Vec3d new_coord = osg::Vec3d(x,y,0.0) * matrix;
                //OSG_NOTICE<<"    pointer event new_coord.x()="<<new_coord.x()<<" new_coord.y()="<<new_coord.y()<<std::endl;
                event.addPointerData(new osgGA::PointerData(view_masterCamera, new_coord.x(), master_min_x, master_max_x,
                                                                               new_coord.y(), master_min_y, master_max_y));
            }
            else if (!slave->_useMastersSceneData)
            {
                // Are their any RTT Camera's that this Camera depends upon for textures?

                osg::ref_ptr<osgUtil::LineSegmentIntersector> ray = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, x,y);
                osgUtil::IntersectionVisitor iv(ray.get());
                camera->accept(iv);
                if (ray->containsIntersections())
                {
                    osg::Vec3 tc;
                    osg::Texture* texture = ray->getFirstIntersection().getTextureLookUp(tc);
                    if (texture)
                    {
                        // look up Texture in RTT Camera's.
                        for(unsigned int i=0; i<view->getNumSlaves();++i)
                        {
                            osg::Camera* slave_camera = view->getSlave(i)._camera.get();
                            if (slave_camera)
                            {
                                osg::Camera::BufferAttachmentMap::const_iterator ba_itr = slave_camera->getBufferAttachmentMap().find(osg::Camera::COLOR_BUFFER);
                                if (ba_itr != slave_camera->getBufferAttachmentMap().end())
                                {
                                    if (ba_itr->second._texture == texture)
                                    {
                                        osg::TextureRectangle* tr = dynamic_cast<osg::TextureRectangle*>(ba_itr->second._texture.get());
                                        osg::TextureCubeMap* tcm = dynamic_cast<osg::TextureCubeMap*>(ba_itr->second._texture.get());
                                        if (tr)
                                        {
                                            event.addPointerData(new osgGA::PointerData(slave_camera, tc.x(), 0.0f, static_cast<float>(tr->getTextureWidth()),
                                                                                                           tc.y(), 0.0f, static_cast<float>(tr->getTextureHeight())));
                                        }
                                        else if (tcm)
                                        {
                                            OSG_INFO<<"  Slave has matched texture cubemap"<<ba_itr->second._texture.get()<<", "<<ba_itr->second._face<<std::endl;
                                        }
                                        else
                                        {
                                            event.addPointerData(new osgGA::PointerData(slave_camera, tc.x(), 0.0f, 1.0f,
                                                                                                           tc.y(), 0.0f, 1.0f));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void CompositeViewer::generatePointerData(osgGA::GUIEventAdapter& event)
{
    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(event.getGraphicsContext());
    if (!gw) return;

    float x = event.getX();
    float y = event.getY();

    bool invert_y = event.getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS;
    if (invert_y && gw->getTraits()) y = gw->getTraits()->height - y;

    event.addPointerData(new osgGA::PointerData(gw, x, 0, gw->getTraits()->width,
                                                    y, 0, gw->getTraits()->height));

    event.setMouseYOrientationAndUpdateCoords(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);

    typedef std::vector<osg::Camera*> CameraVector;
    CameraVector activeCameras;

    osg::GraphicsContext::Cameras& cameras = gw->getCameras();
    for(osg::GraphicsContext::Cameras::iterator citr = cameras.begin();
        citr != cameras.end();
        ++citr)
    {
        osg::Camera* camera = *citr;
        if (camera->getAllowEventFocus() &&
            camera->getRenderTargetImplementation()==osg::Camera::FRAME_BUFFER)
        {
            osg::Viewport* viewport = camera ? camera->getViewport() : 0;
            if (viewport &&
                x >= viewport->x() && y >= viewport->y() &&
                x <= (viewport->x()+viewport->width()) && y <= (viewport->y()+viewport->height()) )
            {
                activeCameras.push_back(camera);
            }
        }
    }

    std::sort(activeCameras.begin(), activeCameras.end(), osg::CameraRenderOrderSortOp());

    osg::Camera* camera = activeCameras.empty() ? 0 : activeCameras.back();

    if (camera)
    {
        osg::Viewport* viewport = camera ? camera->getViewport() : 0;

        event.addPointerData(new osgGA::PointerData(camera, (x-viewport->x())/viewport->width()*2.0f-1.0f, -1.0, 1.0,
                                                            (y-viewport->y())/viewport->height()*2.0f-1.0f, -1.0, 1.0));

        osgViewer::View* view = dynamic_cast<osgViewer::View*>(camera->getView());
        osg::Camera* view_masterCamera = view ? view->getCamera() : 0;

        // if camera isn't the master it must be a slave and could need reprojecting.
        if (view && camera!=view_masterCamera)
        {
            generateSlavePointerData(camera, event);
        }
    }
}

void CompositeViewer::reprojectPointerData(osgGA::GUIEventAdapter& source_event, osgGA::GUIEventAdapter& dest_event)
{
    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(dest_event.getGraphicsContext());
    if (!gw) return;

    float x = dest_event.getX();
    float y = dest_event.getY();

    bool invert_y = dest_event.getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS;
    if (invert_y && gw->getTraits()) y = gw->getTraits()->height - y;

    dest_event.addPointerData(new osgGA::PointerData(gw, x, 0, gw->getTraits()->width,
                                                         y, 0, gw->getTraits()->height));

    dest_event.setMouseYOrientationAndUpdateCoords(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);

    osg::Camera* camera = (source_event.getNumPointerData()>=2) ? dynamic_cast<osg::Camera*>(source_event.getPointerData(1)->object.get()) : 0;
    osg::Viewport* viewport = camera ? camera->getViewport() : 0;

    if (!viewport) return;

    dest_event.addPointerData(new osgGA::PointerData(camera, (x-viewport->x())/viewport->width()*2.0f-1.0f, -1.0, 1.0,
                                                             (y-viewport->y())/viewport->height()*2.0f-1.0f, -1.0, 1.0));

    osgViewer::View* view = dynamic_cast<osgViewer::View*>(camera->getView());
    osg::Camera* view_masterCamera = view ? view->getCamera() : 0;

    // if camera isn't the master it must be a slave and could need reprojecting.
    if (view && camera!=view_masterCamera)
    {
        generateSlavePointerData(camera, dest_event);
    }
}

struct SortEvents
{
    bool operator() (const osg::ref_ptr<osgGA::Event>& lhs,const osg::ref_ptr<osgGA::Event>& rhs) const
    {
        return lhs->getTime() < rhs->getTime();
    }
};

void CompositeViewer::eventTraversal()
{
    if (_done) return;

    if (_views.empty()) return;

    double cutOffTime = _frameStamp->getReferenceTime();

    double beginEventTraversal = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());

    // need to copy events from the GraphicsWindow's into local EventQueue for each view;
    typedef std::map<osgViewer::View*, osgGA::EventQueue::Events> ViewEventsMap;
    ViewEventsMap viewEventsMap;

    Contexts contexts;
    getContexts(contexts);

    // set done if there are no windows
    checkWindowStatus(contexts);
    if (_done) return;

    osgGA::EventQueue::Events all_events;

    for(Contexts::iterator citr = contexts.begin();
        citr != contexts.end();
        ++citr)
    {
        osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(*citr);
        if (gw)
        {
            gw->checkEvents();

            osgGA::EventQueue::Events gw_events;
            gw->getEventQueue()->takeEvents(gw_events, cutOffTime);

            for(osgGA::EventQueue::Events::iterator itr = gw_events.begin();
                itr != gw_events.end();
                ++itr)
            {
                osgGA::GUIEventAdapter* ea = (*itr)->asGUIEventAdapter();
                if (ea) ea->setGraphicsContext(gw);
            }

            all_events.insert(all_events.end(), gw_events.begin(), gw_events.end());
        }
    }

    // sort all the events in time order so we can make sure we pass them all on in the correct order.
    all_events.sort(SortEvents());

    // pass on pointer data onto non mouse events to keep the position data usable by all recipients of all events.
    for(osgGA::EventQueue::Events::iterator itr = all_events.begin();
        itr != all_events.end();
        ++itr)
    {
        osgGA::GUIEventAdapter* event = (*itr)->asGUIEventAdapter();
        if (!event) continue;

        switch(event->getEventType())
        {
            case(osgGA::GUIEventAdapter::PUSH):
            case(osgGA::GUIEventAdapter::RELEASE):
            case(osgGA::GUIEventAdapter::DOUBLECLICK):
            case(osgGA::GUIEventAdapter::MOVE):
            case(osgGA::GUIEventAdapter::DRAG):
            {
                if ((event->getEventType()!=osgGA::GUIEventAdapter::DRAG && event->getEventType()!=osgGA::GUIEventAdapter::RELEASE) ||
                    !_previousEvent ||
                    _previousEvent->getGraphicsContext()!=event->getGraphicsContext() ||
                    _previousEvent->getNumPointerData()<2)
                {
                    generatePointerData(*event);
                }
                else
                {
                    reprojectPointerData(*_previousEvent, *event);
                }

                _previousEvent = event;

                break;
            }
            default:
                if (_previousEvent.valid()) event->copyPointerDataFrom(*_previousEvent);
                break;
        }

        osgGA::PointerData* pd = event->getNumPointerData()>0 ? event->getPointerData(event->getNumPointerData()-1) : 0;
        osg::Camera* camera = pd ? dynamic_cast<osg::Camera*>(pd->object.get()) : 0;
        osgViewer::View* view = camera ? dynamic_cast<osgViewer::View*>(camera->getView()) : 0;

        if (!view)
        {
            if (_viewWithFocus.valid())
            {
                // OSG_NOTICE<<"Falling back to using _viewWithFocus"<<std::endl;
                view = _viewWithFocus.get();
            }
            else if (!_views.empty())
            {
                // OSG_NOTICE<<"Falling back to using first view as one with focus"<<std::endl;
                view = _views[0].get();
            }
        }

        // reassign view with focus
        if (_viewWithFocus != view)  _viewWithFocus = view;

        if (view)
        {
            viewEventsMap[view].push_back( event );

            osgGA::GUIEventAdapter* eventState = view->getEventQueue()->getCurrentEventState();
            eventState->copyPointerDataFrom(*event);
        }

        _previousEvent = event;
    }

    // handle any close windows
    for(osgGA::EventQueue::Events::iterator itr = all_events.begin();
        itr != all_events.end();
        ++itr)
    {
        osgGA::GUIEventAdapter* event = (*itr)->asGUIEventAdapter();
        if (!event) continue;

        switch(event->getEventType())
        {
            case(osgGA::GUIEventAdapter::CLOSE_WINDOW):
            {
                bool wasThreading = areThreadsRunning();
                if (wasThreading) stopThreading();

                if (event->getGraphicsContext())
                {
                    event->getGraphicsContext()->close();
                }

                if (wasThreading) startThreading();

                break;
            }
            default:
                break;
        }
    }


    for(RefViews::iterator vitr = _views.begin();
        vitr != _views.end();
        ++vitr)
    {
        View* view = vitr->get();

        // get events from user Devices attached to Viewer.
        for(osgViewer::View::Devices::iterator eitr = view->getDevices().begin();
            eitr != view->getDevices().end();
            ++eitr)
        {
            osgGA::Device* es = eitr->get();
            if (es->getCapabilities() & osgGA::Device::RECEIVE_EVENTS)
                es->checkEvents();

            // open question, will we need to reproject mouse coordinates into current view's coordinate frame as is down for GraphicsWindow provided events?
            // for now assume now and just get the events directly without any reprojection.
            es->getEventQueue()->takeEvents(viewEventsMap[view], cutOffTime);
        }

        // create a frame event for the new frame.
        {
            osg::ref_ptr<osgGA::GUIEventAdapter> event = view->getEventQueue()->frame( getFrameStamp()->getReferenceTime() );
            
            if (!_previousEvent || _previousEvent->getNumPointerData()<2)
            {
                generatePointerData(*event);
            }
            else
            {
                reprojectPointerData(*_previousEvent, *event);
            }
        }
        

        view->getEventQueue()->takeEvents(viewEventsMap[view], cutOffTime);
    }

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
                osgGA::GUIEventAdapter* event = (*itr)->asGUIEventAdapter();
                if (!event) continue;

                // ignore event if it's already been handled.
                if (event->getHandled()) continue;

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

            if (view && view->getSceneData())
            {
                for(osgGA::EventQueue::Events::iterator itr = veitr->second.begin();
                    itr != veitr->second.end();
                    ++itr)
                {
                    osgGA::Event* event = itr->get();

                    _eventVisitor->reset();
                    _eventVisitor->addEvent( event );

                    view->getSceneData()->accept(*_eventVisitor);

                    // Do EventTraversal for slaves with their own subgraph
                    for(unsigned int i=0; i<view->getNumSlaves(); ++i)
                    {
                        osg::View::Slave& slave = view->getSlave(i);
                        osg::Camera* camera = slave._camera.get();
                        if(camera && !slave._useMastersSceneData)
                        {
                            camera->accept(*_eventVisitor);
                        }
                    }

                    // call any camera event callbacks, but only traverse that callback, don't traverse its subgraph
                    // leave that to the scene update traversal.
                    osg::NodeVisitor::TraversalMode tm = _eventVisitor->getTraversalMode();
                    _eventVisitor->setTraversalMode(osg::NodeVisitor::TRAVERSE_NONE);

                    if (view->getCamera() && view->getCamera()->getEventCallback()) view->getCamera()->accept(*_eventVisitor);

                    for(unsigned int i=0; i<view->getNumSlaves(); ++i)
                    {
                        osg::View::Slave& slave = view->getSlave(i);
                        osg::Camera* camera = view->getSlave(i)._camera.get();
                        if (camera && slave._useMastersSceneData && camera->getEventCallback())
                        {
                            camera->accept(*_eventVisitor);
                        }
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
        _eventVisitor->setActionAdapter(view);

        for(osgGA::EventQueue::Events::iterator itr = veitr->second.begin();
            itr != veitr->second.end();
            ++itr)
        {
            osgGA::Event* event = itr->get();
            for(View::EventHandlers::iterator hitr = view->getEventHandlers().begin();
                hitr != view->getEventHandlers().end();
                ++hitr)
            {
                (*hitr)->handle( event, view, _eventVisitor.get());
            }
        }
    }

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
            osgGA::Event* event = itr->get();
            if (view->getCameraManipulator())
            {
                view->getCameraManipulator()->handle( event, view, _eventVisitor.get());
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


    if (_incrementalCompileOperation.valid())
    {
        // merge subgraphs that have been compiled by the incremental compiler operation.
        _incrementalCompileOperation->mergeCompiledSubgraphs(getFrameStamp());
    }

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
            // Do UpdateTraversal for slaves with their own subgraph
            for(unsigned int i=0; i<view->getNumSlaves(); ++i)
            {
                osg::View::Slave& slave = view->getSlave(i);
                osg::Camera* camera = slave._camera.get();
                if(camera && !slave._useMastersSceneData)
                {
                    camera->accept(*_updateVisitor);
                }
            }

            // call any camera update callbacks, but only traverse that callback, don't traverse its subgraph
            // leave that to the scene update traversal.
            osg::NodeVisitor::TraversalMode tm = _updateVisitor->getTraversalMode();
            _updateVisitor->setTraversalMode(osg::NodeVisitor::TRAVERSE_NONE);

            if (view->getCamera() && view->getCamera()->getUpdateCallback()) view->getCamera()->accept(*_updateVisitor);

            for(unsigned int i=0; i<view->getNumSlaves(); ++i)
            {
                osg::View::Slave& slave = view->getSlave(i);
                osg::Camera* camera = slave._camera.get();
                if (camera && slave._useMastersSceneData && camera->getUpdateCallback())
                {
                    camera->accept(*_updateVisitor);
                }
            }

            _updateVisitor->setTraversalMode(tm);
        }


        if (view->getCameraManipulator())
        {
            view->setFusionDistance( view->getCameraManipulator()->getFusionDistanceMode(),
                                    view->getCameraManipulator()->getFusionDistanceValue() );

            view->getCameraManipulator()->updateCamera(*(view->getCamera()));

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
