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

#include <float.h>
#include <osg/Math>
#include <osg/Notify>
#include <osgVolume/ImageUtils>

using namespace osgVolume;

namespace osgVolume
{

struct FindRangeOperator
{
    FindRangeOperator():
        _rmin(FLT_MAX),
        _rmax(-FLT_MAX),
        _gmin(FLT_MAX),
        _gmax(-FLT_MAX),
        _bmin(FLT_MAX),
        _bmax(-FLT_MAX),
        _amin(FLT_MAX),
        _amax(-FLT_MAX) {}
        
    float _rmin, _rmax, _gmin, _gmax, _bmin, _bmax, _amin, _amax;

    inline void luminance(float l) { rgba(l,l,l,l); } 
    inline void alpha(float a) { rgba(1.0f,1.0f,1.0f,a); } 
    inline void luminance_alpha(float l,float a) { rgba(l,l,l,a); } 
    inline void rgb(float r,float g,float b) { rgba(r,g,b,1.0f);  }
    inline void rgba(float r,float g,float b,float a)
    {
        _rmin = osg::minimum(r,_rmin); 
        _rmax = osg::maximum(r,_rmax); 
        _gmin = osg::minimum(g,_gmin); 
        _gmax = osg::maximum(g,_gmax); 
        _bmin = osg::minimum(b,_bmin); 
        _bmax = osg::maximum(b,_bmax); 
        _amin = osg::minimum(a,_amin); 
        _amax = osg::maximum(a,_amax);
    }



};

struct OffsetAndScaleOperator
{
    OffsetAndScaleOperator(const osg::Vec4& offset, const osg::Vec4& scale):
        _offset(offset), 
        _scale(scale) {}

    osg::Vec4 _offset;
    osg::Vec4 _scale;

    inline void luminance(float& l) const { l= _offset.r() + l*_scale.r(); } 
    inline void alpha(float& a) const { a = _offset.a() + a*_scale.a(); } 
    inline void luminance_alpha(float& l,float& a) const
    {
        l= _offset.r() + l*_scale.r(); 
        a = _offset.a() + a*_scale.a();
    } 
    inline void rgb(float& r,float& g,float& b) const
    {
        r = _offset.r() + r*_scale.r(); 
        g = _offset.g() + g*_scale.g(); 
        b = _offset.b() + b*_scale.b();
    }
    inline void rgba(float& r,float& g,float& b,float& a) const
    {
        r = _offset.r() + r*_scale.r(); 
        g = _offset.g() + g*_scale.g(); 
        b = _offset.b() + b*_scale.b();
        a = _offset.a() + a*_scale.a();
    }
};

}

bool osgVolume::computeMinMax(const osg::Image* image, osg::Vec4& minValue, osg::Vec4& maxValue)
{
    if (!image) return false;

    osgVolume::FindRangeOperator rangeOp;    
    readImage(image, rangeOp);
    minValue.r() = rangeOp._rmin;
    minValue.g() = rangeOp._gmin;
    minValue.b() = rangeOp._bmin;
    minValue.a() = rangeOp._amin;

    maxValue.r() = rangeOp._rmax;
    maxValue.g() = rangeOp._gmax;
    maxValue.b() = rangeOp._bmax;
    maxValue.a() = rangeOp._amax;
    
    return minValue.r()<=maxValue.r() && 
           minValue.g()<=maxValue.g() &&
           minValue.b()<=maxValue.b() &&
           minValue.a()<=maxValue.a();
}

bool osgVolume::offsetAndScaleImage(osg::Image* image, const osg::Vec4& offset, const osg::Vec4& scale)
{
    if (!image) return false;

    osgVolume::modifyImage(image,osgVolume::OffsetAndScaleOperator(offset, scale));
    
    return true;
}



