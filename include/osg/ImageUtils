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

#ifndef OSG_IMAGEUTILS
#define OSG_IMAGEUTILS 1

#include <osg/Export>

#include <osg/Image>
#include <osg/Vec3i>

namespace osg {

template <typename T, class O>
void _readRow(unsigned int num, GLenum pixelFormat, const T* data, O& operation)
{
    switch(pixelFormat)
    {
        case(GL_INTENSITY):         { for(unsigned int i=0;i<num;++i) { T v=*data++; operation.rgba( operation.cast(v),operation.cast(v),operation.cast(v),operation.cast(v)); } }  break;
        case(GL_LUMINANCE):         { for(unsigned int i=0;i<num;++i) { operation.luminance(operation.cast(*data++)); } }  break;
        case(GL_ALPHA):             { for(unsigned int i=0;i<num;++i) { operation.alpha(operation.cast(*data++)); } }  break;
        case(GL_LUMINANCE_ALPHA):   { for(unsigned int i=0;i<num;++i) { T l=*data++; T a = *data++; operation.luminance_alpha(operation.cast(l),operation.cast(a)); } }  break;
        case(GL_RGB):               { for(unsigned int i=0;i<num;++i) { T r=*data++; T g=*data++; T b=*data++; operation.rgb(operation.cast(r),operation.cast(g),operation.cast(b)); } } break;
        case(GL_RGBA):              { for(unsigned int i=0;i<num;++i) { T r=*data++; T g=*data++; T b=*data++; T a=*data++; operation.rgba(operation.cast(r),operation.cast(g),operation.cast(b),operation.cast(a)); } } break;
        case(GL_BGR):               { for(unsigned int i=0;i<num;++i) { T b=*data++; T g=*data++; T r=*data++; operation.rgb(operation.cast(r),operation.cast(g),operation.cast(b)); } }  break;
        case(GL_BGRA):              { for(unsigned int i=0;i<num;++i) { T b=*data++; T g=*data++; T r=*data++; T a=*data++; operation.rgba(operation.cast(r),operation.cast(g),operation.cast(b),operation.cast(a)); } }  break;
    }
}


template <class O>
void readRow(unsigned int num, GLenum pixelFormat, GLenum dataType, const unsigned char* data, O& operation)
{
    switch(dataType)
    {
        case(GL_BYTE):              _readRow(num, pixelFormat, (const char*)data,            operation); break;
        case(GL_UNSIGNED_BYTE):     _readRow(num, pixelFormat, (const unsigned char*)data,   operation); break;
        case(GL_SHORT):             _readRow(num, pixelFormat, (const short*) data,          operation); break;
        case(GL_UNSIGNED_SHORT):    _readRow(num, pixelFormat, (const unsigned short*)data,  operation); break;
        case(GL_INT):               _readRow(num, pixelFormat, (const int*) data,            operation); break;
        case(GL_UNSIGNED_INT):      _readRow(num, pixelFormat, (const unsigned int*) data,   operation); break;
        case(GL_FLOAT):             _readRow(num, pixelFormat, (const float*) data,          operation); break;
        case(GL_DOUBLE):            _readRow(num, pixelFormat, (const double*) data,         operation); break;
    }
}


template <class O>
void readImage(const osg::Image* image, O& operation)
{
    if (!image) return;

    for(int r=0;r<image->r();++r)
    {
        for(int t=0;t<image->t();++t)
        {
            readRow(image->s(), image->getPixelFormat(), image->getDataType(), image->data(0,t,r), operation);
        }
    }
}

/** Convenience method for making it easy to cast all pixel channels types to a unit float RGBA form.*/
struct CastAndScaleToFloatOperation
{
    float cast(char v) { return static_cast<float>(v)*(1.0f/128.0f); }
    float cast(unsigned char v) { return static_cast<float>(v)*(1.0f/255.0f); }
    float cast(short v) { return static_cast<float>(v)*(1.0f/32768.0f); }
    float cast(unsigned short v) { return static_cast<float>(v)*(1.0f/65535.0f); }
    float cast(int v) { return static_cast<float>(v)*(1.0f/2147483648.0f); }
    float cast(unsigned int v) { return static_cast<float>(v)*(1.0f/4294967295.0f); }
    float cast(float v) { return v; }
    float cast(double v) { return static_cast<double>(v); }
};

#if 0
template <typename T, class O>
void _readRow(unsigned int num, GLenum pixelFormat, const T* data,float scale, O& operation)
{
    switch(pixelFormat)
    {
        case(GL_LUMINANCE):         { for(unsigned int i=0;i<num;++i) { float l = float(*data++)*scale; operation.luminance(l); } }  break;
        case(GL_ALPHA):             { for(unsigned int i=0;i<num;++i) { float a = float(*data++)*scale; operation.alpha(a); } }  break;
        case(GL_LUMINANCE_ALPHA):   { for(unsigned int i=0;i<num;++i) { float l = float(*data++)*scale; float a = float(*data++)*scale; operation.luminance_alpha(l,a); } }  break;
        case(GL_RGB):               { for(unsigned int i=0;i<num;++i) { float r = float(*data++)*scale; float g = float(*data++)*scale; float b = float(*data++)*scale; operation.rgb(r,g,b); } }  break;
        case(GL_RGBA):              { for(unsigned int i=0;i<num;++i) { float r = float(*data++)*scale; float g = float(*data++)*scale; float b = float(*data++)*scale; float a = float(*data++)*scale; operation.rgba(r,g,b,a); } }  break;
        case(GL_BGR):               { for(unsigned int i=0;i<num;++i) { float b = float(*data++)*scale; float g = float(*data++)*scale; float r = float(*data++)*scale; operation.rgb(r,g,b); } }  break;
        case(GL_BGRA):              { for(unsigned int i=0;i<num;++i) { float b = float(*data++)*scale; float g = float(*data++)*scale; float r = float(*data++)*scale; float a = float(*data++)*scale; operation.rgba(r,g,b,a); } }  break;
    }
}

template <class O>
void readRow(unsigned int num, GLenum pixelFormat, GLenum dataType, const unsigned char* data, O& operation)
{
    switch(dataType)
    {
        case(GL_BYTE):              _readRow(num,pixelFormat, (const char*)data,            1.0f/128.0f,        operation); break;
        case(GL_UNSIGNED_BYTE):     _readRow(num,pixelFormat, (const unsigned char*)data,   1.0f/255.0f,        operation); break;
        case(GL_SHORT):             _readRow(num,pixelFormat, (const short*) data,          1.0f/32768.0f,      operation); break;
        case(GL_UNSIGNED_SHORT):    _readRow(num,pixelFormat, (const unsigned short*)data,  1.0f/65535.0f,      operation); break;
        case(GL_INT):               _readRow(num,pixelFormat, (const int*) data,            1.0f/2147483648.0f, operation); break;
        case(GL_UNSIGNED_INT):      _readRow(num,pixelFormat, (const unsigned int*) data,   1.0f/4294967295.0f, operation); break;
        case(GL_FLOAT):             _readRow(num,pixelFormat, (const float*) data,          1.0f,               operation); break;
    }
}

template <class O>
void readImage(const osg::Image* image, O& operation)
{
    if (!image) return;

    for(int r=0;r<image->r();++r)
    {
        for(int t=0;t<image->t();++t)
        {
            readRow(image->s(), image->getPixelFormat(), image->getDataType(), image->data(0,t,r), operation);
        }
    }
}
#endif



// example ModifyOperator
// struct ModifyOperator
// {
//     inline void luminance(float& l) const {}
//     inline void alpha(float& a) const {}
//     inline void luminance_alpha(float& l,float& a) const {}
//     inline void rgb(float& r,float& g,float& b) const {}
//     inline void rgba(float& r,float& g,float& b,float& a) const {}
// };


template <typename T, class M>
void _modifyRow(unsigned int num, GLenum pixelFormat, T* data,float scale, const M& operation)
{
    float inv_scale = 1.0f/scale;
    switch(pixelFormat)
    {
        case(GL_LUMINANCE):         { for(unsigned int i=0;i<num;++i) { float l = float(*data)*scale; operation.luminance(l); *data++ = T(l*inv_scale); } }  break;
        case(GL_ALPHA):             { for(unsigned int i=0;i<num;++i) { float a = float(*data)*scale; operation.alpha(a); *data++ = T(a*inv_scale); } }  break;
        case(GL_LUMINANCE_ALPHA):   { for(unsigned int i=0;i<num;++i) { float l = float(*data)*scale; float a = float(*(data+1))*scale; operation.luminance_alpha(l,a); *data++ = T(l*inv_scale); *data++ = T(a*inv_scale); } }  break;
        case(GL_RGB):               { for(unsigned int i=0;i<num;++i) { float r = float(*data)*scale; float g = float(*(data+1))*scale; float b = float(*(data+2))*scale; operation.rgb(r,g,b); *data++ = T(r*inv_scale); *data++ = T(g*inv_scale); *data++ = T(b*inv_scale); } }  break;
        case(GL_RGBA):              { for(unsigned int i=0;i<num;++i) { float r = float(*data)*scale; float g = float(*(data+1))*scale; float b = float(*(data+2))*scale; float a = float(*(data+3))*scale; operation.rgba(r,g,b,a); *data++ = T(r*inv_scale); *data++ = T(g*inv_scale); *data++ = T(b*inv_scale); *data++ = T(a*inv_scale); } }  break;
        case(GL_BGR):               { for(unsigned int i=0;i<num;++i) { float b = float(*data)*scale; float g = float(*(data+1))*scale; float r = float(*(data+2))*scale; operation.rgb(r,g,b); *data++ = T(b*inv_scale); *data++ = T(g*inv_scale); *data++ = T(r*inv_scale); } }  break;
        case(GL_BGRA):              { for(unsigned int i=0;i<num;++i) { float b = float(*data)*scale; float g = float(*(data+1))*scale; float r = float(*(data+2))*scale; float a = float(*(data+3))*scale; operation.rgba(r,g,b,a); *data++ = T(b*inv_scale); *data++ = T(g*inv_scale); *data++ = T(r*inv_scale); *data++ = T(a*inv_scale); } }  break;
    }
}

template <class M>
void modifyRow(unsigned int num, GLenum pixelFormat, GLenum dataType, unsigned char* data, const M& operation)
{
    switch(dataType)
    {
        case(GL_BYTE):              _modifyRow(num,pixelFormat, (char*)data,            1.0f/128.0f,        operation); break;
        case(GL_UNSIGNED_BYTE):     _modifyRow(num,pixelFormat, (unsigned char*)data,   1.0f/255.0f,        operation); break;
        case(GL_SHORT):             _modifyRow(num,pixelFormat, (short*) data,          1.0f/32768.0f,      operation); break;
        case(GL_UNSIGNED_SHORT):    _modifyRow(num,pixelFormat, (unsigned short*)data,  1.0f/65535.0f,      operation); break;
        case(GL_INT):               _modifyRow(num,pixelFormat, (int*) data,            1.0f/2147483648.0f, operation); break;
        case(GL_UNSIGNED_INT):      _modifyRow(num,pixelFormat, (unsigned int*) data,   1.0f/4294967295.0f, operation); break;
        case(GL_FLOAT):             _modifyRow(num,pixelFormat, (float*) data,          1.0f,               operation); break;
    }
}

template <class M>
void modifyImage(osg::Image* image, const M& operation)
{
    if (!image) return;

    for(int r=0;r<image->r();++r)
    {
        for(int t=0;t<image->t();++t)
        {
            modifyRow(image->s(), image->getPixelFormat(), image->getDataType(), image->data(0,t,r), operation);
        }
    }
}

/** Compute the min max colour values in the image.*/
extern OSG_EXPORT bool computeMinMax(const osg::Image* image, osg::Vec4& min, osg::Vec4& max);

/** Compute the min max colour values in the image.*/
extern OSG_EXPORT bool offsetAndScaleImage(osg::Image* image, const osg::Vec4& offset, const osg::Vec4& scale);

/** Compute source image to destination image.*/
extern OSG_EXPORT bool copyImage(const osg::Image* srcImage, int src_s, int src_t, int src_r, int width, int height, int depth,
                                       osg::Image* destImage, int dest_s, int dest_t, int dest_r, bool doRescale = false);

/** Compute the min max colour values in the image.*/
extern OSG_EXPORT bool clearImageToColor(osg::Image* image, const osg::Vec4& colour);

typedef std::vector< osg::ref_ptr<osg::Image> > ImageList;

/** Search through the list of Images and find the maximum number of components used among the images.*/
extern OSG_EXPORT unsigned int maximimNumOfComponents(const ImageList& imageList);

/** create a 3D osg::Image from a list of osg::Image.*/
extern OSG_EXPORT osg::Image* createImage3D(const ImageList& imageList,
            GLenum desiredPixelFormat,
            int s_maximumImageSize = 1024,
            int t_maximumImageSize = 1024,
            int r_maximumImageSize = 1024,
            bool resizeToPowerOfTwo = false);

/** create a 3D osg::Image from a list of osg::Image.*/
extern OSG_EXPORT osg::Image* createImage3DWithAlpha(const ImageList& imageList,
            int s_maximumImageSize = 1024,
            int t_maximumImageSize = 1024,
            int r_maximumImageSize = 1024,
            bool resizeToPowerOfTwo = false);




/** create a 2D osg::Image that provides a point at the center of the image.
 *  The colour across th image is computed from a balance between the center color and the background color controlled by the power of the radius from the center.*/
extern OSG_EXPORT osg::Image* createSpotLightImage(const osg::Vec4& centerColour, const osg::Vec4& backgroudColour, unsigned int size, float power);


enum ColorSpaceOperation
{
    NO_COLOR_SPACE_OPERATION,
    MODULATE_ALPHA_BY_LUMINANCE,
    MODULATE_ALPHA_BY_COLOR,
    REPLACE_ALPHA_WITH_LUMINANCE,
    REPLACE_RGB_WITH_LUMINANCE
};

/** Convert the RGBA values in a Image based on a ColorSpaceOperation defined scheme.*/
extern OSG_EXPORT osg::Image* colorSpaceConversion(ColorSpaceOperation op, osg::Image* image, const osg::Vec4& colour);

/** Create a copy of an osg::Image. converting the origin and orientation to standard lower left OpenGL style origin .*/
extern OSG_EXPORT osg::Image* createImageWithOrientationConversion(const osg::Image* srcImage, const osg::Vec3i& srcOrigin, const osg::Vec3i& srcRow, const osg::Vec3i& srcColumn, const osg::Vec3i& srcLayer);

}


#endif
