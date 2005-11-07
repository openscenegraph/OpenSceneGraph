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

//////////////////////////////////////////////////////////////////////////////
//
// Picking intersection visitor.
//

class PickIntersectVisitor : public osgUtil::IntersectVisitor
{
public:
    PickIntersectVisitor()
    { 
    }
    virtual ~PickIntersectVisitor() {}
    
    HitList& getIntersections(osg::Node *scene, osg::Vec3 nr, osg::Vec3 fr)
    { 
        // option for non-sceneView users: you need to get the screen perp line and call getIntersections
        // if you are using Projection nodes you should also call setxy to define the xp,yp positions for use with
        // the ray transformed by Projection
        _lineSegment = new osg::LineSegment;
        _lineSegment->set(nr,fr); // make a line segment
        
        //std::cout<<"near "<<nr<<std::endl;
        //std::cout<<"far "<<fr<<std::endl;
        
        addLineSegment(_lineSegment.get());

        scene->accept(*this);
        return getHitList(_lineSegment.get());
    }
private:
    osg::ref_ptr<osg::LineSegment> _lineSegment;
    friend class osgUtil::IntersectVisitor;
};

// PickVisitor traverses whole scene and checks below all Projection nodes
class PickVisitor : public osg::NodeVisitor
{
public:
    PickVisitor()
    { 
        xp=yp=0;    
        setTraversalMode(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
    }
    ~PickVisitor()
    {
    }

    void setTraversalMask(osg::Node::NodeMask traversalMask)
    {
        NodeVisitor::setTraversalMask(traversalMask);
        _piv.setTraversalMask(traversalMask);   
    }

    // Aug 2003 added to pass the nodemaskOverride to the PickIntersectVisitor
    //   may be used make the visitor override the nodemask to visit invisible actions
    inline void setNodeMaskOverride(osg::Node::NodeMask mask)
    {
        _piv.setNodeMaskOverride(mask);
        _nodeMaskOverride = mask;
    }


    virtual void apply(osg::Projection& pr)
    {  
        osg::Matrixd mt;
        mt.invert(pr.getMatrix());
        osg::Vec3 npt=osg::Vec3(xp,yp,-1.0f) * mt, farpt=osg::Vec3(xp,yp,1.0f) * mt;

        // traversing the nodes children, using the projection direction
        for (unsigned int i=0; i<pr.getNumChildren(); i++) 
        {
            osg::Node *nodech=pr.getChild(i);
            osgUtil::IntersectVisitor::HitList &hli=_piv.getIntersections(nodech,npt, farpt);
            for(osgUtil::IntersectVisitor::HitList::iterator hitr=hli.begin();
                hitr!=hli.end();
                ++hitr)
            { // add the projection hits to the scene hits.
                    // This is why _lineSegment is retained as a member of PickIntersectVisitor
                _PIVsegHitList.push_back(*hitr);
            }
            traverse(*nodech);
        }
    }

    virtual void apply(osg::CameraNode& camera)
    {
        // partial fix for CameraNode... current code 
        // assumes CameraNode has an absolute projection matrix, rather than an acculated one,
        // will need to think about how to handle relative project matrices. RO November 2005.

        osg::Matrixd mt;
        mt.invert(camera.getProjectionMatrix());
        osg::Vec3 npt=osg::Vec3(xp,yp,-1.0f) * mt, farpt=osg::Vec3(xp,yp,1.0f) * mt;

        osgUtil::IntersectVisitor::HitList &hli=_piv.getIntersections(&camera,npt, farpt);
        for(osgUtil::IntersectVisitor::HitList::iterator hitr=hli.begin();
            hitr!=hli.end();
            ++hitr)
        {
            // add the projection hits to the scene hits.
            // This is why _lineSegment is retained as a member of PickIntersectVisitor
            _PIVsegHitList.push_back(*hitr);
        }

    }

    osgUtil::IntersectVisitor::HitList& getHits(osg::Node *node, const osg::Matrixd &projm, const float x, const float y)
    { 
        setxy(x,y);    
        
        _PIVsegHitList.clear();

        // utility for non=sceneview viewers
        // x,y are values returned by 
        osg::Matrixd inverseMVPW;
        inverseMVPW.invert(projm);
        osg::Vec3 near_point = osg::Vec3(x,y,-1.0f)*inverseMVPW;
        osg::Vec3 far_point = osg::Vec3(x,y,1.0f)*inverseMVPW;
        
        if (near_point.valid() && far_point.valid())
        {
            _PIVsegHitList = _piv.getIntersections(node,near_point,far_point); // fill hitlist
        }

        // traverse for any projection/camera nodes.        
        node->accept(*this);

        return _PIVsegHitList;
    }

    inline void setxy(float xpt, float ypt) { xp=xpt; yp=ypt; }
    inline bool hits() const { return _PIVsegHitList.size()>0;}
    
private:

    PickIntersectVisitor _piv;
    float xp, yp; // start point in viewport fraction coordiantes
    osgUtil::IntersectVisitor::HitList       _PIVsegHitList;
};


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

        // do automatic conversion between RefNodePath and NodePath.
        osg::NodePath tmpPath = _viewer->getCoordindateSystemNodePath();
        
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
    _writeImageFileName("saved_image.jpg"),
    _recordingAnimationPath(false),
    _recordingStartTime(0.0)
{
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
    _writeImageFileName("saved_image.jpg"),
    _recordingAnimationPath(false),
    _recordingStartTime(0.0)
{
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
    _writeImageFileName("saved_image.jpg"),
    _recordingAnimationPath(false),
    _recordingStartTime(0.0)
{
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
    _writeImageFileName("saved_image.jpg"),
    _recordingAnimationPath(false),
    _recordingStartTime(0.0)
{
    _position[0] = 0.0;
    _position[1] = 0.0;
    _position[2] = 0.0;
    _speed = 0.0;

    // report the usage options.
    if (arguments.getApplicationUsage())
    {
        arguments.getApplicationUsage()->addCommandLineOption("-p <filename>","Specify camera path file to animate the camera through the loaded scene");
        arguments.getApplicationUsage()->addCommandLineOption("--run-till-frame-number <integer>","Specify the number of frame to run");
        arguments.getApplicationUsage()->addCommandLineOption("--run-till-elapsed-time","Specify the about of time to run");
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


void Viewer::setCoordindateSystemNodePath(const osg::NodePath& nodePath)
{
    _coordinateSystemNodePath.clear();
    std::copy(nodePath.begin(),
              nodePath.end(),
              std::back_inserter(_coordinateSystemNodePath));
}

void Viewer::computeActiveCoordindateSystemNodePath()
{
    // now search for CoordinateSystemNode's for which we want to track.
    osg::Node* subgraph = getTopMostSceneData();
    
    if (subgraph)
    {
        CollectedCoordinateSystemNodesVisitor ccsnv;
        subgraph->accept(ccsnv);

        if (!ccsnv._pathToCoordinateSystemNode.empty())
        {
           setCoordindateSystemNodePath(ccsnv._pathToCoordinateSystemNode);
           return;
        }
    }  
    // otherwise no node path found so reset to empty.
    setCoordindateSystemNodePath(osg::NodePath());
}

void Viewer::updatedSceneData()
{
    OsgCameraGroup::updatedSceneData();

    // refresh the coordinate system node path.
    computeActiveCoordindateSystemNodePath();

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
    if (_kbm.valid() && _kbmcb.valid()) _kbm->setCallback(_kbmcb.get());
}

void Viewer::setUpViewer(unsigned int options)
{

    // set up the keyboard and mouse handling.
    Producer::InputArea *ia = getCameraConfig()->getInputArea();
    
    if (!_kbm)
    {
        _kbm = ia ?
                   (new Producer::KeyboardMouse(ia)) : 
                   (new Producer::KeyboardMouse(getCamera(0)->getRenderSurface()));
                   
    }
    
    // set the keyboard mouse callback to catch the events from the windows.
    if (!_kbmcb)
        _kbmcb = new osgProducer::KeyboardMouseCallback( _kbm.get(), _done, (options & ESCAPE_SETS_DONE)!=0 );
        
    _kbmcb->setStartTick(_start_tick);
    
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


        // set up an alphafunc by default to speed up blending operations.
        osg::AlphaFunc* alphafunc = new osg::AlphaFunc;
        alphafunc->setFunction(osg::AlphaFunc::GREATER,0.0f);
        globalStateSet->setAttributeAndModes(alphafunc, osg::StateAttribute::ON);
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
    _thread_model = thread_model;
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

#ifndef SINGLE_THREAD_KEYBOARDMOUSE
    // kick start the keyboard mouse if needed.
    if (_kbm.valid() && !_kbm->isRunning()) _kbm->startThread();
#endif
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
    
    // force a sync before we intialize the keyswitch manipulator to home
    // so that Producer has a chance to set up the windows before we do
    // any work on them.
    OsgCameraGroup::sync();
 
    if (_keyswitchManipulator.valid() && _keyswitchManipulator->getCurrentMatrixManipulator())
    {
        _keyswitchManipulator->setCoordinateFrameCallback(new ViewerCoordinateFrameCallback(this));

        osg::ref_ptr<osgProducer::EventAdapter> init_event = _kbmcb->createEventAdapter();
        init_event->adaptFrame(0.0);
    
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

    // get the event since the last frame.
    osgProducer::KeyboardMouseCallback::EventQueue queue;
    if (_kbmcb.valid()) _kbmcb->getEventQueue(queue);

    // create an event to signal the new frame.
    osg::ref_ptr<osgProducer::EventAdapter> frame_event = new osgProducer::EventAdapter;
    frame_event->adaptFrame(_frameStamp->getReferenceTime());
    queue.push_back(frame_event);

    if (_eventVisitor.valid())
    {
        _eventVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
    }

    // dispatch the events in order of arrival.
    for(osgProducer::KeyboardMouseCallback::EventQueue::iterator event_itr=queue.begin();
        event_itr!=queue.end();
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
    matrix.get(_orientation);

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

bool Viewer::computeIntersections(float x,float y,unsigned int cameraNum,osg::Node *node,osgUtil::IntersectVisitor::HitList& hits,osg::Node::NodeMask traversalMask)
{
    float pixel_x,pixel_y;
    if (computePixelCoords(x,y,cameraNum,pixel_x,pixel_y))
    {

        Producer::Camera* camera=getCamera(cameraNum);

        int pr_wx, pr_wy;
        unsigned int pr_width, pr_height;
        camera->getProjectionRectangle( pr_wx, pr_wy, pr_width, pr_height );

        // convert into clip coords.
        float rx = 2.0f*(pixel_x - (float)pr_wx)/(float)pr_width-1.0f;
        float ry = 2.0f*(pixel_y - (float)pr_wy)/(float)pr_height-1.0f;

        //std::cout << "    rx "<<rx<<"  "<<ry<<std::endl;

        osgProducer::OsgSceneHandler* sh = dynamic_cast<osgProducer::OsgSceneHandler*>(camera->getSceneHandler());
        osgUtil::SceneView* sv = sh?sh->getSceneView():0;
        osg::Matrixd vum;
        if (sv!=0)
        {
            vum.set(sv->getViewMatrix() *
                    sv->getProjectionMatrix());
        }
        else
        {
            vum.set(osg::Matrixd(camera->getViewMatrix()) *
                    osg::Matrixd(camera->getProjectionMatrix()));
        }

        PickVisitor iv;
        iv.setTraversalMask(traversalMask);
        
        osgUtil::IntersectVisitor::HitList localHits;        
        localHits = iv.getHits(node, vum, rx,ry);
        
        if (localHits.empty()) return false;
        
        hits.insert(hits.begin(),localHits.begin(),localHits.end());
        
        return true;
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
      osg::ref_ptr<EventAdapter> ea = new EventAdapter;
      ea->adaptKeyPress(_kbmcb->getTime(), osgGA::GUIEventAdapter::KEY_KP_1+no);
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
    if (_kbmcb.valid())
    {
        osg::notify(osg::INFO) << "requestWarpPointer x= "<<x<<" y="<<y<<std::endl;
    
        EventAdapter::_s_mx = x;
        EventAdapter::_s_my = y;
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

