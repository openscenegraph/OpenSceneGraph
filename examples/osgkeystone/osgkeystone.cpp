/* OpenSceneGraph example, osganimate.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/Notify>
#include <osg/io_utils>
#include <osg/TextureRectangle>
#include <osg/TexMat>
#include <osg/Stencil>
#include <osg/PolygonStipple>
#include <osg/ValueObject>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgViewer/Keystone>

using namespace osgViewer;



osg::Texture* createDistortionTexture(int width, int height)
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

osg::Camera* assignRenderToTextureCamera(osgViewer::View* view, osg::GraphicsContext* gc, int width, int height, osg::Texture* texture)
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

    view->addSlave(camera.get(), osg::Matrixd(), osg::Matrixd());

    return camera.release();
}

osg::Camera* assignKeystoneDistortionCamera(osgViewer::View* view, osg::DisplaySettings* ds, osg::GraphicsContext* gc, int x, int y, int width, int height, GLenum buffer, osg::Texture* texture, Keystone* keystone)
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

    view->addSlave(camera.get(), osg::Matrixd(), osg::Matrixd(), false);

    return camera.release();
}



struct StereoSlaveCallback : public osg::View::Slave::UpdateSlaveCallback
{
    StereoSlaveCallback(osg::DisplaySettings* ds, double eyeScale):_ds(ds), _eyeScale(eyeScale) {}

    virtual void updateSlave(osg::View& view, osg::View::Slave& slave)
    {
        osg::Camera* camera = slave._camera.get();
        osgViewer::View* viewer_view = dynamic_cast<osgViewer::View*>(&view);

        if (_ds.valid() && camera && viewer_view)
        {

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

    osg::ref_ptr<osg::DisplaySettings> _ds;
    double _eyeScale;
};

osg::Camera* assignStereoCamera(osgViewer::View* view, osg::DisplaySettings* ds, osg::GraphicsContext* gc, int x, int y, int width, int height, GLenum buffer, double eyeScale)
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;

    camera->setGraphicsContext(gc);
    camera->setViewport(new osg::Viewport(x,y, width, height));
    camera->setDrawBuffer(buffer);
    camera->setReadBuffer(buffer);

    // add this slave camera to the viewer, with a shift left of the projection matrix
    view->addSlave(camera.get(), osg::Matrixd::identity(), osg::Matrixd::identity());

    // assign update callback to maintain the correct view and projection matrices
    osg::View::Slave& slave = view->getSlave(view->getNumSlaves()-1);
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


void setUpViewForStereo(osgViewer::View* view, osg::DisplaySettings* ds)
{
    if (!ds->getStereo()) return;

    ds->setUseSceneViewForStereoHint(false);


    std::string filename("keystone.osgt");
    osg::ref_ptr<Keystone> keystone = osgDB::readFile<Keystone>(filename);
    keystone->setUserValue("filename",filename);
    
    OSG_NOTICE<<"Keystone "<<keystone.get()<<std::endl;
    
    if (!keystone) keystone = new Keystone;

   
    // set up view's main camera
    {
        double height = osg::DisplaySettings::instance()->getScreenHeight();
        double width = osg::DisplaySettings::instance()->getScreenWidth();
        double distance = osg::DisplaySettings::instance()->getScreenDistance();
        double vfov = osg::RadiansToDegrees(atan2(height/2.0f,distance)*2.0);

        view->getCamera()->setProjectionMatrixAsPerspective( vfov, width/height, 1.0f,10000.0f);
    }
    

    int screenNum = 0;

    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        OSG_NOTICE<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    // unsigned int numScreens = wsi->getNumScreens(si);
    
    osg::GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();

    // displayNum has not been set so reset it to 0.
    if (si.displayNum<0) si.displayNum = 0;

    si.screenNum = screenNum;

    unsigned int width, height;
    wsi->getScreenResolution(si, width, height);

//    width/=2; height/=2;

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

    OSG_NOTICE<<"traits->stencil="<<traits->stencil<<std::endl;


    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    if (!gc)
    {
        OSG_NOTICE<<"GraphicsWindow has not been created successfully."<<std::endl;
        return;
    }

    switch(ds->getStereoMode())
    {
        case(osg::DisplaySettings::QUAD_BUFFER):
        {
            // left Camera left buffer
            osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK_LEFT : GL_FRONT_LEFT, -1.0);
            left_camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            left_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);

            // right Camera right buffer
            osg::ref_ptr<osg::Camera> right_camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK_RIGHT : GL_FRONT_RIGHT, 1.0);
            right_camera->setClearMask(GL_DEPTH_BUFFER_BIT);
            right_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 1);

            // for keystone:
            // left camera to render to left texture
            // right camera to render to right texture
            // left keystone camera to render to left buffer
            // left keystone camera to render to right buffer
            // one keystone and editing for the one window
            
            if (keystone.valid())
            {
                // for keystone:
                // left camera to render to left texture using whole viewport of left texture
                // right camera to render to right texture using whole viewport of right texture
                // left keystone camera to render to left viewport/window
                // right keystone camera to render to right viewport/window
                // two keystone, one for each of the left and right viewports/windows

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
                osg::ref_ptr<osg::Camera> left_keystone_camera = assignKeystoneDistortionCamera(view, ds, gc.get(),
                                                                                0, 0, traits->width, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK_LEFT : GL_FRONT_LEFT,
                                                                                left_texture, keystone.get());

                left_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);

                // attach Keystone editing event handler.
                left_keystone_camera->addEventCallback(new KeystoneHandler(keystone.get()));

                
                // create Keystone right distortion camera
                osg::ref_ptr<osg::Camera> right_keystone_camera = assignKeystoneDistortionCamera(view, ds, gc.get(),
                                                                                0, 0, traits->width, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK_RIGHT : GL_FRONT_RIGHT,
                                                                                right_texture, keystone.get());

                right_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 3);
                right_keystone_camera->setAllowEventFocus(false);
                
            }

            break;
        }
        case(osg::DisplaySettings::ANAGLYPHIC):
        {
            // left Camera red
            osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, -1.0);
            left_camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            left_camera->getOrCreateStateSet()->setAttribute(new osg::ColorMask(true, false, false, true));
            left_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);

            // right Camera cyan
            osg::ref_ptr<osg::Camera> right_camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, 1.0);
            right_camera->setClearMask(GL_DEPTH_BUFFER_BIT);
            right_camera->getOrCreateStateSet()->setAttribute(new osg::ColorMask(false, true, true, true));
            right_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 1);

            if (keystone.valid())
            {
                // for keystone:
                // left camera to render to texture using red colour mask
                // right camera to render to same texture using cyan colour mask
                // keystone camera to render to whole screen without colour masks
                // one keystone and editing for the one window

                // create distortion texture
                osg::ref_ptr<osg::Texture> texture = createDistortionTexture(traits->width, traits->height);

                // convert to RTT Camera
                left_camera->setDrawBuffer(GL_FRONT);
                left_camera->setReadBuffer(GL_FRONT);
                left_camera->setAllowEventFocus(false);
                left_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                left_camera->attach(osg::Camera::COLOR_BUFFER, texture.get());


                // convert to RTT Camera
                right_camera->setDrawBuffer(GL_FRONT);
                right_camera->setReadBuffer(GL_FRONT);
                right_camera->setAllowEventFocus(false);
                right_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                right_camera->attach(osg::Camera::COLOR_BUFFER, texture.get());


                // create Keystone distortion camera
                osg::ref_ptr<osg::Camera> camera = assignKeystoneDistortionCamera(view, ds, gc.get(),
                                                                                0, 0, traits->width, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                texture, keystone.get());

                camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);
                
                // attach Keystone editing event handler.
                camera->addEventCallback(new KeystoneHandler(keystone.get()));
            }

            break;
        }
        case(osg::DisplaySettings::HORIZONTAL_SPLIT):
        {
            bool left_eye_left_viewport = ds->getSplitStereoHorizontalEyeMapping()==osg::DisplaySettings::LEFT_EYE_LEFT_VIEWPORT;
            int left_start = (left_eye_left_viewport) ? 0 : traits->width/2;
            int right_start = (left_eye_left_viewport) ? traits->width/2 : 0;

            // left viewport Camera
            osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(view, ds, gc,
                               left_start, 0, traits->width/2, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT,
                               1.0);

            // right viewport Camera
            osg::ref_ptr<osg::Camera> right_camera = assignStereoCamera(view, ds, gc,
                               right_start, 0, traits->width/2, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT,
                               1.0);

            if (keystone.valid())
            {
                // for keystone:
                // left camera to render to left texture using whole viewport of left texture
                // right camera to render to right texture using whole viewport of right texture
                // left keystone camera to render to left viewport/window
                // right keystone camera to render to right viewport/window
                // two keystone, one for each of the left and right viewports/windows
                
                keystone->setName("left");

                // create distortion texture
                osg::ref_ptr<osg::Texture> left_texture = createDistortionTexture(traits->width/2, traits->height);

                // convert to RTT Camera
                left_camera->setViewport(0, 0, traits->width/2, traits->height);
                left_camera->setDrawBuffer(GL_FRONT);
                left_camera->setReadBuffer(GL_FRONT);
                left_camera->setAllowEventFocus(true);
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
                right_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                right_camera->attach(osg::Camera::COLOR_BUFFER, right_texture.get());
                

                // create Keystone left distortion camera
                keystone->setGridColor(osg::Vec4(1.0f,0.0f,0.0,1.0));
                osg::ref_ptr<osg::Camera> left_keystone_camera = assignKeystoneDistortionCamera(view, ds, gc.get(),
                                                                                left_start, 0, traits->width/2, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                left_texture, keystone.get());

                left_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);

                // attach Keystone editing event handler.
                left_keystone_camera->addEventCallback(new KeystoneHandler(keystone.get()));


                osg::ref_ptr<Keystone> right_keystone = new Keystone;
                right_keystone->setGridColor(osg::Vec4(0.0f,1.0f,0.0,1.0));
                
                right_keystone->setName("right");

                // create Keystone right distortion camera
                osg::ref_ptr<osg::Camera> right_keystone_camera = assignKeystoneDistortionCamera(view, ds, gc.get(),
                                                                                right_start, 0, traits->width/2, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                right_texture, right_keystone.get());

                right_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 3);

                // attach Keystone editing event handler.
                right_keystone_camera->addEventCallback(new KeystoneHandler(right_keystone.get()));

                view->getCamera()->setAllowEventFocus(false);
                
            }
            
            break;
        }
        case(osg::DisplaySettings::VERTICAL_SPLIT):
        {
            bool left_eye_bottom_viewport = ds->getSplitStereoVerticalEyeMapping()==osg::DisplaySettings::LEFT_EYE_BOTTOM_VIEWPORT;
            int left_start = (left_eye_bottom_viewport) ? 0 : traits->height/2;
            int right_start = (left_eye_bottom_viewport) ? traits->height/2 : 0;
            
            // bottom viewport Camera
            osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(view, ds, gc,
                               0, left_start, traits->width, traits->height/2, traits->doubleBuffer ? GL_BACK : GL_FRONT,
                               1.0);

            // top vieport camera
            osg::ref_ptr<osg::Camera> right_camera = assignStereoCamera(view, ds, gc,
                               0, right_start, traits->width, traits->height/2, traits->doubleBuffer ? GL_BACK : GL_FRONT,
                               1.0);

            // for keystone:
            // left camera to render to left texture using whole viewport of left texture
            // right camera to render to right texture using whole viewport of right texture
            // left keystone camera to render to left viewport/window
            // right keystone camera to render to right viewport/window
            // two keystone, one for each of the left and right viewports/windows

            if (keystone.valid())
            {
                // for keystone:
                // left camera to render to left texture using whole viewport of left texture
                // right camera to render to right texture using whole viewport of right texture
                // left keystone camera to render to left viewport/window
                // right keystone camera to render to right viewport/window
                // two keystone, one for each of the left and right viewports/windows

                // create distortion texture
                osg::ref_ptr<osg::Texture> left_texture = createDistortionTexture(traits->width, traits->height/2);

                // convert to RTT Camera
                left_camera->setViewport(0, 0, traits->width, traits->height/2);
                left_camera->setDrawBuffer(GL_FRONT);
                left_camera->setReadBuffer(GL_FRONT);
                left_camera->setAllowEventFocus(true);
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
                right_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                right_camera->attach(osg::Camera::COLOR_BUFFER, right_texture.get());
                

                // create Keystone left distortion camera
                keystone->setGridColor(osg::Vec4(1.0f,0.0f,0.0,1.0));
                osg::ref_ptr<osg::Camera> left_keystone_camera = assignKeystoneDistortionCamera(view, ds, gc.get(),
                                                                                0, left_start, traits->width, traits->height/2,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                left_texture, keystone.get());

                left_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);

                // attach Keystone editing event handler.
                left_keystone_camera->addEventCallback(new KeystoneHandler(keystone.get()));


                osg::ref_ptr<Keystone> right_keystone = new Keystone;
                right_keystone->setGridColor(osg::Vec4(0.0f,1.0f,0.0,1.0));
                
                // create Keystone right distortion camera
                osg::ref_ptr<osg::Camera> right_keystone_camera = assignKeystoneDistortionCamera(view, ds, gc.get(),
                                                                                0, right_start, traits->width, traits->height/2,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                right_texture, right_keystone.get());

                right_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 3);

                // attach Keystone editing event handler.
                right_keystone_camera->addEventCallback(new KeystoneHandler(right_keystone.get()));

                view->getCamera()->setAllowEventFocus(false);
                
            }

            break;
        }
        case(osg::DisplaySettings::LEFT_EYE):
        {
            // single window, whole window, just left eye offsets
            osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, -1.0);

            // for keystone:
            // treat as standard keystone correction.
            // left eye camera to render to texture
            // keystone camera then render to window
            // one keystone and editing for window

            if (keystone.valid())
            {
                // for keystone:
                // left camera to render to texture using red colour mask
                // right camera to render to same texture using cyan colour mask
                // keystone camera to render to whole screen without colour masks
                // one keystone and editing for the one window

                // create distortion texture
                osg::ref_ptr<osg::Texture> texture = createDistortionTexture(traits->width, traits->height);

                // convert to RTT Camera
                left_camera->setDrawBuffer(GL_FRONT);
                left_camera->setReadBuffer(GL_FRONT);
                left_camera->setAllowEventFocus(false);
                left_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                left_camera->attach(osg::Camera::COLOR_BUFFER, texture.get());


                // create Keystone distortion camera
                osg::ref_ptr<osg::Camera> camera = assignKeystoneDistortionCamera(view, ds, gc.get(),
                                                                                0, 0, traits->width, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                texture, keystone.get());

                camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);
                
                // attach Keystone editing event handler.
                camera->addEventCallback(new KeystoneHandler(keystone.get()));
            }
            break;
        }
        case(osg::DisplaySettings::RIGHT_EYE):
        {
            // single window, whole window, just right eye offsets
            osg::ref_ptr<osg::Camera> right_camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, 1.0);

            // for keystone:
            // treat as standard keystone correction.
            // left eye camera to render to texture
            // keystone camera then render to window
            // one keystone and editing for window

            if (keystone.valid())
            {
                // for keystone:
                // left camera to render to texture using red colour mask
                // right camera to render to same texture using cyan colour mask
                // keystone camera to render to whole screen without colour masks
                // one keystone and editing for the one window

                // create distortion texture
                osg::ref_ptr<osg::Texture> texture = createDistortionTexture(traits->width, traits->height);

                // convert to RTT Camera
                right_camera->setDrawBuffer(GL_FRONT);
                right_camera->setReadBuffer(GL_FRONT);
                right_camera->setAllowEventFocus(false);
                right_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                right_camera->attach(osg::Camera::COLOR_BUFFER, texture.get());

                // create Keystone distortion camera
                osg::ref_ptr<osg::Camera> camera = assignKeystoneDistortionCamera(view, ds, gc.get(),
                                                                                0, 0, traits->width, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                texture, keystone.get());

                camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);
                
                // attach Keystone editing event handler.
                camera->addEventCallback(new KeystoneHandler(keystone.get()));
            }
            break;
        }
        case(osg::DisplaySettings::HORIZONTAL_INTERLACE):
        case(osg::DisplaySettings::VERTICAL_INTERLACE):
        case(osg::DisplaySettings::CHECKERBOARD):
        {
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
                view->addSlave(camera.get(), false);

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

            OSG_NOTICE<<"view->getNumSlaves()="<<view->getNumSlaves()<<std::endl;
            // left Camera
            {
                osg::ref_ptr<osg::Camera> camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, -1.0);
                camera->setClearMask(0);
                camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
                camera->setRenderOrder(osg::Camera::NESTED_RENDER, 1);

                osg::ref_ptr<osg::Stencil> stencil = new osg::Stencil;
                stencil->setFunction(osg::Stencil::EQUAL, 0, ~0u);
                stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
                camera->getOrCreateStateSet()->setAttributeAndModes(stencil.get(), osg::StateAttribute::ON);
            }

            // right Camera
            {
                osg::ref_ptr<osg::Camera> camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, 1.0);
                camera->setClearMask(GL_DEPTH_BUFFER_BIT);
                camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);

                osg::ref_ptr<osg::Stencil> stencil = new osg::Stencil;
                stencil->setFunction(osg::Stencil::NOTEQUAL, 0, ~0u);
                stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
                camera->getOrCreateStateSet()->setAttributeAndModes(stencil.get(), osg::StateAttribute::ON);
            }
            break;
        }
    }
}


void setUpViewForKeystone(osgViewer::View* view, Keystone* keystone)
{
    int screenNum = 0;
    
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        OSG_NOTICE<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    osg::GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();

    // displayNum has not been set so reset it to 0.
    if (si.displayNum<0) si.displayNum = 0;

    si.screenNum = screenNum;

    unsigned int width, height;
    wsi->getScreenResolution(si, width, height);

//    width/=2; height/=2;

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
        OSG_NOTICE<<"GraphicsWindow has not been created successfully."<<std::endl;
        return;
    }

    osg::DisplaySettings* ds = osg::DisplaySettings::instance();


    // create distortion texture
    osg::ref_ptr<osg::Texture> texture = createDistortionTexture(width, height);

    // create RTT Camera
    assignRenderToTextureCamera(view, gc.get(), width, height, texture);

    // create Keystone distortion camera
    osg::ref_ptr<osg::Camera> camera = assignKeystoneDistortionCamera(view, ds, gc.get(),
                                                                      0, 0, width, height,
                                                                      traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                      texture, keystone);
    // attach Keystone editing event handler.
    camera->addEventCallback(new KeystoneHandler(keystone));
    
}

int main( int argc, char **argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    // initialize the viewer.
    osgViewer::Viewer viewer(arguments);

    osg::DisplaySettings* ds = viewer.getDisplaySettings() ? viewer.getDisplaySettings() : osg::DisplaySettings::instance().get();
    ds->readCommandLine(arguments);

    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);

    if (!model)
    {
        OSG_NOTICE<<"No models loaded, please specify a model file on the command line"<<std::endl;
        return 1;
    }


    OSG_NOTICE<<"Stereo "<<ds->getStereo()<<std::endl;
    OSG_NOTICE<<"StereoMode "<<ds->getStereoMode()<<std::endl;

    viewer.setSceneData(model.get());
    
    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    // add camera manipulator
    viewer.setCameraManipulator(new osgGA::TrackballManipulator());

    if (ds->getStereo())
    {
        setUpViewForStereo(&viewer, ds);
    }
    else
    {
        setUpViewForKeystone(&viewer, new Keystone);
    }
    
    viewer.realize();

    while(!viewer.done())
    {
        viewer.advance();
        viewer.eventTraversal();
        viewer.updateTraversal();
        viewer.renderingTraversals();
    }
    return 0;
}
