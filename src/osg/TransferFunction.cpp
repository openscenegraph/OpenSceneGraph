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

#include <osg/Notify>
#include <osg/io_utils>


using namespace osg;

///////////////////////////////////////////////////////////////////////
//
// TransferFunction base class
//
TransferFunction::TransferFunction()
{
}

TransferFunction::TransferFunction(const TransferFunction& tf, const CopyOp& copyop):
    Object(tf,copyop)
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
}

TransferFunction1D::TransferFunction1D(const TransferFunction1D& tf, const CopyOp& copyop):
    TransferFunction(tf,copyop)
{
    allocate(tf.getNumberImageCells());
    assign(tf._colorMap);
}

void TransferFunction1D::allocate(unsigned int numX)
{
    _image = new osg::Image;
    _image->allocateImage(numX,1,1,GL_RGBA, GL_FLOAT);
    
    updateImage();
}

void TransferFunction1D::clear(const osg::Vec4& color)
{
    ColorMap newColours;
    newColours[getMinimum()] = color;
    newColours[getMaximum()] = color;

    assign(newColours);
}

void TransferFunction1D::assignToImage(float lower_v, const osg::Vec4& lower_c, float upper_v, const osg::Vec4& upper_c)
{
    int endPos = getNumberImageCells()-1;
    float minimum = _colorMap.begin()->first;
    float maximum = _colorMap.rbegin()->first;
    float multiplier = float(endPos)/(maximum - minimum);
    osg::Vec4* imageData = reinterpret_cast<osg::Vec4*>(_image->data());

    float lower_iPos = (lower_v - minimum)*multiplier;
    float upper_iPos = (upper_v - minimum)*multiplier;

    int start_iPos = static_cast<int>(ceilf(lower_iPos));
    if (start_iPos < 0) start_iPos = 0;
    if (start_iPos > endPos) return;

    int end_iPos = static_cast<int>(floorf(upper_iPos));
    if (end_iPos < 0) return;
    if (end_iPos > endPos) end_iPos = endPos;

    // clamp to ends to avoid any precision issues
    if (lower_v == minimum) start_iPos = 0;
    if (upper_v == maximum) end_iPos = endPos;

    //OSG_NOTICE<<"TransferFunction1D::assignToImage[lower_v="<<lower_v<<", lower_c="<<lower_c<<", upper_v="<<upper_v<<" upper_c="<<upper_c<<std::endl;
    //OSG_NOTICE<<"  lower_iPos="<<lower_iPos<<"  start_iPpos="<<start_iPos<<std::endl;
    //OSG_NOTICE<<"  upper_iPos="<<upper_iPos<<"  end_iPpos="<<end_iPos<<std::endl;

    // upper_iPos can be identical to lower_iPos in case an underflow occurred while calculation the values;
    // if both values are identical delta_c is identical to the null vector otherwise it is calculated like follows:
    Vec4 delta_c;
    if (upper_iPos != lower_iPos)
      delta_c = (upper_c-lower_c)/(upper_iPos-lower_iPos);

    float iPos = static_cast<float>(start_iPos);
    for(int i=start_iPos; i<=end_iPos; ++i, ++iPos)
    {
        imageData[i] = lower_c + delta_c*(iPos-lower_iPos);
        //OSG_NOTICE<<"    imageData["<<i<<"] = "<<imageData[i]<<std::endl;
    }

    _image->dirty();
}


void TransferFunction1D::setColor(float v, const osg::Vec4& color, bool updateImage)
{
    if (!updateImage)
    {
        _colorMap[v] = color;
        return;
    }

    if (!_image) allocate(1024);

    if (_colorMap.empty() || v<getMinimum() || v>getMaximum())
    {
        _colorMap[v] = color;

        assign(_colorMap);
        return;
    }

    _colorMap[v] = color;

    ColorMap::iterator itr = _colorMap.find(v);

    if (itr != _colorMap.begin())
    {
        ColorMap::iterator previous_itr = itr;
        --previous_itr;

        assignToImage(previous_itr->first, previous_itr->second, v, color);
    }

    ColorMap::iterator next_itr = itr;
    ++next_itr;

    if (next_itr != _colorMap.end())
    {
        assignToImage(v, color, next_itr->first, next_itr->second);
    }
}

osg::Vec4 TransferFunction1D::getColor(float v) const
{
    if (_colorMap.empty()) return osg::Vec4(1.0f,1.0f,1.0f,1.0f);
    if (_colorMap.size()==1) return _colorMap.begin()->second;

    if (v <= _colorMap.begin()->first) return _colorMap.begin()->second;
    if (v >= _colorMap.rbegin()->first) return _colorMap.rbegin()->second;

    // need to implement
    std::pair<ColorMap::const_iterator, ColorMap::const_iterator> range = _colorMap.equal_range(v);

    // we have an identical match
    if (v == range.first->first) return range.first->second;

    // range.first will be at the next element after v, so move it before.
    --range.first;

    float vBefore = range.first->first;
    const osg::Vec4& cBefore = range.first->second;

    float vAfter = range.second->first;
    const osg::Vec4& cAfter = range.second->second;

    float r = (v-vBefore)/(vAfter-vBefore);

    return cBefore*(1.0f-r) + cAfter*r;
}


void TransferFunction1D::assign(const ColorMap& newColours)
{
    if (&_colorMap != &newColours) _colorMap = newColours;

    updateImage();
}

void TransferFunction1D::updateImage()
{
    if (_colorMap.empty()) return;

    if (!_image || _image->data()==0) allocate(1024);

    osg::Vec4* imageData = reinterpret_cast<osg::Vec4*>(_image->data());

    if (_colorMap.size()==1)
    {
        osg::Vec4 color = _colorMap.begin()->second;

        for(int i=0; i<_image->s(); ++i)
        {
            imageData[i] = color;
        }
        _image->dirty();
        return;
    }

    ColorMap::const_iterator lower_itr = _colorMap.begin();
    ColorMap::const_iterator upper_itr = lower_itr;
    ++upper_itr;

    for(;
        upper_itr != _colorMap.end();
        ++upper_itr)
    {
        float lower_v = lower_itr->first;
        const osg::Vec4& lower_c = lower_itr->second;
        float upper_v = upper_itr->first;
        const osg::Vec4& upper_c = upper_itr->second;

        assignToImage(lower_v, lower_c, upper_v, upper_c);

        lower_itr = upper_itr;
    }

    _image->dirty();
}
