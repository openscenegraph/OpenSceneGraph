#include <osg/LightSource>
#include <osg/ApplicationUsage>

#include <osgUtil/UpdateVisitor>
#include <osgUtil/PickVisitor>

#include <osgDB/Registry>

#include <osgGA/AnimationPathManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/StateSetManipulator>

#include <osgProducer/Viewer>
#include <osgProducer/ViewerEventHandler>

using namespace Producer;
using namespace osgProducer;


Viewer::Viewer():
    _done(0),
    _frameNumber(0),
    _kbmcb(0),
    _recordingAnimationPath(false)
{
}

Viewer::Viewer(Producer::CameraConfig *cfg):
    OsgCameraGroup(cfg),
    _done(false),
    _frameNumber(0),
    _kbmcb(0),
    _recordingAnimationPath(false)
{
}

Viewer::Viewer(const std::string& configFile):
    OsgCameraGroup(configFile),
    _done(false),
    _frameNumber(0),
    _kbmcb(0),
    _recordingAnimationPath(false)
{
}

Viewer::Viewer(osg::ArgumentParser& arguments):
    OsgCameraGroup(arguments),
    _done(false),
    _frameNumber(0),
    _kbmcb(0),
    _recordingAnimationPath(false)
{
    // report the usage options.
    if (arguments.getApplicationUsage())
    {
        arguments.getApplicationUsage()->addCommandLineOption("-p <filename>","Specify camera path file to animate the camera through the loaded scene");
    }

    osg::DisplaySettings::instance()->readCommandLine(arguments);
    osgDB::readCommandLine(arguments);

    std::string pathfile;
    while (arguments.read("-p",pathfile))
    {
	osg::ref_ptr<osgGA::AnimationPathManipulator> apm = new osgGA::AnimationPathManipulator(pathfile);
	if( apm.valid() && apm->valid() ) 
        {
            unsigned int num = addCameraManipulator(apm.get());
            selectCameraManipulator(num);
        }
    }
}


void Viewer::setUpViewer(unsigned int options)
{

    // set up the keyboard and mouse handling.
    Producer::InputArea *ia = getCameraConfig()->getInputArea();
    Producer::KeyboardMouse *kbm = ia ?
                                   (new Producer::KeyboardMouse(ia)) : 
                                   (new Producer::KeyboardMouse(getCamera(0)->getRenderSurface()));

    // set up the time and frame counter.
    _frameNumber = 0;
    _start_tick = _timer.tick();

    // set the keyboard mouse callback to catch the events from the windows.
    if (!_kbmcb)
        _kbmcb = new osgProducer::KeyboardMouseCallback( kbm, _done, (options & ESCAPE_SETS_DONE)!=0 );
        
    _kbmcb->setStartTick(_start_tick);
    
    // register the callback with the keyboard mouse manger.
    kbm->setCallback( _kbmcb );
    //kbm->allowContinuousMouseMotionUpdate(true);
    kbm->startThread();



    // set the globa state
    
    osg::ref_ptr<osg::StateSet> globalStateSet = new osg::StateSet;
    setGlobalStateSet(globalStateSet.get());
    {
        globalStateSet->setGlobalDefaults();
        // enable depth testing by default.
        globalStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

        // enable lighting by default
        globalStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);

        // set up an alphafunc by default to speed up blending operations.
        osg::AlphaFunc* alphafunc = new osg::AlphaFunc;
        alphafunc->setFunction(osg::AlphaFunc::GREATER,0.0f);
        globalStateSet->setAttributeAndModes(alphafunc, osg::StateAttribute::ON);
    }
    
    
    // add either a headlight or sun light to the scene.
    osg::LightSource* lightsource = new osg::LightSource;
    setSceneDecorator(lightsource);
    {
        osg::Light* light = new osg::Light;
        lightsource->setLight(light);
        lightsource->setReferenceFrame(osg::LightSource::RELATIVE_TO_ABSOLUTE); // headlight.
        lightsource->setLocalStateSetModes(osg::StateAttribute::ON);
    }

    
    
    _updateVisitor = new osgUtil::UpdateVisitor;
    _updateVisitor->setFrameStamp(_frameStamp.get());

    // create a camera to use with the manipulators.
    _old_style_osg_camera = new osg::Camera;

    if (options&TRACKBALL_MANIPULATOR) addCameraManipulator(new osgGA::TrackballManipulator);
    if (options&FLIGHT_MANIPULATOR) addCameraManipulator(new osgGA::FlightManipulator);
    if (options&DRIVE_MANIPULATOR) addCameraManipulator(new osgGA::DriveManipulator);
    
    if (options&STATE_MANIPULATOR)
    {
        osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator;
        statesetManipulator->setStateSet(getGlobalStateSet());
        _eventHandlerList.push_back(statesetManipulator.get());
    }
        
    if (options&VIEWER_MANIPULATOR)
    {
        getEventHandlerList().push_back(new ViewerEventHandler(this));
    }
    
}

unsigned int Viewer::addCameraManipulator(osgGA::CameraManipulator* cm)
{
    if (!cm) return 0xfffff;
    
    // create a key switch manipulator if one doesn't already exist.
    if (!_keyswitchManipulator)
    {
        _keyswitchManipulator = new osgGA::KeySwitchCameraManipulator;
        _eventHandlerList.push_back(_keyswitchManipulator.get());
    }
    
    unsigned int num = _keyswitchManipulator->getNumCameraManipualtors();
    _keyswitchManipulator->addNumberedCameraManipulator(cm);
    
    return num;
}

bool Viewer::done() const
{
    return _done || !validForRendering();
}

void Viewer::setViewByMatrix( const Producer::Matrix & pm)
{
    CameraGroup::setViewByMatrix(pm);
    
    if (_keyswitchManipulator.valid() && _old_style_osg_camera.valid())
    {
        // now convert Producer matrix to an osg::Matrix so we can update
        // the internal camera...
        
        osg::Matrix matrix(pm.ptr());
    
        _old_style_osg_camera->home();
        _old_style_osg_camera->transformLookAt(matrix);
        osg::ref_ptr<osgProducer::EventAdapter> init_event = _kbmcb->createEventAdapter();
        _keyswitchManipulator->init(*init_event,*this);
    }
}

bool Viewer::realize( ThreadingModel thread_model )
{
    if( _realized ) return _realized;
    _thread_model = thread_model;
    return realize();
}

bool Viewer::realize()
{
    if (_realized) return _realized;

    OsgCameraGroup::realize();
    
    // force a sync before we intialize the keyswitch manipulator to home
    // so that Producer has a chance to set up the windows before we do
    // any work on them.
    OsgCameraGroup::sync();
 
    if (_keyswitchManipulator.valid() && _keyswitchManipulator->getCurrentCameraManipulator())
    {
        osg::ref_ptr<osgProducer::EventAdapter> init_event = _kbmcb->createEventAdapter();
        init_event->adaptFrame(0.0);
    
        _keyswitchManipulator->setCamera(_old_style_osg_camera.get());
        _keyswitchManipulator->setNode(getSceneDecorator());
        _keyswitchManipulator->home(*init_event,*this);
    }
    
    // set up osg::State objects with the _done prt to allow early termination of 
    // draw traversal.
    for(SceneHandlerList::iterator p=_shvec.begin(); p!=_shvec.end(); p++ )
    {
        (*p)->getState()->setAbortRenderingPtr(&_done);
        (*p)->setCamera(_old_style_osg_camera.get());
    }
    
    return _realized;
}

void Viewer::sync()
{
    OsgCameraGroup::sync();

    // set the frame stamp for the new frame.
    double time_since_start = _timer.delta_s(_start_tick,_timer.tick());
    _frameStamp->setFrameNumber(_frameNumber++);
    _frameStamp->setReferenceTime(time_since_start);
}        

void Viewer::update()
{
    // get the event since the last frame.
    osgProducer::KeyboardMouseCallback::EventQueue queue;
    if (_kbmcb) _kbmcb->getEventQueue(queue);

    // create an event to signal the new frame.
    osg::ref_ptr<osgProducer::EventAdapter> frame_event = new osgProducer::EventAdapter;
    frame_event->adaptFrame(_frameStamp->getReferenceTime());
    queue.push_back(frame_event);

    // dispatch the events in order of arrival.
    for(osgProducer::KeyboardMouseCallback::EventQueue::iterator event_itr=queue.begin();
        event_itr!=queue.end();
        ++event_itr)
    {
        bool handled = false;
        for(EventHandlerList::iterator handler_itr=_eventHandlerList.begin();
            handler_itr!=_eventHandlerList.end() && !handled;
            ++handler_itr)
        {   
            handled = (*handler_itr)->handle(*(*event_itr),*this);
        }
    }
    
    if (_updateVisitor.valid())
    {
        _updateVisitor->setTraversalNumber(_frameStamp->getFrameNumber());

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        getSceneData()->accept(*_updateVisitor);
    }
    
    // update the main producer camera
    if (_old_style_osg_camera.valid()) 
    {
        CameraGroup::setViewByMatrix(Producer::Matrix(_old_style_osg_camera->getModelViewMatrix().ptr()));
    }
}

void Viewer::frame()
{

    if (getRecordingAnimationPath() && getAnimationPath())
    {
        osg::Matrix matrix;
        matrix.invert(getViewMatrix());
        osg::Quat quat;
        quat.set(matrix);
        getAnimationPath()->insert(_frameStamp->getReferenceTime(),osg::AnimationPath::ControlPoint(matrix.getTrans(),quat));
    }

    OsgCameraGroup::frame();
}

bool Viewer::computePixelCoords(float x,float y,unsigned int cameraNum,float& pixel_x,float& pixel_y)
{
    Producer::KeyboardMouse* km = getKeyboardMouse();
    if (!km) return false;

    if (cameraNum>=getNumberOfCameras()) return false;

    Producer::Camera* camera=getCamera(cameraNum);
    Producer::RenderSurface* rs = camera->getRenderSurface();

    //std::cout << "checking camara "<<i<<std::endl;

    if (km->computePixelCoords(x,y,rs,pixel_x,pixel_y))
    {
        //std::cout << "    compute pixel coords "<<pixel_x<<"  "<<pixel_y<<std::endl;

        int pr_wx, pr_wy;
        unsigned int pr_width, pr_height;
        camera->getProjectionRect( pr_wx, pr_wy, pr_width, pr_height );

        int rs_wx, rs_wy;
        unsigned int rs_width, rs_height;
        rs->getWindowRect( rs_wx, rs_wy, rs_width, rs_height );

        pixel_x -= (float)rs_wx;
        pixel_y -= (float)rs_wy;

        //std::cout << "    wx = "<<pr_wx<<"  wy = "<<pr_wy<<" width="<<pr_width<<" height="<<pr_height<<std::endl;

        if (pixel_x<(float)pr_wx) return false;
        if (pixel_x>(float)(pr_wx+pr_width)) return false;

        if (pixel_y<(float)pr_wy) return false;
        if (pixel_y>(float)(pr_wy+pr_height)) return false;

        return true;
    }
    return false;
}

bool Viewer::computeNearFar(float x,float y,unsigned int cameraNum,osg::Vec3& near, osg::Vec3& far)
{
    if (cameraNum>=getSceneHandlerList().size()) return false;

    OsgSceneHandler* scenehandler = getSceneHandlerList()[cameraNum].get();
    
    float pixel_x,pixel_y;
    if (computePixelCoords(x,y,cameraNum,pixel_x,pixel_y))
    {
        return scenehandler->projectWindowXYIntoObject(pixel_x,pixel_y,near,far);
    }
    return false;

}

bool Viewer::computeIntersections(float x,float y,unsigned int cameraNum,osgUtil::IntersectVisitor::HitList& hits)
{
    float pixel_x,pixel_y;
    if (computePixelCoords(x,y,cameraNum,pixel_x,pixel_y))
    {

        Producer::Camera* camera=getCamera(cameraNum);

        int pr_wx, pr_wy;
        unsigned int pr_width, pr_height;
        camera->getProjectionRect( pr_wx, pr_wy, pr_width, pr_height );

        // convert into clip coords.
        float rx = 2.0f*(pixel_x - (float)pr_wx)/(float)pr_width-1.0f;
        float ry = 2.0f*(pixel_y - (float)pr_wy)/(float)pr_height-1.0f;

        //std::cout << "    rx "<<rx<<"  "<<ry<<std::endl;

        osgProducer::OsgSceneHandler* sh = dynamic_cast<osgProducer::OsgSceneHandler*>(camera->getSceneHandler());
        osg::Matrix vum;
        if (sh!=0 && sh->getModelViewMatrix()!=0 && sh->getProjectionMatrix()!=0)
        {
            vum.set((*(sh->getModelViewMatrix())) *
                    (*(sh->getProjectionMatrix())));
        }
        else
        {
            vum.set(osg::Matrix(camera->getViewMatrix()) *
                    osg::Matrix(camera->getProjectionMatrix()));
        }

        osgUtil::PickVisitor iv;
        
        osgUtil::IntersectVisitor::HitList localHits;        
        localHits = iv.getHits(getSceneData(), vum, rx,ry);
        
        if (localHits.empty()) return false;
        
        hits.insert(hits.begin(),localHits.begin(),localHits.end());
        
        return true;
    }
    return false;
}

bool Viewer::computeIntersections(float x,float y,osgUtil::IntersectVisitor::HitList& hits)
{
    bool hitFound = false;
    osgUtil::IntersectVisitor::HitList hlist;
    for(unsigned int i=0;i<getNumberOfCameras();++i)
    {
        if (computeIntersections(x,y,i,hits)) hitFound = true;
    }
    return hitFound;
}

void Viewer::selectCameraManipulator(unsigned int no)
{
    if (_keyswitchManipulator.valid()) _keyswitchManipulator->selectCameraManipulator(no);
}

void Viewer::requestWarpPointer(float x,float y)
{
    if (_kbmcb)
    {
        EventAdapter::_s_mx = x;
        EventAdapter::_s_my = y;
        _kbmcb->getKeyboardMouse()->positionPointer(x,y);
        return;
    }   
}

void Viewer::getUsage(osg::ApplicationUsage& usage) const
{
    if (_kbmcb && _kbmcb->getEscapeSetDone()) 
    {
        usage.addKeyboardMouseBinding("Escape","Exit the application");
    }

    for(EventHandlerList::const_iterator itr=_eventHandlerList.begin();
        itr!=_eventHandlerList.end();
        ++itr)
    {
        (*itr)->getUsage(usage);
    }
}
