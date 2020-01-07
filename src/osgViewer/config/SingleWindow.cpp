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

#include <osgViewer/config/SingleWindow>
#include <osgViewer/Renderer>
#include <osgViewer/View>
#include <osgViewer/GraphicsWindow>
#include <osgViewer/Keystone>

#include <osg/TextureRectangle>
#include <osg/Texture1D>
#include <osg/TexMat>
#include <osg/Stencil>
#include <osg/PolygonStipple>

#include <osg/io_utils>

using namespace osgViewer;

void SingleWindow::configure(osgViewer::View& view) const
{
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        OSG_NOTICE<<"SingleWindow::configure() : Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    osg::DisplaySettings* ds = getActiveDisplaySetting(view);
    
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits(ds);

    traits->readDISPLAY();
    if (traits->displayNum<0) traits->displayNum = 0;

    traits->screenNum = _screenNum;
    traits->x = _x;
    traits->y = _y;
    traits->width = _width;
    traits->height = _height;
    traits->windowDecoration = _windowDecoration;
    traits->overrideRedirect = _overrideRedirect;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    
    if (traits->width<=0 || traits->height<=0 ) 
    {
        osg::GraphicsContext::ScreenIdentifier si;
        si.readDISPLAY();

        // displayNum has not been set so reset it to 0.
        if (si.displayNum<0) si.displayNum = 0;

        si.screenNum = _screenNum;

        unsigned int width, height;
        wsi->getScreenResolution(si, width, height);
        if (traits->width<=0) traits->width = width;
        if (traits->height<=0) traits->height = height;
    }
    
    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

    view.getCamera()->setGraphicsContext(gc.get());

    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
    if (gw)
    {
        OSG_INFO<<"SingleWindow::configure - GraphicsWindow has been created successfully."<<std::endl;
        gw->getEventQueue()->getCurrentEventState()->setWindowRectangle(traits->x, traits->y, traits->width, traits->height );
    }
    else
    {
        OSG_NOTICE<<"SingleWindow::configure - GraphicsWindow has not been created successfully."<<std::endl;
        return;
    }

    double fovy, aspectRatio, zNear, zFar;
    view.getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    double newAspectRatio = double(traits->width) / double(traits->height);
    double aspectRatioChange = newAspectRatio / aspectRatio;
    if (aspectRatioChange != 1.0)
    {
        view.getCamera()->getProjectionMatrix() *= osg::Matrix::scale(1.0/aspectRatioChange,1.0,1.0);
    }

    view.getCamera()->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));

    GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;

    view.getCamera()->setDrawBuffer(buffer);
    view.getCamera()->setReadBuffer(buffer);

    if (ds->getKeystoneHint())
    {
        if (ds->getKeystoneHint() && !ds->getKeystoneFileNames().empty()) 
        {
            osgViewer::Keystone::loadKeystoneFiles(ds);
        }
        if (ds->getKeystones().empty()) ds->getKeystones().push_back(new Keystone);
        
        view.assignStereoOrKeystoneToCamera(view.getCamera(), ds);
    }
    else if (ds->getStereo() && ds->getUseSceneViewForStereoHint())
    {
        view.assignStereoOrKeystoneToCamera(view.getCamera(), ds);
    }
}
