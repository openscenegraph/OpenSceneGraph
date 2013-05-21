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

#include <osgViewer/config/AcrossAllScreens>
#include <osgViewer/config/SingleScreen>
#include <osgViewer/Renderer>
#include <osgViewer/View>
#include <osgViewer/GraphicsWindow>

#include <osg/io_utils>

using namespace osgViewer;

void AcrossAllScreens::configure(osgViewer::View& view) const
{
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        OSG_NOTICE<<"AcrossAllScreens::configure() : Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    osg::DisplaySettings* ds = getActiveDisplaySetting(view);

    double fovy, aspectRatio, zNear, zFar;
    view.getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    osg::GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();

    // displayNum has not been set so reset it to 0.
    if (si.displayNum<0) si.displayNum = 0;

    unsigned int numScreens = wsi->getNumScreens(si);
    if (numScreens==1)
    {
        osg::ref_ptr<SingleScreen> ss = new SingleScreen(0);
        ss->configure(view);
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

        bool stereoSplitScreens = numScreens==2 &&
                                 ds->getStereoMode()==osg::DisplaySettings::HORIZONTAL_SPLIT &&
                                 ds->getStereo();

        for(unsigned int i=0; i<numScreens; ++i)
        {
            si.screenNum = i;

            unsigned int width, height;
            wsi->getScreenResolution(si, width, height);

            osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits(ds);
            traits->hostName = si.hostName;
            traits->displayNum = si.displayNum;
            traits->screenNum = si.screenNum;
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
                OSG_INFO<<"  GraphicsWindow has been created successfully."<<gw<<std::endl;

                gw->getEventQueue()->getCurrentEventState()->setWindowRectangle(traits->x, traits->y, traits->width, traits->height );
            }
            else
            {
                OSG_NOTICE<<"  GraphicsWindow has not been created successfully."<<std::endl;
            }

            camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));

            GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
            camera->setDrawBuffer(buffer);
            camera->setReadBuffer(buffer);

            if (stereoSplitScreens)
            {
                unsigned int leftCameraNum = (ds->getSplitStereoHorizontalEyeMapping()==osg::DisplaySettings::LEFT_EYE_LEFT_VIEWPORT) ? 0 : 1;

                osg::ref_ptr<osg::DisplaySettings> ds_local = new osg::DisplaySettings(*ds);
                ds_local->setStereoMode(leftCameraNum==i ? osg::DisplaySettings::LEFT_EYE : osg::DisplaySettings::RIGHT_EYE);
                camera->setDisplaySettings(ds_local.get());

                view.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd() );
            }
            else
            {
                double newAspectRatio = double(traits->width) / double(traits->height);
                double aspectRatioChange = newAspectRatio / aspectRatio;

                view.addSlave(camera.get(), osg::Matrixd::translate( translate_x - aspectRatioChange, 0.0, 0.0) * osg::Matrix::scale(1.0/aspectRatioChange,1.0,1.0), osg::Matrixd() );
                translate_x -= aspectRatioChange * 2.0;
            }
        }
    }

    view.assignSceneDataToCameras();
}
