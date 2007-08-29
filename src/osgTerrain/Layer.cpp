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

Layer::Layer(const Layer& layer,const osg::CopyOp& copyop):
    osg::Object(layer,copyop),
    _filename(layer._filename)
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

template <typename T, class O>    
void _processRow(unsigned int num, GLenum pixelFormat, T* data,const O& operation)
{
    switch(pixelFormat)
    {
        case(GL_LUMINANCE):         { for(unsigned int i=0;i<num;++i) { operation(*data++); } }  break;
        case(GL_ALPHA):             { for(unsigned int i=0;i<num;++i) { operation(*data++); } }  break;
        case(GL_LUMINANCE_ALPHA):   { for(unsigned int i=0;i<num;++i) { operation(*data++); operation(*data++); } }  break;
        case(GL_RGB):               { for(unsigned int i=0;i<num;++i) { operation(*data++); operation(*data++); operation(*data++); } }  break;
        case(GL_RGBA):              { for(unsigned int i=0;i<num;++i) { operation(*data++); operation(*data++); operation(*data++); operation(*data++);  } }  break;
        case(GL_BGR):               { for(unsigned int i=0;i<num;++i) { operation(*data++); operation(*data++); operation(*data++); } }  break;
        case(GL_BGRA):              { for(unsigned int i=0;i<num;++i) { operation(*data++); operation(*data++); operation(*data++); operation(*data++);  } }  break;
    }
}

template <class O>    
void processRow(unsigned int num, GLenum pixelFormat, GLenum dataType, unsigned char* data, const O& operation)
{
    switch(dataType)
    {
        case(GL_BYTE):              _processRow(num,pixelFormat, (char*)data,            operation); break;
        case(GL_UNSIGNED_BYTE):     _processRow(num,pixelFormat, (unsigned char*)data,   operation); break;
        case(GL_SHORT):             _processRow(num,pixelFormat, (short*) data,          operation); break;
        case(GL_UNSIGNED_SHORT):    _processRow(num,pixelFormat, (unsigned short*)data,  operation); break;
        case(GL_INT):               _processRow(num,pixelFormat, (int*) data,            operation); break;
        case(GL_UNSIGNED_INT):      _processRow(num,pixelFormat, (unsigned int*) data,   operation); break;
        case(GL_FLOAT):             _processRow(num,pixelFormat, (float*) data,          operation); break;
    }
}

template <class O>    
void processImage(osg::Image* image, const O& operation)
{
    if (!image) return;
    
    for(int r=0;r<image->r();++r)
    {
        for(int t=0;t<image->t();++t)
        {
            processRow(image->s(), image->getPixelFormat(), image->getDataType(), image->data(0,t,r), operation);
        }
    }
}

struct TransformOperator
{
    TransformOperator(float offset, float scale):
        _offset(offset),
        _scale(scale) {}

    inline void operator() (unsigned char& v) const { v = (unsigned char)(_offset + (float)v * _scale); }
    inline void operator() (unsigned short& v) const { v = (unsigned short)(_offset + (float)v * _scale); }
    inline void operator() (unsigned int& v) const { v = (unsigned int)(_offset + (float)v * _scale); }
    inline void operator() (char& v) const { v = (char)(_offset + (float)v * _scale); }
    inline void operator() (short& v) const { v = (short)(_offset + (float)v * _scale); }
    inline void operator() (int& v) const { v = (int)(_offset + (float)v * _scale); }
    inline void operator() (float& v) const { v = _offset + v * _scale; }
    
    float _offset, _scale;
};


bool ImageLayer::transform(float offset, float scale)
{
    if (!_image.valid()) return false;

    osg::notify(osg::NOTICE)<<"ImageLayer::transform("<<offset<<","<<scale<<")"<<std::endl;;

    processImage(_image.get(), TransformOperator(offset,scale));
    
    dirty();

    return true;
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

    return true;
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

void ImageLayer::dirty()
{
    if (_image.valid()) _image->dirty();
}

void ImageLayer::setModifiedCount(unsigned int value)
{
    if (!_image) return;
    else _image->setModifiedCount(value);
}

unsigned int ImageLayer::getModifiedCount() const
{
    if (!_image) return 0;
    else return _image->getModifiedCount();
}


/////////////////////////////////////////////////////////////////////////////
//
// HieghtFieldLayer
//
HeightFieldLayer::HeightFieldLayer():
    _modifiedCount(0)
{
}

HeightFieldLayer::HeightFieldLayer(const HeightFieldLayer& hfLayer,const osg::CopyOp& copyop):
    Layer(hfLayer,copyop),
    _modifiedCount(0),
    _heightField(hfLayer._heightField)
{
    if (_heightField.valid()) ++_modifiedCount;
}


void HeightFieldLayer::setHeightField(osg::HeightField* hf)
{
    _heightField = hf;
    dirty();
}



bool HeightFieldLayer::transform(float offset, float scale)
{
    if (!_heightField) return false;

    osg::FloatArray* heights = _heightField->getFloatArray();
    if (!heights) return false;
    
    osg::notify(osg::NOTICE)<<"HeightFieldLayer::transform("<<offset<<","<<scale<<")"<<std::endl;;

    for(osg::FloatArray::iterator itr = heights->begin();
        itr != heights->end();
        ++itr)
    {
        *itr = offset + (*itr) * scale;
    }
    
    dirty();

    return true;
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

void HeightFieldLayer::dirty()
{
    ++_modifiedCount;
}

void HeightFieldLayer::setModifiedCount(unsigned int value)
{
    _modifiedCount = value;
}

unsigned int HeightFieldLayer::getModifiedCount() const
{
    return _modifiedCount;
}


/////////////////////////////////////////////////////////////////////////////
//
// CompositeLayer
//
CompositeLayer::CompositeLayer()
{
}

CompositeLayer::CompositeLayer(const CompositeLayer& compositeLayer,const osg::CopyOp& copyop):
    Layer(compositeLayer,copyop)
{
}


void CompositeLayer::clear()
{
    _layers.clear();
}

/////////////////////////////////////////////////////////////////////////////
//
// ProxyLayer
//
ProxyLayer::ProxyLayer()
{
}

ProxyLayer::ProxyLayer(const ProxyLayer& proxyLayer,const osg::CopyOp& copyop):
    Layer(proxyLayer,copyop)
{
}

ProxyLayer::~ProxyLayer()
{
}

