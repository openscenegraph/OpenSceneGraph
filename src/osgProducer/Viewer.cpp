#include <osg/LightSource>
#include <osg/ApplicationUsage>

#include <osgUtil/UpdateVisitor>

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

//////////////////////////////////////////////////////////////////////////////
//
// Picking intersection visitor.
//

class PickIntersectVisitor : public osgUtil::IntersectVisitor
{
public:
    PickIntersectVisitor()
    { 
        setNodeMaskOverride(0xffffffff); // need to make the visitor override the nodemask to visit invisible actions
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
        setNodeMaskOverride(0xffffffff); // need to make the visitor override the nodemask to visit invisible actions
        setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    }
    ~PickVisitor() {}

    virtual void apply(osg::Projection& pr)
    { // stack the intersect rays, transform to new projection, traverse
        // Assumes that the Projection is an absolute projection
        osg::Matrix mt;
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

    osgUtil::IntersectVisitor::HitList& getHits(osg::Node *node, const osg::Vec3& near_point, const osg::Vec3& far_point)
    {
        // High level get intersection with sceneview using a ray from x,y on the screen
        // sets xp,yp as pixels scaled to a mapping of (-1,1, -1,1); needed for Projection from x,y pixels

        // first get the standard hits in un-projected nodes
        _PIVsegHitList=_piv.getIntersections(node,near_point,far_point); // fill hitlist

        // then get hits in projection nodes
        traverse(*node); // check for projection nodes
        return _PIVsegHitList;
    }

    osgUtil::IntersectVisitor::HitList& getHits(osg::Node *node, const osg::Matrix &projm, const float x, const float y)
    { 
        // utility for non=sceneview viewers
        // x,y are values returned by 
        osg::Matrix inverseMVPW;
        inverseMVPW.invert(projm);
        osg::Vec3 near_point = osg::Vec3(x,y,-1.0f)*inverseMVPW;
        osg::Vec3 far_point = osg::Vec3(x,y,1.0f)*inverseMVPW;
        setxy(x,y);    
        getHits(node,near_point,far_point);
        return _PIVsegHitList;
    }

    inline void setxy(float xpt, float ypt) { xp=xpt; yp=ypt; }
    inline bool hits() const { return _PIVsegHitList.size()>0;}
    
private:

    PickIntersectVisitor _piv;
    float xp, yp; // start point in viewport fraction coordiantes
    osgUtil::IntersectVisitor::HitList       _PIVsegHitList;
};

//////////////////////////////////////////////////////////////////////////////
//
// osgProducer::Viewer implemention
//
Viewer::Viewer():
    _done(false),
    _kbmcb(0),
    _recordingAnimationPath(false)
{
}

Viewer::Viewer(Producer::CameraConfig *cfg):
    OsgCameraGroup(cfg),
    _done(false),
    _kbmcb(0),
    _recordingAnimationPath(false)
{
}

Viewer::Viewer(const std::string& configFile):
    OsgCameraGroup(configFile),
    _done(false),
    _kbmcb(0),
    _recordingAnimationPath(false)
{
}

Viewer::Viewer(osg::ArgumentParser& arguments):
    OsgCameraGroup(arguments),
    _done(false),
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

    
    if (!_updateVisitor) _updateVisitor = new osgUtil::UpdateVisitor;
    
    _updateVisitor->setFrameStamp(_frameStamp.get());

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

unsigned int Viewer::addCameraManipulator(osgGA::MatrixManipulator* cm)
{
    if (!cm) return 0xfffff;
    
    // create a key switch manipulator if one doesn't already exist.
    if (!_keyswitchManipulator)
    {
        _keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;
        _eventHandlerList.push_back(_keyswitchManipulator.get());
    }
    
    unsigned int num = _keyswitchManipulator->getNumMatrixManipualtors();
    _keyswitchManipulator->addNumberedMatrixManipulator(cm);
    
    return num;
}

bool Viewer::done() const
{
    return _done || !validForRendering();
}

void Viewer::setViewByMatrix( const Producer::Matrix & pm)
{
    CameraGroup::setViewByMatrix(pm);
    
    if (_keyswitchManipulator.valid())
    {
        // now convert Producer matrix to an osg::Matrix so we can update
        // the internal camera...
        
        osg::Matrix matrix(pm.ptr());
        _keyswitchManipulator->setByInverseMatrix(matrix);
    }
}

bool Viewer::realize( ThreadingModel thread_model )
{
    if( _realized ) return _realized;
    _thread_model = thread_model;
    return realize();
}

class DatabasePagerCompileCallback : public OsgSceneHandler::Callback
{
public:

        DatabasePagerCompileCallback(DatabasePager* databasePager):
            _databasePager(databasePager)
        {}

       virtual void operator()(OsgSceneHandler& sh, Producer::Camera& camera)
       {
            
            sh.drawImplementation(camera);

            _databasePager->compileRenderingObjects(*(sh.getSceneView()->getState()));
       }
       
       osg::ref_ptr<DatabasePager> _databasePager;
};

bool Viewer::realize()
{
    if (_realized) return _realized;

    OsgCameraGroup::realize();


    // by default set up the DatabasePager.
    {    
        _databasePager = new DatabasePager;
        _databasePager->registerPagedLODs(getTopMostSceneData());

        for(SceneHandlerList::iterator p=_shvec.begin();
            p!=_shvec.end();
            ++p)
        {
            // pass the database pager to the cull visitor so node can send requests to the pager.
            (*p)->getSceneView()->getCullVisitor()->setDatabaseRequestHandler(_databasePager.get());
            
            // set up a draw callback to pre compile any rendering object of database has loaded, 
            // but not yet merged with the main scene graph.
            (*p)->setDrawCallback(new DatabasePagerCompileCallback(_databasePager.get()));
            
            // tell the database pager which graphic context the compile of rendering objexts is needed.
            _databasePager->setCompileRenderingObjectsForContexID((*p)->getSceneView()->getState()->getContextID(),true);
        }
    }
    
    // force a sync before we intialize the keyswitch manipulator to home
    // so that Producer has a chance to set up the windows before we do
    // any work on them.
    OsgCameraGroup::sync();
 
    if (_keyswitchManipulator.valid() && _keyswitchManipulator->getCurrentMatrixManipulator())
    {
        osg::ref_ptr<osgProducer::EventAdapter> init_event = _kbmcb->createEventAdapter();
        init_event->adaptFrame(0.0);
    
        _keyswitchManipulator->setNode(getSceneDecorator());
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
    
    if (_databasePager.valid())
    {
        // removed any children PagedLOD children that havn't been visited in the cull traversal recently.
        _databasePager->removeExpiredSubgraphs(_frameStamp->getReferenceTime());
        
        // add the newly loaded data into the scene graph.
        _databasePager->addLoadedDataToSceneGraph(_frameStamp->getReferenceTime());
    }    
    
    
    if (_updateVisitor.valid())
    {
        _updateVisitor->setTraversalNumber(_frameStamp->getFrameNumber());

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        getSceneData()->accept(*_updateVisitor);
    }
    
    // update the main producer camera
    if (_keyswitchManipulator.valid() && _keyswitchManipulator->getCurrentMatrixManipulator()) 
    {
        osgGA::MatrixManipulator* mm = _keyswitchManipulator->getCurrentMatrixManipulator();
        osg::Matrix matrix = mm->getInverseMatrix();
        CameraGroup::setViewByMatrix(Producer::Matrix(matrix.ptr()));

        setFusionDistance(mm->getFusionDistanceMode(),mm->getFusionDistanceValue());

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
        return sv->projectWindowXYIntoObject((int)(pixel_x+0.5f),(int)(pixel_y+0.5f),near_point,far_point);
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
        camera->getProjectionRectangle( pr_wx, pr_wy, pr_width, pr_height );

        // convert into clip coords.
        float rx = 2.0f*(pixel_x - (float)pr_wx)/(float)pr_width-1.0f;
        float ry = 2.0f*(pixel_y - (float)pr_wy)/(float)pr_height-1.0f;

        //std::cout << "    rx "<<rx<<"  "<<ry<<std::endl;

        osgProducer::OsgSceneHandler* sh = dynamic_cast<osgProducer::OsgSceneHandler*>(camera->getSceneHandler());
        osgUtil::SceneView* sv = sh?sh->getSceneView():0;
        osg::Matrix vum;
        if (sv!=0)
        {
            vum.set(sv->getViewMatrix() *
                    sv->getProjectionMatrix());
        }
        else
        {
            vum.set(osg::Matrix(camera->getViewMatrix()) *
                    osg::Matrix(camera->getProjectionMatrix()));
        }

        PickVisitor iv;
        
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
    if (_keyswitchManipulator.valid()) _keyswitchManipulator->selectMatrixManipulator(no);
}

void Viewer::requestWarpPointer(float x,float y)
{
    if (_kbmcb)
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
