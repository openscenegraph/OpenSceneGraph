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
            _volumeScene->osg::Group::traverse(*nv);
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



void VolumeScene::traverse(osg::NodeVisitor& nv)
{
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
    if (cv)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_viewDataMapMutex);
        osg::ref_ptr<ViewData>& viewData = _viewDataMap[cv];
        if (!viewData)
        {
            OSG_NOTICE<<"Creating ViewData"<<std::endl;


            unsigned textureWidth = 512;
            unsigned textureHeight = 512;

            viewData = new ViewData;

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

            viewData->_colorTexture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
            viewData->_colorTexture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
            viewData->_colorTexture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));


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

            viewData->_rttCamera->setClearColor(osg::Vec4(1.0,0.5,0.5,1.0));

            // tell the camera to use OpenGL frame buffer object where supported.
            viewData->_rttCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);


            viewData->_rttCamera->setReferenceFrame(osg::Transform::RELATIVE_RF);
            viewData->_rttCamera->setProjectionMatrix(osg::Matrixd::identity());
            viewData->_rttCamera->setViewMatrix(osg::Matrixd::identity());

            // create mesh for rendering the RTT textures onto the screen
            osg::ref_ptr<osg::Geode> geode = new osg::Geode;
            geode->addDrawable(osg::createTexturedQuadGeometry(osg::Vec3(-1.0f,-1.0f,-1.0f),osg::Vec3(2.0f,0.0f,-1.0f),osg::Vec3(0.0f,2.0f,-1.0f)));
            //geode->getOrCreateStateSet()->setTextureAttributeAndModes(0, viewData->_colorTexture.get(), osg::StateAttribute::ON);
            geode->getOrCreateStateSet()->setTextureAttributeAndModes(0, viewData->_depthTexture.get(), osg::StateAttribute::ON);
            geode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
            geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

            viewData->_postCamera = new osg::Camera;
            viewData->_postCamera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
            viewData->_postCamera->addChild(geode);

        }
        else
        {
            OSG_NOTICE<<"Reusing ViewData"<<std::endl;

        }

        viewData->_parentRenderStage = cv->getCurrentRenderStage();

        OSG_NOTICE<<"Ready to traverse RTT Camera"<<std::endl;
        viewData->_rttCamera->accept(nv);

        OSG_NOTICE<<"Ready to traverse POST Camera"<<std::endl;
        viewData->_postCamera->accept(nv);
    }
    else
    {
        Group::traverse(nv);
    }
}
