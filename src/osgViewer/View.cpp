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

View::View()
{
    // osg::notify(osg::NOTICE)<<"Constructing osgViewer::View"<<std::endl;

    // make sure View is safe to reference multi-threaded.
    setThreadSafeRefUnref(true);

    setEventQueue(new osgGA::EventQueue);
}

View::~View()
{
    // osg::notify(osg::NOTICE)<<"Destructing osgViewer::View"<<std::endl;
}

void View::setSceneData(osg::Node* node)
{
    _scene = new osgViewer::Scene;
    _scene->setSceneData(node);
    
    assignSceneDataToCameras();
}

void View::setCameraManipulator(osgGA::MatrixManipulator* manipulator)
{
    _cameraManipulator = manipulator;
    if (_cameraManipulator.valid() && getSceneData())
    {
        _cameraManipulator->setNode(getSceneData());
        
        osg::ref_ptr<osgGA::GUIEventAdapter> dummyEvent = _eventQueue->createEvent();

        _cameraManipulator->home(*dummyEvent, *this);
    }
}

void View::addEventHandler(osgGA::GUIEventHandler* eventHandler)
{ 
    _eventHandlers.push_back(eventHandler);
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

    double fovx = atan(tan(osg::DegreesToRadians(fovy*0.5)) * aspectRatio) * 2.0;

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

            _camera->setViewport(new osg::Viewport(0, 0, width, height));
            
            GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
            
            _camera->setDrawBuffer(buffer);
            _camera->setReadBuffer(buffer);

    }
    else
    {
    
        bool cylindericalScreen = false;
        
        if (cylindericalScreen)
        {
            double rotate_x = - double(numScreens-1) * 0.5 * fovx;

            for(unsigned int i=0; i<numScreens; ++i, rotate_x += fovx)
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

                camera->setViewport(new osg::Viewport(0, 0, width, height));

                GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
                camera->setDrawBuffer(buffer);
                camera->setReadBuffer(buffer);

                addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate( rotate_x, 0.0, 1.0, 0.0));

            }
        }
        else
        {
            double translate_x = double(numScreens) - 1.0;

            for(unsigned int i=0; i<numScreens; ++i, translate_x -= 2.0)
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

                camera->setViewport(new osg::Viewport(0, 0, width, height));

                GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
                camera->setDrawBuffer(buffer);
                camera->setReadBuffer(buffer);

                addSlave(camera.get(), osg::Matrixd::translate( translate_x, 0.0, 0.0), osg::Matrixd() );

            }
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

    _camera->setViewport(new osg::Viewport(0, 0, width, height));

    GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;

    _camera->setDrawBuffer(buffer);
    _camera->setReadBuffer(buffer);
}

void View::assignSceneDataToCameras()
{
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
        if (slave._camera.valid())
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
    osg::notify(osg::NOTICE)<<"View::requestWarpPointer("<<x<<","<<y<<")"<<std::endl;

    float local_x, local_y;
    const osg::Camera* camera = getCameraContainingPosition(x, y, local_x, local_y);
    if (camera)
    {
        const osgViewer::GraphicsWindow* gw = dynamic_cast<const osgViewer::GraphicsWindow*>(camera->getGraphicsContext());
        if (gw)
        {
            if (gw->getEventQueue()->getCurrentEventState()->getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS)
            {
                local_y = gw->getTraits()->height - local_y;
            }
            const_cast<osgViewer::GraphicsWindow*>(gw)->requestWarpPointer(local_x, local_y);
        }
    }
    else
    {
        osg::notify(osg::NOTICE)<<"View::requestWarpPointer failed no camera containing pointer"<<std::endl;
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
   
    double epsilon = 0.5;

    if (_camera->getGraphicsContext() && _camera->getViewport())
    {
        const osg::Viewport* viewport = _camera->getViewport();
        
        osg::Vec2d new_coord(x, y);
        
        osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(_camera->getGraphicsContext());
        if (gw && gw->getEventQueue()->getCurrentEventState()->getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS)
        {
            new_coord.y() = gw->getTraits()->height - new_coord.y();
        }
        
        if (viewport && 
            new_coord.x() >= (viewport->x()-epsilon) && new_coord.y() >= (viewport->y()-epsilon) &&
            new_coord.x() < (viewport->x()+viewport->width()-1.0+epsilon) && new_coord.y() <= (viewport->y()+viewport->height()-1.0+epsilon) )
        {
            local_x = new_coord.x();
            local_y = new_coord.y();
            return _camera.get();
        }
    }

    osg::Matrix masterCameraVPW = getCamera()->getViewMatrix() * getCamera()->getProjectionMatrix();
    
    x = (x - eventState->getXmin()) * 2.0 / (eventState->getXmax()-eventState->getXmin()) - 1.0;
    y = (y - eventState->getYmin())* 2.0 / (eventState->getYmax()-eventState->getYmin()) - 1.0;

    if (view_invert_y) y = - y;
    
    // osg::notify(osg::NOTICE)<<"  remapped ("<<x<<","<<y<<")"<<std::endl;;
    // osg::notify(osg::NOTICE)<<"  number of slaves = "<<getNumSlaves()<<std::endl;;
    

    for(unsigned i=0; i<getNumSlaves(); ++i)
    {
        const Slave& slave = getSlave(i);
        if (slave._camera.valid())
        {
            const osg::Camera* camera = slave._camera.get();
            const osg::Viewport* viewport = camera ? camera->getViewport() : 0;

            osg::Matrix localCameraVPW = camera->getViewMatrix() * camera->getProjectionMatrix();
            if (viewport) localCameraVPW *= viewport->computeWindowMatrix();

            osg::Matrix matrix( osg::Matrix::inverse(masterCameraVPW) * localCameraVPW );

            osg::Vec3d new_coord = osg::Vec3d(x,y,0.0) * matrix;

            // osg::notify(osg::NOTICE)<<"  new_coord "<<new_coord<<std::endl;;

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
