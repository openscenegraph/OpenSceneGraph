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

void osgTerrain::extractSetNameAndFileName(const std::string& compoundstring, std::string& setname, std::string& filename)
{
    std::string::size_type setcolonpos = compoundstring.find("set:");
    if (setcolonpos==std::string::npos)
    {
        setname = "";
        filename = compoundstring;
        return;
    }

    if (compoundstring.size()==4)
    {
        setname = "";
        filename = "";
        return;
    }

    std::string::size_type secondcolonpos = compoundstring.find_first_of(':', setcolonpos+4);
    if (secondcolonpos==std::string::npos)
    {
        setname = compoundstring.substr(setcolonpos+4,std::string::npos);
        filename = "";
        return;
    }

    setname = compoundstring.substr(setcolonpos+4,secondcolonpos-setcolonpos-4);
    filename = compoundstring.substr(secondcolonpos+1, std::string::npos);
}

std::string osgTerrain::createCompoundSetNameAndFileName(const std::string& setname, const std::string& filename)
{
    if (setname.empty()) return filename;
    return std::string("set:")+setname+std::string(":")+filename;
}


Layer::Layer():
    _minLevel(0),
    _maxLevel(MAXIMUM_NUMBER_OF_LEVELS),
    _minFilter(osg::Texture::LINEAR_MIPMAP_LINEAR),
    _magFilter(osg::Texture::LINEAR)
{
}

Layer::Layer(const Layer& layer,const osg::CopyOp& copyop):
    osg::Object(layer,copyop),
    _filename(layer._filename),
    _minLevel(layer._minLevel),
    _maxLevel(layer._maxLevel),
    _minFilter(layer._minFilter),
    _magFilter(layer._magFilter)
{
}

Layer::~Layer()
{
}

osg::BoundingSphere Layer::computeBound(bool treatAsElevationLayer) const
{
    osg::BoundingSphere bs;
    if (!getLocator()) return bs;

    if (treatAsElevationLayer)
    {
        osg::BoundingBox bb;
        unsigned int numColumns = getNumColumns();
        unsigned int numRows = getNumRows();
        for(unsigned int r=0;r<numRows;++r)
        {
            for(unsigned int c=0;c<numColumns;++c)
            {
                float value = 0.0f;
                bool validValue = getValidValue(c,r, value);
                if (validValue)
                {
                    osg::Vec3d ndc, v;
                    ndc.x() = ((double)c)/(double)(numColumns-1),
                    ndc.y() = ((double)r)/(double)(numRows-1);
                    ndc.z() = value;

                    if (getLocator()->convertLocalToModel(ndc, v))
                    {
                        bb.expandBy(v);
                    }
                }
            }
        }
        bs.expandBy(bb);
    }
    else
    {

        osg::Vec3d v;
        if (getLocator()->convertLocalToModel(osg::Vec3d(0.5,0.5,0.0), v))
        {
            bs.center() = v;
        }

        if (getLocator()->convertLocalToModel(osg::Vec3d(0.0,0.0,0.0), v))
        {
            bs.radius() = (bs.center() - v).length();
        }

    }

    return bs;
}


/////////////////////////////////////////////////////////////////////////////
//
// ImageLayer
//
ImageLayer::ImageLayer(osg::Image* image):
    _image(image)
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

    OSG_INFO<<"ImageLayer::transform("<<offset<<","<<scale<<")"<<std::endl;;

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
            // OSG_NOTICE<<"byte "<<value<<std::endl;
            break;
        case(GL_UNSIGNED_BYTE):
            value = *data;
            // OSG_NOTICE<<"Unsigned byte "<<value<<std::endl;
            break;
        case(GL_SHORT):
            value = *((const short*)data);
            // OSG_NOTICE<<"Short "<<value<<std::endl;
            break;
        case(GL_UNSIGNED_SHORT):
            value = *((const unsigned short*)data);
            // OSG_NOTICE<<"Unsigned Short "<<value<<std::endl;
            break;
        case(GL_INT):
            value = *((const int*)data);
            // OSG_NOTICE<<"Int "<<value<<std::endl;
            break;
        case(GL_UNSIGNED_INT):
            // OSG_NOTICE<<"Unsigned Int "<<value<<std::endl;
            value = *((const unsigned int*)data);
            break;
        case(GL_FLOAT):
            // OSG_NOTICE<<"Float "<<value<<std::endl;
            value = *((const float*)data);
            break;
        default:
            value = _defaultValue.x();
            return false;
    }

    return true;
}

bool ImageLayer::getValue(unsigned int /*i*/, unsigned int /*j*/, osg::Vec2& /*value*/) const
{
    OSG_NOTICE<<"Not implemented yet"<<std::endl;
    return false;
}

bool ImageLayer::getValue(unsigned int /*i*/, unsigned int /*j*/, osg::Vec3& /*value*/) const
{
    OSG_NOTICE<<"Not implemented yet"<<std::endl;
    return false;
}

bool ImageLayer::getValue(unsigned int /*i*/, unsigned int /*j*/, osg::Vec4& /*value*/) const
{
    OSG_NOTICE<<"Not implemented yet"<<std::endl;
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
// ContourLayer
//
ContourLayer::ContourLayer(osg::TransferFunction1D* tf):
    _tf(tf)
{
    _minFilter = osg::Texture::NEAREST;
    _magFilter = osg::Texture::NEAREST;
}

ContourLayer::ContourLayer(const ContourLayer& contourLayer,const osg::CopyOp& copyop):
    Layer(contourLayer, copyop),
    _tf(contourLayer._tf)
{
}

void ContourLayer::setTransferFunction(osg::TransferFunction1D* tf)
{
    _tf = tf;
}

bool ContourLayer::transform(float offset, float scale)
{
    if (!_tf) return false;

    OSG_INFO<<"ContourLayer::transform("<<offset<<","<<scale<<")"<<std::endl;;

    osg::TransferFunction1D::ColorMap newColorMap = _tf->getColorMap();
    for(osg::TransferFunction1D::ColorMap::iterator itr = newColorMap.begin();
        itr != newColorMap.end();
        ++itr)
    {
        osg::Vec4& value = itr->second;
        value.r() = offset + value.r()* scale;
        value.g() = offset + value.g()* scale;
        value.b() = offset + value.b()* scale;
        value.a() = offset + value.a()* scale;
    }

    _tf->assign(newColorMap);

    dirty();

    return true;
}

bool ContourLayer::getValue(unsigned int i, unsigned int /*j*/, float& value) const
{
    if (!_tf) return false;

    const osg::Vec4& v = _tf->getPixelValue(i);
    value = v[0];

    return true;
}

bool ContourLayer::getValue(unsigned int i, unsigned int /*j*/, osg::Vec2& value) const
{
    if (!_tf) return false;

    const osg::Vec4& v = _tf->getPixelValue(i);
    value.x() = v.x();
    value.y() = v.y();

    return true;
}

bool ContourLayer::getValue(unsigned int i, unsigned int /*j*/, osg::Vec3& value) const
{
    if (!_tf) return false;

    const osg::Vec4& v = _tf->getPixelValue(i);
    value.x() = v.x();
    value.y() = v.y();
    value.z() = v.z();

    return true;
}

bool ContourLayer::getValue(unsigned int i, unsigned int /*j*/, osg::Vec4& value) const
{
    if (!_tf) return false;

    value = _tf->getPixelValue(i);

    return true;
}

void ContourLayer::dirty()
{
    if (getImage()) getImage()->dirty();
}

void ContourLayer::setModifiedCount(unsigned int value)
{
    if (getImage()) getImage()->setModifiedCount(value);
}

unsigned int ContourLayer::getModifiedCount() const
{
    if (!getImage()) return 0;
    else return getImage()->getModifiedCount();
}


/////////////////////////////////////////////////////////////////////////////
//
// HeightFieldLayer
//
HeightFieldLayer::HeightFieldLayer(osg::HeightField* hf):
    _modifiedCount(0),
    _heightField(hf)
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

    OSG_INFO<<"HeightFieldLayer::transform("<<offset<<","<<scale<<")"<<std::endl;;

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

void ProxyLayer::setFileName(const std::string& filename)
{
    _filename = filename;
    if (_implementation.valid())
    {
        _implementation->setFileName(_filename);
    }
}

unsigned int ProxyLayer::getNumColumns() const
{
    if (_implementation.valid()) return _implementation->getNumColumns();
    else return 0;
}

unsigned int ProxyLayer::getNumRows() const
{
    if (_implementation.valid()) return _implementation->getNumRows();
    else return 0;
}

bool ProxyLayer::transform(float offset, float scale)
{
    if (_implementation.valid()) return _implementation->transform(offset,scale);
    else return false;
}

bool ProxyLayer::getValue(unsigned int i, unsigned int j, float& value) const
{
    if (_implementation.valid()) return _implementation->getValue(i,j,value);
    else return false;
}

bool ProxyLayer::getValue(unsigned int i, unsigned int j, osg::Vec2& value) const
{
    if (_implementation.valid()) return _implementation->getValue(i,j,value);
    else return false;
}

bool ProxyLayer::getValue(unsigned int i, unsigned int j, osg::Vec3& value) const
{
    if (_implementation.valid()) return _implementation->getValue(i,j,value);
    else return false;
}

bool ProxyLayer::getValue(unsigned int i, unsigned int j, osg::Vec4& value) const
{
    if (_implementation.valid()) return _implementation->getValue(i,j,value);
    else return false;
}

void ProxyLayer::dirty()
{
    if (_implementation.valid()) _implementation->dirty();
}

void ProxyLayer::setModifiedCount(unsigned int value)
{
    if (_implementation.valid()) _implementation->setModifiedCount(value);
}

unsigned int ProxyLayer::getModifiedCount() const
{
    return _implementation.valid() ? _implementation->getModifiedCount() : 0;
}


osg::BoundingSphere ProxyLayer::computeBound(bool treatAsElevationLayer) const
{
    if (_implementation.valid()) return _implementation->computeBound(treatAsElevationLayer);
    else return osg::BoundingSphere();
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

void CompositeLayer::setCompoundName(unsigned int i, const std::string& compoundname)
{
    std::string setname;
    std::string filename;
    extractSetNameAndFileName(compoundname, setname, filename);

    _layers[i].setname = setname;
    _layers[i].filename = filename;
}

std::string CompositeLayer::getCompoundName(unsigned int i) const
{
    return createCompoundSetNameAndFileName(_layers[i].setname, _layers[i].filename);
}

void CompositeLayer::addLayer(const std::string& compoundname)
{
    std::string setname;
    std::string filename;
    extractSetNameAndFileName(compoundname, setname, filename);

    _layers.push_back(CompoundNameLayer(setname,filename,0));
}

void CompositeLayer::addLayer(const std::string& setname, const std::string& filename)
{
    _layers.push_back(CompoundNameLayer(setname,filename,0));
}

/////////////////////////////////////////////////////////////////////////////
//
// SwitchLayer
//
SwitchLayer::SwitchLayer():
    _activeLayer(-1)
{
}

SwitchLayer::SwitchLayer(const SwitchLayer& switchLayer,const osg::CopyOp& copyop):
    CompositeLayer(switchLayer,copyop),
    _activeLayer(switchLayer._activeLayer)
{
}
