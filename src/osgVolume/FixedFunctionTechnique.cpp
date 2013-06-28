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

#include <osgVolume/FixedFunctionTechnique>
#include <osgVolume/VolumeTile>

#include <osg/Billboard>
#include <osg/Texture3D>
#include <osg/Geometry>
#include <osg/ClipNode>
#include <osg/Material>
#include <osg/TexGenNode>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/BlendEquation>
#include <osg/TexEnv>
#include <osg/io_utils>

using namespace osgVolume;

FixedFunctionTechnique::FixedFunctionTechnique():
    _numSlices(500)
{
}

FixedFunctionTechnique::FixedFunctionTechnique(const FixedFunctionTechnique& fft,const osg::CopyOp& copyop):
    VolumeTechnique(fft,copyop),
    _numSlices(fft._numSlices)
{
}

FixedFunctionTechnique::~FixedFunctionTechnique()
{
}

void FixedFunctionTechnique::setNumSlices(unsigned int numSlices)
{
    if (_numSlices==numSlices) return;

    _numSlices = numSlices;

    if (_volumeTile) _volumeTile->setDirty(true);
}

osg::Node* createCube(const osg::Vec3& center, float size, unsigned int numSlices)
{

    // set up the Geometry.
    osg::Geometry* geom = new osg::Geometry;

    float halfSize = size*0.5f;
    float y = halfSize;
    float dy =-size/(float)(numSlices-1);

    //y = -halfSize;
    //dy *= 0.5;

    osg::Vec3Array* coords = new osg::Vec3Array(4*numSlices);
    geom->setVertexArray(coords);
    for(unsigned int i=0;i<numSlices;++i, y+=dy)
    {
        (*coords)[i*4+0].set(-halfSize,y,halfSize);
        (*coords)[i*4+1].set(-halfSize,y,-halfSize);
        (*coords)[i*4+2].set(halfSize,y,-halfSize);
        (*coords)[i*4+3].set(halfSize,y,halfSize);
    }

    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0].set(0.0f,-1.0f,0.0f);
    geom->setNormalArray(normals, osg::Array::BIND_OVERALL);

    osg::Vec4Array* colors = new osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,1.0);
    geom->setColorArray(colors, osg::Array::BIND_OVERALL);

    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,coords->size()));

    osg::Billboard* billboard = new osg::Billboard;
    billboard->setMode(osg::Billboard::POINT_ROT_WORLD);
    billboard->addDrawable(geom);
    billboard->setPosition(0,center);

    return billboard;
}


void FixedFunctionTechnique::init()
{
    OSG_INFO<<"FixedFunctionTechnique::init()"<<std::endl;

    if (!_volumeTile)
    {
        OSG_NOTICE<<"FixedFunctionTechnique::init(), error no volume tile assigned."<<std::endl;
        return;
    }

    if (_volumeTile->getLayer()==0)
    {
        OSG_NOTICE<<"FixedFunctionTechnique::init(), error no layer assigend to volume tile."<<std::endl;
        return;
    }

    if (_volumeTile->getLayer()->getImage()==0)
    {
        OSG_NOTICE<<"FixedFunctionTechnique::init(), error no image assigned to layer."<<std::endl;
        return;
    }

    float alphaFuncValue = 0.1;

    osg::Image* image_3d = 0;
    osgVolume::Locator* masterLocator = _volumeTile->getLocator();
    osg::Texture::InternalFormatMode internalFormatMode = osg::Texture::USE_IMAGE_DATA_FORMAT;

    image_3d = _volumeTile->getLayer()->getImage();

    CollectPropertiesVisitor cpv;
    if (_volumeTile->getLayer()->getProperty())
    {
        _volumeTile->getLayer()->getProperty()->accept(cpv);
    }

    if (cpv._isoProperty.valid())
    {
        alphaFuncValue = cpv._isoProperty->getValue();
    }

    if (cpv._afProperty.valid())
    {
        alphaFuncValue = cpv._afProperty->getValue();
    }


    if (_volumeTile->getLayer() && !masterLocator)
    {
        masterLocator = _volumeTile->getLayer()->getLocator();
        OSG_NOTICE<<"assigning locator = "<<masterLocator<<std::endl;
    }

    osg::Matrix matrix;
    if (masterLocator)
    {
        matrix = masterLocator->getTransform();
    }

    OSG_NOTICE<<"Matrix = "<<matrix<<std::endl;

    osg::Texture::FilterMode minFilter = osg::Texture::NEAREST;
    osg::Texture::FilterMode magFilter = osg::Texture::NEAREST;

    osg::Vec3d v000 = osg::Vec3d(0.0,0.0,0.0) * matrix;
    osg::Vec3d v100 = osg::Vec3d(1.0,0.0,0.0) * matrix;
    osg::Vec3d v010 = osg::Vec3d(0.0,1.0,0.0) * matrix;
    osg::Vec3d v110 = osg::Vec3d(1.0,1.0,0.0) * matrix;

    osg::Vec3d v001 = osg::Vec3d(0.0,0.0,1.0) * matrix;
    osg::Vec3d v101 = osg::Vec3d(1.0,0.0,1.0) * matrix;
    osg::Vec3d v011 = osg::Vec3d(0.0,1.0,1.0) * matrix;
    osg::Vec3d v111 = osg::Vec3d(1.0,1.0,1.0) * matrix;

    double cubeSize = (v111-v000).length();
    osg::Vec3d center = (v000+v111)*0.5;

    osg::ClipNode* clipnode = new osg::ClipNode;
    clipnode->addChild(createCube(center, cubeSize, _numSlices));

    clipnode->addClipPlane(new osg::ClipPlane(0, osg::Plane(v000, v011, v001)));
    clipnode->addClipPlane(new osg::ClipPlane(1, osg::Plane(v100, v111, v110)));
    clipnode->addClipPlane(new osg::ClipPlane(2, osg::Plane(v000, v101, v100)));
    clipnode->addClipPlane(new osg::ClipPlane(3, osg::Plane(v110, v011, v010)));
    clipnode->addClipPlane(new osg::ClipPlane(4, osg::Plane(v000, v110, v010)));
    clipnode->addClipPlane(new osg::ClipPlane(5, osg::Plane(v001, v111, v101)));

    osg::TexGenNode* texgenNode_0 = new osg::TexGenNode;
    texgenNode_0->addChild(clipnode);
    texgenNode_0->setTextureUnit(0);
    texgenNode_0->getTexGen()->setMode(osg::TexGen::EYE_LINEAR);
    texgenNode_0->getTexGen()->setPlanesFromMatrix(osg::Matrix::inverse(matrix));

    osg::StateSet* stateset = texgenNode_0->getOrCreateStateSet();

    stateset->setMode(GL_LIGHTING,osg::StateAttribute::ON);
    stateset->setMode(GL_BLEND,osg::StateAttribute::ON);

    if (cpv._afProperty.valid())
    {
        stateset->setAttributeAndModes(cpv._afProperty->getAlphaFunc(), osg::StateAttribute::ON);
    }
    else
    {
        stateset->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GREATER,alphaFuncValue), osg::StateAttribute::ON);
    }

    osg::Material* material = new osg::Material;
    material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    stateset->setAttributeAndModes(material);

    if (cpv._mipProperty.valid())
    {
        stateset->setAttribute(new osg::BlendFunc(osg::BlendFunc::ONE, osg::BlendFunc::ONE));
        stateset->setAttribute(new osg::BlendEquation(osg::BlendEquation::RGBA_MAX));
    }


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

    stateset->setTextureMode(0,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
    stateset->setTextureMode(0,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
    stateset->setTextureMode(0,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);

    stateset->setTextureAttributeAndModes(0,new osg::TexEnv(),osg::StateAttribute::ON);

    _node = texgenNode_0;
}

void FixedFunctionTechnique::update(osgUtil::UpdateVisitor* /*uv*/)
{
//    OSG_NOTICE<<"FixedFunctionTechnique:update(osgUtil::UpdateVisitor* nv):"<<std::endl;
}

void FixedFunctionTechnique::cull(osgUtil::CullVisitor* cv)
{
    //OSG_NOTICE<<"FixedFunctionTechnique::cull(osgUtil::CullVisitor* nv)"<<std::endl;
    if (_node.valid())
    {
        _node->accept(*cv);
    }
}

void FixedFunctionTechnique::cleanSceneGraph()
{
    OSG_NOTICE<<"FixedFunctionTechnique::cleanSceneGraph()"<<std::endl;
}

void FixedFunctionTechnique::traverse(osg::NodeVisitor& nv)
{
    // OSG_NOTICE<<"FixedFunctionTechnique::traverse(osg::NodeVisitor& nv)"<<std::endl;
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

