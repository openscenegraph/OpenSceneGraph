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

#include <osgVolume/Layer>

#include <osg/ImageUtils>
#include <osg/ImageStream>
#include <osg/Endian>
#include <osg/Notify>
#include <osg/io_utils>

using namespace osgVolume;

Layer::Layer():
    _minFilter(osg::Texture::LINEAR),
    _magFilter(osg::Texture::LINEAR)
{
}

Layer::Layer(const Layer& layer,const osg::CopyOp& copyop):
    osg::Object(layer,copyop),
    _filename(layer._filename),
    _minFilter(layer._minFilter),
    _magFilter(layer._magFilter)
{
}

Layer::~Layer()
{
}

osg::BoundingSphere Layer::computeBound() const
{
    if (!getLocator()) return osg::BoundingSphere();
    
    osg::Vec3d left, right;
    getLocator()->computeLocalBounds(left, right);
    
    //osg::notify(osg::NOTICE)<<"left = "<<left<<std::endl;
    //osg::notify(osg::NOTICE)<<"right = "<<right<<std::endl;

    return osg::BoundingSphere((left+right)*0.5, (right-left).length()*0.5);
}


void Layer::addProperty(Property* property)
{
    if (!property) return;

    if (!_property) 
    {
        _property = property;
        return;
    }
    
    CompositeProperty* cp = dynamic_cast<CompositeProperty*>(_property.get());
    if (cp)
    {
        cp->addProperty(property);
    }
    else
    {
        cp = new CompositeProperty;
        cp->addProperty(property);
        cp->addProperty(_property.get());
        _property = cp;
    }
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

bool ImageLayer::computeMinMax(osg::Vec4& minValue, osg::Vec4& maxValue)
{
    if (_image.valid()) return osg::computeMinMax(_image.get(), minValue, maxValue);
    else return false;
}

void ImageLayer::offsetAndScaleImage(const osg::Vec4& offset, const osg::Vec4& scale)
{
    if (!_image) return;
    
    osg::offsetAndScaleImage(_image.get(), offset, scale);
}

void ImageLayer::rescaleToZeroToOneRange()
{
    osg::Vec4 minValue, maxValue;
    if (computeMinMax(minValue, maxValue))
    {
        float minComponent = minValue[0];
        minComponent = osg::minimum(minComponent,minValue[1]);
        minComponent = osg::minimum(minComponent,minValue[2]);
        minComponent = osg::minimum(minComponent,minValue[3]);

        float maxComponent = maxValue[0];
        maxComponent = osg::maximum(maxComponent,maxValue[1]);
        maxComponent = osg::maximum(maxComponent,maxValue[2]);
        maxComponent = osg::maximum(maxComponent,maxValue[3]);
        
        float scale = 0.99f/(maxComponent-minComponent);
        float offset = -minComponent * scale;

        offsetAndScaleImage(osg::Vec4(offset, offset, offset, offset),
                            osg::Vec4(scale, scale, scale, scale));
    }
}

void ImageLayer::translateMinToZero()
{
    osg::Vec4 minValue, maxValue;
    if (computeMinMax(minValue, maxValue))
    {
        float minComponent = minValue[0];
        minComponent = osg::minimum(minComponent,minValue[1]);
        minComponent = osg::minimum(minComponent,minValue[2]);
        minComponent = osg::minimum(minComponent,minValue[3]);

        float offset = -minComponent;

        offsetAndScaleImage(osg::Vec4(offset, offset, offset, offset),
                            osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }
}

bool ImageLayer::requiresUpdateTraversal() const
{
    return dynamic_cast<osg::ImageStream*>(_image.get())!=0;
}

void ImageLayer::update(osg::NodeVisitor& nv)
{
    if (_image.valid()) _image->update(&nv);
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

bool CompositeLayer::requiresUpdateTraversal() const
{
    for(Layers::const_iterator itr = _layers.begin();
        itr != _layers.end();
        ++itr)
    {
        if (itr->layer->requiresUpdateTraversal()) return true;
    }
    
    return false;
}

void CompositeLayer::update(osg::NodeVisitor& nv)
{
    for(Layers::const_iterator itr = _layers.begin();
        itr != _layers.end();
        ++itr)
    {
        itr->layer->update(nv);
    }
    
}


/////////////////////////////////////////////////////////////////////////////
//
// createNormalMapTexture
//
osg::Image* osgVolume::createNormalMapTexture(osg::Image* image_3d)
{
    osg::notify(osg::INFO)<<"Computing NormalMapTexture"<<std::endl;

    GLenum dataType = image_3d->getDataType();

    unsigned int sourcePixelIncrement = 1;
    unsigned int alphaOffset = 0; 
    switch(image_3d->getPixelFormat())
    {
    case(GL_ALPHA):
    case(GL_LUMINANCE):
        sourcePixelIncrement = 1;
        alphaOffset = 0;
        break;
    case(GL_LUMINANCE_ALPHA):
        sourcePixelIncrement = 2;
        alphaOffset = 1;
        break;
    case(GL_RGB):
        sourcePixelIncrement = 3;
        alphaOffset = 0;
        break;
    case(GL_RGBA):
        sourcePixelIncrement = 4;
        alphaOffset = 3;
        break;
    default:
        osg::notify(osg::NOTICE)<<"Source pixel format not support for normal map generation."<<std::endl;
        return 0;
    }


    osg::ref_ptr<osg::Image> normalmap_3d = new osg::Image;
    normalmap_3d->allocateImage(image_3d->s(),image_3d->t(),image_3d->r(),
                            GL_RGBA,GL_UNSIGNED_BYTE);

    if (osg::getCpuByteOrder()==osg::LittleEndian) alphaOffset = sourcePixelIncrement-alphaOffset-1;

    for(int r=1;r<image_3d->r()-1;++r)
    {
        for(int t=1;t<image_3d->t()-1;++t)
        {

            if (dataType==GL_UNSIGNED_BYTE)
            {        
                unsigned char* ptr = image_3d->data(1,t,r)+alphaOffset;
                unsigned char* left = image_3d->data(0,t,r)+alphaOffset;
                unsigned char* right = image_3d->data(2,t,r)+alphaOffset;
                unsigned char* above = image_3d->data(1,t+1,r)+alphaOffset;
                unsigned char* below = image_3d->data(1,t-1,r)+alphaOffset;
                unsigned char* in = image_3d->data(1,t,r+1)+alphaOffset;
                unsigned char* out = image_3d->data(1,t,r-1)+alphaOffset;

                unsigned char* destination = (unsigned char*) normalmap_3d->data(1,t,r);

                for(int s=1;s<image_3d->s()-1;++s)
                {

                    osg::Vec3 grad((float)(*left)-(float)(*right),
                                   (float)(*below)-(float)(*above),
                                   (float)(*out) -(float)(*in));

                    grad.normalize();

                    if (grad.x()==0.0f && grad.y()==0.0f && grad.z()==0.0f)
                    {
                        grad.set(128.0f,128.0f,128.0f);
                    }
                    else
                    {
                        grad.x() = osg::clampBetween((grad.x()+1.0f)*128.0f,0.0f,255.0f);
                        grad.y() = osg::clampBetween((grad.y()+1.0f)*128.0f,0.0f,255.0f);
                        grad.z() = osg::clampBetween((grad.z()+1.0f)*128.0f,0.0f,255.0f);
                    }

                    *(destination++) = (unsigned char)(grad.x()); // scale and bias X.
                    *(destination++) = (unsigned char)(grad.y()); // scale and bias Y.
                    *(destination++) = (unsigned char)(grad.z()); // scale and bias Z.

                    *destination++ = *ptr;

                    ptr += sourcePixelIncrement;
                    left += sourcePixelIncrement;
                    right += sourcePixelIncrement;
                    above += sourcePixelIncrement;
                    below += sourcePixelIncrement;
                    in += sourcePixelIncrement;
                    out += sourcePixelIncrement;
                }
            }
            else if (dataType==GL_SHORT)
            {
                short* ptr = (short*)(image_3d->data(1,t,r)+alphaOffset);
                short* left = (short*)(image_3d->data(0,t,r)+alphaOffset);
                short* right = (short*)(image_3d->data(2,t,r)+alphaOffset);
                short* above = (short*)(image_3d->data(1,t+1,r)+alphaOffset);
                short* below = (short*)(image_3d->data(1,t-1,r)+alphaOffset);
                short* in = (short*)(image_3d->data(1,t,r+1)+alphaOffset);
                short* out = (short*)(image_3d->data(1,t,r-1)+alphaOffset);

                unsigned char* destination = (unsigned char*) normalmap_3d->data(1,t,r);

                for(int s=1;s<image_3d->s()-1;++s)
                {

                    osg::Vec3 grad((float)(*left)-(float)(*right),
                                   (float)(*below)-(float)(*above),
                                   (float)(*out) -(float)(*in));

                    grad.normalize();

                    //osg::notify(osg::NOTICE)<<"normal "<<grad<<std::endl;

                    if (grad.x()==0.0f && grad.y()==0.0f && grad.z()==0.0f)
                    {
                        grad.set(128.0f,128.0f,128.0f);
                    }
                    else
                    {
                        grad.x() = osg::clampBetween((grad.x()+1.0f)*128.0f,0.0f,255.0f);
                        grad.y() = osg::clampBetween((grad.y()+1.0f)*128.0f,0.0f,255.0f);
                        grad.z() = osg::clampBetween((grad.z()+1.0f)*128.0f,0.0f,255.0f);
                    }
                    

                    *(destination++) = (unsigned char)(grad.x()); // scale and bias X.
                    *(destination++) = (unsigned char)(grad.y()); // scale and bias Y.
                    *(destination++) = (unsigned char)(grad.z()); // scale and bias Z.

                    *destination++ = *ptr/128;

                    ptr += sourcePixelIncrement;
                    left += sourcePixelIncrement;
                    right += sourcePixelIncrement;
                    above += sourcePixelIncrement;
                    below += sourcePixelIncrement;
                    in += sourcePixelIncrement;
                    out += sourcePixelIncrement;
                }
            }
            else if (dataType==GL_UNSIGNED_SHORT)
            {
                unsigned short* ptr = (unsigned short*)(image_3d->data(1,t,r)+alphaOffset);
                unsigned short* left = (unsigned short*)(image_3d->data(0,t,r)+alphaOffset);
                unsigned short* right = (unsigned short*)(image_3d->data(2,t,r)+alphaOffset);
                unsigned short* above = (unsigned short*)(image_3d->data(1,t+1,r)+alphaOffset);
                unsigned short* below = (unsigned short*)(image_3d->data(1,t-1,r)+alphaOffset);
                unsigned short* in = (unsigned short*)(image_3d->data(1,t,r+1)+alphaOffset);
                unsigned short* out = (unsigned short*)(image_3d->data(1,t,r-1)+alphaOffset);

                unsigned char* destination = (unsigned char*) normalmap_3d->data(1,t,r);

                for(int s=1;s<image_3d->s()-1;++s)
                {

                    osg::Vec3 grad((float)(*left)-(float)(*right),
                                   (float)(*below)-(float)(*above),
                                   (float)(*out) -(float)(*in));

                    grad.normalize();

                    if (grad.x()==0.0f && grad.y()==0.0f && grad.z()==0.0f)
                    {
                        grad.set(128.0f,128.0f,128.0f);
                    }
                    else
                    {
                        grad.x() = osg::clampBetween((grad.x()+1.0f)*128.0f,0.0f,255.0f);
                        grad.y() = osg::clampBetween((grad.y()+1.0f)*128.0f,0.0f,255.0f);
                        grad.z() = osg::clampBetween((grad.z()+1.0f)*128.0f,0.0f,255.0f);
                    }

                    *(destination++) = (unsigned char)(grad.x()); // scale and bias X.
                    *(destination++) = (unsigned char)(grad.y()); // scale and bias Y.
                    *(destination++) = (unsigned char)(grad.z()); // scale and bias Z.

                    *destination++ = *ptr/256;

                    ptr += sourcePixelIncrement;
                    left += sourcePixelIncrement;
                    right += sourcePixelIncrement;
                    above += sourcePixelIncrement;
                    below += sourcePixelIncrement;
                    in += sourcePixelIncrement;
                    out += sourcePixelIncrement;
                }
            }
        }
    }
    
    
    osg::notify(osg::INFO)<<"Created NormalMapTexture"<<std::endl;
    
    return normalmap_3d.release();
}

/////////////////////////////////////////////////////////////////////////////
//
// applyTransferFunction
//
struct ApplyTransferFunctionOperator
{
    ApplyTransferFunctionOperator(osg::TransferFunction1D* tf, unsigned char* data):
        _tf(tf),
        _data(data) {}
        
    inline void luminance(float l) const
    {
        osg::Vec4 c = _tf->getColor(l);
        //std::cout<<"l = "<<l<<" c="<<c<<std::endl;
        *(_data++) = (unsigned char)(c[0]*255.0f + 0.5f);
        *(_data++) = (unsigned char)(c[1]*255.0f + 0.5f);
        *(_data++) = (unsigned char)(c[2]*255.0f + 0.5f);
        *(_data++) = (unsigned char)(c[3]*255.0f + 0.5f);
    }
     
    inline void alpha(float a) const
    {
        luminance(a);
    } 
    
    inline void luminance_alpha(float l,float a) const
    { 
        luminance(l);
    }
     
    inline void rgb(float r,float g,float b) const
    {
        luminance((r+g+b)*0.3333333);
    }
    
    inline void rgba(float r,float g,float b,float a) const
    {
        luminance(a);
    }
    
    mutable osg::ref_ptr<osg::TransferFunction1D> _tf;
    mutable unsigned char* _data;
};

osg::Image* osgVolume::applyTransferFunction(osg::Image* image, osg::TransferFunction1D* transferFunction)
{
    osg::notify(osg::INFO)<<"Applying transfer function"<<std::endl;
    
    osg::Image* output_image = new osg::Image;
    output_image->allocateImage(image->s(),image->t(), image->r(), GL_RGBA, GL_UNSIGNED_BYTE);
    
    ApplyTransferFunctionOperator op(transferFunction, output_image->data());
    osg::readImage(image,op); 
    
    return output_image;
}
