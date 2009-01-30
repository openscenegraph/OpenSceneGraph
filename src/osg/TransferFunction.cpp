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


void TransferFunction1D::assign(const ValueMap& vcm, bool updateMinMaxRange)
{
    if (vcm.empty()) return;
    
    if (updateMinMaxRange)
    {
        _minimum = vcm.begin()->first;
        _maximum = vcm.rbegin()->first;
    }
    
    if (_colors.empty()) allocate(1024);
    
    
    float multiplier = float(_colors.size()-1)/(_maximum - _minimum);
    
    if (vcm.size()==1)
    {
        osg::Vec4 color = vcm.begin()->second;
        if (_minimum == _maximum)
        {
            clear(color);
        }
        else
        {
            float iPos = (vcm.begin()->first - _minimum)*multiplier;
            if (iPos>=0.0f || iPos<float(_colors.size()))
            {
                float iFloor = floorf(iPos);
                unsigned int i = (unsigned int)(iFloor);
                _colors[i] = color;
            }
        }
        _image->dirty();
        return;
    }
    
    ValueMap::const_iterator lower_itr = vcm.begin();
    ValueMap::const_iterator upper_itr = lower_itr;
    ++upper_itr;
    
    for(;
        upper_itr != vcm.end();
        ++upper_itr)
    {
        float lower_v = lower_itr->first;
        const osg::Vec4& lower_c = lower_itr->second;
        float upper_v = upper_itr->first;
        const osg::Vec4& upper_c = upper_itr->second;
        
        float lower_iPos = (lower_v - _minimum)*multiplier; 
        float upper_iPos = (upper_v - _minimum)*multiplier;

        float start_iPos = ceilf(lower_iPos);
        if (start_iPos<0.0f) start_iPos=0.0f;
        if (start_iPos>float(_colors.size()-1)) break;
        
        float end_iPos = floorf(upper_iPos);
        if (end_iPos<0.0f) continue;
        if (end_iPos>float(_colors.size()-1)) end_iPos=_colors.size()-1;

        Vec4 delta_c = (upper_c-lower_c)/(upper_iPos-lower_iPos);
        unsigned int i=static_cast<unsigned int>(start_iPos);
        for(float iPos=start_iPos;
            iPos<=end_iPos; 
            ++iPos, ++i)
        {
            _colors[i] = lower_c + delta_c*(iPos-lower_v);
        }
        
        lower_itr = upper_itr;      
    }

    _image->dirty();
}

