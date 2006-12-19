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

View::View()
{
    // osg::notify(osg::NOTICE)<<"Constructing osgViewer::View"<<std::endl;

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

void View::setUpViewAcrossAllScreens()
{
    osg::GraphicsContext::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
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
            traits->_x = 0;
            traits->_y = 0;
            traits->_width = width;
            traits->_height = height;
            traits->_windowDecoration = false;
            traits->_doubleBuffer = true;
            traits->_sharedContext = 0;
            
            osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
            
            _camera->setGraphicsContext(gc.get());
            
            osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
            if (gw)
            {
                osg::notify(osg::NOTICE)<<"  GraphicsWindow has been created successfully."<<std::endl;
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
    
        for(unsigned int i=0; i<numScreens; ++i, rotate_x += fovx)
        {
            unsigned int width, height;
            wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(i), width, height);

            osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
            traits->_screenNum = i;
            traits->_x = 0;
            traits->_y = 0;
            traits->_width = width;
            traits->_height = height;
            traits->_windowDecoration = false;
            traits->_doubleBuffer = true;
            traits->_sharedContext = 0;
            
            osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
            
            osg::ref_ptr<osg::Camera> camera = new osg::Camera;
            camera->setGraphicsContext(gc.get());
            
            osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
            if (gw)
            {
                osg::notify(osg::NOTICE)<<"  GraphicsWindow has been created successfully."<<gw<<std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"  GraphicsWindow has not been created successfully."<<std::endl;
            }
            
            camera->setViewport(new osg::Viewport(0, 0, width, height));
            
            addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate( rotate_x, 0.0, 1.0, 0.0));
            
        }
    }
    
    setUpRenderingSupport();
    assignSceneDataToCameras();
}

void View::setUpRenderingSupport()
{
    if (_camera.valid() && _camera->getGraphicsContext())
    {
        osg::ref_ptr<osgUtil::SceneView> sceneView = new osgUtil::SceneView;
        sceneView->setState(_camera->getGraphicsContext()->getState());
        sceneView->setCamera(_camera.get());
        _camera->setRenderingCache(0, sceneView.get());
    }
    
    for(unsigned i=0; i<getNumSlaves(); ++i)
    {
        Slave& slave = getSlave(i);
        if (slave._camera.valid() && slave._camera->getGraphicsContext())
        {
            osg::ref_ptr<osgUtil::SceneView> sceneView = new osgUtil::SceneView;
            sceneView->setDefaults();
            sceneView->setState(slave._camera->getGraphicsContext()->getState());
            sceneView->setCamera(slave._camera.get());
            slave._camera->setRenderingCache(0, sceneView.get());
        }
    }
}

void View::assignSceneDataToCameras()
{
    osg::Node* sceneData = _scene.valid() ? _scene->getSceneData() : 0;
    
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
