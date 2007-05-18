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

#include <osgViewer/View>
#include <osgViewer/GraphicsWindow>

#include <osg/io_utils>

#include <osgUtil/IntersectionVisitor>

using namespace osgViewer;

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
    
    osg::NodePath _pathToCoordinateSystemNode;
};


/** callback class to use to allow matrix manipulators to querry the application for the local coordinate frame.*/
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
    
    osg::observer_ptr<osgViewer::View> _view;
};

View::View():
    _fusionDistanceMode(osgUtil::SceneView::PROPORTIONAL_TO_SCREEN_DISTANCE),
    _fusionDistanceValue(1.0f)
{
    // osg::notify(osg::NOTICE)<<"Constructing osgViewer::View"<<std::endl;

    // make sure View is safe to reference multi-threaded.
    setThreadSafeRefUnref(true);

    setEventQueue(new osgGA::EventQueue);
}


View::View(const osgViewer::View& view, const osg::CopyOp& copyop):
    osg::View(view,copyop),
    osgGA::GUIActionAdapter(),
    _fusionDistanceMode(view._fusionDistanceMode),
    _fusionDistanceValue(view._fusionDistanceValue)
{
}

View::~View()
{
    // osg::notify(osg::NOTICE)<<"Destructing osgViewer::View"<<std::endl;
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

void View::setSceneData(osg::Node* node)
{
    _scene = new osgViewer::Scene;
    _scene->setSceneData(node);
    
    computeActiveCoordinateSystemNodePath();

    assignSceneDataToCameras();
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
    for(ObserveredNodePath::const_iterator itr = _coordinateSystemNodePath.begin();
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

    unsigned int numScreens = wsi->getNumScreens();
    if (numScreens==1)
    {
        unsigned int width, height;
        wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);

        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
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
        if (ds->getStereo() && (ds->getStereoMode() == osg::DisplaySettings::QUAD_BUFFER)) {
            traits->quadBufferStereo = true;
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
            unsigned int width, height;
            wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(i), width, height);
            translate_x += double(width) / (double(height) * aspectRatio);
        }
    
        for(unsigned int i=0; i<numScreens; ++i)
        {
            unsigned int width, height;
            wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(i), width, height);

            osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
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
            if (ds->getStereo() && (ds->getStereoMode() == osg::DisplaySettings::QUAD_BUFFER)) {
                traits->quadBufferStereo = true;
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

            double newAspectRatio = double(traits->width) / double(traits->height);
            double aspectRatioChange = newAspectRatio / aspectRatio;

            addSlave(camera.get(), osg::Matrixd::translate( translate_x - aspectRatioChange, 0.0, 0.0) * osg::Matrix::scale(1.0/aspectRatioChange,1.0,1.0), osg::Matrixd() );
            translate_x -= aspectRatioChange * 2.0;
        }
    }

    assignSceneDataToCameras();
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

    unsigned int width, height;
    wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(screenNum), width, height);

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->screenNum = screenNum;
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
    if (ds->getStereo() && (ds->getStereoMode() == osg::DisplaySettings::QUAD_BUFFER)) {
        traits->quadBufferStereo = true;
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
    }

    for(unsigned i=0; i<getNumSlaves(); ++i)
    {
        Slave& slave = getSlave(i);
        if (slave._camera.valid() && slave._useMastersSceneData)
        {
            slave._camera->removeChildren(0,slave._camera->getNumChildren());
            if (sceneData) slave._camera->addChild(sceneData);
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
    bool view_invert_y = eventState->getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS;

    // osg::notify(osg::NOTICE)<<"View::getCameraContainingPosition("<<x<<","<<y<<",..,..) view_invert_y="<<view_invert_y<<std::endl;
   
    double epsilon = 0.5;

    if (_camera->getGraphicsContext() && _camera->getViewport())
    {
        const osg::Viewport* viewport = _camera->getViewport();
        
        double new_x = static_cast<double>(_camera->getGraphicsContext()->getTraits()->width) * (x - eventState->getXmin())/(eventState->getXmax()-eventState->getXmin());
        double new_y = view_invert_y ?
                       static_cast<double>(_camera->getGraphicsContext()->getTraits()->height) * (1.0 - (y- eventState->getYmin())/(eventState->getYmax()-eventState->getYmin())) :
                       static_cast<double>(_camera->getGraphicsContext()->getTraits()->height) * (y - eventState->getYmin())/(eventState->getYmax()-eventState->getXmin());
        
        // osg::notify(osg::NOTICE)<<"  new_x="<<new_x<<","<<new_y<<std::endl;


        if (viewport && 
            new_x >= (viewport->x()-epsilon) && new_y >= (viewport->y()-epsilon) &&
            new_x < (viewport->x()+viewport->width()-1.0+epsilon) && new_y <= (viewport->y()+viewport->height()-1.0+epsilon) )
        {
            local_x = new_x;
            local_y = new_y;

            // osg::notify(osg::NOTICE)<<"Returning master camera"<<std::endl;

            return _camera.get();
        }
    }

    osg::Matrix masterCameraVPW = getCamera()->getViewMatrix() * getCamera()->getProjectionMatrix();
    
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
            // osg::notify(osg::NOTICE)<<"Testing slave camera "<<slave._camera->getName()<<std::endl;

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

bool View::computeIntersections(float x,float y, osgUtil::LineSegmentIntersector::Intersections& intersections,osg::Node::NodeMask traversalMask)
{
    if (!_camera.valid()) return false;

    float local_x, local_y = 0.0;    
    const osg::Camera* camera = getCameraContainingPosition(x, y, local_x, local_y);
    if (!camera) camera = _camera.get();
    

    osgUtil::LineSegmentIntersector::CoordinateFrame cf = camera->getViewport() ? osgUtil::Intersector::WINDOW : osgUtil::Intersector::PROJECTION;
    osgUtil::LineSegmentIntersector* picker = new osgUtil::LineSegmentIntersector(cf, local_x, local_y);

#if 0
    osg::notify(osg::NOTICE)<<"View::computeIntersections(x="<<x<<", y="<<y<<", local_x="<<local_x<<", local_y="<<local_y<<") "<<cf<<std::endl;
    osg::notify(osg::NOTICE)<<"  viewport ("<<camera->getViewport()->x()<<","<<camera->getViewport()->y()<<","<<camera->getViewport()->width()<<","<<camera->getViewport()->height()<<")"<<std::endl;

    const osg::GraphicsContext::Traits* traits = camera->getGraphicsContext() ? camera->getGraphicsContext()->getTraits() : 0;
    if (traits)
    {
        osg::notify(osg::NOTICE)<<"  window ("<<traits->x<<","<<traits->y<<","<<traits->width<<","<<traits->height<<")"<<std::endl;
    }
#endif

    osgUtil::IntersectionVisitor iv(picker);
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

    return false;
}

bool View::computeIntersections(float x,float y, osg::NodePath& nodePath, osgUtil::LineSegmentIntersector::Intersections& intersections,osg::Node::NodeMask traversalMask)
{
    if (!_camera.valid()) return false;
    
    float local_x, local_y = 0.0;    
    const osg::Camera* camera = getCameraContainingPosition(x, y, local_x, local_y);
    if (!camera) camera = _camera.get();
    
    osg::Matrix matrix = osg::computeWorldToLocal(nodePath) *  camera->getViewMatrix() * camera->getProjectionMatrix();

    double zNear = -1.0;
    double zFar = 1.0;
    if (camera->getViewport())
    {
        matrix.postMult(camera->getViewport()->computeWindowMatrix());
        zNear = 0.0;
        zFar = 1.0;
    }

    osg::Matrix inverse;
    inverse.invert(matrix);

    osg::Vec3d startVertex = osg::Vec3d(local_x,local_y,zNear) * inverse;
    osg::Vec3d endVertex = osg::Vec3d(local_x,local_y,zFar) * inverse;
    
    osgUtil::LineSegmentIntersector* picker = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::MODEL, startVertex, endVertex);
    
    osgUtil::IntersectionVisitor iv(picker);
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


/////////////////////////////////////////////////////////////////////////////
//
// EndOfDynamicDrawBlock
//
EndOfDynamicDrawBlock::EndOfDynamicDrawBlock(unsigned int numberOfBlocks):
    _numberOfBlocks(numberOfBlocks),
    _blockCount(0)
{
}

void EndOfDynamicDrawBlock::completed(osg::State* /*state*/)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(_mut);
    if (_blockCount>0)
    {
        --_blockCount;

        if (_blockCount==0)
        {
            // osg::notify(osg::NOTICE)<<"Released"<<std::endl;
            _cond.broadcast();
        }
    }
}

void EndOfDynamicDrawBlock::block()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(_mut);
    if (_blockCount)
        _cond.wait(&_mut);
}

void EndOfDynamicDrawBlock::release()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(_mut);
    if (_blockCount)
    {
        _blockCount = 0;
        _cond.broadcast();
    }
}

void EndOfDynamicDrawBlock::reset()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(_mut);
    if (_numberOfBlocks!=_blockCount)
    {
        if (_numberOfBlocks==0) _cond.broadcast();
        _blockCount = _numberOfBlocks;
    }
}

void EndOfDynamicDrawBlock::setNumOfBlocks(unsigned int blockCount)
{
    _numberOfBlocks = blockCount;
}

EndOfDynamicDrawBlock::~EndOfDynamicDrawBlock()
{
    _blockCount = 0;
    release();
}
