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
#include <string.h>

#include <osg/Math>
#include <osg/Notify>
#include <osg/ImageUtils>

namespace osg
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

bool computeMinMax(const osg::Image* image, osg::Vec4& minValue, osg::Vec4& maxValue)
{
    if (!image) return false;

    osg::FindRangeOperator rangeOp;    
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

bool offsetAndScaleImage(osg::Image* image, const osg::Vec4& offset, const osg::Vec4& scale)
{
    if (!image) return false;

    osg::modifyImage(image,osg::OffsetAndScaleOperator(offset, scale));
    
    return true;
}

template<typename SRC, typename DEST>
void _copyRowAndScale(const SRC* src, DEST* dest, int num, float scale)
{
    if (scale==1.0)
    {
        for(int i=0; i<num; ++i)
        {
            *dest = DEST(*src);
            ++dest; ++src;
        }
    }
    else
    {
        for(int i=0; i<num; ++i)
        {
            *dest = DEST(float(*src)*scale);
            ++dest; ++src;
        }
    }
}

template<typename DEST>
void _copyRowAndScale(const unsigned char* src, GLenum srcDataType, DEST* dest, int num, float scale)
{
    switch(srcDataType)
    {
        case(GL_BYTE):              _copyRowAndScale((char*)src, dest, num, scale); break;
        case(GL_UNSIGNED_BYTE):     _copyRowAndScale((unsigned char*)src, dest, num, scale); break;
        case(GL_SHORT):             _copyRowAndScale((short*)src, dest, num, scale); break;
        case(GL_UNSIGNED_SHORT):    _copyRowAndScale((unsigned short*)src, dest, num, scale); break;
        case(GL_INT):               _copyRowAndScale((int*)src, dest, num, scale); break;
        case(GL_UNSIGNED_INT):      _copyRowAndScale((unsigned int*)src, dest, num, scale); break;
        case(GL_FLOAT):             _copyRowAndScale((float*)src, dest, num, scale); break;
    }
}

void _copyRowAndScale(const unsigned char* src, GLenum srcDataType, unsigned char* dest, GLenum dstDataType, int num, float scale)
{
    switch(dstDataType)
    {
        case(GL_BYTE):              _copyRowAndScale(src, srcDataType, (char*)dest, num, scale); break;
        case(GL_UNSIGNED_BYTE):     _copyRowAndScale(src, srcDataType, (unsigned char*)dest, num, scale); break;
        case(GL_SHORT):             _copyRowAndScale(src, srcDataType, (short*)dest, num, scale); break;
        case(GL_UNSIGNED_SHORT):    _copyRowAndScale(src, srcDataType, (unsigned short*)dest, num, scale); break;
        case(GL_INT):               _copyRowAndScale(src, srcDataType, (int*)dest, num, scale); break;
        case(GL_UNSIGNED_INT):      _copyRowAndScale(src, srcDataType, (unsigned int*)dest, num, scale); break;
        case(GL_FLOAT):             _copyRowAndScale(src, srcDataType, (float*)dest, num, scale); break;
    }
}

struct RecordRowOperator
{
    RecordRowOperator(unsigned int num):_colours(num),_pos(0) {}

    mutable std::vector<osg::Vec4>  _colours;
    mutable unsigned int            _pos;
    
    inline void luminance(float l) const { rgba(l,l,l,1.0f); } 
    inline void alpha(float a) const { rgba(1.0f,1.0f,1.0f,a); } 
    inline void luminance_alpha(float l,float a) const { rgba(l,l,l,a);  } 
    inline void rgb(float r,float g,float b) const { rgba(r,g,b,1.0f); }
    inline void rgba(float r,float g,float b,float a) const { _colours[_pos++].set(r,g,b,a); }
};

struct WriteRowOperator
{
    WriteRowOperator():_pos(0) {}
    WriteRowOperator(unsigned int num):_colours(num),_pos(0) {}

    std::vector<osg::Vec4>  _colours;
    mutable unsigned int    _pos;
    
    inline void luminance(float& l) const { l = _colours[_pos++].r(); } 
    inline void alpha(float& a) const { a = _colours[_pos++].a(); } 
    inline void luminance_alpha(float& l,float& a) const { l = _colours[_pos].r(); a = _colours[_pos++].a(); } 
    inline void rgb(float& r,float& g,float& b) const { r = _colours[_pos].r(); g = _colours[_pos].g(); b = _colours[_pos].b(); }
    inline void rgba(float& r,float& g,float& b,float& a) const {  r = _colours[_pos].r(); g = _colours[_pos].g(); b = _colours[_pos].b(); a = _colours[_pos++].a(); }
};

bool copyImage(const osg::Image* srcImage, int src_s, int src_t, int src_r, int width, int height, int depth,
                          osg::Image* destImage, int dest_s, int dest_t, int dest_r, bool doRescale)
{
    if ((src_s+width) > (dest_s + destImage->s()))
    {
        osg::notify(osg::NOTICE)<<"copyImage("<<srcImage<<", "<<src_s<<", "<< src_t<<", "<<src_r<<", "<<width<<", "<<height<<", "<<depth<<std::endl;
        osg::notify(osg::NOTICE)<<"          "<<destImage<<", "<<dest_s<<", "<< dest_t<<", "<<dest_r<<", "<<doRescale<<")"<<std::endl;
        osg::notify(osg::NOTICE)<<"   input width too large."<<std::endl;
        return false;
    }

    if ((src_t+height) > (dest_t + destImage->t()))
    {
        osg::notify(osg::NOTICE)<<"copyImage("<<srcImage<<", "<<src_s<<", "<< src_t<<", "<<src_r<<", "<<width<<", "<<height<<", "<<depth<<std::endl;
        osg::notify(osg::NOTICE)<<"          "<<destImage<<", "<<dest_s<<", "<< dest_t<<", "<<dest_r<<", "<<doRescale<<")"<<std::endl;
        osg::notify(osg::NOTICE)<<"   input height too large."<<std::endl;
        return false;
    }

    if ((src_r+depth) > (dest_r + destImage->r()))
    {
        osg::notify(osg::NOTICE)<<"copyImage("<<srcImage<<", "<<src_s<<", "<< src_t<<", "<<src_r<<", "<<width<<", "<<height<<", "<<depth<<std::endl;
        osg::notify(osg::NOTICE)<<"          "<<destImage<<", "<<dest_s<<", "<< dest_t<<", "<<dest_r<<", "<<doRescale<<")"<<std::endl;
        osg::notify(osg::NOTICE)<<"   input depth too large."<<std::endl;
        return false;
    }

    float scale = 1.0f;
    if (doRescale && srcImage->getDataType() != destImage->getDataType())
    {
        switch(srcImage->getDataType())
        {
            case(GL_BYTE):              scale = 1.0f/128.0f ; break;
            case(GL_UNSIGNED_BYTE):     scale = 1.0f/255.0f; break;
            case(GL_SHORT):             scale = 1.0f/32768.0f; break;
            case(GL_UNSIGNED_SHORT):    scale = 1.0f/65535.0f; break;
            case(GL_INT):               scale = 1.0f/2147483648.0f; break;
            case(GL_UNSIGNED_INT):      scale = 1.0f/4294967295.0f; break;
            case(GL_FLOAT):             scale = 1.0f; break;
        }
        switch(destImage->getDataType())
        {
            case(GL_BYTE):              scale *= 128.0f ; break;
            case(GL_UNSIGNED_BYTE):     scale *= 255.0f; break;
            case(GL_SHORT):             scale *= 32768.0f; break;
            case(GL_UNSIGNED_SHORT):    scale *= 65535.0f; break;
            case(GL_INT):               scale *= 2147483648.0f; break;
            case(GL_UNSIGNED_INT):      scale *= 4294967295.0f; break;
            case(GL_FLOAT):             scale *= 1.0f; break;
        }
    }

    if (srcImage->getPixelFormat() == destImage->getPixelFormat())
    {
        //osg::notify(osg::NOTICE)<<"copyImage("<<srcImage<<", "<<src_s<<", "<< src_t<<", "<<src_r<<", "<<width<<", "<<height<<", "<<depth<<std::endl;
        //osg::notify(osg::NOTICE)<<"          "<<destImage<<", "<<dest_s<<", "<< dest_t<<", "<<dest_r<<", "<<doRescale<<")"<<std::endl;

        if (srcImage->getDataType() == destImage->getDataType() && !doRescale)
        {
            //osg::notify(osg::NOTICE)<<"   Compatible pixelFormat and dataType."<<std::endl;
            for(int slice = 0; slice<depth; ++slice)
            {
                for(int row = 0; row<height; ++row)
                {
                    const unsigned char* srcData = srcImage->data(src_s, src_t+row, src_r+slice);
                    unsigned char* destData = destImage->data(dest_s, dest_t+row, dest_r+slice);
                    memcpy(destData, srcData, (width*destImage->getPixelSizeInBits())/8);
                }
            }
            return true;
        }
        else
        {
            //osg::notify(osg::NOTICE)<<"   Compatible pixelFormat and incompatible dataType."<<std::endl;
            for(int slice = 0; slice<depth; ++slice)
            {
                for(int row = 0; row<height; ++row)
                {
                    const unsigned char* srcData = srcImage->data(src_s, src_t+row, src_r+slice);
                    unsigned char* destData = destImage->data(dest_s, dest_t+row, dest_r+slice);
                    unsigned int numComponents = osg::Image::computeNumComponents(destImage->getPixelFormat());
                    
                    _copyRowAndScale(srcData, srcImage->getDataType(), destData, destImage->getDataType(), (width*numComponents), scale);
                }
            }
            
            return true;
        }
    }
    else
    {
        osg::notify(osg::NOTICE)<<"copyImage("<<srcImage<<", "<<src_s<<", "<< src_t<<", "<<src_r<<", "<<width<<", "<<height<<", "<<depth<<std::endl;
        osg::notify(osg::NOTICE)<<"          "<<destImage<<", "<<dest_s<<", "<< dest_t<<", "<<dest_r<<", "<<doRescale<<")"<<std::endl;
                
        RecordRowOperator readOp(width);
        WriteRowOperator writeOp;

        for(int slice = 0; slice<depth; ++slice)
        {
            for(int row = 0; row<height; ++row)
            {

                // reset the indices to beginning
                readOp._pos = 0;
                writeOp._pos = 0;
            
                // read the pixels into readOp's _colour array
                osg::readRow(width, srcImage->getPixelFormat(), srcImage->getDataType(), srcImage->data(src_s,src_t+row,src_r+slice), readOp);
                                
                // pass readOp's _colour array contents over to writeOp (note this is just a pointer swap).
                writeOp._colours.swap(readOp._colours);
                
                osg::modifyRow(width, destImage->getPixelFormat(), destImage->getDataType(), destImage->data(dest_s, dest_t+row,dest_r+slice), writeOp);

                // return readOp's _colour array contents back to its rightful owner.
                writeOp._colours.swap(readOp._colours);
            }
        }
        
        return false;
    }

}


struct SetToColourOperator
{
    SetToColourOperator(const osg::Vec4& colour):
        _colour(colour) {}

    inline void luminance(float& l) const { l = (_colour.r()+_colour.g()+_colour.b())*0.333333; } 
    inline void alpha(float& a) const { a = _colour.a(); } 
    inline void luminance_alpha(float& l,float& a) const { l = (_colour.r()+_colour.g()+_colour.b())*0.333333; a = _colour.a(); }
    inline void rgb(float& r,float& g,float& b) const { r = _colour.r(); g = _colour.g(); b = _colour.b(); }
    inline void rgba(float& r,float& g,float& b,float& a) const { r = _colour.r(); g = _colour.g(); b = _colour.b(); a = _colour.a(); }

    osg::Vec4 _colour;
};

bool clearImageToColor(osg::Image* image, const osg::Vec4& colour)
{
    if (!image) return false;

    modifyImage(image, SetToColourOperator(colour));
    
    return true;
}

}

