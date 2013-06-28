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

#include <osgVolume/RayTracedTechnique>
#include <osgVolume/VolumeTile>

#include <osg/Geometry>
#include <osg/io_utils>

#include <osg/Program>
#include <osg/TexGen>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/TransferFunction>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

namespace osgVolume
{

class TransformLocatorCallback : public Locator::LocatorCallback
{
    public:

        TransformLocatorCallback(osg::MatrixTransform* transform): _transform(transform) {}

        void locatorModified(Locator* locator)
        {
            if (_transform.valid()) _transform->setMatrix(locator->getTransform());
        }

    protected:

        osg::observer_ptr<osg::MatrixTransform> _transform;
};


class TexGenLocatorCallback : public Locator::LocatorCallback
{
    public:

        TexGenLocatorCallback(osg::TexGen* texgen, Locator* geometryLocator, Locator* imageLocator):
            _texgen(texgen),
            _geometryLocator(geometryLocator),
            _imageLocator(imageLocator) {}

        void locatorModified(Locator*)
        {
            if (!_texgen || !_geometryLocator || !_imageLocator) return;

            _texgen->setPlanesFromMatrix(
                _geometryLocator->getTransform() *
                osg::Matrix::inverse(_imageLocator->getTransform()));
        }

    protected:

        osg::observer_ptr<osg::TexGen> _texgen;
        osg::observer_ptr<osgVolume::Locator> _geometryLocator;
        osg::observer_ptr<osgVolume::Locator> _imageLocator;
};


RayTracedTechnique::RayTracedTechnique()
{
}

RayTracedTechnique::RayTracedTechnique(const RayTracedTechnique& fft,const osg::CopyOp& copyop):
    VolumeTechnique(fft,copyop)
{
}

RayTracedTechnique::~RayTracedTechnique()
{
}

enum ShadingModel
{
    Standard,
    Light,
    Isosurface,
    MaximumIntensityProjection
};

void RayTracedTechnique::init()
{
    OSG_INFO<<"RayTracedTechnique::init()"<<std::endl;

    if (!_volumeTile)
    {
        OSG_NOTICE<<"RayTracedTechnique::init(), error no volume tile assigned."<<std::endl;
        return;
    }

    if (_volumeTile->getLayer()==0)
    {
        OSG_NOTICE<<"RayTracedTechnique::init(), error no layer assigend to volume tile."<<std::endl;
        return;
    }

    if (_volumeTile->getLayer()->getImage()==0)
    {
        OSG_NOTICE<<"RayTracedTechnique::init(), error no image assigned to layer."<<std::endl;
        return;
    }

     ShadingModel shadingModel = Isosurface;
     float alphaFuncValue = 0.1;

    _transform = new osg::MatrixTransform;

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    _transform->addChild(geode.get());

    osg::Image* image_3d = 0;
    osg::TransferFunction1D* tf = 0;
    Locator* masterLocator = _volumeTile->getLocator();
    Locator* layerLocator = _volumeTile->getLayer()->getLocator();

    image_3d = _volumeTile->getLayer()->getImage();


    CollectPropertiesVisitor cpv;
    if (_volumeTile->getLayer()->getProperty())
    {
        _volumeTile->getLayer()->getProperty()->accept(cpv);
    }

    if (cpv._isoProperty.valid())
    {
        shadingModel = Isosurface;
        alphaFuncValue = cpv._isoProperty->getValue();
    }
    else if (cpv._mipProperty.valid())
    {
        shadingModel = MaximumIntensityProjection;
    }
    else if (cpv._lightingProperty.valid())
    {
        shadingModel = Light;
    }
    else
    {
        shadingModel = Standard;
    }

    if (cpv._tfProperty.valid())
    {
        tf = dynamic_cast<osg::TransferFunction1D*>(cpv._tfProperty->getTransferFunction());
    }

    if (cpv._afProperty.valid())
    {
        alphaFuncValue = cpv._afProperty->getValue();
    }


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

    OSG_INFO<<"RayTracedTechnique::init() : geometryMatrix = "<<geometryMatrix<<std::endl;
    OSG_INFO<<"RayTracedTechnique::init() : imageMatrix = "<<imageMatrix<<std::endl;

    osg::Texture::InternalFormatMode internalFormatMode = osg::Texture::USE_IMAGE_DATA_FORMAT;

    {

        osg::Texture::FilterMode minFilter = osg::Texture::LINEAR;
        osg::Texture::FilterMode magFilter = osg::Texture::LINEAR;

        osg::StateSet* stateset = geode->getOrCreateStateSet();

        stateset->setMode(GL_ALPHA_TEST,osg::StateAttribute::ON);

        osg::Program* program = new osg::Program;
        stateset->setAttribute(program);

        // get shaders from source

        osg::Shader* vertexShader = osgDB::readShaderFile(osg::Shader::VERTEX, "shaders/volume.vert");
        if (vertexShader)
        {
            program->addShader(vertexShader);
        }
        else
        {
            #include "Shaders/volume_vert.cpp"
            program->addShader(new osg::Shader(osg::Shader::VERTEX, volume_vert));
        }

        {
            // set up the 3d texture itself,
            // note, well set the filtering up so that mip mapping is disabled,
            // gluBuild3DMipsmaps doesn't do a very good job of handled the
            // imbalanced dimensions of the 256x256x4 texture.
            osg::Texture3D* texture3D = new osg::Texture3D;
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

            stateset->setTextureAttributeAndModes(0,texture3D,osg::StateAttribute::ON);

            osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
            stateset->addUniform(baseTextureSampler);
        }


        bool enableBlending = false;

        if (tf)
        {
            float tfScale = 1.0f;
            float tfOffset = 0.0f;

            ImageLayer* imageLayer = dynamic_cast<ImageLayer*>(_volumeTile->getLayer());
            if (imageLayer)
            {
                tfOffset = (imageLayer->getTexelOffset()[3] - tf->getMinimum()) / (tf->getMaximum() - tf->getMinimum());
                tfScale = imageLayer->getTexelScale()[3] / (tf->getMaximum() - tf->getMinimum());
            }
            else
            {
                tfOffset = -tf->getMinimum() / (tf->getMaximum()-tf->getMinimum());
                tfScale = 1.0f / (tf->getMaximum()-tf->getMinimum());
            }
            osg::ref_ptr<osg::Texture1D> tf_texture = new osg::Texture1D;
            tf_texture->setImage(tf->getImage());

            tf_texture->setResizeNonPowerOfTwoHint(false);
            tf_texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
            tf_texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
            tf_texture->setWrap(osg::Texture::WRAP_R,osg::Texture::CLAMP_TO_EDGE);

            stateset->setTextureAttributeAndModes(1, tf_texture.get(), osg::StateAttribute::ON);
            stateset->addUniform(new osg::Uniform("tfTexture",1));
            stateset->addUniform(new osg::Uniform("tfOffset",tfOffset));
            stateset->addUniform(new osg::Uniform("tfScale",tfScale));

        }

        if (shadingModel==MaximumIntensityProjection)
        {
            enableBlending = true;

            if (tf)
            {
                osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "shaders/volume_tf_mip.frag");
                if (fragmentShader)
                {
                    program->addShader(fragmentShader);
                }
                else
                {
                    #include "Shaders/volume_tf_mip_frag.cpp"
                    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_tf_mip_frag));
                }

                osg::Uniform* tfTextureSampler = new osg::Uniform("tfTexture",1);
                stateset->addUniform(tfTextureSampler);

            }
            else
            {
                osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "shaders/volume_mip.frag");
                if (fragmentShader)
                {
                    program->addShader(fragmentShader);
                }
                else
                {
                    #include "Shaders/volume_mip_frag.cpp"
                    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_mip_frag));
                }
            }
        }
        else if (shadingModel==Isosurface)
        {

            enableBlending = true;

            stateset->addUniform(cpv._isoProperty->getUniform());

            if (tf)
            {
                osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "shaders/volume_tf_iso.frag");
                if (fragmentShader)
                {
                    program->addShader(fragmentShader);
                }
                else
                {
                    #include "Shaders/volume_tf_iso_frag.cpp"
                    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_tf_iso_frag));
                }
            }
            else
            {
                osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "shaders/volume_iso.frag");
                if (fragmentShader)
                {
                    OSG_INFO<<"Shader found"<<std::endl;

                    program->addShader(fragmentShader);
                }
                else
                {
                    OSG_INFO<<"No Shader found"<<std::endl;

                    #include "Shaders/volume_iso_frag.cpp"
                    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_iso_frag));
                }
            }
        }
        else if (shadingModel==Light)
        {
            enableBlending = true;

            if (tf)
            {
                osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "shaders/volume_lit_tf.frag");
                if (fragmentShader)
                {
                    program->addShader(fragmentShader);
                }
                else
                {
                    #include "Shaders/volume_lit_tf_frag.cpp"
                    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_lit_tf_frag));
                }

            }
            else
            {
                osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "shaders/volume_lit.frag");
                if (fragmentShader)
                {
                    program->addShader(fragmentShader);
                }
                else
                {
                    #include "Shaders/volume_lit_frag.cpp"
                    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_lit_frag));
                }
            }
        }
        else
        {
            enableBlending = true;

            if (tf)
            {
                osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "shaders/volume_tf.frag");
                if (fragmentShader)
                {
                    program->addShader(fragmentShader);
                }
                else
                {
                    #include "Shaders/volume_tf_frag.cpp"
                    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_tf_frag));
                }

            }
            else
            {
                osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "shaders/volume.frag");
                if (fragmentShader)
                {
                    program->addShader(fragmentShader);
                }
                else
                {
                    #include "Shaders/volume_frag.cpp"
                    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_frag));
                }
            }
        }

        if (cpv._sampleDensityProperty.valid())
            stateset->addUniform(cpv._sampleDensityProperty->getUniform());
        else
            stateset->addUniform(new osg::Uniform("SampleDensityValue",0.0005f));


        if (cpv._transparencyProperty.valid())
            stateset->addUniform(cpv._transparencyProperty->getUniform());
        else
            stateset->addUniform(new osg::Uniform("TransparencyValue",1.0f));


        if (cpv._afProperty.valid())
            stateset->addUniform(cpv._afProperty->getUniform());
        else
            stateset->addUniform(new osg::Uniform("AlphaFuncValue",alphaFuncValue));


        if (enableBlending)
        {
            stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
            stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        }


        stateset->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

        osg::TexGen* texgen = new osg::TexGen;
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

        stateset->setTextureAttributeAndModes(0, texgen, osg::StateAttribute::ON);

    }

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
        drawElements->push_back(0);
        drawElements->push_back(1);
        drawElements->push_back(2);
        drawElements->push_back(3);

        // bottom
        drawElements->push_back(3);
        drawElements->push_back(2);
        drawElements->push_back(6);
        drawElements->push_back(7);

        // left
        drawElements->push_back(0);
        drawElements->push_back(3);
        drawElements->push_back(7);
        drawElements->push_back(4);

        // right
        drawElements->push_back(5);
        drawElements->push_back(6);
        drawElements->push_back(2);
        drawElements->push_back(1);

        // front
        drawElements->push_back(1);
        drawElements->push_back(0);
        drawElements->push_back(4);
        drawElements->push_back(5);

        // top
        drawElements->push_back(7);
        drawElements->push_back(6);
        drawElements->push_back(5);
        drawElements->push_back(4);

        geom->addPrimitiveSet(drawElements);

        geode->addDrawable(geom);

    }

    if (cpv._sampleDensityWhenMovingProperty.valid())
    {
        _whenMovingStateSet = new osg::StateSet;
        _whenMovingStateSet->addUniform(cpv._sampleDensityWhenMovingProperty->getUniform(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
    }
}

void RayTracedTechnique::update(osgUtil::UpdateVisitor* /*uv*/)
{
//    OSG_NOTICE<<"RayTracedTechnique:update(osgUtil::UpdateVisitor* nv):"<<std::endl;
}

void RayTracedTechnique::cull(osgUtil::CullVisitor* cv)
{
    if (!_transform.valid()) return;

    if (_whenMovingStateSet.valid())
    {
        bool moving = false;
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
            ModelViewMatrixMap::iterator itr = _modelViewMatrixMap.find(cv->getIdentifier());
            if (itr!=_modelViewMatrixMap.end())
            {
                osg::Matrix newModelViewMatrix = *(cv->getModelViewMatrix());
                osg::Matrix& previousModelViewMatrix = itr->second;
                moving = (newModelViewMatrix != previousModelViewMatrix);

                previousModelViewMatrix = newModelViewMatrix;
            }
            else
            {
                _modelViewMatrixMap[cv->getIdentifier()] = *(cv->getModelViewMatrix());
            }
        }

        if (moving)
        {
            cv->pushStateSet(_whenMovingStateSet.get());
            _transform->accept(*cv);
            cv->popStateSet();
        }
        else
        {
            _transform->accept(*cv);
        }
    }
    else
    {
        _transform->accept(*cv);
    }
}

void RayTracedTechnique::cleanSceneGraph()
{
    OSG_NOTICE<<"RayTracedTechnique::cleanSceneGraph()"<<std::endl;
}

void RayTracedTechnique::traverse(osg::NodeVisitor& nv)
{
    // OSG_NOTICE<<"RayTracedTechnique::traverse(osg::NodeVisitor& nv)"<<std::endl;
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
