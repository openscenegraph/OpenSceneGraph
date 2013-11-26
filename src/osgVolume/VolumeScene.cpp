/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield
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

#include <osgVolume/VolumeScene>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/io_utils>
#include <osgDB/ReadFile>
#include <OpenThreads/ScopedLock>

using namespace osgVolume;

class RTTCameraCullCallback : public osg::NodeCallback
{
    public:

        RTTCameraCullCallback(VolumeScene* vs):
            _volumeScene(vs) {}

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
#if 1
            osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
            if (cv)
            {
                osg::Camera* camera = cv->getCurrentCamera();
                OSG_NOTICE<<"Current camera="<<camera<<", node="<<node<<std::endl;
                OSG_NOTICE<<"  before near ="<<cv->getCalculatedNearPlane()<<std::endl;
                OSG_NOTICE<<"  before far ="<<cv->getCalculatedFarPlane()<<std::endl;
            }

            _volumeScene->osg::Group::traverse(*nv);
            if (cv)
            {
                OSG_NOTICE<<"  after near ="<<cv->getCalculatedNearPlane()<<std::endl;
                OSG_NOTICE<<"  after far ="<<cv->getCalculatedFarPlane()<<std::endl;
            }
#else
            osg::Group* group = dynamic_cast<osg::Group*>(node);
            if (group)
            {
                OSG_NOTICE<<"Traversing RTT subgraph, "<<group->className()<<", "<<nv->className()<<std::endl;
                group->osg::Group::traverse(*nv);
            }
            else
            {
                OSG_NOTICE<<"Can't traverse RTT subgraph"<<std::endl;
            }
#endif
        }

    protected:

        virtual ~RTTCameraCullCallback() {}

        osgVolume::VolumeScene* _volumeScene;
};


VolumeScene::ViewData::ViewData()
{
}


VolumeScene::VolumeScene()
{
}

VolumeScene::VolumeScene(const VolumeScene& vs, const osg::CopyOp& copyop):
    osg::Group(vs,copyop)
{
}


VolumeScene::~VolumeScene()
{
}

void VolumeScene::tileVisited(osg::NodeVisitor* nv, osgVolume::VolumeTile* tile)
{
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
    if (cv)
    {
        osg::ref_ptr<ViewData> viewData;

        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_viewDataMapMutex);
            viewData = _viewDataMap[cv];
        }

        if (viewData.valid())
        {
            OSG_NOTICE<<"tileVisited("<<tile<<")"<<std::endl;
            viewData->_tiles.push_back(nv->getNodePath());
        }
    }
}


void VolumeScene::traverse(osg::NodeVisitor& nv)
{
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
    if (cv)
    {
        osg::ref_ptr<ViewData> viewData;
        bool initializeViewData = false;
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_viewDataMapMutex);
            if (!_viewDataMap[cv])
            {
                _viewDataMap[cv] = new ViewData;
                initializeViewData = true;
            }

            viewData = _viewDataMap[cv];
        }

        if (initializeViewData)
        {
            OSG_NOTICE<<"Creating ViewData"<<std::endl;


            unsigned textureWidth = 512;
            unsigned textureHeight = 512;

            osg::Viewport* viewport = cv->getCurrentRenderStage()->getViewport();
            if (viewport)
            {
                textureWidth = viewport->width();
                textureHeight = viewport->height();
            }

            // set up depth texture
            viewData->_depthTexture = new osg::Texture2D;

            viewData->_depthTexture->setTextureSize(textureWidth, textureHeight);
            viewData->_depthTexture->setInternalFormat(GL_DEPTH_COMPONENT);
            viewData->_depthTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
            viewData->_depthTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

            viewData->_depthTexture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
            viewData->_depthTexture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
            viewData->_depthTexture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

            // set up color texture
            viewData->_colorTexture = new osg::Texture2D;

            viewData->_colorTexture->setTextureSize(textureWidth, textureHeight);
            viewData->_colorTexture->setInternalFormat(GL_RGBA);
            viewData->_colorTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
            viewData->_colorTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

            viewData->_colorTexture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_EDGE);
            viewData->_colorTexture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_EDGE);


            // set up the RTT Camera to capture the main scene to a color and depth texture that can be used in post processing
            viewData->_rttCamera = new osg::Camera;
            viewData->_rttCamera->attach(osg::Camera::DEPTH_BUFFER, viewData->_depthTexture.get());
            viewData->_rttCamera->attach(osg::Camera::COLOR_BUFFER, viewData->_colorTexture.get());
            viewData->_rttCamera->setCullCallback(new RTTCameraCullCallback(this));
            viewData->_rttCamera->setViewport(0,0,textureWidth,textureHeight);

            // clear the depth and colour bufferson each clear.
            viewData->_rttCamera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            // set the camera to render before the main camera.
            viewData->_rttCamera->setRenderOrder(osg::Camera::PRE_RENDER);

            // tell the camera to use OpenGL frame buffer object where supported.
            viewData->_rttCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);


            viewData->_rttCamera->setReferenceFrame(osg::Transform::RELATIVE_RF);
            viewData->_rttCamera->setProjectionMatrix(osg::Matrixd::identity());
            viewData->_rttCamera->setViewMatrix(osg::Matrixd::identity());

            // create mesh for rendering the RTT textures onto the screen
            osg::ref_ptr<osg::Geode> geode = new osg::Geode;
            geode->addDrawable(osg::createTexturedQuadGeometry(osg::Vec3(-1.0f,-1.0f,-1.0f),osg::Vec3(2.0f,0.0f,-1.0f),osg::Vec3(0.0f,2.0f,-1.0f)));

            osg::ref_ptr<osg::StateSet> stateset = geode->getOrCreateStateSet();

            stateset->setTextureAttributeAndModes(0, viewData->_colorTexture.get(), osg::StateAttribute::ON);
            stateset->setTextureAttributeAndModes(1, viewData->_depthTexture.get(), osg::StateAttribute::ON);
            stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
            stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

            osg::ref_ptr<osg::Program> program = new osg::Program;
            stateset->setAttribute(program);

            // get vertex shaders from source
            osg::ref_ptr<osg::Shader> vertexShader = osgDB::readRefShaderFile(osg::Shader::VERTEX, "shaders/volume_color_depth.vert");
            if (vertexShader.valid())
            {
                program->addShader(vertexShader.get());
            }
#if 0
            else
            {
                #include "Shaders/volume_color_depth_vert.cpp"
                program->addShader(new osg::Shader(osg::Shader::VERTEX, volume_color_depth_vert));
            }
#endif
            // get fragment shaders from source
            osg::ref_ptr<osg::Shader> fragmentShader = osgDB::readRefShaderFile(osg::Shader::FRAGMENT, "shaders/volume_color_depth.frag");
            if (fragmentShader.valid())
            {
                program->addShader(fragmentShader.get());
            }
#if 0
            else
            {
                #include "Shaders/volume_color_depth_frag.cpp"
                program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_color_depth_frag));
            }
#endif
            stateset->addUniform(new osg::Uniform("colorTexture",0));
            stateset->addUniform(new osg::Uniform("depthTexture",1));

            viewData->_postCamera = new osg::Camera;
            viewData->_postCamera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
            viewData->_postCamera->addChild(geode);

        }
        else
        {
            OSG_NOTICE<<"Reusing ViewData"<<std::endl;

        }

        // new frame so need to clear last frames log of VolumeTiles
        viewData->_tiles.clear();

        // record the parent render stage
        viewData->_parentRenderStage = cv->getCurrentRenderStage();

        osg::Viewport* viewport = cv->getCurrentRenderStage()->getViewport();
        if (viewport)
        {
            if (viewport->width() != viewData->_colorTexture->getTextureWidth() ||
                viewport->height() != viewData->_colorTexture->getTextureHeight())
            {
                OSG_NOTICE<<"Need to change texture size to "<<viewport->width()<<", "<< viewport->height()<<std::endl;
                viewData->_colorTexture->setTextureSize(viewport->width(), viewport->height());
                viewData->_colorTexture->dirtyTextureObject();
                viewData->_depthTexture->setTextureSize(viewport->width(), viewport->height());
                viewData->_depthTexture->dirtyTextureObject();
                viewData->_rttCamera->setViewport(0, 0, viewport->width(), viewport->height());
                viewData->_postCamera->setViewport(0, 0, viewport->width(), viewport->height());
                if (viewData->_rttCamera->getRenderingCache())
                {
                    viewData->_rttCamera->getRenderingCache()->releaseGLObjects(0);
                }
            }
        }

        osg::Camera* parentCamera = cv->getCurrentCamera();
        viewData->_rttCamera->setClearColor(parentCamera->getClearColor());

        OSG_NOTICE<<"Ready to traverse RTT Camera"<<std::endl;
        OSG_NOTICE<<"   RTT Camera ProjectionMatrix Before "<<viewData->_rttCamera->getProjectionMatrix()<<std::endl;
        viewData->_rttCamera->accept(nv);

        OSG_NOTICE<<"   RTT Camera ProjectionMatrix After "<<viewData->_rttCamera->getProjectionMatrix()<<std::endl;
        OSG_NOTICE<<"   cv ProjectionMatrix After "<<*(cv->getProjectionMatrix())<<std::endl;

        OSG_NOTICE<<"  after RTT near ="<<cv->getCalculatedNearPlane()<<std::endl;
        OSG_NOTICE<<"  after RTT far ="<<cv->getCalculatedFarPlane()<<std::endl;

        OSG_NOTICE<<"tileVisited()"<<viewData->_tiles.size()<<std::endl;


        OSG_NOTICE<<"Ready to traverse POST Camera"<<std::endl;
        viewData->_postCamera->accept(nv);
    }
    else
    {
        Group::traverse(nv);
    }
}
