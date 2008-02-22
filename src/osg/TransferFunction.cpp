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

#include <osg/TransferFunction>

using namespace osg;

///////////////////////////////////////////////////////////////////////
//
// TransferFunction base class
//
TransferFunction::TransferFunction()
{
}

TransferFunction::~TransferFunction()
{
}

///////////////////////////////////////////////////////////////////////
//
// TransferFunction1D class
//
TransferFunction1D::TransferFunction1D()
{
    _minimum = 0.0;
    _maximum = 1.0;
}

void TransferFunction1D::setInputRange(float minimum, float maximum)
{
    _minimum = minimum;
    _maximum = maximum;
}

void TransferFunction1D::allocate(unsigned int numX)
{
    _colors.resize(numX);
    _image = new osg::Image;
    _image->setImage(numX,1,1,GL_RGBA, GL_RGBA, GL_FLOAT, (unsigned char*)&_colors[0], osg::Image::NO_DELETE);
}

void TransferFunction1D::clear(const osg::Vec4& color)
{
    for(Colors::iterator itr = _colors.begin();
        itr != _colors.end();
        ++itr)
    {
        *itr = color;
    }
}
