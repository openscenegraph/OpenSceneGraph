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

#include <osgViewer/config/WoWVxDisplay>
#include <osgViewer/Renderer>
#include <osgViewer/View>
#include <osgViewer/GraphicsWindow>

#include <osg/io_utils>

#include <osg/TextureRectangle>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/TexMat>
#include <osg/Stencil>
#include <osg/PolygonStipple>
#include <osg/ValueObject>

using namespace osgViewer;

void WoWVxDisplay::configure(osgViewer::View& view) const
{
    OSG_INFO<<"WoWVxDisplay::configure(...)"<<std::endl;

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

    si.screenNum = _screenNum;

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
        OSG_NOTICE<<"GraphicsWindow has not been created successfully."<<std::endl;
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

        view.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd());
    }

    // WoW display set up.
    {
        osg::Texture1D *textureHeader = new osg::Texture1D();
        // Set up the header
        {
            unsigned char header[]= {0xF1,_wow_content,_wow_factor,_wow_offset,0x00,0x00,0x00,0x00,0x00,0x00};
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
            stateset->addUniform( new osg::Uniform("wow_disparity_M", _wow_disparity_M));
            stateset->addUniform( new osg::Uniform("wow_disparity_Zd", _wow_disparity_Zd));
            stateset->addUniform( new osg::Uniform("wow_disparity_vz", _wow_disparity_vz));
            stateset->addUniform( new osg::Uniform("wow_disparity_C", _wow_disparity_C));

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
                    "             gl_FragColor.b = 0.0;                                                                                 " \
                    "         else                                                                                                      " \
                    "             gl_FragColor.b = 1.0;                                                                                 " \
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

            view.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd(), false);
        }
    }
}

#if 0

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


void View::StereoSlaveCallback::updateSlave(osg::View& view, osg::View::Slave& slave)
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


void View::setUpViewForStereo()
{
    osg::DisplaySettings* ds = _displaySettings.valid() ? _displaySettings.get() : osg::DisplaySettings::instance().get();    
    if (!ds->getStereo()) return;

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
        double height = osg::DisplaySettings::instance()->getScreenHeight();
        double width = osg::DisplaySettings::instance()->getScreenWidth();
        double distance = osg::DisplaySettings::instance()->getScreenDistance();
        double vfov = osg::RadiansToDegrees(atan2(height/2.0f,distance)*2.0);

        getCamera()->setProjectionMatrixAsPerspective( vfov, width/height, 1.0f,10000.0f);
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
            osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(ds, gc.get(), 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK_LEFT : GL_FRONT_LEFT, -1.0);
            left_camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            left_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);

            // right Camera right buffer
            osg::ref_ptr<osg::Camera> right_camera = assignStereoCamera(ds, gc.get(), 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK_RIGHT : GL_FRONT_RIGHT, 1.0);
            right_camera->setClearMask(GL_DEPTH_BUFFER_BIT);
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

                // create distortion texture
                osg::ref_ptr<osg::Texture> texture = createDistortionTexture(traits->width, traits->height);

                // convert to RTT Camera
                left_camera->setDrawBuffer(GL_FRONT);
                left_camera->setReadBuffer(GL_FRONT);
                left_camera->setAllowEventFocus(false);
                right_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);
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
                osg::ref_ptr<osg::Camera> camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                                0, 0, traits->width, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                texture.get(), keystone.get());

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
            osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(ds, gc.get(),
                               left_start, 0, traits->width/2, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT,
                               1.0);

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

                getCamera()->setAllowEventFocus(false);
                
            }
            
            break;
        }
        case(osg::DisplaySettings::VERTICAL_SPLIT):
        {
            bool left_eye_bottom_viewport = ds->getSplitStereoVerticalEyeMapping()==osg::DisplaySettings::LEFT_EYE_BOTTOM_VIEWPORT;
            int left_start = (left_eye_bottom_viewport) ? 0 : traits->height/2;
            int right_start = (left_eye_bottom_viewport) ? traits->height/2 : 0;
            
            // bottom viewport Camera
            osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(ds, gc.get(),
                               0, left_start, traits->width, traits->height/2, traits->doubleBuffer ? GL_BACK : GL_FRONT,
                               1.0);

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

                getCamera()->setAllowEventFocus(false);
                
            }

            break;
        }
        case(osg::DisplaySettings::LEFT_EYE):
        {
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
                osg::ref_ptr<osg::Camera> camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                                0, 0, traits->width, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                texture.get(), keystone.get());

                camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);
                
                // attach Keystone editing event handler.
                camera->addEventCallback(new KeystoneHandler(keystone.get()));
            }
            break;
        }
        case(osg::DisplaySettings::RIGHT_EYE):
        {
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
                osg::ref_ptr<osg::Camera> camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                                0, 0, traits->width, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                texture.get(), keystone.get());

                camera->setRenderOrder(osg::Camera::NESTED_RENDER, 1);
                
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
                osg::ref_ptr<osg::Camera> camera = assignStereoCamera(ds, gc.get(), 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, -1.0);
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
                osg::ref_ptr<osg::Camera> camera = assignStereoCamera(ds, gc.get(), 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, 1.0);
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


void View::setUpViewForKeystone(Keystone* keystone)
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

    osg::DisplaySettings* ds = _displaySettings.valid() ? _displaySettings.get() : osg::DisplaySettings::instance().get();    

    // create distortion texture
    osg::ref_ptr<osg::Texture> texture = createDistortionTexture(width, height);

    // create RTT Camera
    assignRenderToTextureCamera(gc.get(), width, height, texture.get());

    // create Keystone distortion camera
    osg::ref_ptr<osg::Camera> camera = assignKeystoneDistortionCamera(ds, gc.get(),
                                                                      0, 0, width, height,
                                                                      traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                      texture.get(), keystone);
    // attach Keystone editing event handler.
    camera->addEventCallback(new KeystoneHandler(keystone));
    
}

#endif