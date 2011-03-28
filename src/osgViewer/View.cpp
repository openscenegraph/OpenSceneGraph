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

#include <osgUtil/Optimizer>
#include <osgUtil/IntersectionVisitor>

#include <iterator>

using namespace osgViewer;

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
    
    osg::NodePath _pathToCoordinateSystemNode;
};


/** callback class to use to allow matrix manipulators to query the application for the local coordinate frame.*/
class ViewerCoordinateFrameCallback : public osgGA::MatrixManipulator::CoordinateFrameCallback
{
public:

    ViewerCoordinateFrameCallback(osgViewer::View* view):
        _view(view) {}
    
    virtual osg::CoordinateFrame getCoordinateFrame(const osg::Vec3d& position) const
    {
        osg::notify(osg::INFO)<<"getCoordinateFrame("<<position<<")"<<std::endl;

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
    
    osg::observer_ptr<osgViewer::View> _view;
};


View::View():
    _fusionDistanceMode(osgUtil::SceneView::PROPORTIONAL_TO_SCREEN_DISTANCE),
    _fusionDistanceValue(1.0f)
{
    // osg::notify(osg::NOTICE)<<"Constructing osgViewer::View"<<std::endl;

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
    osg::View(view,copyop),
    osgGA::GUIActionAdapter(),
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
    osg::notify(osg::INFO)<<"Destructing osgViewer::View"<<std::endl;
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
        
        rhs_osgViewer->_coordinateSystemNodePath.clear();
        
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
    osg::notify(osg::INFO)<<"View::init()"<<std::endl;
    
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
}

void View::setSceneData(osg::Node* node)
{
    if (node==_scene->getSceneData()) return;

    osg::ref_ptr<Scene> scene = Scene::getScene(node);

    if (scene)
    {
        osg::notify(osg::INFO)<<"View::setSceneData() Sharing scene "<<scene.get()<<std::endl;
        _scene = scene;
    }
    else
    {
        if (_scene->referenceCount()!=1)
        {
            // we are not the only reference to the Scene so we cannot reuse it.
            _scene = new Scene;
            osg::notify(osg::INFO)<<"View::setSceneData() Allocating new scene"<<_scene.get()<<std::endl;
        }
        else
        {
            osg::notify(osg::INFO)<<"View::setSceneData() Reusing exisitng scene"<<_scene.get()<<std::endl;
        }

        _scene->setSceneData(node);
    }

    if (getSceneData())
    {        
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


void View::setCameraManipulator(osgGA::MatrixManipulator* manipulator)
{
    _cameraManipulator = manipulator;
    
    if (_cameraManipulator.valid())
    {
        _cameraManipulator->setCoordinateFrameCallback(new ViewerCoordinateFrameCallback(this));

        if (getSceneData()) _cameraManipulator->setNode(getSceneData());
        
        osg::ref_ptr<osgGA::GUIEventAdapter> dummyEvent = _eventQueue->createEvent();

        _cameraManipulator->home(*dummyEvent, *this);
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
    _eventHandlers.push_back(eventHandler);
}

void View::setCoordinateSystemNodePath(const osg::NodePath& nodePath)
{
    _coordinateSystemNodePath.clear();
    std::copy(nodePath.begin(),
              nodePath.end(),
              std::back_inserter(_coordinateSystemNodePath));
}

osg::NodePath View::getCoordinateSystemNodePath() const
{
    osg::NodePath nodePath;
    for(ObserverNodePath::const_iterator itr = _coordinateSystemNodePath.begin();
        itr != _coordinateSystemNodePath.end();
        ++itr)
    {
        nodePath.push_back(const_cast<osg::Node*>(itr->get()));
    }
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

void View::setUpViewAcrossAllScreens()
{
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi) 
    {
        osg::notify(osg::NOTICE)<<"View::setUpViewAcrossAllScreens() : Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }
    
    osg::DisplaySettings* ds = _displaySettings.valid() ? _displaySettings.get() : osg::DisplaySettings::instance();
    
    double fovy, aspectRatio, zNear, zFar;        
    _camera->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    osg::GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();
    
    // displayNum has not been set so reset it to 0.
    if (si.displayNum<0) si.displayNum = 0;
    
    unsigned int numScreens = wsi->getNumScreens(si);
    if (numScreens==1)
    {
        if (si.screenNum<0) si.screenNum = 0;
    
        unsigned int width, height;
        wsi->getScreenResolution(si, width, height);

        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->hostName = si.hostName;
        traits->displayNum = si.displayNum;
        traits->screenNum = si.screenNum;
        traits->x = 0;
        traits->y = 0;
        traits->width = width;
        traits->height = height;
        traits->alpha = ds->getMinimumNumAlphaBits();
        traits->stencil = ds->getMinimumNumStencilBits();
        traits->windowDecoration = false;
        traits->doubleBuffer = true;
        traits->sharedContext = 0;
        traits->sampleBuffers = ds->getMultiSamples();
        traits->samples = ds->getNumMultiSamples();
        if (ds->getStereo())
        {
            switch(ds->getStereoMode())
            {
                case(osg::DisplaySettings::QUAD_BUFFER): traits->quadBufferStereo = true; break;
                case(osg::DisplaySettings::VERTICAL_INTERLACE):
                case(osg::DisplaySettings::CHECKERBOARD):
                case(osg::DisplaySettings::HORIZONTAL_INTERLACE): traits->stencil = 8; break;
                default: break;
            }
        }
        
        osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

        _camera->setGraphicsContext(gc.get());
        
        osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
        if (gw)
        {
            osg::notify(osg::INFO)<<"  GraphicsWindow has been created successfully."<<std::endl;
            gw->getEventQueue()->getCurrentEventState()->setWindowRectangle(0, 0, width, height );
        }
        else
        {
            osg::notify(osg::NOTICE)<<"  GraphicsWindow has not been created successfully."<<std::endl;
        }

        double newAspectRatio = double(traits->width) / double(traits->height);
        double aspectRatioChange = newAspectRatio / aspectRatio;
        if (aspectRatioChange != 1.0)
        {
            _camera->getProjectionMatrix() *= osg::Matrix::scale(1.0/aspectRatioChange,1.0,1.0);
        }

        _camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));

        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;

        _camera->setDrawBuffer(buffer);
        _camera->setReadBuffer(buffer);

    }
    else
    {

        double translate_x = 0.0;

        for(unsigned int i=0; i<numScreens; ++i)
        {
            si.screenNum = i;

            unsigned int width, height;
            wsi->getScreenResolution(si, width, height);
            translate_x += double(width) / (double(height) * aspectRatio);
        }
        
        bool stereoSlitScreens = numScreens==2 &&
                                 ds->getStereoMode()==osg::DisplaySettings::HORIZONTAL_SPLIT &&
                                 ds->getStereo();
    
        for(unsigned int i=0; i<numScreens; ++i)
        {
            si.screenNum = i;
        
            unsigned int width, height;
            wsi->getScreenResolution(si, width, height);

            osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
            traits->hostName = si.hostName;
            traits->displayNum = si.displayNum;
            traits->screenNum = si.screenNum;
            traits->screenNum = i;
            traits->x = 0;
            traits->y = 0;
            traits->width = width;
            traits->height = height;
            traits->alpha = ds->getMinimumNumAlphaBits();
            traits->stencil = ds->getMinimumNumStencilBits();
            traits->windowDecoration = false;
            traits->doubleBuffer = true;
            traits->sharedContext = 0;
            traits->sampleBuffers = ds->getMultiSamples();
            traits->samples = ds->getNumMultiSamples();
            if (ds->getStereo())
            {
                switch(ds->getStereoMode())
                {
                    case(osg::DisplaySettings::QUAD_BUFFER): traits->quadBufferStereo = true; break;
                    case(osg::DisplaySettings::VERTICAL_INTERLACE):
                    case(osg::DisplaySettings::CHECKERBOARD):
                    case(osg::DisplaySettings::HORIZONTAL_INTERLACE): traits->stencil = 8; break;
                    default: break;
                }
            }

            osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

            osg::ref_ptr<osg::Camera> camera = new osg::Camera;
            camera->setGraphicsContext(gc.get());

            osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
            if (gw)
            {
                osg::notify(osg::INFO)<<"  GraphicsWindow has been created successfully."<<gw<<std::endl;

                gw->getEventQueue()->getCurrentEventState()->setWindowRectangle(traits->x, traits->y, traits->width, traits->height );
            }
            else
            {
                osg::notify(osg::NOTICE)<<"  GraphicsWindow has not been created successfully."<<std::endl;
            }

            camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));

            GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
            camera->setDrawBuffer(buffer);
            camera->setReadBuffer(buffer);

            if (stereoSlitScreens)
            {
                unsigned int leftCameraNum = (ds->getSplitStereoHorizontalEyeMapping()==osg::DisplaySettings::LEFT_EYE_LEFT_VIEWPORT) ? 0 : 1;
                
                osg::ref_ptr<osg::DisplaySettings> ds_local = new osg::DisplaySettings(*ds);
                ds_local->setStereoMode(leftCameraNum==i ? osg::DisplaySettings::LEFT_EYE : osg::DisplaySettings::RIGHT_EYE);
                camera->setDisplaySettings(ds_local.get());
                
                addSlave(camera.get(), osg::Matrixd(), osg::Matrixd() );
            }
            else
            {
                double newAspectRatio = double(traits->width) / double(traits->height);
                double aspectRatioChange = newAspectRatio / aspectRatio;

                addSlave(camera.get(), osg::Matrixd::translate( translate_x - aspectRatioChange, 0.0, 0.0) * osg::Matrix::scale(1.0/aspectRatioChange,1.0,1.0), osg::Matrixd() );
                translate_x -= aspectRatioChange * 2.0;
            }
        }
    }

    assignSceneDataToCameras();
}

void View::setUpViewInWindow(int x, int y, int width, int height, unsigned int screenNum)
{
    osg::DisplaySettings* ds = _displaySettings.valid() ? _displaySettings.get() : osg::DisplaySettings::instance();

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

    traits->readDISPLAY();
    if (traits->displayNum<0) traits->displayNum = 0;
    
    traits->screenNum = screenNum;
    traits->x = x;
    traits->y = y;
    traits->width = width;
    traits->height = height;
    traits->alpha = ds->getMinimumNumAlphaBits();
    traits->stencil = ds->getMinimumNumStencilBits();
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    traits->sampleBuffers = ds->getMultiSamples();
    traits->samples = ds->getNumMultiSamples();
    if (ds->getStereo())
    {
        switch(ds->getStereoMode())
        {
            case(osg::DisplaySettings::QUAD_BUFFER): traits->quadBufferStereo = true; break;
            case(osg::DisplaySettings::VERTICAL_INTERLACE):
            case(osg::DisplaySettings::CHECKERBOARD):
            case(osg::DisplaySettings::HORIZONTAL_INTERLACE): traits->stencil = 8; break;
            default: break;
        }
    }

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

    _camera->setGraphicsContext(gc.get());

    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
    if (gw)
    {
        osg::notify(osg::INFO)<<"View::setUpViewOnSingleScreen - GraphicsWindow has been created successfully."<<std::endl;
        gw->getEventQueue()->getCurrentEventState()->setWindowRectangle(x, y, width, height );
    }
    else
    {
        osg::notify(osg::NOTICE)<<"  GraphicsWindow has not been created successfully."<<std::endl;
    }

    double fovy, aspectRatio, zNear, zFar;        
    _camera->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    double newAspectRatio = double(traits->width) / double(traits->height);
    double aspectRatioChange = newAspectRatio / aspectRatio;
    if (aspectRatioChange != 1.0)
    {
        _camera->getProjectionMatrix() *= osg::Matrix::scale(1.0/aspectRatioChange,1.0,1.0);
    }

    _camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));

    GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;

    _camera->setDrawBuffer(buffer);
    _camera->setReadBuffer(buffer);
}

void View::setUpViewOnSingleScreen(unsigned int screenNum)
{
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi) 
    {
        osg::notify(osg::NOTICE)<<"View::setUpViewOnSingleScreen() : Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    osg::DisplaySettings* ds = _displaySettings.valid() ? _displaySettings.get() : osg::DisplaySettings::instance();

    osg::GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();
    
    // displayNum has not been set so reset it to 0.
    if (si.displayNum<0) si.displayNum = 0;

    si.screenNum = screenNum;

    unsigned int width, height;
    wsi->getScreenResolution(si, width, height);

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->hostName = si.hostName;
    traits->displayNum = si.displayNum;
    traits->screenNum = si.screenNum;
    traits->x = 0;
    traits->y = 0;
    traits->width = width;
    traits->height = height;
    traits->alpha = ds->getMinimumNumAlphaBits();
    traits->stencil = ds->getMinimumNumStencilBits();
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    traits->sampleBuffers = ds->getMultiSamples();
    traits->samples = ds->getNumMultiSamples();
    if (ds->getStereo())
    {
        switch(ds->getStereoMode())
        {
            case(osg::DisplaySettings::QUAD_BUFFER): traits->quadBufferStereo = true; break;
            case(osg::DisplaySettings::VERTICAL_INTERLACE):
            case(osg::DisplaySettings::CHECKERBOARD):
            case(osg::DisplaySettings::HORIZONTAL_INTERLACE): traits->stencil = 8; break;
            default: break;
        }
    }

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

    _camera->setGraphicsContext(gc.get());

    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
    if (gw)
    {
        osg::notify(osg::INFO)<<"View::setUpViewOnSingleScreen - GraphicsWindow has been created successfully."<<std::endl;
        gw->getEventQueue()->getCurrentEventState()->setWindowRectangle(0, 0, width, height );
    }
    else
    {
        osg::notify(osg::NOTICE)<<"  GraphicsWindow has not been created successfully."<<std::endl;
    }

    double fovy, aspectRatio, zNear, zFar;        
    _camera->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    double newAspectRatio = double(traits->width) / double(traits->height);
    double aspectRatioChange = newAspectRatio / aspectRatio;
    if (aspectRatioChange != 1.0)
    {
        _camera->getProjectionMatrix() *= osg::Matrix::scale(1.0/aspectRatioChange,1.0,1.0);
    }

    _camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));

    GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;

    _camera->setDrawBuffer(buffer);
    _camera->setReadBuffer(buffer);
}

static osg::Geometry* create3DSphericalDisplayDistortionMesh(const osg::Vec3& origin, const osg::Vec3& widthVector, const osg::Vec3& heightVector, double sphere_radius, double collar_radius,osg::Image* intensityMap, const osg::Matrix& projectorMatrix)
{
    osg::Vec3d center(0.0,0.0,0.0);
    osg::Vec3d eye(0.0,0.0,0.0);
    
    double distance = sqrt(sphere_radius*sphere_radius - collar_radius*collar_radius);
    
    bool centerProjection = false;

    osg::Vec3d projector = eye - osg::Vec3d(0.0,0.0, distance);
    
    osg::notify(osg::INFO)<<"create3DSphericalDisplayDistortionMesh : Projector position = "<<projector<<std::endl;
    osg::notify(osg::INFO)<<"create3DSphericalDisplayDistortionMesh : distance = "<<distance<<std::endl;


    // create the quad to visualize.
    osg::Geometry* geometry = new osg::Geometry();

    geometry->setSupportsDisplayList(false);

    osg::Vec3 xAxis(widthVector);
    float width = widthVector.length();
    xAxis /= width;

    osg::Vec3 yAxis(heightVector);
    float height = heightVector.length();
    yAxis /= height;
    
    int noSteps = 50;

    osg::Vec3Array* vertices = new osg::Vec3Array;
    osg::Vec3Array* texcoords0 = new osg::Vec3Array;
    osg::Vec2Array* texcoords1 = intensityMap==0 ? new osg::Vec2Array : 0;
    osg::Vec4Array* colors = new osg::Vec4Array;

    osg::Vec3 bottom = origin;
    osg::Vec3 dx = xAxis*(width/((float)(noSteps-1)));
    osg::Vec3 dy = yAxis*(height/((float)(noSteps-1)));
    
    osg::Vec3d screenCenter = origin + widthVector*0.5f + heightVector*0.5f;
    float screenRadius = heightVector.length() * 0.5f;

    osg::Vec3 cursor = bottom;
    int i,j;

    if (centerProjection)
    {
        for(i=0;i<noSteps;++i)
        {
            osg::Vec3 cursor = bottom+dy*(float)i;
            for(j=0;j<noSteps;++j)
            {
                osg::Vec2 delta(cursor.x() - screenCenter.x(), cursor.y() - screenCenter.y());
                double theta = atan2(-delta.y(), delta.x());
                double phi = osg::PI_2 * delta.length() / screenRadius;
                if (phi > osg::PI_2) phi = osg::PI_2;

                phi *= 2.0;
                
                if (theta<0.0) theta += 2.0*osg::PI;

                // osg::notify(osg::NOTICE)<<"theta = "<<theta<< "phi="<<phi<<std::endl;

                osg::Vec3 texcoord(sin(phi) * cos(theta),
                                   sin(phi) * sin(theta),
                                   cos(phi));

                vertices->push_back(cursor);
                texcoords0->push_back(texcoord * projectorMatrix);

                osg::Vec2 texcoord1(theta/(2.0*osg::PI), 1.0f - phi/osg::PI_2);
                if (intensityMap)
                {            
                    colors->push_back(intensityMap->getColor(texcoord1));
                }
                else
                {
                    colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
                    if (texcoords1) texcoords1->push_back( texcoord1 );
                }

                cursor += dx;
            }
            // osg::notify(osg::NOTICE)<<std::endl;
        }
    }
    else
    {
        for(i=0;i<noSteps;++i)
        {
            osg::Vec3 cursor = bottom+dy*(float)i;
            for(j=0;j<noSteps;++j)
            {
                osg::Vec2 delta(cursor.x() - screenCenter.x(), cursor.y() - screenCenter.y());
                double theta = atan2(-delta.y(), delta.x());
                double phi = osg::PI_2 * delta.length() / screenRadius;
                if (phi > osg::PI_2) phi = osg::PI_2;
                if (theta<0.0) theta += 2.0*osg::PI;

                // osg::notify(osg::NOTICE)<<"theta = "<<theta<< "phi="<<phi<<std::endl;
                
                double f = distance * sin(phi);
                double e = distance * cos(phi) + sqrt( sphere_radius*sphere_radius - f*f);
                double l = e * cos(phi);
                double h = e * sin(phi);
                double z = l - distance;
                
                osg::Vec3 texcoord(h * cos(theta) / sphere_radius,
                                   h * sin(theta) / sphere_radius,
                                   z / sphere_radius);

                vertices->push_back(cursor);
                texcoords0->push_back(texcoord * projectorMatrix);

                osg::Vec2 texcoord1(theta/(2.0*osg::PI), 1.0f - phi/osg::PI_2);
                if (intensityMap)
                {            
                    colors->push_back(intensityMap->getColor(texcoord1));
                }
                else
                {
                    colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
                    if (texcoords1) texcoords1->push_back( texcoord1 );
                }

                cursor += dx;
            }
            // osg::notify(osg::NOTICE)<<std::endl;
        }
    }
    
    // pass the created vertex array to the points geometry object.
    geometry->setVertexArray(vertices);

    geometry->setColorArray(colors);
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    geometry->setTexCoordArray(0,texcoords0);
    if (texcoords1) geometry->setTexCoordArray(1,texcoords1);

    for(i=0;i<noSteps-1;++i)
    {
        osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(osg::PrimitiveSet::QUAD_STRIP);
        for(j=0;j<noSteps;++j)
        {
            elements->push_back(j+(i+1)*noSteps);
            elements->push_back(j+(i)*noSteps);
        }
        geometry->addPrimitiveSet(elements);
    }
    
    return geometry;
}

void View::setUpViewFor3DSphericalDisplay(double radius, double collar, unsigned int screenNum, osg::Image* intensityMap, const osg::Matrixd& projectorMatrix)
{
    osg::notify(osg::INFO)<<"View::setUpViewFor3DSphericalDisplay(rad="<<radius<<", cllr="<<collar<<", sn="<<screenNum<<", im="<<intensityMap<<")"<<std::endl;
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi) 
    {
        osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    osg::GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();
    
    // displayNum has not been set so reset it to 0.
    if (si.displayNum<0) si.displayNum = 0;

    si.screenNum = screenNum;

    unsigned int width, height;
    wsi->getScreenResolution(si, width, height);


    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->hostName = si.hostName;
    traits->displayNum = si.displayNum;
    traits->screenNum = si.screenNum;
    traits->x = 0;
    traits->y = 0;
    traits->width = width;
    traits->height = height;
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    if (!gc)
    {
        osg::notify(osg::NOTICE)<<"GraphicsWindow has not been created successfully."<<std::endl;
        return;
    }
    
    bool applyIntensityMapAsColours = true;

    int tex_width = 512;
    int tex_height = 512;

    int camera_width = tex_width;
    int camera_height = tex_height;

    osg::TextureCubeMap* texture = new osg::TextureCubeMap;

    texture->setTextureSize(tex_width, tex_height);
    texture->setInternalFormat(GL_RGB);
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_R,osg::Texture::CLAMP_TO_EDGE);
    
#if 0    
    osg::Camera::RenderTargetImplementation renderTargetImplementation = osg::Camera::SEPERATE_WINDOW;
    GLenum buffer = GL_FRONT;
#else
    osg::Camera::RenderTargetImplementation renderTargetImplementation = osg::Camera::FRAME_BUFFER_OBJECT;
    GLenum buffer = GL_FRONT;
#endif

    // front face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("Front face camera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);
        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, osg::TextureCubeMap::POSITIVE_Y);

        addSlave(camera.get(), osg::Matrixd(), osg::Matrixd());
    }

    
    // top face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("Top face camera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, osg::TextureCubeMap::POSITIVE_Z);

        addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(-90.0f), 1.0,0.0,0.0));
    }

    // left face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("Left face camera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, osg::TextureCubeMap::NEGATIVE_X);

        addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(-90.0f), 0.0,1.0,0.0) * osg::Matrixd::rotate(osg::inDegrees(-90.0f), 0.0,0.0,1.0));
    }

    // right face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("Right face camera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, osg::TextureCubeMap::POSITIVE_X);

        addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(90.0f), 0.0,1.0,0.0 ) * osg::Matrixd::rotate(osg::inDegrees(90.0f), 0.0,0.0,1.0));
    }

    // bottom face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setName("Bottom face camera");
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, osg::TextureCubeMap::NEGATIVE_Z);

        addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(90.0f), 1.0,0.0,0.0) * osg::Matrixd::rotate(osg::inDegrees(180.0f), 0.0,0.0,1.0));
    }

    // back face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("Back face camera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, osg::TextureCubeMap::NEGATIVE_Y);

        addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(180.0f), 1.0,0.0,0.0));
    }
    
    getCamera()->setProjectionMatrixAsPerspective(90.0f, 1.0, 1, 1000.0);

    // distortion correction set up.
    {
        osg::Geode* geode = new osg::Geode();
        geode->addDrawable(create3DSphericalDisplayDistortionMesh(osg::Vec3(0.0f,0.0f,0.0f), osg::Vec3(width,0.0f,0.0f), osg::Vec3(0.0f,height,0.0f), radius, collar, applyIntensityMapAsColours ? intensityMap : 0, projectorMatrix));

        // new we need to add the texture to the mesh, we do so by creating a 
        // StateSet to contain the Texture StateAttribute.
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

        if (!applyIntensityMapAsColours && intensityMap)
        {
            stateset->setTextureAttributeAndModes(1, new osg::Texture2D(intensityMap), osg::StateAttribute::ON);
        }

        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
        camera->setClearColor( osg::Vec4(0.0,0.0,0.0,1.0) );
        camera->setViewport(new osg::Viewport(0, 0, width, height));
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        camera->setAllowEventFocus(false);
        camera->setInheritanceMask(camera->getInheritanceMask() & ~osg::CullSettings::CLEAR_COLOR & ~osg::CullSettings::COMPUTE_NEAR_FAR_MODE);
        //camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
        
        camera->setProjectionMatrixAsOrtho2D(0,width,0,height);
        camera->setViewMatrix(osg::Matrix::identity());

        // add subgraph to render
        camera->addChild(geode);
        
        camera->setName("DistortionCorrectionCamera");

        addSlave(camera.get(), osg::Matrixd(), osg::Matrixd(), false);
    }
    
    getCamera()->setNearFarRatio(0.0001f);
    
    if (getLightingMode()==osg::View::HEADLIGHT)
    {
        // set a local light source for headlight to ensure that lighting is consistent across sides of cube.
        getLight()->setPosition(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
    }
}

static osg::Geometry* createParoramicSphericalDisplayDistortionMesh(const osg::Vec3& origin, const osg::Vec3& widthVector, const osg::Vec3& heightVector, double sphere_radius, double collar_radius, osg::Image* intensityMap, const osg::Matrix& projectorMatrix)
{
    osg::Vec3d center(0.0,0.0,0.0);
    osg::Vec3d eye(0.0,0.0,0.0);
    
    double distance = sqrt(sphere_radius*sphere_radius - collar_radius*collar_radius);
    bool flip = false;
    bool texcoord_flip = false;

    osg::Vec3d projector = eye - osg::Vec3d(0.0,0.0, distance);
    
    
    osg::notify(osg::INFO)<<"createParoramicSphericalDisplayDistortionMesh : Projector position = "<<projector<<std::endl;
    osg::notify(osg::INFO)<<"createParoramicSphericalDisplayDistortionMesh : distance = "<<distance<<std::endl;

    // create the quad to visualize.
    osg::Geometry* geometry = new osg::Geometry();

    geometry->setSupportsDisplayList(false);

    osg::Vec3 xAxis(widthVector);
    float width = widthVector.length();
    xAxis /= width;

    osg::Vec3 yAxis(heightVector);
    float height = heightVector.length();
    yAxis /= height;
    
    int noSteps = 160;

    osg::Vec3Array* vertices = new osg::Vec3Array;
    osg::Vec2Array* texcoords0 = new osg::Vec2Array;
    osg::Vec2Array* texcoords1 = intensityMap==0 ? new osg::Vec2Array : 0;
    osg::Vec4Array* colors = new osg::Vec4Array;

    osg::Vec3 bottom = origin;
    osg::Vec3 dx = xAxis*(width/((float)(noSteps-2)));
    osg::Vec3 dy = yAxis*(height/((float)(noSteps-1)));
    
    osg::Vec3 top = origin + yAxis*height;

    osg::Vec3 screenCenter = origin + widthVector*0.5f + heightVector*0.5f;
    float screenRadius = heightVector.length() * 0.5f;
    
    geometry->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

    for(int i=0;i<noSteps;++i)
    {
        osg::Vec3 cursor = bottom+dy*(float)i;
        for(int j=0;j<noSteps;++j)
        {
            osg::Vec2 texcoord(double(i)/double(noSteps-1), double(j)/double(noSteps-1));
            double theta = texcoord.x() * 2.0 * osg::PI;
            double phi = (1.0-texcoord.y()) * osg::PI;
            
            if (texcoord_flip) texcoord.y() = 1.0f - texcoord.y();

            osg::Vec3 pos(sin(phi)*sin(theta), sin(phi)*cos(theta), cos(phi));
            pos = pos*projectorMatrix;
            
            double alpha = atan2(pos.x(), pos.y());
            if (alpha<0.0) alpha += 2.0*osg::PI;
            
            double beta = atan2(sqrt(pos.x()*pos.x() + pos.y()*pos.y()), pos.z());
            if (beta<0.0) beta += 2.0*osg::PI;

            double gamma = atan2(sqrt(double(pos.x()*pos.x() + pos.y()*pos.y())), double(pos.z()+distance));
            if (gamma<0.0) gamma += 2.0*osg::PI;
            
            
            osg::Vec3 v = screenCenter + osg::Vec3(sin(alpha)*gamma*2.0/osg::PI, -cos(alpha)*gamma*2.0/osg::PI, 0.0f)*screenRadius;
             
            if (flip)
                vertices->push_back(osg::Vec3(v.x(), top.y()-(v.y()-origin.y()),v.z()));
            else
                vertices->push_back(v);

            texcoords0->push_back( texcoord );
        
            osg::Vec2 texcoord1(alpha/(2.0*osg::PI), 1.0f - beta/osg::PI);
            if (intensityMap)
            {
                colors->push_back(intensityMap->getColor(texcoord1));
            }
            else
            {
                colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
                if (texcoords1) texcoords1->push_back( texcoord1 );
            }


        }
    }


    // pass the created vertex array to the points geometry object.
    geometry->setVertexArray(vertices);

    geometry->setColorArray(colors);
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    geometry->setTexCoordArray(0,texcoords0);
    if (texcoords1) geometry->setTexCoordArray(1,texcoords1);

    osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES);
    geometry->addPrimitiveSet(elements);

    for(int i=0;i<noSteps-1;++i)
    {
        for(int j=0;j<noSteps-1;++j)
        {
            int i1 = j+(i+1)*noSteps;
            int i2 = j+(i)*noSteps;
            int i3 = j+1+(i)*noSteps;
            int i4 = j+1+(i+1)*noSteps;
            
            osg::Vec3& v1 = (*vertices)[i1];
            osg::Vec3& v2 = (*vertices)[i2];
            osg::Vec3& v3 = (*vertices)[i3];
            osg::Vec3& v4 = (*vertices)[i4];

            if ((v1-screenCenter).length()>screenRadius) continue;
            if ((v2-screenCenter).length()>screenRadius) continue;
            if ((v3-screenCenter).length()>screenRadius) continue;
            if ((v4-screenCenter).length()>screenRadius) continue;
            
            elements->push_back(i1);
            elements->push_back(i2);
            elements->push_back(i3);
            
            elements->push_back(i1);
            elements->push_back(i3);
            elements->push_back(i4);
        }
    }
   
    return geometry;
}

void View::setUpViewForPanoramicSphericalDisplay(double radius, double collar, unsigned int screenNum, osg::Image* intensityMap, const osg::Matrixd& projectorMatrix)
{
    osg::notify(osg::INFO)<<"View::setUpViewForPanoramicSphericalDisplay(rad="<<radius<<", cllr="<<collar<<", sn="<<screenNum<<", im="<<intensityMap<<")"<<std::endl;

    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi) 
    {
        osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    osg::GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();
    
    // displayNum has not been set so reset it to 0.
    if (si.displayNum<0) si.displayNum = 0;

    si.screenNum = screenNum;

    unsigned int width, height;
    wsi->getScreenResolution(si, width, height);

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->hostName = si.hostName;
    traits->displayNum = si.displayNum;
    traits->screenNum = si.screenNum;
    traits->x = 0;
    traits->y = 0;
    traits->width = width;
    traits->height = height;
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    

    bool applyIntensityMapAsColours = true;

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    if (!gc)
    {
        osg::notify(osg::NOTICE)<<"GraphicsWindow has not been created successfully."<<std::endl;
        return;
    }

    int tex_width = width;
    int tex_height = height;

    int camera_width = tex_width;
    int camera_height = tex_height;

    osg::TextureRectangle* texture = new osg::TextureRectangle;

    texture->setTextureSize(tex_width, tex_height);
    texture->setInternalFormat(GL_RGB);
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);
    
#if 0    
    osg::Camera::RenderTargetImplementation renderTargetImplementation = osg::Camera::SEPERATE_WINDOW;
    GLenum buffer = GL_FRONT;
#else
    osg::Camera::RenderTargetImplementation renderTargetImplementation = osg::Camera::FRAME_BUFFER_OBJECT;
    GLenum buffer = GL_FRONT;
#endif

    // front face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("Front face camera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);
        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture);

        addSlave(camera.get(), osg::Matrixd(), osg::Matrixd());
    }

    // distortion correction set up.
    {
        osg::Geode* geode = new osg::Geode();
        geode->addDrawable(createParoramicSphericalDisplayDistortionMesh(osg::Vec3(0.0f,0.0f,0.0f), osg::Vec3(width,0.0f,0.0f), osg::Vec3(0.0f,height,0.0f), radius, collar, applyIntensityMapAsColours ? intensityMap : 0, projectorMatrix));

        // new we need to add the texture to the mesh, we do so by creating a 
        // StateSet to contain the Texture StateAttribute.
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

        osg::TexMat* texmat = new osg::TexMat;
        texmat->setScaleByTextureRectangleSize(true);
        stateset->setTextureAttributeAndModes(0, texmat, osg::StateAttribute::ON);

        if (!applyIntensityMapAsColours && intensityMap)
        {
            stateset->setTextureAttributeAndModes(1, new osg::Texture2D(intensityMap), osg::StateAttribute::ON);
        }

        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
        camera->setClearColor( osg::Vec4(0.0,0.0,0.0,1.0) );
        camera->setViewport(new osg::Viewport(0, 0, width, height));
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        camera->setAllowEventFocus(false);
        camera->setInheritanceMask(camera->getInheritanceMask() & ~osg::CullSettings::CLEAR_COLOR & ~osg::CullSettings::COMPUTE_NEAR_FAR_MODE);
        //camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
        
        camera->setProjectionMatrixAsOrtho2D(0,width,0,height);
        camera->setViewMatrix(osg::Matrix::identity());

        // add subgraph to render
        camera->addChild(geode);
        
        camera->setName("DistortionCorrectionCamera");

        addSlave(camera.get(), osg::Matrixd(), osg::Matrixd(), false);
    }
}

void View::setUpViewForWoWVxDisplay(unsigned int screenNum, unsigned char wow_content, unsigned char wow_factor, unsigned char wow_offset, float wow_disparity_Zd, float wow_disparity_vz, float wow_disparity_M, float wow_disparity_C)
{
    osg::notify(osg::INFO)<<"View::setUpViewForWoWVxDisplay(...)"<<std::endl;

    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi) 
    {
        osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    osg::GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();
    
    // displayNum has not been set so reset it to 0.
    if (si.displayNum<0) si.displayNum = 0;
    
    si.screenNum = screenNum;

    unsigned int width, height;
    wsi->getScreenResolution(si, width, height);

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->hostName = si.hostName;
    traits->displayNum = si.displayNum;
    traits->screenNum = si.screenNum;
    traits->x = 0;
    traits->y = 0;
    traits->width = width;
    traits->height = height;
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    if (!gc)
    {
        osg::notify(osg::NOTICE)<<"GraphicsWindow has not been created successfully."<<std::endl;
        return;
    }

    int tex_width = width;
    int tex_height = height;

    int camera_width = tex_width;
    int camera_height = tex_height;

    osg::Texture2D* texture = new osg::Texture2D;
    texture->setTextureSize(tex_width, tex_height);
    texture->setInternalFormat(GL_RGB);
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

    osg::Texture2D* textureD = new osg::Texture2D;
    textureD->setTextureSize(tex_width, tex_height);
    textureD->setInternalFormat(GL_DEPTH_COMPONENT);
    textureD->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    textureD->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

#if 0    
    osg::Camera::RenderTargetImplementation renderTargetImplementation = osg::Camera::SEPERATE_WINDOW;
    GLenum buffer = GL_FRONT;
#else
    osg::Camera::RenderTargetImplementation renderTargetImplementation = osg::Camera::FRAME_BUFFER_OBJECT;
    GLenum buffer = GL_FRONT;
#endif

    // front face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("Front face camera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);
        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture);
        camera->attach(osg::Camera::DEPTH_BUFFER, textureD);

        addSlave(camera.get(), osg::Matrixd(), osg::Matrixd());
    }

    // WoW display set up.
    {
        osg::Texture1D *textureHeader = new osg::Texture1D();
        // Set up the header
        {
            unsigned char header[]= {0xF1,wow_content,wow_factor,wow_offset,0x00,0x00,0x00,0x00,0x00,0x00};
            // Calc the CRC32
            {
                unsigned long _register = 0;
                for(int i = 0; i < 10; ++i) {
                    unsigned char mask = 0x80;
                    unsigned char byte = header[i];
                    for (int j = 0; j < 8; ++j)
                    {
                        bool topBit = (_register & 0x80000000) != 0;
                        _register <<= 1;
                        _register ^= ((byte & mask) != 0? 0x1: 0x0);
                        if (topBit)
                        {
                            _register ^= 0x04c11db7;
                        }
                        mask >>= 1;
                    }
                }
                unsigned char *p = (unsigned char*) &_register;
                for(size_t i = 0; i < 4; ++i)
                {
                    header[i+6] = p[3-i];
                }
            }

            osg::ref_ptr<osg::Image> imageheader = new osg::Image();
            imageheader->allocateImage(256,1,1,GL_LUMINANCE,GL_UNSIGNED_BYTE);
            {
                unsigned char *cheader = imageheader->data();
                for (int x=0; x<256; ++x){
                    cheader[x] = 0;
                }
                for (int x=0; x<=9; ++x){
                    for (int y=7; y>=0; --y){
                        int i = 2*(7-y)+16*x;            
                        cheader[i] = (((1<<(y))&(header[x])) << (7-(y)));
                    }
                }
            }            
            textureHeader->setImage(imageheader.get());
        }

        // Create the Screen Aligned Quad
        osg::Geode* geode = new osg::Geode();
        {
            osg::Geometry* geom = new osg::Geometry;

            osg::Vec3Array* vertices = new osg::Vec3Array;
            vertices->push_back(osg::Vec3(0,height,0));
            vertices->push_back(osg::Vec3(0,0,0));
            vertices->push_back(osg::Vec3(width,0,0));
            vertices->push_back(osg::Vec3(width,height,0));
            geom->setVertexArray(vertices);

            osg::Vec2Array* tex = new osg::Vec2Array;
            tex->push_back(osg::Vec2(0,1));
            tex->push_back(osg::Vec2(0,0));
            tex->push_back(osg::Vec2(1,0));
            tex->push_back(osg::Vec2(1,1));
            geom->setTexCoordArray(0,tex);

            geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
            geode->addDrawable(geom);

            // new we need to add the textures to the quad, and setting up the shader.
            osg::StateSet* stateset = geode->getOrCreateStateSet();
            stateset->setTextureAttributeAndModes(0, textureHeader,osg::StateAttribute::ON);
            stateset->setTextureAttributeAndModes(1, texture,osg::StateAttribute::ON);
            stateset->setTextureAttributeAndModes(2, textureD,osg::StateAttribute::ON);
            stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

            osg::ref_ptr<osg::Program> programShader = new osg::Program();
            stateset->setAttribute(programShader.get(), osg::StateAttribute::ON);
            stateset->addUniform( new osg::Uniform("wow_width", (int)width));
            stateset->addUniform( new osg::Uniform("wow_height", (int)height));
            stateset->addUniform( new osg::Uniform("wow_disparity_M", wow_disparity_M));
            stateset->addUniform( new osg::Uniform("wow_disparity_Zd", wow_disparity_Zd));
            stateset->addUniform( new osg::Uniform("wow_disparity_vz", wow_disparity_vz));
            stateset->addUniform( new osg::Uniform("wow_disparity_C", wow_disparity_C));

            stateset->addUniform(new osg::Uniform("wow_header", 0));
            stateset->addUniform(new osg::Uniform("wow_tcolor", 1));
            stateset->addUniform(new osg::Uniform("wow_tdepth", 2));
            
            osg::Shader *frag = new osg::Shader(osg::Shader::FRAGMENT);
            frag->setShaderSource(" "\
                    " uniform sampler1D wow_header;                                                                                   " \
                    " uniform sampler2D wow_tcolor;                                                                                   " \
                    " uniform sampler2D wow_tdepth;                                                                                   " \
                    "                                                                                                                 " \
                    " uniform int wow_width;                                                                                          " \
                    " uniform int wow_height;                                                                                         " \
                    " uniform float wow_disparity_M;                                                                                  " \
                    " uniform float wow_disparity_Zd;                                                                                 " \
                    " uniform float wow_disparity_vz;                                                                                 " \
                    " uniform float wow_disparity_C;                                                                                  " \
                    "                                                                                                                 " \
                    " float disparity(float Z)                                                                                        " \
                    " {                                                                                                               " \
                    "     return (wow_disparity_M*(1.0-(wow_disparity_vz/(Z-wow_disparity_Zd+wow_disparity_vz)))                        " \
                    "                   + wow_disparity_C) / 255.0;                                                                   " \
                    " }                                                                                                               " \
                    "                                                                                                                 " \
                    " void main()                                                                                                     " \
                    " {                                                                                                               " \
                    "       vec2 pos = (gl_FragCoord.xy / vec2(wow_width/2,wow_height) );                                             " \
                    "         if (gl_FragCoord.x > float(wow_width/2))                                                                  " \
                    "         {                                                                                                         " \
                    "             gl_FragColor = vec4(disparity(( texture2D(wow_tdepth, pos - vec2(1,0))).z));                          " \
                    "         }                                                                                                         " \
                    "         else{                                                                                                     " \
                    "             gl_FragColor = texture2D(wow_tcolor, pos);                                                            " \
                    "         }                                                                                                         " \
                    "     if ( (gl_FragCoord.y >= float(wow_height-1)) && (gl_FragCoord.x < 256.0) )                                    " \
                    "     {                                                                                                             " \
                    "         float pos = gl_FragCoord.x/256.0;                                                                         " \
                    "         float blue = texture1D(wow_header, pos).b;                                                                " \
                    "         if ( blue < 0.5)                                                                                          " \
                    "             gl_FragColor.b -=0.5;                                                                                 " \
                    "         else                                                                                                      " \
                    "             gl_FragColor.b += 0.5;                                                                                " \
                    "     }                                                                                                             " \
                    " }                                                                                                               " );

            programShader->addShader(frag);
        }

        // Create the Camera
        {
            osg::ref_ptr<osg::Camera> camera = new osg::Camera;
            camera->setGraphicsContext(gc.get());
            camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
            camera->setClearColor( osg::Vec4(0.0,0.0,0.0,1.0) );
            camera->setViewport(new osg::Viewport(0, 0, width, height));
            GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
            camera->setDrawBuffer(buffer);
            camera->setReadBuffer(buffer);
            camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
            camera->setAllowEventFocus(false);
            camera->setInheritanceMask(camera->getInheritanceMask() & ~osg::CullSettings::CLEAR_COLOR & ~osg::CullSettings::COMPUTE_NEAR_FAR_MODE);
            //camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
            
            camera->setProjectionMatrixAsOrtho2D(0,width,0,height);
            camera->setViewMatrix(osg::Matrix::identity());

            // add subgraph to render
            camera->addChild(geode);
            
            camera->setName("WoWCamera");

            addSlave(camera.get(), osg::Matrixd(), osg::Matrixd(), false);
        }
    }
}



void View::assignSceneDataToCameras()
{
    // osg::notify(osg::NOTICE)<<"View::assignSceneDataToCameras()"<<std::endl;

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
}

void View::requestContinuousUpdate(bool)
{
}

void View::requestWarpPointer(float x,float y)
{
    osg::notify(osg::INFO)<<"View::requestWarpPointer("<<x<<","<<y<<")"<<std::endl;
    
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
        osg::notify(osg::INFO)<<"View::requestWarpPointer failed no camera containing pointer"<<std::endl;
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

    double epsilon = 0.5;

    if (_camera->getGraphicsContext() &&
        (!gw || _camera->getGraphicsContext()==gw) &&
        _camera->getViewport())
    {
        const osg::Viewport* viewport = _camera->getViewport();
        
        double new_x = x;
        double new_y = y;

        if (!gw)
        {
            new_x = static_cast<double>(_camera->getGraphicsContext()->getTraits()->width) * (x - eventState->getXmin())/(eventState->getXmax()-eventState->getXmin());
            new_y = view_invert_y ?
                       static_cast<double>(_camera->getGraphicsContext()->getTraits()->height) * (1.0 - (y- eventState->getYmin())/(eventState->getYmax()-eventState->getYmin())) :
                       static_cast<double>(_camera->getGraphicsContext()->getTraits()->height) * (y - eventState->getYmin())/(eventState->getYmax()-eventState->getXmin());
        }
        
        if (viewport && 
            new_x >= (viewport->x()-epsilon) && new_y >= (viewport->y()-epsilon) &&
            new_x < (viewport->x()+viewport->width()-1.0+epsilon) && new_y <= (viewport->y()+viewport->height()-1.0+epsilon) )
        {
            local_x = new_x;
            local_y = new_y;

            osg::notify(osg::INFO)<<"Returning master camera"<<std::endl;

            return _camera.get();
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
            osg::notify(osg::INFO)<<"Testing slave camera "<<slave._camera->getName()<<std::endl;

            const osg::Camera* camera = slave._camera.get();
            const osg::Viewport* viewport = camera ? camera->getViewport() : 0;

            osg::Matrix localCameraVPW = camera->getViewMatrix() * camera->getProjectionMatrix();
            if (viewport) localCameraVPW *= viewport->computeWindowMatrix();

            osg::Matrix matrix( osg::Matrix::inverse(masterCameraVPW) * localCameraVPW );

            osg::Vec3d new_coord = osg::Vec3d(x,y,0.0) * matrix;

            //osg::notify(osg::NOTICE)<<"  x="<<x<<" y="<<y<<std::endl;;
            //osg::notify(osg::NOTICE)<<"  eventState->getXmin()="<<eventState->getXmin()<<" eventState->getXmax()="<<eventState->getXmax()<<std::endl;;
            //osg::notify(osg::NOTICE)<<"  new_coord "<<new_coord<<std::endl;;

            if (viewport && 
                new_coord.x() >= (viewport->x()-epsilon) && new_coord.y() >= (viewport->y()-epsilon) &&
                new_coord.x() < (viewport->x()+viewport->width()-1.0+epsilon) && new_coord.y() <= (viewport->y()+viewport->height()-1.0+epsilon) )
            {
                // osg::notify(osg::NOTICE)<<"  in viewport "<<std::endl;;
                
                local_x = new_coord.x();
                local_y = new_coord.y();

                return camera;
            }
            else
            {
                // osg::notify(osg::NOTICE)<<"  not in viewport "<<viewport->x()<<" "<<(viewport->x()+viewport->width())<<std::endl;;
            }

        }
    }
    
    local_x = x;
    local_y = y;
    
    return 0;
}

bool View::computeIntersections(float x,float y, osgUtil::LineSegmentIntersector::Intersections& intersections, osg::Node::NodeMask traversalMask)
{
    if (!_camera.valid()) return false;

    float local_x, local_y = 0.0;    
    const osg::Camera* camera = getCameraContainingPosition(x, y, local_x, local_y);
    if (!camera) camera = _camera.get();
    

    osgUtil::LineSegmentIntersector::CoordinateFrame cf = camera->getViewport() ? osgUtil::Intersector::WINDOW : osgUtil::Intersector::PROJECTION;
    osg::ref_ptr< osgUtil::LineSegmentIntersector > picker = new osgUtil::LineSegmentIntersector(cf, local_x, local_y);

#if 0
    osg::notify(osg::NOTICE)<<"View::computeIntersections(x="<<x<<", y="<<y<<", local_x="<<local_x<<", local_y="<<local_y<<") "<<cf<<std::endl;
    osg::notify(osg::NOTICE)<<"  viewport ("<<camera->getViewport()->x()<<","<<camera->getViewport()->y()<<","<<camera->getViewport()->width()<<","<<camera->getViewport()->height()<<")"<<std::endl;

    const osg::GraphicsContext::Traits* traits = camera->getGraphicsContext() ? camera->getGraphicsContext()->getTraits() : 0;
    if (traits)
    {
        osg::notify(osg::NOTICE)<<"  window ("<<traits->x<<","<<traits->y<<","<<traits->width<<","<<traits->height<<")"<<std::endl;
    }
#endif

    osgUtil::IntersectionVisitor iv(picker.get());
    iv.setTraversalMask(traversalMask);
    
    
#if 1
    const_cast<osg::Camera*>(camera)->accept(iv);
#else    

    // timing test code paths for comparing KdTree based intersections vs conventional intersections

    iv.setUseKdTreeWhenAvailable(true);
    iv.setDoDummyTraversal(true);

    const_cast<osg::Camera*>(camera)->accept(iv);
    

    osg::Timer_t before = osg::Timer::instance()->tick();
    const_cast<osg::Camera*>(camera)->accept(iv);

    osg::Timer_t after_dummy = osg::Timer::instance()->tick();

    int intersectsBeforeKdTree = picker->getIntersections().size();

    iv.setDoDummyTraversal(false);
    const_cast<osg::Camera*>(camera)->accept(iv);
    osg::Timer_t after_kdTree_2 = osg::Timer::instance()->tick();

    int intersectsBeforeConventional = picker->getIntersections().size();

    iv.setUseKdTreeWhenAvailable(false);
    const_cast<osg::Camera*>(camera)->accept(iv);
    osg::Timer_t after = osg::Timer::instance()->tick();
    
    int intersectsAfterConventional = picker->getIntersections().size();

    double timeDummy = osg::Timer::instance()->delta_m(before, after_dummy);
    double timeKdTree = osg::Timer::instance()->delta_m(after_dummy, after_kdTree_2);
    double timeConventional = osg::Timer::instance()->delta_m(after_kdTree_2, after);
    
    osg::notify(osg::NOTICE)<<"Using Dummy                    "<<timeDummy<<std::endl;
    osg::notify(osg::NOTICE)<<"      KdTrees                  "<<timeKdTree
                            <<"\tNum intersects = "<<intersectsBeforeConventional-intersectsBeforeKdTree<<std::endl;
    osg::notify(osg::NOTICE)<<"      KdTrees - Traversal      "<<timeKdTree-timeDummy<<std::endl;
    osg::notify(osg::NOTICE)<<"      Conventional             "<<timeConventional
                            <<"\tNum intersects = "<<intersectsAfterConventional-intersectsBeforeConventional<<std::endl;
    osg::notify(osg::NOTICE)<<"      Conventional - Traversal "<<timeConventional-timeDummy<<std::endl;
    osg::notify(osg::NOTICE)<<"      Delta                    "<<timeConventional/timeKdTree<<std::endl;
    osg::notify(osg::NOTICE)<<"      Delta sans Traversal     "<<(timeConventional-timeDummy)/(timeKdTree-timeDummy)<<std::endl;
    osg::notify(osg::NOTICE)<<std::endl;
#endif    

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

bool View::computeIntersections(float x,float y, const osg::NodePath& nodePath, osgUtil::LineSegmentIntersector::Intersections& intersections,osg::Node::NodeMask traversalMask)
{
    if (!_camera.valid() || nodePath.empty()) return false;
    
    float local_x, local_y = 0.0;    
    const osg::Camera* camera = getCameraContainingPosition(x, y, local_x, local_y);
    if (!camera) camera = _camera.get();
    
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
    if (camera->getViewport())
    {
        matrix.postMult(camera->getViewport()->computeWindowMatrix());
        zNear = 0.0;
        zFar = 1.0;
    }

    osg::Matrixd inverse;
    inverse.invert(matrix);

    osg::Vec3d startVertex = osg::Vec3d(local_x,local_y,zNear) * inverse;
    osg::Vec3d endVertex = osg::Vec3d(local_x,local_y,zFar) * inverse;
    
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

