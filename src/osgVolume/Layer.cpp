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

#include <osg/Notify>
#include <osg/io_utils>

using namespace osgVolume;

Layer::Layer():
    _minFilter(osg::Texture::LINEAR_MIPMAP_LINEAR),
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
