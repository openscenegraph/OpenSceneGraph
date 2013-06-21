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

#include <osgViewer/Renderer>
#include <osgViewer/View>
#include <osgViewer/GraphicsWindow>

#include <osg/io_utils>

#include <osg/TextureCubeMap>
#include <osg/TextureRectangle>
#include <osg/Texture1D>
#include <osg/TexMat>
#include <osg/Stencil>
#include <osg/PolygonStipple>
#include <osg/ValueObject>

#include <osgUtil/Optimizer>
#include <osgUtil/ShaderGen>
#include <osgUtil/IntersectionVisitor>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

// view configurations.
#include <osgViewer/config/AcrossAllScreens>
#include <osgViewer/config/SingleWindow>
#include <osgViewer/config/SingleScreen>
#include <osgViewer/config/SphericalDisplay>
#include <osgViewer/config/PanoramicSphericalDisplay>
#include <osgViewer/config/WoWVxDisplay>


#include <iterator>

using namespace osgViewer;

osg::DisplaySettings* ViewConfig::getActiveDisplaySetting(osgViewer::View& view) const
{
    return view.getDisplaySettings()  ? view.getDisplaySettings() : osg::DisplaySettings::instance().get();
}

class CollectedCoordinateSystemNodesVisitor : public osg::NodeVisitor
{
public:

    CollectedCoordinateSystemNodesVisitor():
        NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN) {}

    META_NodeVisitor("osgViewer","CollectedCoordinateSystemNodesVisitor")

    virtual void apply(osg::Node& node)
    {
        traverse(node);
    }

    virtual void apply(osg::CoordinateSystemNode& node)
    {
        if (_pathToCoordinateSystemNode.empty())
        {
            OSG_DEBUG<<"Found CoordinateSystemNode node"<<std::endl;
            OSG_DEBUG<<"     CoordinateSystem = "<<node.getCoordinateSystem()<<std::endl;
            _pathToCoordinateSystemNode = getNodePath();
        }
        else
        {
            OSG_DEBUG<<"Found additional CoordinateSystemNode node, but ignoring"<<std::endl;
            OSG_DEBUG<<"     CoordinateSystem = "<<node.getCoordinateSystem()<<std::endl;
        }
        traverse(node);
    }

    osg::NodePath _pathToCoordinateSystemNode;
};


/** callback class to use to allow matrix manipulators to query the application for the local coordinate frame.*/
class ViewerCoordinateFrameCallback : public osgGA::CameraManipulator::CoordinateFrameCallback
{
public:

    ViewerCoordinateFrameCallback(osgViewer::View* view):
        _view(view) {}

    virtual osg::CoordinateFrame getCoordinateFrame(const osg::Vec3d& position) const
    {
        OSG_DEBUG<<"getCoordinateFrame("<<position<<")"<<std::endl;

        osg::NodePath tmpPath = _view->getCoordinateSystemNodePath();

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
                coordinateFrame.preMultScale(osg::Vec3d(1.0/x.length(),1.0/y.length(),1.0/z.length()));

                // reapply the position.
                coordinateFrame.setTrans(pos);

                OSG_DEBUG<<"csn->computeLocalCoordinateFrame(position)* osg::computeLocalToWorld(tmpPath)"<<coordinateFrame<<std::endl;

            }
            else
            {
                OSG_DEBUG<<"osg::computeLocalToWorld(tmpPath)"<<std::endl;
                coordinateFrame =  osg::computeLocalToWorld(tmpPath);
            }
            return coordinateFrame;
        }
        else
        {
            OSG_DEBUG<<"   no coordinate system found, using default orientation"<<std::endl;
            return osg::Matrixd::translate(position);
        }
    }

protected:
    virtual ~ViewerCoordinateFrameCallback() {}

    osg::observer_ptr<osgViewer::View> _view;
};


View::View():
    _fusionDistanceMode(osgUtil::SceneView::PROPORTIONAL_TO_SCREEN_DISTANCE),
    _fusionDistanceValue(1.0f)
{
    // OSG_NOTICE<<"Constructing osgViewer::View"<<std::endl;

    _startTick = 0;

    _frameStamp = new osg::FrameStamp;
    _frameStamp->setFrameNumber(0);
    _frameStamp->setReferenceTime(0);
    _frameStamp->setSimulationTime(0);

    _scene = new Scene;

    // make sure View is safe to reference multi-threaded.
    setThreadSafeRefUnref(true);

    // need to attach a Renderer to the master camera which has been default constructed
    getCamera()->setRenderer(createRenderer(getCamera()));

    setEventQueue(new osgGA::EventQueue);

    setStats(new osg::Stats("View"));
}


View::View(const osgViewer::View& view, const osg::CopyOp& copyop):
    osg::Object(true),
    osg::View(view,copyop),
    osgGA::GUIActionAdapter(),
    _startTick(0),
    _fusionDistanceMode(view._fusionDistanceMode),
    _fusionDistanceValue(view._fusionDistanceValue)
{
    _scene = new Scene;

    // need to attach a Renderer to the master camera which has been default constructed
    getCamera()->setRenderer(createRenderer(getCamera()));

    setEventQueue(new osgGA::EventQueue);

    setStats(new osg::Stats("View"));
}

View::~View()
{
    OSG_INFO<<"Destructing osgViewer::View"<<std::endl;
}

void View::take(osg::View& rhs)
{
    osg::View::take(rhs);

#if 1
    osgViewer::View* rhs_osgViewer = dynamic_cast<osgViewer::View*>(&rhs);
    if (rhs_osgViewer)
    {

        // copy across rhs
        _startTick = rhs_osgViewer->_startTick;
        _frameStamp = rhs_osgViewer->_frameStamp;

        if (rhs_osgViewer->getSceneData())
        {
            _scene = rhs_osgViewer->_scene;
        }

        if (rhs_osgViewer->_cameraManipulator.valid())
        {
            _cameraManipulator = rhs_osgViewer->_cameraManipulator;
        }

        _eventHandlers.insert(_eventHandlers.end(), rhs_osgViewer->_eventHandlers.begin(), rhs_osgViewer->_eventHandlers.end());

        _coordinateSystemNodePath = rhs_osgViewer->_coordinateSystemNodePath;

        _displaySettings = rhs_osgViewer->_displaySettings;
        _fusionDistanceMode = rhs_osgViewer->_fusionDistanceMode;
        _fusionDistanceValue = rhs_osgViewer->_fusionDistanceValue;


        // clear rhs
        rhs_osgViewer->_frameStamp = 0;
        rhs_osgViewer->_scene = 0;
        rhs_osgViewer->_cameraManipulator = 0;
        rhs_osgViewer->_eventHandlers.clear();

        rhs_osgViewer->_coordinateSystemNodePath.clearNodePath();

        rhs_osgViewer->_displaySettings = 0;
    }
#endif
    computeActiveCoordinateSystemNodePath();
    assignSceneDataToCameras();
}

osg::GraphicsOperation* View::createRenderer(osg::Camera* camera)
{
    Renderer* render = new Renderer(camera);
    camera->setStats(new osg::Stats("Camera"));
    return render;
}


void View::init()
{
    OSG_INFO<<"View::init()"<<std::endl;

    osg::ref_ptr<osgGA::GUIEventAdapter> initEvent = _eventQueue->createEvent();
    initEvent->setEventType(osgGA::GUIEventAdapter::FRAME);

    if (_cameraManipulator.valid())
    {
        _cameraManipulator->init(*initEvent, *this);
    }
}

void View::setStartTick(osg::Timer_t tick)
{
    _startTick = tick;
    
    for(Devices::iterator eitr = _eventSources.begin();
        eitr != _eventSources.end();
        ++eitr)
    {
        (*eitr)->getEventQueue()->setStartTick(_startTick);
    }
}

void View::setSceneData(osg::Node* node)
{
    if (node==_scene->getSceneData()) return;

    osg::ref_ptr<Scene> scene = Scene::getScene(node);

    if (scene)
    {
        OSG_INFO<<"View::setSceneData() Sharing scene "<<scene.get()<<std::endl;
        _scene = scene;
    }
    else
    {
        if (_scene->referenceCount()!=1)
        {
            // we are not the only reference to the Scene so we cannot reuse it.
            _scene = new Scene;
            OSG_INFO<<"View::setSceneData() Allocating new scene"<<_scene.get()<<std::endl;
        }
        else
        {
            OSG_INFO<<"View::setSceneData() Reusing exisitng scene"<<_scene.get()<<std::endl;
        }

        _scene->setSceneData(node);
    }

    if (getSceneData())
    {
        #if defined(OSG_GLES2_AVAILABLE)
            osgUtil::ShaderGenVisitor sgv;
            getSceneData()->getOrCreateStateSet();
            getSceneData()->accept(sgv);
        #endif

        // now make sure the scene graph is set up with the correct DataVariance to protect the dynamic elements of
        // the scene graph from being run in parallel.
        osgUtil::Optimizer::StaticObjectDetectionVisitor sodv;
        getSceneData()->accept(sodv);

        // make sure that existing scene graph objects are allocated with thread safe ref/unref
        if (getViewerBase() &&
            getViewerBase()->getThreadingModel()!=ViewerBase::SingleThreaded)
        {
            getSceneData()->setThreadSafeRefUnref(true);
        }

        // update the scene graph so that it has enough GL object buffer memory for the graphics contexts that will be using it.
        getSceneData()->resizeGLObjectBuffers(osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts());
    }

    computeActiveCoordinateSystemNodePath();

    assignSceneDataToCameras();
}

void View::setDatabasePager(osgDB::DatabasePager* dp)
{
    _scene->setDatabasePager(dp);
}

osgDB::DatabasePager* View::getDatabasePager()
{
    return _scene->getDatabasePager();
}

const osgDB::DatabasePager* View::getDatabasePager() const
{
    return _scene->getDatabasePager();
}


void View::setImagePager(osgDB::ImagePager* dp)
{
    _scene->setImagePager(dp);
}

osgDB::ImagePager* View::getImagePager()
{
    return _scene->getImagePager();
}

const osgDB::ImagePager* View::getImagePager() const
{
    return _scene->getImagePager();
}


void View::setCameraManipulator(osgGA::CameraManipulator* manipulator, bool resetPosition)
{
    _cameraManipulator = manipulator;

    if (_cameraManipulator.valid())
    {
        _cameraManipulator->setCoordinateFrameCallback(new ViewerCoordinateFrameCallback(this));

        if (getSceneData()) _cameraManipulator->setNode(getSceneData());

        if (resetPosition)
        {
            osg::ref_ptr<osgGA::GUIEventAdapter> dummyEvent = _eventQueue->createEvent();
            _cameraManipulator->home(*dummyEvent, *this);
        }
    }
}

void View::home()
{
    if (_cameraManipulator.valid())
    {
        osg::ref_ptr<osgGA::GUIEventAdapter> dummyEvent = _eventQueue->createEvent();
        _cameraManipulator->home(*dummyEvent, *this);
    }
}


void View::addEventHandler(osgGA::GUIEventHandler* eventHandler)
{
    EventHandlers::iterator itr = std::find(_eventHandlers.begin(), _eventHandlers.end(), eventHandler);
    if (itr == _eventHandlers.end())
    {
        _eventHandlers.push_back(eventHandler);
    }
}

void View::removeEventHandler(osgGA::GUIEventHandler* eventHandler)
{
    EventHandlers::iterator itr = std::find(_eventHandlers.begin(), _eventHandlers.end(), eventHandler);
    if (itr != _eventHandlers.end())
    {
        _eventHandlers.erase(itr);
    }
}

void View::setCoordinateSystemNodePath(const osg::NodePath& nodePath)
{
    _coordinateSystemNodePath.setNodePath(nodePath);
}

osg::NodePath View::getCoordinateSystemNodePath() const
{
    osg::NodePath nodePath;
    _coordinateSystemNodePath.getNodePath(nodePath);
    return nodePath;
}

void View::computeActiveCoordinateSystemNodePath()
{
    // now search for CoordinateSystemNode's for which we want to track.
    osg::Node* subgraph = getSceneData();

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


void View::apply(ViewConfig* config)
{
    if (config)
    {
        OSG_INFO<<"Applying osgViewer::ViewConfig : "<<config->className()<<std::endl;
        config->configure(*this);
    }
    _lastAppliedViewConfig = config;
}

void View::setUpViewAcrossAllScreens()
{
    apply(new osgViewer::AcrossAllScreens());
}

void View::setUpViewInWindow(int x, int y, int width, int height, unsigned int screenNum)
{
    apply(new osgViewer::SingleWindow(x, y, width, height, screenNum));
}

void View::setUpViewOnSingleScreen(unsigned int screenNum)
{
    apply(new osgViewer::SingleScreen(screenNum));
}

void View::setUpViewFor3DSphericalDisplay(double radius, double collar, unsigned int screenNum, osg::Image* intensityMap, const osg::Matrixd& projectorMatrix)
{
    apply(new osgViewer::SphericalDisplay(radius, collar, screenNum, intensityMap, projectorMatrix));
}

void View::setUpViewForPanoramicSphericalDisplay(double radius, double collar, unsigned int screenNum, osg::Image* intensityMap, const osg::Matrixd& projectorMatrix)
{
    apply(new osgViewer::PanoramicSphericalDisplay(radius, collar, screenNum, intensityMap, projectorMatrix));
}

void View::setUpViewForWoWVxDisplay(unsigned int screenNum, unsigned char wow_content, unsigned char wow_factor, unsigned char wow_offset, float wow_disparity_Zd, float wow_disparity_vz, float wow_disparity_M, float wow_disparity_C)
{
    apply(new osgViewer::WoWVxDisplay(screenNum, wow_content, wow_factor, wow_offset, wow_disparity_Zd,wow_disparity_vz, wow_disparity_M, wow_disparity_C));
}

DepthPartitionSettings::DepthPartitionSettings(DepthMode mode):
    _mode(mode),
    _zNear(1.0), _zMid(5.0), _zFar(1000.0)
{}

bool DepthPartitionSettings::getDepthRange(osg::View& view, unsigned int partition, double& zNear, double& zFar)
{
    switch(_mode)
    {
        case(FIXED_RANGE):
        {
            if (partition==0)
            {
                zNear = _zNear;
                zFar = _zMid;
                return true;
            }
            else if (partition==1)
            {
                zNear = _zMid;
                zFar = _zFar;
                return true;
            }
            return false;
        }
        case(BOUNDING_VOLUME):
        {
            osgViewer::View* view_withSceneData = dynamic_cast<osgViewer::View*>(&view);
            const osg::Node* node = view_withSceneData ? view_withSceneData->getSceneData() : 0;
            if (!node) return false;

            const osg::Camera* masterCamera = view.getCamera();
            if (!masterCamera) return false;

            osg::BoundingSphere bs = node->getBound();
            const osg::Matrixd& viewMatrix = masterCamera->getViewMatrix();
            //osg::Matrixd& projectionMatrix = masterCamera->getProjectionMatrix();

            osg::Vec3d lookVectorInWorldCoords = osg::Matrixd::transform3x3(viewMatrix,osg::Vec3d(0.0,0.0,-1.0));
            lookVectorInWorldCoords.normalize();

            osg::Vec3d nearPointInWorldCoords = bs.center() - lookVectorInWorldCoords*bs.radius();
            osg::Vec3d farPointInWorldCoords = bs.center() + lookVectorInWorldCoords*bs.radius();

            osg::Vec3d nearPointInEyeCoords = nearPointInWorldCoords * viewMatrix;
            osg::Vec3d farPointInEyeCoords = farPointInWorldCoords * viewMatrix;

#if 0
            OSG_NOTICE<<std::endl;
            OSG_NOTICE<<"viewMatrix = "<<viewMatrix<<std::endl;
            OSG_NOTICE<<"lookVectorInWorldCoords = "<<lookVectorInWorldCoords<<std::endl;
            OSG_NOTICE<<"nearPointInWorldCoords = "<<nearPointInWorldCoords<<std::endl;
            OSG_NOTICE<<"farPointInWorldCoords = "<<farPointInWorldCoords<<std::endl;
            OSG_NOTICE<<"nearPointInEyeCoords = "<<nearPointInEyeCoords<<std::endl;
            OSG_NOTICE<<"farPointInEyeCoords = "<<farPointInEyeCoords<<std::endl;
#endif
            double minZNearRatio = 0.00001;


            if (masterCamera->getDisplaySettings())
            {
                OSG_NOTICE<<"Has display settings"<<std::endl;
            }

            double scene_zNear = -nearPointInEyeCoords.z();
            double scene_zFar = -farPointInEyeCoords.z();
            if (scene_zNear<=0.0) scene_zNear = minZNearRatio * scene_zFar;

            double scene_zMid = sqrt(scene_zFar*scene_zNear);

#if 0
            OSG_NOTICE<<"scene_zNear = "<<scene_zNear<<std::endl;
            OSG_NOTICE<<"scene_zMid = "<<scene_zMid<<std::endl;
            OSG_NOTICE<<"scene_zFar = "<<scene_zFar<<std::endl;
#endif
            if (partition==0)
            {
                zNear = scene_zNear;
                zFar = scene_zMid;
                return true;
            }
            else if (partition==1)
            {
                zNear = scene_zMid;
                zFar = scene_zFar;
                return true;
            }

            return false;
        }
        default: return false;
    }
}

namespace osgDepthPartition {

struct MyUpdateSlaveCallback : public osg::View::Slave::UpdateSlaveCallback
{
    MyUpdateSlaveCallback(DepthPartitionSettings* dps, unsigned int partition):_dps(dps), _partition(partition) {}

    virtual void updateSlave(osg::View& view, osg::View::Slave& slave)
    {
        slave.updateSlaveImplementation(view);

        if (!_dps) return;

        osg::Camera* camera = slave._camera.get();

        double computed_zNear;
        double computed_zFar;
        if (!_dps->getDepthRange(view, _partition, computed_zNear, computed_zFar))
        {
            OSG_NOTICE<<"Switching off Camera "<<camera<<std::endl;
            camera->setNodeMask(0x0);
            return;
        }
        else
        {
            camera->setNodeMask(0xffffff);
        }

        if (camera->getProjectionMatrix()(0,3)==0.0 &&
            camera->getProjectionMatrix()(1,3)==0.0 &&
            camera->getProjectionMatrix()(2,3)==0.0)
        {
            double left, right, bottom, top, zNear, zFar;
            camera->getProjectionMatrixAsOrtho(left, right, bottom, top, zNear, zFar);
            camera->setProjectionMatrixAsOrtho(left, right, bottom, top, computed_zNear, computed_zFar);
        }
        else
        {
            double left, right, bottom, top, zNear, zFar;
            camera->getProjectionMatrixAsFrustum(left, right, bottom, top, zNear, zFar);

            double nr = computed_zNear / zNear;
            camera->setProjectionMatrixAsFrustum(left * nr, right * nr, bottom * nr, top * nr, computed_zNear, computed_zFar);
        }
    }

    osg::ref_ptr<DepthPartitionSettings> _dps;
    unsigned int _partition;
};


typedef std::list< osg::ref_ptr<osg::Camera> > Cameras;

Cameras getActiveCameras(osg::View& view)
{
    Cameras activeCameras;

    if (view.getCamera() && view.getCamera()->getGraphicsContext())
    {
        activeCameras.push_back(view.getCamera());
    }

    for(unsigned int i=0; i<view.getNumSlaves(); ++i)
    {
        osg::View::Slave& slave = view.getSlave(i);
        if (slave._camera.valid() && slave._camera->getGraphicsContext())
        {
            activeCameras.push_back(slave._camera.get());
        }
    }
    return activeCameras;
}

}

bool View::setUpDepthPartitionForCamera(osg::Camera* cameraToPartition, DepthPartitionSettings* incomming_dps)
{
    osg::ref_ptr<osg::GraphicsContext> context = cameraToPartition->getGraphicsContext();
    if (!context) return false;

    osg::ref_ptr<osg::Viewport> viewport = cameraToPartition->getViewport();
    if (!viewport) return false;

    osg::ref_ptr<DepthPartitionSettings> dps = incomming_dps;
    if (!dps) dps = new DepthPartitionSettings;

    bool useMastersSceneData = true;
    osg::Matrixd projectionOffset;
    osg::Matrixd viewOffset;

    if (getCamera()==cameraToPartition)
    {
        // replace main camera with depth partition cameras
        OSG_INFO<<"View::setUpDepthPartitionForCamera(..) Replacing main Camera"<<std::endl;
    }
    else
    {
        unsigned int i = findSlaveIndexForCamera(cameraToPartition);
        if (i>=getNumSlaves()) return false;

        osg::View::Slave& slave = getSlave(i);

        useMastersSceneData = slave._useMastersSceneData;
        projectionOffset = slave._projectionOffset;
        viewOffset = slave._viewOffset;

        OSG_NOTICE<<"View::setUpDepthPartitionForCamera(..) Replacing slave Camera"<<i<<std::endl;
        removeSlave(i);
    }

    cameraToPartition->setGraphicsContext(0);
    cameraToPartition->setViewport(0);

    // far camera
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(context.get());
        camera->setViewport(viewport.get());

        camera->setDrawBuffer(cameraToPartition->getDrawBuffer());
        camera->setReadBuffer(cameraToPartition->getReadBuffer());

        camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
        camera->setCullingMode(osg::Camera::ENABLE_ALL_CULLING);

        addSlave(camera.get());

        osg::View::Slave& slave = getSlave(getNumSlaves()-1);

        slave._useMastersSceneData = useMastersSceneData;
        slave._projectionOffset = projectionOffset;
        slave._viewOffset = viewOffset;
        slave._updateSlaveCallback =  new osgDepthPartition::MyUpdateSlaveCallback(dps.get(), 1);
    }

    // near camera
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(context.get());
        camera->setViewport(viewport.get());

        camera->setDrawBuffer(cameraToPartition->getDrawBuffer());
        camera->setReadBuffer(cameraToPartition->getReadBuffer());

        camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
        camera->setCullingMode(osg::Camera::ENABLE_ALL_CULLING);
        camera->setClearMask(GL_DEPTH_BUFFER_BIT);

        addSlave(camera.get());

        osg::View::Slave& slave = getSlave(getNumSlaves()-1);
        slave._useMastersSceneData = useMastersSceneData;
        slave._projectionOffset = projectionOffset;
        slave._viewOffset = viewOffset;
        slave._updateSlaveCallback =  new osgDepthPartition::MyUpdateSlaveCallback(dps.get(), 0);
    }

    return true;
}



bool View::setUpDepthPartition(DepthPartitionSettings* dsp)
{
    osgDepthPartition::Cameras originalCameras = osgDepthPartition::getActiveCameras(*this);
    if (originalCameras.empty())
    {
        OSG_INFO<<"osgView::View::setUpDepthPartition(,..), no windows assigned, doing view.setUpViewAcrossAllScreens()"<<std::endl;
        setUpViewAcrossAllScreens();

        originalCameras = osgDepthPartition::getActiveCameras(*this);
        if (originalCameras.empty())
        {
            OSG_NOTICE<<"osgView::View::setUpDepthPartition(View,..) Unable to set up windows for viewer."<<std::endl;
            return false;
        }
    }

    bool threadsWereRunning = getViewerBase()->areThreadsRunning();
    if (threadsWereRunning) getViewerBase()->stopThreading();

    for(osgDepthPartition::Cameras::iterator itr = originalCameras.begin();
        itr != originalCameras.end();
        ++itr)
    {
        setUpDepthPartitionForCamera(itr->get(), dsp);
    }

    if (threadsWereRunning) getViewerBase()->startThreading();

    return true;
}


void View::assignSceneDataToCameras()
{
    // OSG_NOTICE<<"View::assignSceneDataToCameras()"<<std::endl;

    if (_scene.valid() && _scene->getDatabasePager() && getViewerBase())
    {
        _scene->getDatabasePager()->setIncrementalCompileOperation(getViewerBase()->getIncrementalCompileOperation());
    }

    osg::Node* sceneData = _scene.valid() ? _scene->getSceneData() : 0;

    if (_cameraManipulator.valid())
    {
        _cameraManipulator->setNode(sceneData);

        osg::ref_ptr<osgGA::GUIEventAdapter> dummyEvent = _eventQueue->createEvent();

        _cameraManipulator->home(*dummyEvent, *this);
    }

    if (_camera.valid())
    {
        _camera->removeChildren(0,_camera->getNumChildren());
        if (sceneData) _camera->addChild(sceneData);

        Renderer* renderer = dynamic_cast<Renderer*>(_camera->getRenderer());
        if (renderer) renderer->setCompileOnNextDraw(true);

    }

    for(unsigned i=0; i<getNumSlaves(); ++i)
    {
        Slave& slave = getSlave(i);
        if (slave._camera.valid() && slave._useMastersSceneData)
        {
            slave._camera->removeChildren(0,slave._camera->getNumChildren());
            if (sceneData) slave._camera->addChild(sceneData);

            Renderer* renderer = dynamic_cast<Renderer*>(slave._camera->getRenderer());
            if (renderer) renderer->setCompileOnNextDraw(true);
        }
    }
}

void View::requestRedraw()
{
    if (getViewerBase())
    {
        getViewerBase()->_requestRedraw = true;
    }
    else
    {
        OSG_INFO<<"View::requestRedraw(), No viewer base has been assigned yet."<<std::endl;
    }
}

void View::requestContinuousUpdate(bool flag)
{
    if (getViewerBase())
    {
        getViewerBase()->_requestContinousUpdate = flag;
    }
    else
    {
        OSG_INFO<<"View::requestContinuousUpdate(), No viewer base has been assigned yet."<<std::endl;
    }
}

void View::requestWarpPointer(float x,float y)
{
    OSG_INFO<<"View::requestWarpPointer("<<x<<","<<y<<")"<<std::endl;
    
    float local_x, local_y;
    const osg::Camera* camera = getCameraContainingPosition(x, y, local_x, local_y);
    if (camera)
    {
        const osgViewer::GraphicsWindow* gw = dynamic_cast<const osgViewer::GraphicsWindow*>(camera->getGraphicsContext());
        if (gw)
        {
            getEventQueue()->mouseWarped(x,y);
            if (gw->getEventQueue()->getCurrentEventState()->getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS)
            {
                local_y = gw->getTraits()->height - local_y;
            }
            const_cast<osgViewer::GraphicsWindow*>(gw)->getEventQueue()->mouseWarped(local_x,local_y);
            const_cast<osgViewer::GraphicsWindow*>(gw)->requestWarpPointer(local_x, local_y);
        }
    }
    else
    {
        OSG_INFO<<"View::requestWarpPointer failed no camera containing pointer"<<std::endl;
    }
}

bool View::containsCamera(const osg::Camera* camera) const
{
    if (_camera == camera) return true;

    for(unsigned i=0; i<getNumSlaves(); ++i)
    {
        const Slave& slave = getSlave(i);
        if (slave._camera == camera) return true;
    }
    return false;
}


const osg::Camera* View::getCameraContainingPosition(float x, float y, float& local_x, float& local_y) const
{
    const osgGA::GUIEventAdapter* eventState = getEventQueue()->getCurrentEventState();
    const osgViewer::GraphicsWindow* gw = dynamic_cast<const osgViewer::GraphicsWindow*>(eventState->getGraphicsContext());
    bool view_invert_y = eventState->getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS;
    
    // OSG_NOTICE<<"getCameraContainingPosition("<<x<<", "<<y<<") view_invert_y = "<<view_invert_y<<", Xmin() = "<<eventState->getXmin()<<", Xmax() = "<<eventState->getXmax()<<", Ymin() = "<<eventState->getYmin()<<", Ymax() = "<<eventState->getYmax()<<std::endl;

    double epsilon = 0.5;
    
       
    // if master camera has graphics context and eventState context matches then assume coordinates refer
    // to master camera
    bool masterActive = (_camera->getGraphicsContext()!=0 && _camera->getViewport());
    bool eventStateMatchesMaster = (gw!=0) ? _camera->getGraphicsContext()==gw : false; 
    
    if (masterActive && eventStateMatchesMaster)
    {
        // OSG_NOTICE<<"Event state matches master"<<std::endl;
        const osg::Viewport* viewport = _camera->getViewport();
        
        // rescale mouse x,y first to 0 to 1 range
        double new_x = (x-eventState->getXmin())/(eventState->getXmax()-eventState->getXmin());
        double new_y = (y-eventState->getYmin())/(eventState->getYmax()-eventState->getYmin());
        
        // flip y if required
        if (view_invert_y) new_y = 1.0f-new_y;
        
        // rescale mouse x, y to window dimensions so we can check against master Camera's viewport
        new_x *= static_cast<double>(_camera->getGraphicsContext()->getTraits()->width);
        new_y *= static_cast<double>(_camera->getGraphicsContext()->getTraits()->height);
        
        if (new_x >= (viewport->x()-epsilon) && new_y >= (viewport->y()-epsilon) &&
            new_x < (viewport->x()+viewport->width()-1.0+epsilon) && new_y <= (viewport->y()+viewport->height()-1.0+epsilon) )
        {
            local_x = new_x;
            local_y = new_y;

            //OSG_NOTICE<<"Returning master camera"<<std::endl;

            return _camera.get();
        }
        else
        {
            // OSG_NOTICE<<"master camera viewport not matched."<<std::endl;
        }
    }
    
    osg::Matrix masterCameraVPW = getCamera()->getViewMatrix() * getCamera()->getProjectionMatrix();

    // convert to non dimensional
    x = (x - eventState->getXmin()) * 2.0 / (eventState->getXmax()-eventState->getXmin()) - 1.0;
    y = (y - eventState->getYmin())* 2.0 / (eventState->getYmax()-eventState->getYmin()) - 1.0;

    if (view_invert_y) y = - y;

    for(int i=getNumSlaves()-1; i>=0; --i)
    {
        const Slave& slave = getSlave(i);
        if (slave._camera.valid() &&
            slave._camera->getAllowEventFocus() &&
            slave._camera->getRenderTargetImplementation()==osg::Camera::FRAME_BUFFER)
        {
            OSG_INFO<<"Testing slave camera "<<slave._camera->getName()<<std::endl;

            const osg::Camera* camera = slave._camera.get();
            const osg::Viewport* viewport = camera ? camera->getViewport() : 0;

            osg::Matrix localCameraVPW = camera->getViewMatrix() * camera->getProjectionMatrix();
            if (viewport) localCameraVPW *= viewport->computeWindowMatrix();

            osg::Matrix matrix( osg::Matrix::inverse(masterCameraVPW) * localCameraVPW );

            osg::Vec3d new_coord = osg::Vec3d(x,y,0.0) * matrix;

            //OSG_NOTICE<<"  x="<<x<<" y="<<y<<std::endl;;
            //OSG_NOTICE<<"  eventState->getXmin()="<<eventState->getXmin()<<" eventState->getXmax()="<<eventState->getXmax()<<std::endl;;
            //OSG_NOTICE<<"  new_coord "<<new_coord<<std::endl;;

            if (viewport &&
                new_coord.x() >= (viewport->x()-epsilon) && new_coord.y() >= (viewport->y()-epsilon) &&
                new_coord.x() < (viewport->x()+viewport->width()-1.0+epsilon) && new_coord.y() <= (viewport->y()+viewport->height()-1.0+epsilon) )
            {
                // OSG_NOTICE<<"  in viewport "<<std::endl;;

                local_x = new_coord.x();
                local_y = new_coord.y();

                return camera;
            }
            else
            {
                // OSG_NOTICE<<"  not in viewport "<<viewport->x()<<" "<<(viewport->x()+viewport->width())<<std::endl;;
            }

        }
    }

    local_x = x;
    local_y = y;

    return 0;
}

bool View::computeIntersections(float x,float y, osgUtil::LineSegmentIntersector::Intersections& intersections, osg::Node::NodeMask traversalMask)
{
    float local_x, local_y;
    const osg::Camera* camera = getCameraContainingPosition(x, y, local_x, local_y);
    
    OSG_INFO<<"computeIntersections("<<x<<", "<<y<<") local_x="<<local_x<<", local_y="<<local_y<<std::endl;
    
    if (camera) return computeIntersections(camera, (camera->getViewport()==0)?osgUtil::Intersector::PROJECTION : osgUtil::Intersector::WINDOW, local_x, local_y, intersections, traversalMask);
    else return false;
}

bool View::computeIntersections(float x,float y, const osg::NodePath& nodePath, osgUtil::LineSegmentIntersector::Intersections& intersections, osg::Node::NodeMask traversalMask)
{
    float local_x, local_y;
    const osg::Camera* camera = getCameraContainingPosition(x, y, local_x, local_y);
    
    OSG_INFO<<"computeIntersections("<<x<<", "<<y<<") local_x="<<local_x<<", local_y="<<local_y<<std::endl;

    if (camera) return computeIntersections(camera, (camera->getViewport()==0)?osgUtil::Intersector::PROJECTION : osgUtil::Intersector::WINDOW, local_x, local_y, nodePath, intersections, traversalMask);
    else return false;
}

bool View::computeIntersections(const osgGA::GUIEventAdapter& ea, osgUtil::LineSegmentIntersector::Intersections& intersections,osg::Node::NodeMask traversalMask)
{
#if 1
    if (ea.getNumPointerData()>=1)
    {
        const osgGA::PointerData* pd = ea.getPointerData(ea.getNumPointerData()-1);
        const osg::Camera* camera = dynamic_cast<const osg::Camera*>(pd->object.get());
        if (camera) 
        {
            return computeIntersections(camera, osgUtil::Intersector::PROJECTION, pd->getXnormalized(), pd->getYnormalized(), intersections, traversalMask);
        }
    }
#endif
    return computeIntersections(ea.getX(), ea.getY(), intersections, traversalMask);
}

bool View::computeIntersections(const osgGA::GUIEventAdapter& ea, const osg::NodePath& nodePath, osgUtil::LineSegmentIntersector::Intersections& intersections,osg::Node::NodeMask traversalMask)
{
#if 1
    if (ea.getNumPointerData()>=1)
    {
        const osgGA::PointerData* pd = ea.getPointerData(ea.getNumPointerData()-1);
        const osg::Camera* camera = dynamic_cast<const osg::Camera*>(pd->object.get());
        if (camera) 
        {
            return computeIntersections(camera, osgUtil::Intersector::PROJECTION, pd->getXnormalized(), pd->getYnormalized(), nodePath, intersections, traversalMask);
        }
    }
#endif
    return computeIntersections(ea.getX(), ea.getY(), nodePath, intersections, traversalMask);
}

bool View::computeIntersections(const osg::Camera* camera, osgUtil::Intersector::CoordinateFrame cf, float x,float y, osgUtil::LineSegmentIntersector::Intersections& intersections, osg::Node::NodeMask traversalMask)
{
    if (!camera) return false;

    osg::ref_ptr< osgUtil::LineSegmentIntersector > picker = new osgUtil::LineSegmentIntersector(cf, x, y);
    osgUtil::IntersectionVisitor iv(picker.get());
    iv.setTraversalMask(traversalMask);

    const_cast<osg::Camera*>(camera)->accept(iv);

    if (picker->containsIntersections())
    {
        intersections = picker->getIntersections();
        return true;
    }
    else
    {
        intersections.clear();
        return false;
    }
}

bool View::computeIntersections(const osg::Camera* camera, osgUtil::Intersector::CoordinateFrame cf, float x,float y, const osg::NodePath& nodePath, osgUtil::LineSegmentIntersector::Intersections& intersections,osg::Node::NodeMask traversalMask)
{
    if (!camera || nodePath.empty()) return false;

    osg::Matrixd matrix;
    if (nodePath.size()>1)
    {
        osg::NodePath prunedNodePath(nodePath.begin(),nodePath.end()-1);
        matrix = osg::computeLocalToWorld(prunedNodePath);
    }

    matrix.postMult(camera->getViewMatrix());
    matrix.postMult(camera->getProjectionMatrix());

    double zNear = -1.0;
    double zFar = 1.0;
    if (cf==osgUtil::Intersector::WINDOW && camera->getViewport())
    {
        matrix.postMult(camera->getViewport()->computeWindowMatrix());
        zNear = 0.0;
        zFar = 1.0;
    }

    osg::Matrixd inverse;
    inverse.invert(matrix);

    osg::Vec3d startVertex = osg::Vec3d(x,y,zNear) * inverse;
    osg::Vec3d endVertex = osg::Vec3d(x,y,zFar) * inverse;

    osg::ref_ptr< osgUtil::LineSegmentIntersector > picker = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::MODEL, startVertex, endVertex);

    osgUtil::IntersectionVisitor iv(picker.get());
    iv.setTraversalMask(traversalMask);
    nodePath.back()->accept(iv);

    if (picker->containsIntersections())
    {
        intersections = picker->getIntersections();
        return true;
    }
    else
    {
        intersections.clear();
        return false;
    }
}

void View::addDevice(osgGA::Device* eventSource)
{
    Devices::iterator itr = std::find( _eventSources.begin(), _eventSources.end(), eventSource );
    if (itr==_eventSources.end())
    {
        _eventSources.push_back(eventSource);
    }
    
    if (eventSource)
        eventSource->getEventQueue()->setStartTick(getStartTick());
}

void View::removeDevice(osgGA::Device* eventSource)
{
    Devices::iterator itr = std::find( _eventSources.begin(), _eventSources.end(), eventSource );
    if (itr!=_eventSources.end())
    {
        _eventSources.erase(itr);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Methods that support Stereo and Keystone correction.
//
osg::Texture* View::createDistortionTexture(int width, int height)
{
    osg::ref_ptr<osg::TextureRectangle> texture = new osg::TextureRectangle;

    texture->setTextureSize(width, height);
    texture->setInternalFormat(GL_RGB);
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);

    return texture.release();
}

osg::Camera* View::assignRenderToTextureCamera(osg::GraphicsContext* gc, int width, int height, osg::Texture* texture)
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setName("Render to texture camera");
    camera->setGraphicsContext(gc);
    camera->setViewport(new osg::Viewport(0,0,width, height));
    camera->setDrawBuffer(GL_FRONT);
    camera->setReadBuffer(GL_FRONT);
    camera->setAllowEventFocus(false);
    camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

    // attach the texture and use it as the color buffer.
    camera->attach(osg::Camera::COLOR_BUFFER, texture);

    addSlave(camera.get(), osg::Matrixd(), osg::Matrixd());

    return camera.release();
}

osg::Camera* View::assignKeystoneDistortionCamera(osg::DisplaySettings* ds, osg::GraphicsContext* gc, int x, int y, int width, int height, GLenum buffer, osg::Texture* texture, Keystone* keystone)
{
    double screenDistance = ds->getScreenDistance();
    double screenWidth = ds->getScreenWidth();
    double screenHeight = ds->getScreenHeight();
    double fovy = osg::RadiansToDegrees(2.0*atan2(screenHeight/2.0,screenDistance));
    double aspectRatio = screenWidth/screenHeight;

    osg::Geode* geode = keystone->createKeystoneDistortionMesh();

    // new we need to add the texture to the mesh, we do so by creating a
    // StateSet to contain the Texture StateAttribute.
    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    osg::TexMat* texmat = new osg::TexMat;
    texmat->setScaleByTextureRectangleSize(true);
    stateset->setTextureAttributeAndModes(0, texmat, osg::StateAttribute::ON);

    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setGraphicsContext(gc);
    camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
    camera->setClearColor( osg::Vec4(0.0,0.0,0.0,1.0) );
    camera->setViewport(new osg::Viewport(x, y, width, height));
    camera->setDrawBuffer(buffer);
    camera->setReadBuffer(buffer);
    camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
    camera->setInheritanceMask(camera->getInheritanceMask() & ~osg::CullSettings::CLEAR_COLOR & ~osg::CullSettings::COMPUTE_NEAR_FAR_MODE);
    //camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

    camera->setViewMatrix(osg::Matrix::identity());
    camera->setProjectionMatrixAsPerspective(fovy, aspectRatio, 0.1, 1000.0);

    // add subgraph to render
    camera->addChild(geode);

    camera->addChild(keystone->createGrid());

    camera->setName("DistortionCorrectionCamera");

    // camera->addEventCallback(new KeystoneHandler(keystone));

    addSlave(camera.get(), osg::Matrixd(), osg::Matrixd(), false);

    return camera.release();
}



void View::StereoSlaveCallback::updateSlave(osg::View& view, osg::View::Slave& slave)
{
    osg::Camera* camera = slave._camera.get();
    osgViewer::View* viewer_view = dynamic_cast<osgViewer::View*>(&view);

    if (_ds.valid() && camera && viewer_view)
    {
        // inherit any settings applied to the master Camera.
        camera->inheritCullSettings(*(view.getCamera()), camera->getInheritanceMask());
        
        if (_eyeScale<0.0)
        {
            camera->setCullMask(camera->getCullMaskLeft());
        }
        else
        {
            camera->setCullMask(camera->getCullMaskRight());
        }
            
        // set projection matrix
        if (_eyeScale<0.0)
        {
            camera->setProjectionMatrix(_ds->computeLeftEyeProjectionImplementation(view.getCamera()->getProjectionMatrix()));
        }
        else
        {
            camera->setProjectionMatrix(_ds->computeRightEyeProjectionImplementation(view.getCamera()->getProjectionMatrix()));
        }

        double sd = _ds->getScreenDistance();
        double fusionDistance = sd;
        switch(viewer_view->getFusionDistanceMode())
        {
            case(osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE):
                fusionDistance = viewer_view->getFusionDistanceValue();
                break;
            case(osgUtil::SceneView::PROPORTIONAL_TO_SCREEN_DISTANCE):
                fusionDistance *= viewer_view->getFusionDistanceValue();
                break;
        }
        double eyeScale = osg::absolute(_eyeScale) * (fusionDistance/sd);

        if (_eyeScale<0.0)
        {
            camera->setViewMatrix(_ds->computeLeftEyeViewImplementation(view.getCamera()->getViewMatrix(), eyeScale));
        }
        else
        {
            camera->setViewMatrix(_ds->computeRightEyeViewImplementation(view.getCamera()->getViewMatrix(), eyeScale));
        }
    }
    else
    {
        slave.updateSlaveImplementation(view);
    }
}

osg::Camera* View::assignStereoCamera(osg::DisplaySettings* ds, osg::GraphicsContext* gc, int x, int y, int width, int height, GLenum buffer, double eyeScale)
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;

    camera->setGraphicsContext(gc);
    camera->setViewport(new osg::Viewport(x,y, width, height));
    camera->setDrawBuffer(buffer);
    camera->setReadBuffer(buffer);

    // add this slave camera to the viewer, with a shift left of the projection matrix
    addSlave(camera.get(), osg::Matrixd::identity(), osg::Matrixd::identity());

    // assign update callback to maintain the correct view and projection matrices
    osg::View::Slave& slave = getSlave(getNumSlaves()-1);
    slave._updateSlaveCallback =  new StereoSlaveCallback(ds, eyeScale);

    return camera.release();
}

static const GLubyte patternVertEven[] = {
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};

static const GLubyte patternVertOdd[] = {
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};

static const GLubyte patternHorzEven[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};

// 32 x 32 bit array every row is a horizontal line of pixels
//  and the (bitwise) columns a vertical line
//  The following is a checkerboard pattern
static const GLubyte patternCheckerboard[] = {
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA};


void View::assignStereoOrKeystoneToCamera(osg::Camera* camera, osg::DisplaySettings* ds)
{
    if (!camera || camera->getGraphicsContext()==0) return;
    if (!ds->getStereo() && !ds->getKeystoneHint()) return;
    
    ds->setUseSceneViewForStereoHint(false);

    typedef std::vector< osg::ref_ptr<Keystone> > Keystones;
    Keystones keystones;
    if (ds->getKeystoneHint() && !ds->getKeystones().empty())
    {
        for(osg::DisplaySettings::Objects::iterator itr = ds->getKeystones().begin();
            itr != ds->getKeystones().end();
            ++itr)
        {
            Keystone* keystone = dynamic_cast<Keystone*>(itr->get());
            if (keystone) keystones.push_back(keystone);
        }
    }
    
    if (ds->getKeystoneHint())
    {
        while(keystones.size()<2) keystones.push_back(new Keystone);
    }

   
    // set up view's main camera
    {
        double height = ds->getScreenHeight();
        double width = ds->getScreenWidth();
        double distance = ds->getScreenDistance();
        double vfov = osg::RadiansToDegrees(atan2(height/2.0f,distance)*2.0);

        camera->setProjectionMatrixAsPerspective( vfov, width/height, 1.0f,10000.0f);
    }
    

    osg::ref_ptr<osg::GraphicsContext> gc = camera->getGraphicsContext();
    
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = const_cast<osg::GraphicsContext::Traits*>(camera->getGraphicsContext()->getTraits());

    if (!ds->getStereo())
    {
        // load or create a Keystone object
        osg::ref_ptr<osgViewer::Keystone> keystone = 0;
        if (!(ds->getKeystones().empty())) keystone = dynamic_cast<osgViewer::Keystone*>(ds->getKeystones().front().get());
        if (!keystone) keystone = new osgViewer::Keystone;
        
        // create distortion texture
        osg::ref_ptr<osg::Texture> texture = createDistortionTexture(traits->width, traits->height);

        // create RTT Camera
        assignRenderToTextureCamera(gc.get(), traits->width, traits->height, texture.get());

        // create Keystone distortion camera
        osg::ref_ptr<osg::Camera> distortion_camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                        0, 0, traits->width, traits->height,
                                                                        traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                        texture.get(), keystone.get());
        // attach Keystone editing event handler.
        distortion_camera->addEventCallback(new KeystoneHandler(keystone.get()));
        
        camera->setGraphicsContext(0);
        
        return;
    }
    
    switch(ds->getStereoMode())
    {
        case(osg::DisplaySettings::QUAD_BUFFER):
        {
            // disconect the camera from the graphics context.
            camera->setGraphicsContext(0);

            // left Camera left buffer
            osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(ds, gc.get(), 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK_LEFT : GL_FRONT_LEFT, -1.0);
            left_camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            left_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);

            // right Camera right buffer
            osg::ref_ptr<osg::Camera> right_camera = assignStereoCamera(ds, gc.get(), 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK_RIGHT : GL_FRONT_RIGHT, 1.0);
            right_camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            right_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 1);

            // for keystone:
            // left camera to render to left texture
            // right camera to render to right texture
            // left keystone camera to render to left buffer
            // left keystone camera to render to right buffer
            // one keystone and editing for the one window
            
            if (!keystones.empty())
            {
                // for keystone:
                // left camera to render to left texture using whole viewport of left texture
                // right camera to render to right texture using whole viewport of right texture
                // left keystone camera to render to left viewport/window
                // right keystone camera to render to right viewport/window
                // two keystone, one for each of the left and right viewports/windows
                
                osg::ref_ptr<Keystone> keystone = keystones.front();

                // create distortion texture
                osg::ref_ptr<osg::Texture> left_texture = createDistortionTexture(traits->width, traits->height);

                // convert to RTT Camera
                left_camera->setViewport(0, 0, traits->width, traits->height);
                left_camera->setDrawBuffer(GL_FRONT);
                left_camera->setReadBuffer(GL_FRONT);
                left_camera->setAllowEventFocus(true);
                left_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                left_camera->attach(osg::Camera::COLOR_BUFFER, left_texture.get());


                // create distortion texture
                osg::ref_ptr<osg::Texture> right_texture = createDistortionTexture(traits->width, traits->height);

                // convert to RTT Camera
                right_camera->setViewport(0, 0, traits->width, traits->height);
                right_camera->setDrawBuffer(GL_FRONT);
                right_camera->setReadBuffer(GL_FRONT);
                right_camera->setAllowEventFocus(true);
                right_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                right_camera->attach(osg::Camera::COLOR_BUFFER, right_texture.get());
                

                // create Keystone left distortion camera
                keystone->setGridColor(osg::Vec4(1.0f,0.0f,0.0,1.0));
                osg::ref_ptr<osg::Camera> left_keystone_camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                                0, 0, traits->width, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK_LEFT : GL_FRONT_LEFT,
                                                                                left_texture.get(), keystone.get());

                left_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);

                // attach Keystone editing event handler.
                left_keystone_camera->addEventCallback(new KeystoneHandler(keystone.get()));

                
                // create Keystone right distortion camera
                osg::ref_ptr<osg::Camera> right_keystone_camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                                0, 0, traits->width, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK_RIGHT : GL_FRONT_RIGHT,
                                                                                right_texture.get(), keystone.get());

                right_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 3);
                right_keystone_camera->setAllowEventFocus(false);
                
            }

            break;
        }
        case(osg::DisplaySettings::ANAGLYPHIC):
        {
            // disconect the camera from the graphics context.
            camera->setGraphicsContext(0);

            // left Camera red
            osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(ds, gc.get(), 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, -1.0);
            left_camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            left_camera->getOrCreateStateSet()->setAttribute(new osg::ColorMask(true, false, false, true));
            left_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);

            // right Camera cyan
            osg::ref_ptr<osg::Camera> right_camera = assignStereoCamera(ds, gc.get(), 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, 1.0);
            right_camera->setClearMask(GL_DEPTH_BUFFER_BIT);
            right_camera->getOrCreateStateSet()->setAttribute(new osg::ColorMask(false, true, true, true));
            right_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 1);

            if (!keystones.empty())
            {
                // for keystone:
                // left camera to render to texture using red colour mask
                // right camera to render to same texture using cyan colour mask
                // keystone camera to render to whole screen without colour masks
                // one keystone and editing for the one window

                osg::ref_ptr<Keystone> keystone = keystones.front();
                
                bool useTwoTexture = true;
                
                if (useTwoTexture)
                {
                    
                    // create left distortion texture
                    osg::ref_ptr<osg::Texture> left_texture = createDistortionTexture(traits->width, traits->height);

                    // convert to RTT Camera
                    left_camera->setDrawBuffer(GL_FRONT);
                    left_camera->setReadBuffer(GL_FRONT);
                    left_camera->setAllowEventFocus(false);
                    left_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);
                    left_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
                    left_camera->getOrCreateStateSet()->removeAttribute(osg::StateAttribute::COLORMASK);
                    left_camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

                    // attach the texture and use it as the color buffer.
                    left_camera->attach(osg::Camera::COLOR_BUFFER, left_texture.get());

                    // create left distortion texture
                    osg::ref_ptr<osg::Texture> right_texture = createDistortionTexture(traits->width, traits->height);

                    // convert to RTT Camera
                    right_camera->setDrawBuffer(GL_FRONT);
                    right_camera->setReadBuffer(GL_FRONT);
                    right_camera->setAllowEventFocus(false);
                    right_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 1);
                    right_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
                    right_camera->getOrCreateStateSet()->removeAttribute(osg::StateAttribute::COLORMASK);
                    right_camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

                    // attach the texture and use it as the color buffer.
                    right_camera->attach(osg::Camera::COLOR_BUFFER, right_texture.get());

                    // create Keystone left distortion camera
                    osg::ref_ptr<osg::Camera> left_keystone_camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                                    0, 0, traits->width, traits->height,
                                                                                    traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                    left_texture.get(), keystone.get());

                    left_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);
                    left_keystone_camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                    left_keystone_camera->getOrCreateStateSet()->setAttribute(new osg::ColorMask(true, false, false, true));


                    // create Keystone right distortion camera
                    osg::ref_ptr<osg::Camera> right_keystone_camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                                    0, 0, traits->width, traits->height,
                                                                                    traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                    right_texture.get(), keystone.get());

                    right_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 3);
                    right_keystone_camera->setClearMask(GL_DEPTH_BUFFER_BIT);
                    right_keystone_camera->getOrCreateStateSet()->setAttribute(new osg::ColorMask(false, true, true, true));

                    // attach Keystone editing event handler.
                    left_keystone_camera->addEventCallback(new KeystoneHandler(keystone.get()));

                    camera->setAllowEventFocus(false);
                    
                }
                else
                {                    
                    // create distortion texture
                    osg::ref_ptr<osg::Texture> texture = createDistortionTexture(traits->width, traits->height);

                    // convert to RTT Camera
                    left_camera->setDrawBuffer(GL_FRONT);
                    left_camera->setReadBuffer(GL_FRONT);
                    left_camera->setAllowEventFocus(false);
                    left_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);
                    left_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                    // attach the texture and use it as the color buffer.
                    left_camera->attach(osg::Camera::COLOR_BUFFER, texture.get());


                    // convert to RTT Camera
                    right_camera->setDrawBuffer(GL_FRONT);
                    right_camera->setReadBuffer(GL_FRONT);
                    right_camera->setAllowEventFocus(false);
                    right_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 1);
                    right_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                    // attach the texture and use it as the color buffer.
                    right_camera->attach(osg::Camera::COLOR_BUFFER, texture.get());


                    // create Keystone distortion camera
                    osg::ref_ptr<osg::Camera> distortion_camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                                    0, 0, traits->width, traits->height,
                                                                                    traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                    texture.get(), keystone.get());

                    distortion_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);
                    
                    // attach Keystone editing event handler.
                    distortion_camera->addEventCallback(new KeystoneHandler(keystone.get()));

                    camera->setAllowEventFocus(false);
                }
            }

            break;
        }
        case(osg::DisplaySettings::HORIZONTAL_SPLIT):
        {
            // disconect the camera from the graphics context.
            camera->setGraphicsContext(0);

            bool left_eye_left_viewport = ds->getSplitStereoHorizontalEyeMapping()==osg::DisplaySettings::LEFT_EYE_LEFT_VIEWPORT;
            int left_start = (left_eye_left_viewport) ? 0 : traits->width/2;
            int right_start = (left_eye_left_viewport) ? traits->width/2 : 0;

            // left viewport Camera
            osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(ds, gc.get(),
                               left_start, 0, traits->width/2, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT,
                               -1.0);

            // right viewport Camera
            osg::ref_ptr<osg::Camera> right_camera = assignStereoCamera(ds, gc.get(),
                               right_start, 0, traits->width/2, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT,
                               1.0);

            if (!keystones.empty())
            {
                // for keystone:
                // left camera to render to left texture using whole viewport of left texture
                // right camera to render to right texture using whole viewport of right texture
                // left keystone camera to render to left viewport/window
                // right keystone camera to render to right viewport/window
                // two keystone, one for each of the left and right viewports/windows
                
                osg::ref_ptr<Keystone> left_keystone = keystones[0];
                osg::ref_ptr<Keystone> right_keystone = keystones[1];

                // create distortion texture
                osg::ref_ptr<osg::Texture> left_texture = createDistortionTexture(traits->width/2, traits->height);

                // convert to RTT Camera
                left_camera->setViewport(0, 0, traits->width/2, traits->height);
                left_camera->setDrawBuffer(GL_FRONT);
                left_camera->setReadBuffer(GL_FRONT);
                left_camera->setAllowEventFocus(true);
                left_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);
                left_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                left_camera->attach(osg::Camera::COLOR_BUFFER, left_texture.get());


                // create distortion texture
                osg::ref_ptr<osg::Texture> right_texture = createDistortionTexture(traits->width/2, traits->height);

                // convert to RTT Camera
                right_camera->setViewport(0, 0, traits->width/2, traits->height);
                right_camera->setDrawBuffer(GL_FRONT);
                right_camera->setReadBuffer(GL_FRONT);
                right_camera->setAllowEventFocus(true);
                right_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 1);
                right_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                right_camera->attach(osg::Camera::COLOR_BUFFER, right_texture.get());
                

                // create Keystone left distortion camera
                left_keystone->setGridColor(osg::Vec4(1.0f,0.0f,0.0,1.0));
                osg::ref_ptr<osg::Camera> left_keystone_camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                                left_start, 0, traits->width/2, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                left_texture.get(), left_keystone.get());

                left_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);

                // attach Keystone editing event handler.
                left_keystone_camera->addEventCallback(new KeystoneHandler(left_keystone.get()));


                // create Keystone right distortion camera
                right_keystone->setGridColor(osg::Vec4(0.0f,1.0f,0.0,1.0));
                osg::ref_ptr<osg::Camera> right_keystone_camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                                right_start, 0, traits->width/2, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                right_texture.get(), right_keystone.get());

                right_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 3);

                // attach Keystone editing event handler.
                right_keystone_camera->addEventCallback(new KeystoneHandler(right_keystone.get()));

                camera->setAllowEventFocus(false);
                
            }
            
            break;
        }
        case(osg::DisplaySettings::VERTICAL_SPLIT):
        {
            // disconect the camera from the graphics context.
            camera->setGraphicsContext(0);

            bool left_eye_bottom_viewport = ds->getSplitStereoVerticalEyeMapping()==osg::DisplaySettings::LEFT_EYE_BOTTOM_VIEWPORT;
            int left_start = (left_eye_bottom_viewport) ? 0 : traits->height/2;
            int right_start = (left_eye_bottom_viewport) ? traits->height/2 : 0;
            
            // bottom viewport Camera
            osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(ds, gc.get(),
                               0, left_start, traits->width, traits->height/2, traits->doubleBuffer ? GL_BACK : GL_FRONT,
                               -1.0);

            // top vieport camera
            osg::ref_ptr<osg::Camera> right_camera = assignStereoCamera(ds, gc.get(),
                               0, right_start, traits->width, traits->height/2, traits->doubleBuffer ? GL_BACK : GL_FRONT,
                               1.0);

            // for keystone:
            // left camera to render to left texture using whole viewport of left texture
            // right camera to render to right texture using whole viewport of right texture
            // left keystone camera to render to left viewport/window
            // right keystone camera to render to right viewport/window
            // two keystone, one for each of the left and right viewports/windows

            if (!keystones.empty())
            {
                // for keystone:
                // left camera to render to left texture using whole viewport of left texture
                // right camera to render to right texture using whole viewport of right texture
                // left keystone camera to render to left viewport/window
                // right keystone camera to render to right viewport/window
                // two keystone, one for each of the left and right viewports/windows

                osg::ref_ptr<Keystone> left_keystone = keystones[0];
                osg::ref_ptr<Keystone> right_keystone = keystones[1];

                // create distortion texture
                osg::ref_ptr<osg::Texture> left_texture = createDistortionTexture(traits->width, traits->height/2);

                // convert to RTT Camera
                left_camera->setViewport(0, 0, traits->width, traits->height/2);
                left_camera->setDrawBuffer(GL_FRONT);
                left_camera->setReadBuffer(GL_FRONT);
                left_camera->setAllowEventFocus(true);
                left_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);
                left_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                left_camera->attach(osg::Camera::COLOR_BUFFER, left_texture.get());


                // create distortion texture
                osg::ref_ptr<osg::Texture> right_texture = createDistortionTexture(traits->width, traits->height/2);

                // convert to RTT Camera
                right_camera->setViewport(0, 0, traits->width, traits->height/2);
                right_camera->setDrawBuffer(GL_FRONT);
                right_camera->setReadBuffer(GL_FRONT);
                right_camera->setAllowEventFocus(true);
                right_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 1);
                right_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                right_camera->attach(osg::Camera::COLOR_BUFFER, right_texture.get());
                

                // create Keystone left distortion camera
                left_keystone->setGridColor(osg::Vec4(1.0f,0.0f,0.0,1.0));
                osg::ref_ptr<osg::Camera> left_keystone_camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                                0, left_start, traits->width, traits->height/2,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                left_texture.get(), left_keystone.get());

                left_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);

                // attach Keystone editing event handler.
                left_keystone_camera->addEventCallback(new KeystoneHandler(left_keystone.get()));


                // create Keystone right distortion camera
                right_keystone->setGridColor(osg::Vec4(0.0f,1.0f,0.0,1.0));
                osg::ref_ptr<osg::Camera> right_keystone_camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                                0, right_start, traits->width, traits->height/2,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                right_texture.get(), right_keystone.get());

                right_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 3);

                // attach Keystone editing event handler.
                right_keystone_camera->addEventCallback(new KeystoneHandler(right_keystone.get()));

                camera->setAllowEventFocus(false);
                
            }

            break;
        }
        case(osg::DisplaySettings::LEFT_EYE):
        {
            // disconect the camera from the graphics context.
            camera->setGraphicsContext(0);

            // single window, whole window, just left eye offsets
            osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(ds, gc.get(), 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, -1.0);

            // for keystone:
            // treat as standard keystone correction.
            // left eye camera to render to texture
            // keystone camera then render to window
            // one keystone and editing for window

            if (!keystones.empty())
            {
                // for keystone:
                // left camera to render to texture using red colour mask
                // right camera to render to same texture using cyan colour mask
                // keystone camera to render to whole screen without colour masks
                // one keystone and editing for the one window

                osg::ref_ptr<Keystone> keystone = keystones.front();

                // create distortion texture
                osg::ref_ptr<osg::Texture> texture = createDistortionTexture(traits->width, traits->height);

                // convert to RTT Camera
                left_camera->setDrawBuffer(GL_FRONT);
                left_camera->setReadBuffer(GL_FRONT);
                left_camera->setAllowEventFocus(false);
                left_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);
                left_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                left_camera->attach(osg::Camera::COLOR_BUFFER, texture.get());


                // create Keystone distortion camera
                osg::ref_ptr<osg::Camera> distortion_camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                                0, 0, traits->width, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                texture.get(), keystone.get());

                distortion_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);
                
                // attach Keystone editing event handler.
                distortion_camera->addEventCallback(new KeystoneHandler(keystone.get()));
            }
            break;
        }
        case(osg::DisplaySettings::RIGHT_EYE):
        {
            // disconect the camera from the graphics context.
            camera->setGraphicsContext(0);

            // single window, whole window, just right eye offsets
            osg::ref_ptr<osg::Camera> right_camera = assignStereoCamera(ds, gc.get(), 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, 1.0);

            // for keystone:
            // treat as standard keystone correction.
            // left eye camera to render to texture
            // keystone camera then render to window
            // one keystone and editing for window

            if (!keystones.empty())
            {
                // for keystone:
                // left camera to render to texture using red colour mask
                // right camera to render to same texture using cyan colour mask
                // keystone camera to render to whole screen without colour masks
                // one keystone and editing for the one window

                osg::ref_ptr<Keystone> keystone = keystones.front();

                // create distortion texture
                osg::ref_ptr<osg::Texture> texture = createDistortionTexture(traits->width, traits->height);

                // convert to RTT Camera
                right_camera->setDrawBuffer(GL_FRONT);
                right_camera->setReadBuffer(GL_FRONT);
                right_camera->setAllowEventFocus(false);
                right_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);
                right_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                right_camera->attach(osg::Camera::COLOR_BUFFER, texture.get());

                // create Keystone distortion camera
                osg::ref_ptr<osg::Camera> distortion_camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                                             0, 0, traits->width, traits->height,
                                                                                             traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                             texture.get(), keystone.get());

                distortion_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 1);
                
                // attach Keystone editing event handler.
                distortion_camera->addEventCallback(new KeystoneHandler(keystone.get()));
            }
            break;
        }
        case(osg::DisplaySettings::HORIZONTAL_INTERLACE):
        case(osg::DisplaySettings::VERTICAL_INTERLACE):
        case(osg::DisplaySettings::CHECKERBOARD):
        {
            // disconect the camera from the graphics context.
            camera->setGraphicsContext(0);

            // set up the stencil buffer
            {
                osg::ref_ptr<osg::Camera> camera = new osg::Camera;
                camera->setGraphicsContext(gc.get());
                camera->setViewport(0, 0, traits->width, traits->height);
                camera->setDrawBuffer(traits->doubleBuffer ? GL_BACK : GL_FRONT);
                camera->setReadBuffer(camera->getDrawBuffer());
                camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
                camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
                camera->setClearStencil(0);
                camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);
                addSlave(camera.get(), false);

                osg::ref_ptr<osg::Geometry> geometry = osg::createTexturedQuadGeometry(osg::Vec3(-1.0f,-1.0f,0.0f), osg::Vec3(2.0f,0.0f,0.0f), osg::Vec3(0.0f,2.0f,0.0f), 0.0f, 0.0f, 1.0f, 1.0f);
                osg::ref_ptr<osg::Geode> geode = new osg::Geode;
                geode->addDrawable(geometry.get());
                camera->addChild(geode.get());

                geode->setCullingActive(false);
                
                osg::ref_ptr<osg::StateSet> stateset = geode->getOrCreateStateSet();

                // set up stencil
                osg::ref_ptr<osg::Stencil> stencil = new osg::Stencil;
                stencil->setFunction(osg::Stencil::ALWAYS, 1, ~0u);
                stencil->setOperation(osg::Stencil::REPLACE, osg::Stencil::REPLACE, osg::Stencil::REPLACE);
                stencil->setWriteMask(~0u);
                stateset->setAttributeAndModes(stencil.get(), osg::StateAttribute::ON);

                // set up polygon stipple
                if(ds->getStereoMode() == osg::DisplaySettings::VERTICAL_INTERLACE)
                {
                    stateset->setAttributeAndModes(new osg::PolygonStipple(patternVertEven), osg::StateAttribute::ON);
                }
                else if(ds->getStereoMode() == osg::DisplaySettings::HORIZONTAL_INTERLACE)
                {
                    stateset->setAttributeAndModes(new osg::PolygonStipple(patternHorzEven), osg::StateAttribute::ON);
                }
                else
                {
                    stateset->setAttributeAndModes(new osg::PolygonStipple(patternCheckerboard), osg::StateAttribute::ON);
                }

                stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
                stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

            }

            // left Camera
            {
                osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(ds, gc.get(), 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, -1.0);
                left_camera->setClearMask(0);
                left_camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
                left_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 1);

                osg::ref_ptr<osg::Stencil> stencil = new osg::Stencil;
                stencil->setFunction(osg::Stencil::EQUAL, 0, ~0u);
                stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
                left_camera->getOrCreateStateSet()->setAttributeAndModes(stencil.get(), osg::StateAttribute::ON);
            }

            // right Camera
            {
                osg::ref_ptr<osg::Camera> right_camera = assignStereoCamera(ds, gc.get(), 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, 1.0);
                right_camera->setClearMask(GL_DEPTH_BUFFER_BIT);
                right_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);

                osg::ref_ptr<osg::Stencil> stencil = new osg::Stencil;
                stencil->setFunction(osg::Stencil::NOTEQUAL, 0, ~0u);
                stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
                right_camera->getOrCreateStateSet()->setAttributeAndModes(stencil.get(), osg::StateAttribute::ON);
            }
            break;
        }
    }
}
