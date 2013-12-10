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

#include <osgVolume/MultipassTechnique>
#include <osgVolume/VolumeTile>
#include <osgVolume/VolumeScene>

#include <osg/Geometry>
#include <osg/ValueObject>
#include <osg/io_utils>

#include <osg/Program>
#include <osg/Material>
#include <osg/CullFace>
#include <osg/TexGen>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/TransferFunction>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

namespace osgVolume
{


MultipassTechnique::MultipassTechnique()
{
}

MultipassTechnique::MultipassTechnique(const MultipassTechnique& fft,const osg::CopyOp& copyop):
    VolumeTechnique(fft,copyop)
{
}

MultipassTechnique::~MultipassTechnique()
{
}

void MultipassTechnique::init()
{
    OSG_INFO<<"MultipassTechnique::init()"<<std::endl;

    if (!_volumeTile)
    {
        OSG_NOTICE<<"MultipassTechnique::init(), error no volume tile assigned."<<std::endl;
        return;
    }

    if (_volumeTile->getLayer()==0)
    {
        OSG_NOTICE<<"MultipassTechnique::init(), error no layer assigend to volume tile."<<std::endl;
        return;
    }

    if (_volumeTile->getLayer()->getImage()==0)
    {
        OSG_NOTICE<<"MultipassTechnique::init(), error no image assigned to layer."<<std::endl;
        return;
    }

    OSG_NOTICE<<"MultipassTechnique::init() Need to set up"<<std::endl;

    CollectPropertiesVisitor cpv;
    if (_volumeTile->getLayer()->getProperty())
    {
        _volumeTile->getLayer()->getProperty()->accept(cpv);
    }

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    {
        osg::Geometry* geom = new osg::Geometry;

        osg::Vec3Array* coords = new osg::Vec3Array(8);
        (*coords)[0] = osg::Vec3d(0.0,0.0,0.0);
        (*coords)[1] = osg::Vec3d(1.0,0.0,0.0);
        (*coords)[2] = osg::Vec3d(1.0,1.0,0.0);
        (*coords)[3] = osg::Vec3d(0.0,1.0,0.0);
        (*coords)[4] = osg::Vec3d(0.0,0.0,1.0);
        (*coords)[5] = osg::Vec3d(1.0,0.0,1.0);
        (*coords)[6] = osg::Vec3d(1.0,1.0,1.0);
        (*coords)[7] = osg::Vec3d(0.0,1.0,1.0);
        geom->setVertexArray(coords);

        osg::Vec4Array* colours = new osg::Vec4Array(1);
        (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
        geom->setColorArray(colours, osg::Array::BIND_OVERALL);

        osg::DrawElementsUShort* drawElements = new osg::DrawElementsUShort(GL_QUADS);
        // bottom
        drawElements->push_back(3);
        drawElements->push_back(2);
        drawElements->push_back(1);
        drawElements->push_back(0);

        // bottom
        drawElements->push_back(7);//7623
        drawElements->push_back(6);
        drawElements->push_back(2);
        drawElements->push_back(3);

        // left
        drawElements->push_back(4);//4730
        drawElements->push_back(7);
        drawElements->push_back(3);
        drawElements->push_back(0);

        // right
        drawElements->push_back(1);//1265
        drawElements->push_back(2);
        drawElements->push_back(6);
        drawElements->push_back(5);

        // front
        drawElements->push_back(5);//5401
        drawElements->push_back(4);
        drawElements->push_back(0);
        drawElements->push_back(1);

        // top
        drawElements->push_back(4);//4567
        drawElements->push_back(5);
        drawElements->push_back(6);
        drawElements->push_back(7);

        geom->addPrimitiveSet(drawElements);

        geode->addDrawable(geom);

    }

    _transform = new osg::MatrixTransform;

    // handle locators
    Locator* masterLocator = _volumeTile->getLocator();
    Locator* layerLocator = _volumeTile->getLayer()->getLocator();

    if (!masterLocator && layerLocator) masterLocator = layerLocator;
    if (!layerLocator && masterLocator) layerLocator = masterLocator;

    osg::Matrix geometryMatrix;
    if (masterLocator)
    {
        geometryMatrix = masterLocator->getTransform();
        _transform->setMatrix(geometryMatrix);
        masterLocator->addCallback(new TransformLocatorCallback(_transform.get()));
    }

    osg::Matrix imageMatrix;
    if (layerLocator)
    {
        imageMatrix = layerLocator->getTransform();
    }

    OSG_NOTICE<<"MultipassTechnique::init() : geometryMatrix = "<<geometryMatrix<<std::endl;
    OSG_NOTICE<<"MultipassTechnique::init() : imageMatrix = "<<imageMatrix<<std::endl;

    osg::ref_ptr<osg::StateSet> stateset = _transform->getOrCreateStateSet();

    unsigned int texgenTextureUnit = 0;
    unsigned int volumeTextureUnit = 2;

    // set up uniforms
    {
        stateset->addUniform(new osg::Uniform("colorTexture",0));
        stateset->addUniform(new osg::Uniform("depthTexture",1));

        stateset->setMode(GL_ALPHA_TEST,osg::StateAttribute::ON);

        float alphaFuncValue = 0.1;
        if (cpv._isoProperty.valid())
        {
            alphaFuncValue = cpv._isoProperty->getValue();
        }

        if (cpv._sampleRatioProperty.valid())
            stateset->addUniform(cpv._sampleRatioProperty->getUniform());
        else
            stateset->addUniform(new osg::Uniform("SampleRatioValue",1.0f));


        if (cpv._transparencyProperty.valid())
            stateset->addUniform(cpv._transparencyProperty->getUniform());
        else
            stateset->addUniform(new osg::Uniform("TransparencyValue",1.0f));


        if (cpv._afProperty.valid())
            stateset->addUniform(cpv._afProperty->getUniform());
        else
            stateset->addUniform(new osg::Uniform("AlphaFuncValue",alphaFuncValue));

#if 1
        osg::ref_ptr<osg::TexGen> texgen = new osg::TexGen;
        texgen->setMode(osg::TexGen::OBJECT_LINEAR);
        texgen->setPlanesFromMatrix( geometryMatrix * osg::Matrix::inverse(imageMatrix));

        if (masterLocator)
        {
            osg::ref_ptr<TexGenLocatorCallback> locatorCallback = new TexGenLocatorCallback(texgen, masterLocator, layerLocator);
            masterLocator->addCallback(locatorCallback.get());
            if (masterLocator != layerLocator)
            {
                if (layerLocator) layerLocator->addCallback(locatorCallback.get());
            }
        }

        stateset->setTextureAttributeAndModes(texgenTextureUnit, texgen.get(), osg::StateAttribute::ON);
#endif
    }


    // set up 3D texture
    osg::ref_ptr<osg::Image> image_3d = _volumeTile->getLayer()->getImage();
    osg::ref_ptr<osg::Texture3D> texture3D = new osg::Texture3D;
    {
        osg::Texture::InternalFormatMode internalFormatMode = osg::Texture::USE_IMAGE_DATA_FORMAT;
#if 1
        osg::Texture::FilterMode minFilter = osg::Texture::LINEAR;
        osg::Texture::FilterMode magFilter = osg::Texture::LINEAR;
#else
        osg::Texture::FilterMode minFilter = osg::Texture::NEAREST;
        osg::Texture::FilterMode magFilter = osg::Texture::NEAREST;
#endif

        // set up the 3d texture itself,
        // note, well set the filtering up so that mip mapping is disabled,
        // gluBuild3DMipsmaps doesn't do a very good job of handled the
        // imbalanced dimensions of the 256x256x4 texture.
        texture3D->setResizeNonPowerOfTwoHint(false);
        texture3D->setFilter(osg::Texture3D::MIN_FILTER,minFilter);
        texture3D->setFilter(osg::Texture3D::MAG_FILTER, magFilter);
        texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP_TO_BORDER);
        texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP_TO_BORDER);
        texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP_TO_BORDER);
        texture3D->setBorderColor(osg::Vec4(0.0,0.0,0.0,0.0));
        if (image_3d->getPixelFormat()==GL_ALPHA ||
            image_3d->getPixelFormat()==GL_LUMINANCE)
        {
            texture3D->setInternalFormatMode(osg::Texture3D::USE_USER_DEFINED_FORMAT);
            texture3D->setInternalFormat(GL_INTENSITY);
        }
        else
        {
            texture3D->setInternalFormatMode(internalFormatMode);
        }
        texture3D->setImage(image_3d);

        stateset->setTextureAttributeAndModes(volumeTextureUnit, texture3D, osg::StateAttribute::ON);

        osg::ref_ptr<osg::Uniform> baseTextureSampler = new osg::Uniform("volumeTexture", int(volumeTextureUnit));
        stateset->addUniform(baseTextureSampler.get());

        osg::ref_ptr<osg::Uniform> volumeCellSize = new osg::Uniform("volumeCellSize", osg::Vec3(1.0f/static_cast<float>(image_3d->s()),1.0f/static_cast<float>(image_3d->t()),1.0f/static_cast<float>(image_3d->r())));
        stateset->addUniform(volumeCellSize.get());

        OSG_NOTICE<<"Texture Dimensions "<<image_3d->s()<<", "<<image_3d->t()<<", "<<image_3d->r()<<std::endl;
    }



    osg::ref_ptr<osg::Shader> computeRayColorShader = osgDB::readRefShaderFile(osg::Shader::FRAGMENT, "shaders/volume_compute_ray_color.frag");
#if 0
    if (!computeRayColorShader)
    {
        #include "Shaders/volume_compute_ray_color_frag.cpp";
        computeRayColorShader = new osg::Shader(osg::Shader::FRAGMENT, volume_compute_ray_color_frag);
    }
#endif

    // set up the renderin of the front faces
    {
        osg::ref_ptr<osg::Group> front_face_group = new osg::Group;
        front_face_group->addChild(geode.get());
        _transform->addChild(front_face_group.get());

        osg::ref_ptr<osg::StateSet> front_face_stateset = front_face_group->getOrCreateStateSet();
        front_face_stateset->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK), osg::StateAttribute::ON);

        osg::ref_ptr<osg::Program> program = new osg::Program;
        front_face_stateset->setAttribute(program);

        // get vertex shaders from source
        osg::ref_ptr<osg::Shader> vertexShader = osgDB::readRefShaderFile(osg::Shader::VERTEX, "shaders/volume_multipass_front.vert");
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
        osg::ref_ptr<osg::Shader> fragmentShader = osgDB::readRefShaderFile(osg::Shader::FRAGMENT, "shaders/volume_multipass_front.frag");
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

        if (computeRayColorShader.valid())
        {
            program->addShader(computeRayColorShader.get());
        }
    }


    // set up the rendering of the back faces
    {
        osg::ref_ptr<osg::Group> back_face_group = new osg::Group;
        back_face_group->addChild(geode.get());
        _transform->addChild(back_face_group.get());

        osg::ref_ptr<osg::StateSet> back_face_stateset = back_face_group->getOrCreateStateSet();
        back_face_stateset->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT), osg::StateAttribute::ON);

        osg::ref_ptr<osg::Program> program = new osg::Program;
        back_face_stateset->setAttribute(program);

        // get vertex shaders from source
        osg::ref_ptr<osg::Shader> vertexShader = osgDB::readRefShaderFile(osg::Shader::VERTEX, "shaders/volume_multipass_back.vert");
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
        osg::ref_ptr<osg::Shader> fragmentShader = osgDB::readRefShaderFile(osg::Shader::FRAGMENT, "shaders/volume_multipass_back.frag");
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
        if (computeRayColorShader.valid())
        {
            program->addShader(computeRayColorShader.get());
        }

    }


}

void MultipassTechnique::update(osgUtil::UpdateVisitor* /*uv*/)
{
//    OSG_NOTICE<<"MultipassTechnique:update(osgUtil::UpdateVisitor* nv):"<<std::endl;
}

void MultipassTechnique::cull(osgUtil::CullVisitor* cv)
{
    std::string traversalPass;
    bool postTraversal = cv->getUserValue("VolumeSceneTraversal", traversalPass) && traversalPass=="Post";

    // OSG_NOTICE<<"MultipassTechnique::cull()  traversalPass="<<traversalPass<<std::endl;

    if (postTraversal)
    {
        // OSG_NOTICE<<"  OK need to handle postTraversal"<<std::endl;
        _transform->accept(*cv);
    }
    else
    {
        osg::NodePath& nodePath = cv->getNodePath();
        for(osg::NodePath::reverse_iterator itr = nodePath.rbegin();
            itr != nodePath.rend();
            ++itr)
        {
            osgVolume::VolumeScene* vs = dynamic_cast<osgVolume::VolumeScene*>(*itr);
            if (vs)
            {
                vs->tileVisited(cv, getVolumeTile());
                break;
            }
        }
    }
}

void MultipassTechnique::cleanSceneGraph()
{
    OSG_NOTICE<<"MultipassTechnique::cleanSceneGraph()"<<std::endl;
}

void MultipassTechnique::traverse(osg::NodeVisitor& nv)
{
    // OSG_NOTICE<<"MultipassTechnique::traverse(osg::NodeVisitor& nv)"<<std::endl;
    if (!_volumeTile) return;

    // if app traversal update the frame count.
    if (nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
    {
        if (_volumeTile->getDirty()) _volumeTile->init();

        osgUtil::UpdateVisitor* uv = dynamic_cast<osgUtil::UpdateVisitor*>(&nv);
        if (uv)
        {
            update(uv);
            return;
        }

    }
    else if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
        if (cv)
        {
            cull(cv);
            return;
        }
    }


    if (_volumeTile->getDirty())
    {
        OSG_INFO<<"******* Doing init ***********"<<std::endl;
        _volumeTile->init();
    }
}


} // end of osgVolume namespace
