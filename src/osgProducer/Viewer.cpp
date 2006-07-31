#include <osg/LightSource>
#include <osg/ApplicationUsage>
#include <osg/AlphaFunc>
#include <osg/io_utils>

#include <osgUtil/UpdateVisitor>

#include <osgDB/Registry>

#include <osgGA/EventVisitor>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/UFOManipulator>
#include <osgGA/StateSetManipulator>

#include <osgProducer/Viewer>
#include <osgProducer/ViewerEventHandler>

#include <stdio.h>

using namespace Producer;
using namespace osgProducer;
using namespace osg;


#ifdef __APPLE__
#define SINGLE_THREAD_KEYBOARDMOUSE
#endif


class CollectedCoordinateSystemNodesVisitor : public osg::NodeVisitor
{
public:

    CollectedCoordinateSystemNodesVisitor():
        NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN) {}
        
        
    virtual void apply(osg::Node& node)
    {
        traverse(node);
    }

    virtual void apply(osg::CoordinateSystemNode& node)
    {
        if (_pathToCoordinateSystemNode.empty())
        {
            osg::notify(osg::INFO)<<"Found CoordianteSystemNode node"<<std::endl;
            osg::notify(osg::INFO)<<"     CoordinateSystem = "<<node.getCoordinateSystem()<<std::endl;
            _pathToCoordinateSystemNode = getNodePath();
        }
        else
        {
            osg::notify(osg::INFO)<<"Found additional CoordianteSystemNode node, but ignoring"<<std::endl;
            osg::notify(osg::INFO)<<"     CoordinateSystem = "<<node.getCoordinateSystem()<<std::endl;
        }
        traverse(node);
    }
    
    NodePath _pathToCoordinateSystemNode;
};


/** callback class to use to allow matrix manipulators to querry the application for the local coordinate frame.*/
class ViewerCoordinateFrameCallback : public osgGA::MatrixManipulator::CoordinateFrameCallback
{
public:

    ViewerCoordinateFrameCallback(Viewer* viewer):
        _viewer(viewer) {}
        
    
    virtual osg::CoordinateFrame getCoordinateFrame(const osg::Vec3d& position) const
    {
        osg::notify(osg::INFO)<<"getCoordinateFrame("<<position<<")"<<std::endl;

        osg::NodePath tmpPath = _viewer->getCoordinateSystemNodePath();
        
        if (!tmpPath.empty())
        {        
            osg::Matrixd coordinateFrame;

            osg::CoordinateSystemNode* csn = dynamic_cast<osg::CoordinateSystemNode*>(tmpPath.back());
            if (csn)
            {
                osg::Vec3 local_position = position*osg::computeWorldToLocal(tmpPath);
                
                // get the coordinate frame in world coords.
                coordinateFrame = csn->computeLocalCoordinateFrame(local_position)* osg::computeLocalToWorld(tmpPath);

                // keep the position of the coordinate frame to reapply after rescale.
                osg::Vec3d pos = coordinateFrame.getTrans();

                // compensate for any scaling, so that the coordinate frame is a unit size
                osg::Vec3d x(1.0,0.0,0.0);
                osg::Vec3d y(0.0,1.0,0.0);
                osg::Vec3d z(0.0,0.0,1.0);
                x = osg::Matrixd::transform3x3(x,coordinateFrame);
                y = osg::Matrixd::transform3x3(y,coordinateFrame);
                z = osg::Matrixd::transform3x3(z,coordinateFrame);
                coordinateFrame.preMult(osg::Matrixd::scale(1.0/x.length(),1.0/y.length(),1.0/z.length()));

                // reapply the position.
                coordinateFrame.setTrans(pos);

                osg::notify(osg::INFO)<<"csn->computeLocalCoordinateFrame(position)* osg::computeLocalToWorld(tmpPath)"<<coordinateFrame<<std::endl;

            }
            else
            {
                osg::notify(osg::INFO)<<"osg::computeLocalToWorld(tmpPath)"<<std::endl;
                coordinateFrame =  osg::computeLocalToWorld(tmpPath);
            }
            return coordinateFrame;
        }
        else
        {
            osg::notify(osg::INFO)<<"   no coordinate system found, using default orientation"<<std::endl;
            return osg::Matrixd::translate(position);
        }
    }
    
protected:
    virtual ~ViewerCoordinateFrameCallback() {}
    Viewer* _viewer;
};


//////////////////////////////////////////////////////////////////////////////
//
// osgProducer::Viewer implemention
//
Viewer::Viewer():
    _setDoneAtElapsedTimeEnabled(false),
    _setDoneAtElapsedTime(0.0),
    _setDoneAtFrameNumberEnabled(false),
    _setDoneAtFrameNumber(0),
    _done(false),
    _writeImageWhenDone(false),
    _writeImageFileName(getDefaultImageFileName()),
    _recordingAnimationPath(false),
    _recordingStartTime(0.0)
{
    _eventQueue = new osgGA::EventQueue(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);
    _position[0] = 0.0;
    _position[1] = 0.0;
    _position[2] = 0.0;
    _speed = 0.0;
}

Viewer::Viewer(Producer::CameraConfig *cfg):
    OsgCameraGroup(cfg),
    _setDoneAtElapsedTimeEnabled(false),
    _setDoneAtElapsedTime(0.0),
    _setDoneAtFrameNumberEnabled(false),
    _setDoneAtFrameNumber(0),
    _done(false),
    _writeImageWhenDone(false),
    _writeImageFileName(getDefaultImageFileName()),
    _recordingAnimationPath(false),
    _recordingStartTime(0.0)
{
    _eventQueue = new osgGA::EventQueue(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);
    _position[0] = 0.0;
    _position[1] = 0.0;
    _position[2] = 0.0;
    _speed = 0.0;
    _recordingStartTime = 0.0;
}

Viewer::Viewer(const std::string& configFile):
    OsgCameraGroup(configFile),
    _setDoneAtElapsedTimeEnabled(false),
    _setDoneAtElapsedTime(0.0),
    _setDoneAtFrameNumberEnabled(false),
    _setDoneAtFrameNumber(0),
    _done(false),
    _writeImageWhenDone(false),
    _writeImageFileName(getDefaultImageFileName()),
    _recordingAnimationPath(false),
    _recordingStartTime(0.0)
{
    _eventQueue = new osgGA::EventQueue(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);
    _position[0] = 0.0;
    _position[1] = 0.0;
    _position[2] = 0.0;
    _speed = 0.0;
}

Viewer::Viewer(osg::ArgumentParser& arguments):
    OsgCameraGroup(arguments),
    _setDoneAtElapsedTimeEnabled(false),
    _setDoneAtElapsedTime(0.0),
    _setDoneAtFrameNumberEnabled(false),
    _setDoneAtFrameNumber(0),
    _done(false),
    _writeImageWhenDone(false),
    _writeImageFileName(getDefaultImageFileName()),
    _recordingAnimationPath(false),
    _recordingStartTime(0.0)
{
    _eventQueue = new osgGA::EventQueue(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);
    _position[0] = 0.0;
    _position[1] = 0.0;
    _position[2] = 0.0;
    _speed = 0.0;

    // report the usage options.
    if (arguments.getApplicationUsage())
    {
        arguments.getApplicationUsage()->addCommandLineOption("-p <filename>","Specify camera path file to animate the camera through the loaded scene");
        arguments.getApplicationUsage()->addCommandLineOption("--run-till-frame-number <integer>","Specify the number of frame to run");
        arguments.getApplicationUsage()->addCommandLineOption("--run-till-elapsed-time","Specify the amount of time to run");
        arguments.getApplicationUsage()->addCommandLineOption("--clear-color <float>,<float>,<float>[,<float>]","Specify the clear color as RGB or RGBA");
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

    unsigned int frameNumber;
    while (arguments.read("--run-till-frame-number",frameNumber))
    {
        setDoneAtFrameNumber(frameNumber);
    }

    double elapsedTime;
    while (arguments.read("--run-till-elapsed-time",elapsedTime))
    {
        setDoneAtElapsedTime(elapsedTime);
    }

    std::string filename;
    while (arguments.read("--write-image-when-done",filename))
    {
        setWriteImageWhenDone(true);
        setWriteImageFileName(filename);
    }

    std::string colorStr;
    while (arguments.read("--clear-color",colorStr))
    {
        float r, g, b;
        float a = 1.0f;
        int cnt = sscanf( colorStr.c_str(), "%f,%f,%f,%f", &r, &g, &b, &a );
        if( cnt==3 || cnt==4 ) setClearColor( osg::Vec4(r,g,b,a) );
        else osg::notify(osg::WARN)<<"Invalid clear color \""<<colorStr<<"\""<<std::endl;
    }
}

Viewer::~Viewer()
{
}

osg::NodePath Viewer::getCoordinateSystemNodePath() const
{
    osg::NodePath nodePath;
    for(ObserveredNodePath::const_iterator itr = _coordinateSystemNodePath.begin();
        itr != _coordinateSystemNodePath.end();
        ++itr)
    {
        nodePath.push_back(const_cast<osg::Node*>(itr->get()));
    }
    return nodePath;
}

void Viewer::setWriteImageFileName(const std::string& filename)
{
    _writeImageFileName = filename;
    for(EventHandlerList::iterator itr = getEventHandlerList().begin();
        itr != getEventHandlerList().end();
        ++itr)
    {
        ViewerEventHandler* viewerEventHandler = dynamic_cast<ViewerEventHandler*>(itr->get());
        if (viewerEventHandler)
        {
            viewerEventHandler->setWriteImageFileName(filename);
        }
    }
}

const std::string& Viewer::getWriteImageFileName() const
{
    return _writeImageFileName;
}

static osg::ApplicationUsageProxy Viewer_e0(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE, "OSG_IMAGE_FILE_NAME <filename>", "name of snapshot image file" );

const char* Viewer::getDefaultImageFileName()
{
    const char* name = getenv( "OSG_IMAGE_FILE_NAME" );
    return name ? name : "saved_image.jpg";
}


void Viewer::setCoordinateSystemNodePath(const osg::NodePath& nodePath)
{
    _coordinateSystemNodePath.clear();
    std::copy(nodePath.begin(),
              nodePath.end(),
              std::back_inserter(_coordinateSystemNodePath));
}

void Viewer::computeActiveCoordinateSystemNodePath()
{
    // now search for CoordinateSystemNode's for which we want to track.
    osg::Node* subgraph = getTopMostSceneData();
    
    if (subgraph)
    {
        CollectedCoordinateSystemNodesVisitor ccsnv;
        subgraph->accept(ccsnv);

        if (!ccsnv._pathToCoordinateSystemNode.empty())
        {
           setCoordinateSystemNodePath(ccsnv._pathToCoordinateSystemNode);
           return;
        }
    }  
    // otherwise no node path found so reset to empty.
    setCoordinateSystemNodePath(osg::NodePath());
}

void Viewer::updatedSceneData()
{
    OsgCameraGroup::updatedSceneData();

    // refresh the coordinate system node path.
    computeActiveCoordinateSystemNodePath();

    // refresh the camera manipulators    
    if (_keyswitchManipulator.valid()) _keyswitchManipulator->setNode(getTopMostSceneData());
}

void Viewer::setKeyboardMouse(Producer::KeyboardMouse* kbm)
{
    _kbm = kbm;
    if (_kbm.valid() && _kbmcb.valid()) _kbm->setCallback(_kbmcb.get());
}

void Viewer::setKeyboardMouseCallback(osgProducer::KeyboardMouseCallback* kbmcb)
{
    _kbmcb = kbmcb;
    if (_kbm.valid() && _kbmcb.valid())
    {
        _kbm->setCallback(_kbmcb.get());
        _kbmcb->setEventQueue(_eventQueue.get());
    }
}

void Viewer::setUpViewer(unsigned int options)
{

    // set up the keyboard and mouse handling.
    Producer::InputArea *ia = getCameraConfig()->getInputArea();
    
    if (!_kbm)
    {
        setKeyboardMouse(ia ?
                   (new Producer::KeyboardMouse(ia)) : 
                   (new Producer::KeyboardMouse(getCamera(0)->getRenderSurface())) );
                   
    }
    
    // set the keyboard mouse callback to catch the events from the windows.
    if (!_kbmcb)
    {
        setKeyboardMouseCallback(new osgProducer::KeyboardMouseCallback( _kbm.get(), _done, (options & ESCAPE_SETS_DONE)!=0 ));
    }
        
    getEventQueue()->setStartTick(_start_tick);
    
    // register the callback with the keyboard mouse manger.
    _kbm->setCallback( _kbmcb.get() );
    //kbm->allowContinuousMouseMotionUpdate(true);



    // set the globa state
    
    osg::ref_ptr<osg::StateSet> globalStateSet = new osg::StateSet;
    setGlobalStateSet(globalStateSet.get());
    {
        globalStateSet->setGlobalDefaults();
        // enable depth testing by default.
        globalStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

#if 0
        // set up an alphafunc by default to speed up blending operations.
        osg::AlphaFunc* alphafunc = new osg::AlphaFunc;
        alphafunc->setFunction(osg::AlphaFunc::GREATER,0.0f);
        globalStateSet->setAttributeAndModes(alphafunc, osg::StateAttribute::ON);
#endif
    }
    
    if (options & HEAD_LIGHT_SOURCE || options & SKY_LIGHT_SOURCE)
    {
        // enable lighting by default
        globalStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);
        
        // add either a headlight or sun light to the scene.
        osg::LightSource* lightsource = new osg::LightSource;
        setSceneDecorator(lightsource);
        {
            osg::Light* light = new osg::Light;
            lightsource->setLight(light);
            if (options & HEAD_LIGHT_SOURCE)
            {
                lightsource->setReferenceFrame(osg::LightSource::ABSOLUTE_RF); // headlight.
            }
            else
            {
                lightsource->setReferenceFrame(osg::LightSource::RELATIVE_RF); // skylight
            }
            lightsource->setLocalStateSetModes(osg::StateAttribute::ON);
        }
    }
        
    if (!_updateVisitor) _updateVisitor = new osgUtil::UpdateVisitor;
    
    _updateVisitor->setFrameStamp(_frameStamp.get());

    if (!_eventVisitor) _eventVisitor = new osgGA::EventVisitor;
    _eventVisitor->setActionAdapter(this);


    if (options&TRACKBALL_MANIPULATOR) addCameraManipulator(new osgGA::TrackballManipulator);
    if (options&FLIGHT_MANIPULATOR) addCameraManipulator(new osgGA::FlightManipulator);
    if (options&DRIVE_MANIPULATOR) addCameraManipulator(new osgGA::DriveManipulator);
    if (options&TERRAIN_MANIPULATOR) addCameraManipulator(new osgGA::TerrainManipulator);
    if (options&UFO_MANIPULATOR) addCameraManipulator(new osgGA::UFOManipulator);
    
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

unsigned int Viewer::addCameraManipulator(osgGA::MatrixManipulator* cm)
{
    if (!cm) return 0xfffff;
    
    // create a key switch manipulator if one doesn't already exist.
    if (!_keyswitchManipulator)
    {
        _keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;
        _eventHandlerList.push_back(_keyswitchManipulator.get());
    }
    
    unsigned int num = _keyswitchManipulator->getNumMatrixManipulators();
    _keyswitchManipulator->addNumberedMatrixManipulator(cm);
    
    return num;
}

bool Viewer::done() const
{
    return _done || 
           !validForRendering() || 
           (_setDoneAtElapsedTimeEnabled && _setDoneAtElapsedTime<=getFrameStamp()->getReferenceTime()) ||
           (_setDoneAtFrameNumberEnabled && _setDoneAtFrameNumber<=_frameNumber);
}

void Viewer::setViewByMatrix( const Producer::Matrix & pm)
{
    CameraGroup::setViewByMatrix(pm);
    
    if (_keyswitchManipulator.valid())
    {
        // now convert Producer matrix to an osg::Matrix so we can update
        // the internal camera...
        
        osg::Matrixd matrix(pm.ptr());
        _keyswitchManipulator->setByInverseMatrix(matrix);
    }
}

bool Viewer::realize( ThreadingModel thread_model )
{
    if( _realized ) return _realized;
    _threadModel = thread_model;
    return realize();
}

class PostSwapFinishCallback : public Producer::Camera::Callback
{
public:

    PostSwapFinishCallback() {}

    virtual void operator()(const Producer::Camera&)
    {
        // glFinish();
    }
};

bool Viewer::realize()
{
    if (_realized) return _realized;

    OsgCameraGroup::realize();

    // force a sync before we intialize the keyswitch manipulator to home
    // so that Producer has a chance to set up the windows before we do
    // any work on them.
    OsgCameraGroup::sync();

#ifndef SINGLE_THREAD_KEYBOARDMOUSE
    // kick start the keyboard mouse if needed.
    if (_kbm.valid() && !_kbm->isRunning())
    {
        _kbm->startThread();

        while (!_kbm->isRunning())
        {
            // osg::notify(osg::NOTICE)<<"Waiting"<<std::endl;
            OpenThreads::Thread::YieldCurrentThread();
        }
    }
#endif

    if (_kbmcb.valid()) _kbmcb->updateWindowSize();

    // by default set up the DatabasePager.
    {    
        osgDB::DatabasePager* databasePager = osgDB::Registry::instance()->getOrCreateDatabasePager();
        databasePager->registerPagedLODs(getTopMostSceneData());

        for(SceneHandlerList::iterator p=_shvec.begin();
            p!=_shvec.end();
            ++p)
        {
            // pass the database pager to the cull visitor so node can send requests to the pager.
            (*p)->getSceneView()->getCullVisitor()->setDatabaseRequestHandler(databasePager);
                        
            // tell the database pager which graphic context the compile of rendering objexts is needed.
            databasePager->setCompileGLObjectsForContextID((*p)->getSceneView()->getState()->getContextID(),true);
        }

    
        // set up a post swap callback to flush deleted GL objects and compile new GL objects            
        for(unsigned int cameraNum=0;cameraNum<getNumberOfCameras();++cameraNum)
        {
            Producer::Camera* camera=getCamera(cameraNum);
            camera->addPostSwapCallback(new PostSwapFinishCallback());
        }

    }
    
 
    if (_keyswitchManipulator.valid() && _keyswitchManipulator->getCurrentMatrixManipulator() && _eventQueue.valid())
    {
        _keyswitchManipulator->setCoordinateFrameCallback(new ViewerCoordinateFrameCallback(this));

        osg::ref_ptr<osgGA::GUIEventAdapter> init_event = _eventQueue->createEvent();

        _keyswitchManipulator->setNode(getTopMostSceneData());
        _keyswitchManipulator->home(*init_event,*this);
    }
    
    // set up osg::State objects with the _done prt to allow early termination of 
    // draw traversal.
    for(SceneHandlerList::iterator p=_shvec.begin(); p!=_shvec.end(); p++ )
    {
        (*p)->getSceneView()->getState()->setAbortRenderingPtr(&_done);
    }
    
    return _realized;
}


void Viewer::update()
{
#ifdef SINGLE_THREAD_KEYBOARDMOUSE
    if (_kbm.valid() && !_kbm->isRunning()) _kbm->update(*(_kbm->getCallback()));
#endif

    // create an event to signal the new frame.
    getEventQueue()->frame(_frameStamp->getReferenceTime());

    // get the event since the last frame.
    osgGA::EventQueue::Events events;
    getEventQueue()->takeEvents(events);

    if (_eventVisitor.valid())
    {
        _eventVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
    }

    // dispatch the events in order of arrival.
    for(osgGA::EventQueue::Events::iterator event_itr=events.begin();
        event_itr!=events.end();
        ++event_itr)
    {
        bool handled = false;

        if (_eventVisitor.valid())
        {
            _eventVisitor->reset();
            _eventVisitor->addEvent(event_itr->get());
            getTopMostSceneData()->accept(*_eventVisitor);
            if (_eventVisitor->getEventHandled())
                handled = true;
        }

        for(EventHandlerList::iterator handler_itr=_eventHandlerList.begin();
            handler_itr!=_eventHandlerList.end() && !handled;
            ++handler_itr)
        {   
            handled = (*handler_itr)->handle(*(*event_itr),*this,0,0);
        }
        
    }

    if (osgDB::Registry::instance()->getDatabasePager())
    {
        // update the scene graph by remove expired subgraphs and merge newly loaded subgraphs
        osgDB::Registry::instance()->getDatabasePager()->updateSceneGraph(_frameStamp->getReferenceTime());
    }    
    
    if (_updateVisitor.valid())
    {
        _updateVisitor->setTraversalNumber(_frameStamp->getFrameNumber());

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        getTopMostSceneData()->accept(*_updateVisitor);
    }
    

    // update the main producer camera
    if (_keyswitchManipulator.valid() && _keyswitchManipulator->getCurrentMatrixManipulator()) 
    {
        osgGA::MatrixManipulator* mm = _keyswitchManipulator->getCurrentMatrixManipulator();
        osg::Matrixd matrix = mm->getInverseMatrix();
        CameraGroup::setViewByMatrix(Producer::Matrix(matrix.ptr()));

        setFusionDistance(mm->getFusionDistanceMode(),mm->getFusionDistanceValue());

    }
}

void Viewer::frame()
{
    // record the position of the view point.
    osg::Matrixd matrix;
    matrix.invert(getViewMatrix());
    _orientation = matrix.getRotate();

    double newPosition[3];
    newPosition[0] = matrix(3,0);
    newPosition[1] = matrix(3,1);
    newPosition[2] = matrix(3,2);
    
    _speed = sqrtf(osg::square(newPosition[0]-_position[0])+osg::square(newPosition[1]-_position[1])+osg::square(newPosition[2]-_position[2]));
    _position[0] = newPosition[0];
    _position[1] = newPosition[1];
    _position[2] = newPosition[2];
    
#if 0
    osg::Quat::value_type angle;
    osg::Vec3 axis;
    
    osg::Quat roll;
    roll.makeRotate(-osg::PI/2.0f,1,0,0);
    
    _orientation = roll*_orientation;
    
    _orientation.getRotate(angle,axis);
    
    std::cout<<"_position "<<_position[0]<<", "<<_position[1]<<", "<<_position[2]<<"  speed "<<_speed<<"  angle "<<osg::RadiansToDegrees(angle)<<" axis "<<axis<<std::endl;
#endif    
    
    if (getRecordingAnimationPath() && getAnimationPath())
    {
        if (getAnimationPath()->empty()) _recordingStartTime = _frameStamp->getReferenceTime();
        
        getAnimationPath()->insert(_frameStamp->getReferenceTime()-_recordingStartTime,osg::AnimationPath::ControlPoint(osg::Vec3(_position[0],_position[1],_position[2]),_orientation));
    }
    
    if (done() && getWriteImageWhenDone())
    {
        for(EventHandlerList::iterator itr = getEventHandlerList().begin();
            itr != getEventHandlerList().end();
            ++itr)
        {
            ViewerEventHandler* viewerEventHandler = dynamic_cast<ViewerEventHandler*>(itr->get());
            if (viewerEventHandler)
            {
                osg::notify(osg::NOTICE)<<"Need to write image"<<std::endl;    
                viewerEventHandler->setWriteImageOnNextFrame(true);
            }
        }
        
    }

    OsgCameraGroup::frame();


    if (osg::Referenced::getDeleteHandler()) 
    {
        osg::Referenced::getDeleteHandler()->flush();
    }

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
        camera->getProjectionRectangle( pr_wx, pr_wy, pr_width, pr_height );

        int rs_wx, rs_wy;
        unsigned int rs_width, rs_height;
        rs->getWindowRectangle( rs_wx, rs_wy, rs_width, rs_height );

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

bool Viewer::computeNearFarPoints(float x,float y,unsigned int cameraNum,osg::Vec3& near_point, osg::Vec3& far_point)
{
    if (cameraNum>=getSceneHandlerList().size()) return false;

    OsgSceneHandler* scenehandler = getSceneHandlerList()[cameraNum].get();
    osgUtil::SceneView* sv = scenehandler->getSceneView();
    
    float pixel_x,pixel_y;
    if (computePixelCoords(x,y,cameraNum,pixel_x,pixel_y))
    {
        pixel_x-=sv->getViewport()->x();
        pixel_y-=sv->getViewport()->y();

        return sv->projectWindowXYIntoObject((int)(pixel_x+0.5f),(int)(pixel_y+0.5f),near_point,far_point);
    }
    return false;

}

bool Viewer::computeIntersections(float x,float y,unsigned int cameraNum,osg::Node* node,osgUtil::IntersectVisitor::HitList& hits,osg::Node::NodeMask traversalMask)
{
    float pixel_x,pixel_y;
    if (node && computePixelCoords(x,y,cameraNum,pixel_x,pixel_y))
    {

        Producer::Camera* camera=getCamera(cameraNum);

        osgProducer::OsgSceneHandler* sh = dynamic_cast<osgProducer::OsgSceneHandler*>(camera->getSceneHandler());
        osgUtil::SceneView* sv = sh ? sh->getSceneView() : 0;
        osg::Matrixd proj;
        osg::Matrixd view;
        const osg::Viewport* viewport = 0;
        osg::Node* rootNode = 0; 
        if (sv!=0)
        {
            viewport = sv->getViewport();
            proj = sv->getProjectionMatrix();
            view = sv->getViewMatrix();
            rootNode = sv->getSceneData();
        }
        else
        {
            viewport = 0;
            proj = osg::Matrixd(camera->getProjectionMatrix());
            view = osg::Matrixd(camera->getViewMatrix());
        }

        unsigned int numHitsBefore = hits.size();

        osg::NodePathList parentNodePaths = node->getParentalNodePaths(rootNode);
        for(unsigned int i=0;i<parentNodePaths.size();++i)
        {
            osg::NodePath& nodePath = parentNodePaths[i];
            
            // remove the intersection node from the nodePath as it'll be accounted for
            // in the PickVisitor traversal, so we don't double account for its transform.
            if (!nodePath.empty()) nodePath.pop_back();  
            
            osg::Matrixd modelview(view);
            // modify the view matrix so that it accounts for this nodePath's accumulated transform
            if (!nodePath.empty()) modelview.preMult(computeLocalToWorld(nodePath));
            
            osgUtil::PickVisitor pick(viewport, proj, modelview, pixel_x, pixel_y);
            pick.setTraversalMask(traversalMask);
            node->accept(pick);

            // copy all the hits across to the external hits list
            for(osgUtil::PickVisitor::LineSegmentHitListMap::iterator itr = pick.getSegHitList().begin();
                itr != pick.getSegHitList().end();
                ++itr)
            {
                hits.insert(hits.end(),itr->second.begin(), itr->second.end());
            }

        }
        
        // return true if we now have more hits than before
        return hits.size()>numHitsBefore;
    }
    return false;
}

bool Viewer::computeIntersections(float x,float y,unsigned int cameraNum,osgUtil::IntersectVisitor::HitList& hits,osg::Node::NodeMask traversalMask)
{
    return computeIntersections(x,y,cameraNum,getTopMostSceneData(),hits,traversalMask);
}

bool Viewer::computeIntersections(float x,float y,osg::Node *node,osgUtil::IntersectVisitor::HitList& hits,osg::Node::NodeMask traversalMask)
{
    bool hitFound = false;
    for(unsigned int i=0;i<getNumberOfCameras();++i)
    {
        if (computeIntersections(x,y,i,node,hits,traversalMask)) hitFound = true;
    }
    return hitFound;
}


bool Viewer::computeIntersections(float x,float y,osgUtil::IntersectVisitor::HitList& hits,osg::Node::NodeMask traversalMask)
{
    return computeIntersections(x,y,getTopMostSceneData(),hits,traversalMask);
}

void Viewer::selectCameraManipulator(unsigned int no)
{
   if (_keyswitchManipulator.valid())
   {
      _keyswitchManipulator->selectMatrixManipulator(no);
      
      // keyswitch manipulator doesn't yet force manipulators to init themselves
      // so we'll do this mannually.  Note pretty, and needs replacing by a refactor
      // of MatrixMinpulators in the longer term.
      osg::ref_ptr<osgGA::GUIEventAdapter> ea = new osgGA::GUIEventAdapter;
      double time = _kbmcb.valid() ? _kbmcb->getTime() : 0.0;
      ea->setTime(time);
      ea->setEventType(osgGA::GUIEventAdapter::KEYDOWN);
      ea->setKey(osgGA::GUIEventAdapter::KEY_KP_1+no);
      _keyswitchManipulator->init(*ea, *this);
   }
}

void Viewer::getCameraManipulatorNameList( std::list<std::string> &nameList )
{
    osgGA::KeySwitchMatrixManipulator *ksm = getKeySwitchMatrixManipulator();
    osgGA::KeySwitchMatrixManipulator::KeyManipMap &kmmap = ksm->getKeyManipMap();
    osgGA::KeySwitchMatrixManipulator::KeyManipMap::iterator p;
    for( p = kmmap.begin(); p != kmmap.end(); p++ )
    {
        osgGA::KeySwitchMatrixManipulator::NamedManipulator  nm = (*p).second;
        nameList.push_back( nm.first );
    }
}

bool Viewer::selectCameraManipulatorByName( const std::string &name )
{
    unsigned int num = 0xFFFF;
    osgGA::KeySwitchMatrixManipulator *ksm = getKeySwitchMatrixManipulator();
    osgGA::KeySwitchMatrixManipulator::KeyManipMap &kmmap = ksm->getKeyManipMap();
    osgGA::KeySwitchMatrixManipulator::KeyManipMap::iterator p;
    for( p = kmmap.begin(); p != kmmap.end(); p++ )
    {
        int key = (*p).first;
        osgGA::KeySwitchMatrixManipulator::NamedManipulator  nm = (*p).second;
        if( nm.first == name )
            num = key - '1';
    }

    if( num == 0xFFFF )
        return false;

    selectCameraManipulator(num);
    return true;
}

osgGA::MatrixManipulator *Viewer::getCameraManipulatorByName( const std::string &name )
{
    osgGA::KeySwitchMatrixManipulator *ksm = getKeySwitchMatrixManipulator();
    osgGA::KeySwitchMatrixManipulator::KeyManipMap &kmmap = ksm->getKeyManipMap();
    osgGA::KeySwitchMatrixManipulator::KeyManipMap::iterator p;
    for( p = kmmap.begin(); p != kmmap.end(); p++ )
    {
        osgGA::KeySwitchMatrixManipulator::NamedManipulator  nm = (*p).second;
        if( nm.first == name )
            return nm.second.get();
    }
    return 0L;
}



void Viewer::requestRedraw()
{
    //osg::notify(osg::INFO)<<"Viewer::requestRedraw() called"<<std::endl;
}

void Viewer::requestContinuousUpdate(bool)
{
    //osg::notify(osg::INFO)<<"Viewer::requestContinuousUpdate("<<flag<<") called"<<std::endl;
}

void Viewer::requestWarpPointer(float x,float y)
{
    if (_kbmcb.valid() && isRealized())
    {
        osg::notify(osg::INFO) << "requestWarpPointer x= "<<x<<" y="<<y<<std::endl;
    
        getEventQueue()->mouseWarp(x,y);
        _kbmcb->getKeyboardMouse()->positionPointer(x,y);
        return;
    }   
}

void Viewer::getUsage(osg::ApplicationUsage& usage) const
{
    if (_kbmcb.valid() && _kbmcb->getEscapeSetDone()) 
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

