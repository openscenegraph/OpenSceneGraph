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

#include <osgTerrain/Layer>
#include <osg/Notify>

using namespace osgTerrain;

Layer::Layer()
{
}

Layer::Layer(const Layer& Layer,const osg::CopyOp& copyop):
    osg::Object(Layer,copyop)
{
}

Layer::~Layer()
{
}

osg::BoundingSphere Layer::computeBound() const
{
    osg::BoundingSphere bs;
    if (!getLocator()) return bs;

    osg::Vec3d v;
    if (getLocator()->convertLocalToModel(osg::Vec3d(0.0,0.0,0.0), v))
    {
        bs.expandBy(v);
    }
    
    if (getLocator()->convertLocalToModel(osg::Vec3d(1.0,0.0,0.0), v))
    {
        bs.expandBy(v);
    }

    if (getLocator()->convertLocalToModel(osg::Vec3d(1.0,1.0,0.0), v))
    {
        bs.expandBy(v);
    }

    if (getLocator()->convertLocalToModel(osg::Vec3d(0.0,1.0,0.0), v))
    {
        bs.expandBy(v);
    }
    
    return bs;
}


/////////////////////////////////////////////////////////////////////////////
//
// ImageLayer
//
ImageLayer::ImageLayer()
{
}

ImageLayer::ImageLayer(const ImageLayer& imageLayer,const osg::CopyOp& copyop):
    Layer(imageLayer, copyop),
    _image(imageLayer._image)
{
}

void ImageLayer::setImage(osg::Image* image)
{
    _image = image;
}


bool ImageLayer::getValue(unsigned int i, unsigned int j, float& value) const
{
    const unsigned char* data = _image->data(i,j);
    switch(_image->getDataType())
    {
        case(GL_BYTE): 
            value = *((const char*)data); 
            // osg::notify(osg::NOTICE)<<"byte "<<value<<std::endl;
            break;
        case(GL_UNSIGNED_BYTE): 
            value = *data;
            // osg::notify(osg::NOTICE)<<"Unsigned byte "<<value<<std::endl;
            break;
        case(GL_SHORT):
            value = *((const short*)data);
            // osg::notify(osg::NOTICE)<<"Short "<<value<<std::endl;
            break;
        case(GL_UNSIGNED_SHORT):
            value = *((const unsigned short*)data);
            // osg::notify(osg::NOTICE)<<"Unsigned Short "<<value<<std::endl;
            break;
        case(GL_INT):
            value = *((const int*)data);
            // osg::notify(osg::NOTICE)<<"Int "<<value<<std::endl;
            break;
        case(GL_UNSIGNED_INT):
            // osg::notify(osg::NOTICE)<<"Unsigned Int "<<value<<std::endl;
            value = *((const unsigned int*)data);
            break;
        case(GL_FLOAT):
            // osg::notify(osg::NOTICE)<<"Float "<<value<<std::endl;
            value = *((const float*)data);
            break;
        default: 
            value = _defaultValue.x(); 
            return false;
    }

    return false;
}

bool ImageLayer::getValue(unsigned int i, unsigned int j, osg::Vec2& value) const
{
    osg::notify(osg::NOTICE)<<"Not implemented yet"<<std::endl;
    return false;
}

bool ImageLayer::getValue(unsigned int i, unsigned int j, osg::Vec3& value) const
{
    osg::notify(osg::NOTICE)<<"Not implemented yet"<<std::endl;
    return false;
}

bool ImageLayer::getValue(unsigned int i, unsigned int j, osg::Vec4& value) const
{
    osg::notify(osg::NOTICE)<<"Not implemented yet"<<std::endl;
    return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// HieghtFieldLayer
//
HeightFieldLayer::HeightFieldLayer()
{
}

HeightFieldLayer::HeightFieldLayer(const HeightFieldLayer& hfLayer,const osg::CopyOp& copyop):
    Layer(hfLayer,copyop),
    _heightField(hfLayer._heightField)
{
}


void HeightFieldLayer::setHeightField(osg::HeightField* hf)
{
    _heightField = hf;
}


bool HeightFieldLayer::getValue(unsigned int i, unsigned int j, float& value) const
{
    value = _heightField->getHeight(i,j);
    return true;
}

bool HeightFieldLayer::getValue(unsigned int i, unsigned int j, osg::Vec2& value) const
{
    value.x() = _heightField->getHeight(i,j);
    value.y() = _defaultValue.y();
    return true;
}

bool HeightFieldLayer::getValue(unsigned int i, unsigned int j, osg::Vec3& value) const
{
    value.x() = _heightField->getHeight(i,j);
    value.y() = _defaultValue.y();
    value.z() = _defaultValue.z();
    return true;
}

bool HeightFieldLayer::getValue(unsigned int i, unsigned int j, osg::Vec4& value) const
{
    value.x() = _heightField->getHeight(i,j);
    value.y() = _defaultValue.y();
    value.z() = _defaultValue.z();
    value.w() = _defaultValue.w();
    return true;
}
