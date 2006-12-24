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

#include <osgUtil/SceneView>

using namespace osgViewer;

class ActionAdapter : public osgGA::GUIActionAdapter
{
public:
        virtual ~ActionAdapter() {}

        virtual void requestRedraw() { /*osg::notify(osg::NOTICE)<<"requestRedraw()"<<std::endl;*/ }
        virtual void requestContinuousUpdate(bool /*needed*/=true) { /*osg::notify(osg::NOTICE)<<"requestContinuousUpdate("<<needed<<")"<<std::endl;*/ }
        virtual void requestWarpPointer(float x,float y) { osg::notify(osg::NOTICE)<<"requestWarpPointer("<<x<<","<<y<<")"<<std::endl; }

};

View::View()
{
    // osg::notify(osg::NOTICE)<<"Constructing osgViewer::View"<<std::endl;
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

        ActionAdapter aa;
        _cameraManipulator->home(*dummyEvent, aa);
    }
}

void View::setUpViewAcrossAllScreens()
{
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi) 
    {
        osg::notify(osg::NOTICE)<<"View::setUpViewAcrossAllScreens() : Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }
    

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
            traits->windowDecoration = false;
            traits->doubleBuffer = true;
            traits->sharedContext = 0;
            
            osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
            
            _camera->setGraphicsContext(gc.get());
            
            osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
            if (gw)
            {
                osg::notify(osg::INFO)<<"  GraphicsWindow has been created successfully."<<std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"  GraphicsWindow has not been created successfully."<<std::endl;
            }

            _camera->setViewport(new osg::Viewport(0, 0, width, height));
    }
    else
    {
        double rotate_x = - double(numScreens-1) * 0.5 * fovx;
        
        float inputRangeMinX = 0.0f;
        float inputRangeMinY = 0.0f;
        
        float maxHeight = 0.0f;
    
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
                
                gw->getEventQueue()->setUseFixedMouseInputRange(true);
                gw->getEventQueue()->getCurrentEventState()->setInputRange(inputRangeMinX, inputRangeMinY, inputRangeMinX+float(width),inputRangeMinY+float(height) );
                inputRangeMinX += float(width);
                
                if (maxHeight < float(height)) maxHeight = float(height);
            }
            else
            {
                osg::notify(osg::NOTICE)<<"  GraphicsWindow has not been created successfully."<<std::endl;
            }
            
            camera->setViewport(new osg::Viewport(0, 0, width, height));
            
            addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate( rotate_x, 0.0, 1.0, 0.0));
            
        }
        
        getEventQueue()->setUseFixedMouseInputRange(true);
        getEventQueue()->getCurrentEventState()->setInputRange(0.0f, 0.0, inputRangeMinX, maxHeight);
    }
    
    setUpRenderingSupport();
    assignSceneDataToCameras();
}

// Draw operation, that does a draw on the scene graph.
struct RenderingOperation : public osg::GraphicsOperation
{
    RenderingOperation(osgUtil::SceneView* sceneView, osgDB::DatabasePager* databasePager):
        osg::GraphicsOperation("Render",true),
        _sceneView(sceneView),
        _databasePager(databasePager)
    {
        _sceneView->getCullVisitor()->setDatabaseRequestHandler(_databasePager.get());
    }
    
    virtual void operator () (osg::GraphicsContext*)
    {
        if (!_sceneView) return;
    
        _sceneView->cull();
        _sceneView->draw();

        if (_databasePager.valid())
        {
            double availableTime = 0.004; // 4 ms
            _databasePager->compileGLObjects(*(_sceneView->getState()), availableTime);
            _sceneView->flushDeletedGLObjects(availableTime);
        }
    }
    
    osg::observer_ptr<osgUtil::SceneView>    _sceneView;
    osg::observer_ptr<osgDB::DatabasePager>  _databasePager;
};

void View::setUpRenderingSupport()
{
    osg::FrameStamp* frameStamp = _scene->getFrameStamp();

    // what should we do with the old sceneViews?
    _cameraSceneViewMap.clear();

    if (_camera.valid() && _camera->getGraphicsContext())
    {
        osgUtil::SceneView* sceneView = new osgUtil::SceneView;
        _cameraSceneViewMap[_camera] = sceneView;

        sceneView->setDefaults();
        sceneView->setCamera(_camera.get());
        sceneView->setState(_camera->getGraphicsContext()->getState());
        sceneView->setSceneData(getSceneData());
        sceneView->setFrameStamp(frameStamp);

        _camera->getGraphicsContext()->add(new RenderingOperation(sceneView, _scene->getDatabasePager()));        
    }
    
    for(unsigned i=0; i<getNumSlaves(); ++i)
    {
        Slave& slave = getSlave(i);
        if (slave._camera.valid() && slave._camera->getGraphicsContext())
        {
            osgUtil::SceneView* sceneView = new osgUtil::SceneView;
            _cameraSceneViewMap[slave._camera] = sceneView;

            sceneView->setDefaults();
            sceneView->setCamera(slave._camera.get());
            sceneView->setState(slave._camera->getGraphicsContext()->getState());
            sceneView->setSceneData(getSceneData());
            sceneView->setFrameStamp(frameStamp);

            slave._camera->getGraphicsContext()->add(new RenderingOperation(sceneView, _scene->getDatabasePager()));
        }
    }
}

void View::assignSceneDataToCameras()
{
    osg::Node* sceneData = _scene.valid() ? _scene->getSceneData() : 0;
    
    if (_cameraManipulator.valid())
    {
        _cameraManipulator->setNode(sceneData);
        
        osg::ref_ptr<osgGA::GUIEventAdapter> dummyEvent = _eventQueue->createEvent();

        ActionAdapter aa;
        _cameraManipulator->home(*dummyEvent, aa);
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
