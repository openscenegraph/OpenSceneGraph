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

#include <osgViewer/config/SingleScreen>
#include <osgViewer/Renderer>
#include <osgViewer/View>
#include <osgViewer/GraphicsWindow>

#include <osg/io_utils>

using namespace osgViewer;

void SingleScreen::configure(osgViewer::View& view) const
{
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        OSG_NOTICE<<"SingleScreen::configure() : Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    osg::DisplaySettings* ds = getActiveDisplaySetting(view);;

    osg::GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();

    // displayNum has not been set so reset it to 0.
    if (si.displayNum<0) si.displayNum = 0;

    si.screenNum = _screenNum;

    unsigned int width, height;
    wsi->getScreenResolution(si, width, height);

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits(ds);
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

    view.getCamera()->setGraphicsContext(gc.get());

    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
    if (gw)
    {
        OSG_INFO<<"SingleScreen::configure() - GraphicsWindow has been created successfully."<<std::endl;
        gw->getEventQueue()->getCurrentEventState()->setWindowRectangle(0, 0, width, height );
    }
    else
    {
        OSG_NOTICE<<"  GraphicsWindow has not been created successfully."<<std::endl;
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
}
