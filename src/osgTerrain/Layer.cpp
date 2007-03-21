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

/////////////////////////////////////////////////////////////////////////////
//
// ImageLayer
//
ArrayLayer::ArrayLayer()
{
}

ArrayLayer::ArrayLayer(const ArrayLayer& arrayLayer,const osg::CopyOp& copyop):
    Layer(arrayLayer,copyop),
    _array(arrayLayer._array)
{
}

void ArrayLayer::setArray(osg::Array* array)
{
    _array = array;
}
