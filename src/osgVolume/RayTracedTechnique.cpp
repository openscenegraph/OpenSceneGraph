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

using namespace osgVolume;

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
    osg::notify(osg::INFO)<<"RayTracedTechnique::init()"<<std::endl;

    if (!_volumeTile)
    {
        osg::notify(osg::NOTICE)<<"RayTracedTechnique::init(), error no volume tile assigned."<<std::endl;
        return;
    }

    if (_volumeTile->getLayer()==0)
    {
        osg::notify(osg::NOTICE)<<"RayTracedTechnique::init(), error no layer assigend to volume tile."<<std::endl;
        return;
    }

    if (_volumeTile->getLayer()->getImage()==0)
    {
        osg::notify(osg::NOTICE)<<"RayTracedTechnique::init(), error no image assigned to layer."<<std::endl;
        return;
    }

     ShadingModel shadingModel = Isosurface;
     float alphaFuncValue = 0.1;

    _geode = new osg::Geode;

    osg::Image* image_3d = 0;
    osg::TransferFunction1D* tf = 0;
    osgVolume::Locator* masterLocator = _volumeTile->getLocator();

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


    if (_volumeTile->getLayer() && !masterLocator)
    {
        masterLocator = _volumeTile->getLayer()->getLocator();
    }

    osg::Matrix matrix;
    if (masterLocator)
    {
        matrix = masterLocator->getTransform();
    }


    osg::notify(osg::INFO)<<"RayTracedTechnique::init() : matrix = "<<matrix<<std::endl;

    osg::Texture::InternalFormatMode internalFormatMode = osg::Texture::USE_IMAGE_DATA_FORMAT;

    {

        osg::Texture::FilterMode minFilter = osg::Texture::LINEAR;
        osg::Texture::FilterMode magFilter = osg::Texture::LINEAR;

        osg::StateSet* stateset = _geode->getOrCreateStateSet();

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
            texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP_TO_EDGE);
            texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP_TO_EDGE);
            texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP_TO_EDGE);
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

        if (shadingModel==MaximumIntensityProjection)
        {
            enableBlending = true;

            if (tf)
            {
                osg::Texture1D* texture1D = new osg::Texture1D;
                texture1D->setImage(tf->getImage());    
                stateset->setTextureAttributeAndModes(1,texture1D,osg::StateAttribute::ON);

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

            stateset->addUniform(cpv._isoProperty->getUniform());

            if (tf)
            {
                osg::Texture1D* texture1D = new osg::Texture1D;
                texture1D->setImage(tf->getImage());    
                texture1D->setResizeNonPowerOfTwoHint(false);
                texture1D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
                texture1D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
                texture1D->setWrap(osg::Texture::WRAP_R,osg::Texture::CLAMP_TO_EDGE);
                stateset->setTextureAttributeAndModes(1,texture1D,osg::StateAttribute::ON);

                osg::Uniform* tfTextureSampler = new osg::Uniform("tfTexture",1);
                stateset->addUniform(tfTextureSampler);

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
                    osg::notify(osg::NOTICE)<<"Shader found"<<std::endl;
                
                    program->addShader(fragmentShader);
                }
                else
                {
                    osg::notify(osg::NOTICE)<<"No Shader found"<<std::endl;

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
                osg::Texture1D* texture1D = new osg::Texture1D;
                texture1D->setImage(tf->getImage());    
                texture1D->setResizeNonPowerOfTwoHint(false);
                texture1D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
                texture1D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
                texture1D->setWrap(osg::Texture::WRAP_R,osg::Texture::CLAMP_TO_EDGE);
                stateset->setTextureAttributeAndModes(1,texture1D,osg::StateAttribute::ON);

                osg::Uniform* tfTextureSampler = new osg::Uniform("tfTexture",1);
                stateset->addUniform(tfTextureSampler);

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
                osg::notify(osg::INFO)<<"Setting up TF path"<<std::endl;

                osg::Texture1D* texture1D = new osg::Texture1D;
                texture1D->setImage(tf->getImage());    
                texture1D->setResizeNonPowerOfTwoHint(false);
                texture1D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
                texture1D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
                texture1D->setWrap(osg::Texture::WRAP_R,osg::Texture::CLAMP_TO_EDGE);
                stateset->setTextureAttributeAndModes(1,texture1D,osg::StateAttribute::ON);

                osg::Uniform* tfTextureSampler = new osg::Uniform("tfTexture",1);
                stateset->addUniform(tfTextureSampler);

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
        texgen->setPlanesFromMatrix(osg::Matrix::inverse(matrix));

        stateset->setTextureAttributeAndModes(0, texgen, osg::StateAttribute::ON);

    }

    {
        osg::Geometry* geom = new osg::Geometry;

        osg::Vec3Array* coords = new osg::Vec3Array(8);
        (*coords)[0] = osg::Vec3d(0.0,0.0,0.0) * matrix;
        (*coords)[1] = osg::Vec3d(1.0,0.0,0.0) * matrix;
        (*coords)[2] = osg::Vec3d(1.0,1.0,0.0) * matrix;
        (*coords)[3] = osg::Vec3d(0.0,1.0,0.0) * matrix;
        (*coords)[4] = osg::Vec3d(0.0,0.0,1.0) * matrix;
        (*coords)[5] = osg::Vec3d(1.0,0.0,1.0) * matrix;
        (*coords)[6] = osg::Vec3d(1.0,1.0,1.0) * matrix;
        (*coords)[7] = osg::Vec3d(0.0,1.0,1.0) * matrix;
        geom->setVertexArray(coords);

        osg::Vec4Array* colours = new osg::Vec4Array(1);
        (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
        geom->setColorArray(colours);
        geom->setColorBinding(osg::Geometry::BIND_OVERALL);

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

        _geode->addDrawable(geom);

    } 

}

void RayTracedTechnique::update(osgUtil::UpdateVisitor* uv)
{
//    osg::notify(osg::NOTICE)<<"RayTracedTechnique:update(osgUtil::UpdateVisitor* nv):"<<std::endl;
}

void RayTracedTechnique::cull(osgUtil::CullVisitor* cv)
{
    //osg::notify(osg::NOTICE)<<"RayTracedTechnique::cull(osgUtil::CullVisitor* nv)"<<std::endl;    
    if (_geode.valid())
    {
        _geode->accept(*cv);
    }
}

void RayTracedTechnique::cleanSceneGraph()
{
    osg::notify(osg::NOTICE)<<"RayTracedTechnique::cleanSceneGraph()"<<std::endl;
}

void RayTracedTechnique::traverse(osg::NodeVisitor& nv)
{
    // osg::notify(osg::NOTICE)<<"RayTracedTechnique::traverse(osg::NodeVisitor& nv)"<<std::endl;
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
        osg::notify(osg::INFO)<<"******* Doing init ***********"<<std::endl;
        _volumeTile->init();
    }
}
    
