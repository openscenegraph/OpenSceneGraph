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
#include <osg/GL>
#include <osg/GLU>

#include <osg/Image>
#include <osg/Notify>
#include <osg/io_utils>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/Texture2DArray>

#include <string.h>
#include <stdlib.h>

#include "dxtctool.h"

using namespace osg;
using namespace std;

Image::Image()
    :Object(true),
    _fileName(""),
    _writeHint(NO_PREFERENCE),
    _origin(BOTTOM_LEFT),
    _s(0), _t(0), _r(0),
    _internalTextureFormat(0),
    _pixelFormat(0),
    _dataType(0),
    _packing(4),
    _pixelAspectRatio(1.0),
    _allocationMode(USE_NEW_DELETE),
    _data(0L),
    _modifiedCount(0)
{
    setDataVariance(STATIC); 
}

Image::Image(const Image& image,const CopyOp& copyop):
    Object(image,copyop),
    _fileName(image._fileName),
    _writeHint(image._writeHint),
    _origin(image._origin),
    _s(image._s), _t(image._t), _r(image._r),
    _internalTextureFormat(image._internalTextureFormat),
    _pixelFormat(image._pixelFormat),
    _dataType(image._dataType),
    _packing(image._packing),
    _pixelAspectRatio(image._pixelAspectRatio),
    _data(0L),
    _modifiedCount(image._modifiedCount),
    _mipmapData(image._mipmapData)
{
    if (image._data)
    {
        int size = image.getTotalSizeInBytesIncludingMipmaps();
        setData(new unsigned char [size],USE_NEW_DELETE);
        memcpy(_data,image._data,size);
    }

}

Image::~Image()
{
    deallocateData();
}

void Image::deallocateData()
{
    if (_data) {
        if (_allocationMode==USE_NEW_DELETE) delete [] _data;
        else if (_allocationMode==USE_MALLOC_FREE) ::free(_data);
        _data = 0;
    }
}

int Image::compare(const Image& rhs) const
{
    // if at least one filename is empty, then need to test buffer
    // pointers because images could have been created on the fly
    // and therefore we can't rely on file names to get an accurate
    // comparison
    if (getFileName().empty() || rhs.getFileName().empty())
    {
        if (_data<rhs._data) return -1;
        if (_data>rhs._data) return 1;
    }

    // need to test against image contents here...
    COMPARE_StateAttribute_Parameter(_s)
    COMPARE_StateAttribute_Parameter(_t)
    COMPARE_StateAttribute_Parameter(_internalTextureFormat)
    COMPARE_StateAttribute_Parameter(_pixelFormat)
    COMPARE_StateAttribute_Parameter(_dataType)
    COMPARE_StateAttribute_Parameter(_packing)
    COMPARE_StateAttribute_Parameter(_mipmapData)
    COMPARE_StateAttribute_Parameter(_modifiedCount)

    // same buffer + same parameters = same image
    if ((_data || rhs._data) && (_data == rhs._data)) return 0;

    // slowest comparison at the bottom!
    COMPARE_StateAttribute_Parameter(getFileName())    

    return 0;
}

void Image::setFileName(const std::string& fileName)
{
    _fileName = fileName;
}

void Image::setData(unsigned char* data, AllocationMode mode)
{
    deallocateData();
    _data = data;
    _allocationMode = mode;
}


bool Image::isPackedType(GLenum type)
{
    switch(type)
    {
        case(GL_UNSIGNED_BYTE_3_3_2):
        case(GL_UNSIGNED_BYTE_2_3_3_REV):
        case(GL_UNSIGNED_SHORT_5_6_5):
        case(GL_UNSIGNED_SHORT_5_6_5_REV):
        case(GL_UNSIGNED_SHORT_4_4_4_4):
        case(GL_UNSIGNED_SHORT_4_4_4_4_REV):
        case(GL_UNSIGNED_SHORT_5_5_5_1):
        case(GL_UNSIGNED_SHORT_1_5_5_5_REV):
        case(GL_UNSIGNED_INT_8_8_8_8):
        case(GL_UNSIGNED_INT_8_8_8_8_REV):
        case(GL_UNSIGNED_INT_10_10_10_2):
        case(GL_UNSIGNED_INT_2_10_10_10_REV): return true;
        default: return false;
    }    
}


GLenum Image::computePixelFormat(GLenum format)
{
    switch(format)
    {
        case(GL_ALPHA16F_ARB):
        case(GL_ALPHA32F_ARB):
            return GL_ALPHA;
        case(GL_LUMINANCE16F_ARB):
        case(GL_LUMINANCE32F_ARB):
            return GL_LUMINANCE;
        case(GL_INTENSITY16F_ARB):
        case(GL_INTENSITY32F_ARB):
            return GL_INTENSITY;
        case(GL_LUMINANCE_ALPHA16F_ARB):
        case(GL_LUMINANCE_ALPHA32F_ARB):
            return GL_LUMINANCE_ALPHA;
        case(GL_RGB32F_ARB):
        case(GL_RGB16F_ARB):
            return GL_RGB;
        case(GL_RGBA32F_ARB):
        case(GL_RGBA16F_ARB):
            return GL_RGBA;

        case(GL_ALPHA8I_EXT):
        case(GL_ALPHA16I_EXT):
        case(GL_ALPHA32I_EXT):
        case(GL_ALPHA8UI_EXT):
        case(GL_ALPHA16UI_EXT):
        case(GL_ALPHA32UI_EXT):
            return GL_ALPHA_INTEGER_EXT;
        case(GL_LUMINANCE8I_EXT):
        case(GL_LUMINANCE16I_EXT):
        case(GL_LUMINANCE32I_EXT):
        case(GL_LUMINANCE8UI_EXT):
        case(GL_LUMINANCE16UI_EXT):
        case(GL_LUMINANCE32UI_EXT):
            return GL_LUMINANCE_INTEGER_EXT;
        case(GL_INTENSITY8I_EXT):
        case(GL_INTENSITY16I_EXT):
        case(GL_INTENSITY32I_EXT):
        case(GL_INTENSITY8UI_EXT):
        case(GL_INTENSITY16UI_EXT):
        case(GL_INTENSITY32UI_EXT):
            notify(WARN)<<"Image::computePixelFormat("<<std::hex<<format<<std::dec<<") intensity pixel format is not correctly specified, so assume GL_LUMINANCE_INTEGER."<<std::endl;            
            return GL_LUMINANCE_INTEGER_EXT;
        case(GL_LUMINANCE_ALPHA8I_EXT):
        case(GL_LUMINANCE_ALPHA16I_EXT):
        case(GL_LUMINANCE_ALPHA32I_EXT):
        case(GL_LUMINANCE_ALPHA8UI_EXT):
        case(GL_LUMINANCE_ALPHA16UI_EXT):
        case(GL_LUMINANCE_ALPHA32UI_EXT):
            return GL_LUMINANCE_ALPHA_INTEGER_EXT;
        case(GL_RGB32I_EXT):
        case(GL_RGB16I_EXT):
        case(GL_RGB8I_EXT):
        case(GL_RGB32UI_EXT):
        case(GL_RGB16UI_EXT):
        case(GL_RGB8UI_EXT):
            return GL_RGB_INTEGER_EXT;
        case(GL_RGBA32I_EXT):
        case(GL_RGBA16I_EXT):
        case(GL_RGBA8I_EXT):
        case(GL_RGBA32UI_EXT):
        case(GL_RGBA16UI_EXT):
        case(GL_RGBA8UI_EXT):
            return GL_RGBA_INTEGER_EXT;;

        default:
            return format;
    }
}

GLenum Image::computeFormatDataType(GLenum pixelFormat)
{
    switch (pixelFormat)
    {
        case GL_LUMINANCE32F_ARB:
        case GL_LUMINANCE16F_ARB: 
        case GL_LUMINANCE_ALPHA32F_ARB:
        case GL_LUMINANCE_ALPHA16F_ARB: 
        case GL_RGB32F_ARB:
        case GL_RGB16F_ARB: 
        case GL_RGBA32F_ARB:
        case GL_RGBA16F_ARB: return GL_FLOAT;

        case GL_RGBA32UI_EXT:
        case GL_RGB32UI_EXT:
        case GL_LUMINANCE32UI_EXT:
        case GL_LUMINANCE_ALPHA32UI_EXT: return GL_UNSIGNED_INT;

        case GL_RGB16UI_EXT:
        case GL_RGBA16UI_EXT:
        case GL_LUMINANCE16UI_EXT: 
        case GL_LUMINANCE_ALPHA16UI_EXT: return GL_UNSIGNED_SHORT;

        case GL_RGBA8UI_EXT:
        case GL_RGB8UI_EXT:
        case GL_LUMINANCE8UI_EXT:
        case GL_LUMINANCE_ALPHA8UI_EXT:  return GL_UNSIGNED_BYTE;

        case GL_RGBA32I_EXT:  
        case GL_RGB32I_EXT:
        case GL_LUMINANCE32I_EXT:
        case GL_LUMINANCE_ALPHA32I_EXT: return GL_INT;

        case GL_RGBA16I_EXT:
        case GL_RGB16I_EXT:
        case GL_LUMINANCE16I_EXT:
        case GL_LUMINANCE_ALPHA16I_EXT: return GL_SHORT;

        case GL_RGB8I_EXT: 
        case GL_RGBA8I_EXT: 
        case GL_LUMINANCE8I_EXT: 
        case GL_LUMINANCE_ALPHA8I_EXT: return GL_BYTE;

        case GL_RGBA:
        case GL_RGB:
        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA: return GL_UNSIGNED_BYTE;

        default: 
        {
            notify(WARN)<<"error computeFormatType = "<<std::hex<<pixelFormat<<std::dec<<std::endl;
            return 0;
        }
    }
}

unsigned int Image::computeNumComponents(GLenum pixelFormat)
{
    switch(pixelFormat)
    {
        case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT): return 3;
        case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT): return 4;
        case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT): return 4;
        case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT): return 4;
        case(GL_COLOR_INDEX): return 1;
        case(GL_STENCIL_INDEX): return 1;
        case(GL_DEPTH_COMPONENT): return 1;
        case(GL_RED): return 1;
        case(GL_GREEN): return 1;
        case(GL_BLUE): return 1;
        case(GL_ALPHA): return 1;
        case(GL_ALPHA8I_EXT): return 1;
        case(GL_ALPHA8UI_EXT): return 1;
        case(GL_ALPHA16I_EXT): return 1;
        case(GL_ALPHA16UI_EXT): return 1;
        case(GL_ALPHA32I_EXT): return 1;
        case(GL_ALPHA32UI_EXT): return 1;
        case(GL_ALPHA16F_ARB): return 1;
        case(GL_ALPHA32F_ARB): return 1;
        case(GL_RGB): return 3;
        case(GL_BGR): return 3;
        case(GL_RGB8I_EXT): return 3;
        case(GL_RGB8UI_EXT): return 3;
        case(GL_RGB16I_EXT): return 3;
        case(GL_RGB16UI_EXT): return 3;
        case(GL_RGB32I_EXT): return 3;
        case(GL_RGB32UI_EXT): return 3;
        case(GL_RGB16F_ARB): return 3;
        case(GL_RGB32F_ARB): return 3;
        case(GL_RGBA16F_ARB): return 4;
        case(GL_RGBA32F_ARB): return 4;
        case(GL_RGBA): return 4;
        case(GL_BGRA): return 4;
        case(GL_RGBA8): return 4;
        case(GL_LUMINANCE): return 1;
        case(GL_LUMINANCE4): return 1;
        case(GL_LUMINANCE8): return 1;
        case(GL_LUMINANCE12): return 1;
        case(GL_LUMINANCE16): return 1;
        case(GL_LUMINANCE8I_EXT): return 1;
        case(GL_LUMINANCE8UI_EXT): return 1;
        case(GL_LUMINANCE16I_EXT): return 1;
        case(GL_LUMINANCE16UI_EXT): return 1;
        case(GL_LUMINANCE32I_EXT): return 1;
        case(GL_LUMINANCE32UI_EXT): return 1;
        case(GL_LUMINANCE16F_ARB): return 1;
        case(GL_LUMINANCE32F_ARB): return 1;
        case(GL_LUMINANCE4_ALPHA4): return 2;
        case(GL_LUMINANCE6_ALPHA2): return 2;
        case(GL_LUMINANCE8_ALPHA8): return 2;
        case(GL_LUMINANCE12_ALPHA4): return 2;
        case(GL_LUMINANCE12_ALPHA12): return 2;
        case(GL_LUMINANCE16_ALPHA16): return 2;
        case(GL_INTENSITY): return 1;
        case(GL_INTENSITY4): return 1;
        case(GL_INTENSITY8): return 1;
        case(GL_INTENSITY12): return 1;
        case(GL_INTENSITY16): return 1;
        case(GL_INTENSITY8UI_EXT): return 1;
        case(GL_INTENSITY8I_EXT): return 1;
        case(GL_INTENSITY16I_EXT): return 1;
        case(GL_INTENSITY16UI_EXT): return 1;
        case(GL_INTENSITY32I_EXT): return 1;
        case(GL_INTENSITY32UI_EXT): return 1;
        case(GL_INTENSITY16F_ARB): return 1;
        case(GL_INTENSITY32F_ARB): return 1;
        case(GL_LUMINANCE_ALPHA): return 2;
        case(GL_LUMINANCE_ALPHA8I_EXT): return 2;
        case(GL_LUMINANCE_ALPHA8UI_EXT): return 2;
        case(GL_LUMINANCE_ALPHA16I_EXT): return 2;
        case(GL_LUMINANCE_ALPHA16UI_EXT): return 2;
        case(GL_LUMINANCE_ALPHA32I_EXT): return 2;
        case(GL_LUMINANCE_ALPHA32UI_EXT): return 2;
        case(GL_LUMINANCE_ALPHA16F_ARB): return 2;
        case(GL_LUMINANCE_ALPHA32F_ARB): return 2;
        case(GL_HILO_NV): return 2;
        case(GL_DSDT_NV): return 2;
        case(GL_DSDT_MAG_NV): return 3;
        case(GL_DSDT_MAG_VIB_NV): return 4;
        case(GL_RED_INTEGER_EXT): return 1;
        case(GL_GREEN_INTEGER_EXT): return 1;
        case(GL_BLUE_INTEGER_EXT): return 1;
        case(GL_ALPHA_INTEGER_EXT): return 1;
        case(GL_RGB_INTEGER_EXT): return 3;
        case(GL_RGBA_INTEGER_EXT): return 4;
        case(GL_BGR_INTEGER_EXT): return 3;
        case(GL_BGRA_INTEGER_EXT): return 4;
        case(GL_LUMINANCE_INTEGER_EXT): return 1;
        case(GL_LUMINANCE_ALPHA_INTEGER_EXT): return 2;

        default:
        {
            notify(WARN)<<"error pixelFormat = "<<std::hex<<pixelFormat<<std::dec<<std::endl;
            return 0;
        }
    }        
}


unsigned int Image::computePixelSizeInBits(GLenum format,GLenum type)
{

    switch(format)
    {
        case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT): return 4;
        case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT): return 4;
        case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT): return 8;
        case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT): return 8;
        default: break;
    }

    // note, haven't yet added proper handling of the ARB GL_COMPRESSRED_* pathways
    // yet, no clear size for these since its probably implementation dependent
    // which raises the question of how to actually querry for these sizes...
    // will need to revisit this issue, for now just report an error.
    // this is possible a bit of mute point though as since the ARB compressed formats
    // arn't yet used for storing images to disk, so its likely that users wont have
    // osg::Image's for pixel formats set the ARB compressed formats, just using these
    // compressed formats as internal texture modes.  This is very much speculation though
    // if get the below error then its time to revist this issue :-)
    // Robert Osfield, Jan 2005.
    switch(format)
    {
        case(GL_COMPRESSED_ALPHA):
        case(GL_COMPRESSED_LUMINANCE):
        case(GL_COMPRESSED_LUMINANCE_ALPHA):
        case(GL_COMPRESSED_INTENSITY):
        case(GL_COMPRESSED_RGB):
        case(GL_COMPRESSED_RGBA):
            notify(WARN)<<"Image::computePixelSizeInBits(format,type) : cannot compute correct size of compressed format ("<<format<<") returning 0."<<std::endl;
            return 0;
        default: break;
    }

    switch(format)
    {
        case(GL_LUMINANCE4): return 4;
        case(GL_LUMINANCE8): return 8;
        case(GL_LUMINANCE12): return 12;
        case(GL_LUMINANCE16): return 16;
        case(GL_LUMINANCE4_ALPHA4): return 8;
        case(GL_LUMINANCE6_ALPHA2): return 8;
        case(GL_LUMINANCE8_ALPHA8): return 16;
        case(GL_LUMINANCE12_ALPHA4): return 16;
        case(GL_LUMINANCE12_ALPHA12): return 24;
        case(GL_LUMINANCE16_ALPHA16): return 32;
        case(GL_INTENSITY4): return 4;
        case(GL_INTENSITY8): return 8;
        case(GL_INTENSITY12): return 12;
        case(GL_INTENSITY16): return 16;
        default: break;
    }

    switch(type)
    {
   
        case(GL_BITMAP): return computeNumComponents(format);
        
        case(GL_BYTE):
        case(GL_UNSIGNED_BYTE): return 8*computeNumComponents(format);
        
        case(GL_HALF_FLOAT_NV):
        case(GL_SHORT):
        case(GL_UNSIGNED_SHORT): return 16*computeNumComponents(format);
        
        case(GL_INT):
        case(GL_UNSIGNED_INT):
        case(GL_FLOAT): return 32*computeNumComponents(format);
    
    
        case(GL_UNSIGNED_BYTE_3_3_2): 
        case(GL_UNSIGNED_BYTE_2_3_3_REV): return 8;
        
        case(GL_UNSIGNED_SHORT_5_6_5):
        case(GL_UNSIGNED_SHORT_5_6_5_REV):
        case(GL_UNSIGNED_SHORT_4_4_4_4):
        case(GL_UNSIGNED_SHORT_4_4_4_4_REV):
        case(GL_UNSIGNED_SHORT_5_5_5_1):
        case(GL_UNSIGNED_SHORT_1_5_5_5_REV): return 16;
        
        case(GL_UNSIGNED_INT_8_8_8_8):
        case(GL_UNSIGNED_INT_8_8_8_8_REV):
        case(GL_UNSIGNED_INT_10_10_10_2):
        case(GL_UNSIGNED_INT_2_10_10_10_REV): return 32;
        default: 
        {
            notify(WARN)<<"error type = "<<type<<std::endl;
            return 0;
        }
    }    

}

unsigned int Image::computeRowWidthInBytes(int width,GLenum pixelFormat,GLenum type,int packing)
{
    unsigned int pixelSize = computePixelSizeInBits(pixelFormat,type);
    int widthInBits = width*pixelSize;
    int packingInBits = packing*8;
    //notify(INFO) << "width="<<width<<" pixelSize="<<pixelSize<<"  width in bit="<<widthInBits<<" packingInBits="<<packingInBits<<" widthInBits%packingInBits="<<widthInBits%packingInBits<<std::endl;
    return (widthInBits/packingInBits + ((widthInBits%packingInBits)?1:0))*packing;
}

int Image::computeNearestPowerOfTwo(int s,float bias)
{
    if ((s & (s-1))!=0)
    {
        // it isn't so lets find the closest power of two.
        // yes, logf and powf are slow, but this code should
        // only be called during scene graph initilization,
        // if at all, so not critical in the greater scheme.
        float p2 = logf((float)s)/logf(2.0f);
        float rounded_p2 = floorf(p2+bias);
        s = (int)(powf(2.0f,rounded_p2));
    }
    return s;
}

int Image::computeNumberOfMipmapLevels(int s,int t, int r)
{
    int w = maximum(s, t);
    w = maximum(w, r);
    return 1 + static_cast<int>(floor(logf(w)/logf(2.0f)));
}

unsigned int Image::getTotalSizeInBytesIncludingMipmaps() const
{
    if (_mipmapData.empty()) 
    {
        // no mips so just return size of main image
        return getTotalSizeInBytes();
    }
    
    int s = _s;
    int t = _t;
    int r = _r;
    
    unsigned int maxValue = 0;
    for(unsigned int i=0;i<_mipmapData.size() && _mipmapData[i];++i)
    {
        s >>= 1;
        t >>= 1;
        r >>= 1;
        maxValue = maximum(maxValue,_mipmapData[i]);
   }
   
   if (s==0) s=1;
   if (t==0) t=1;
   if (r==0) r=1;
   
   unsigned int sizeOfLastMipMap = computeRowWidthInBytes(s,_pixelFormat,_dataType,_packing)* r*t;
   switch(_pixelFormat)
   {
        case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT):
           sizeOfLastMipMap = maximum(sizeOfLastMipMap, 8u); // block size of 8
           break;
        case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT):
           sizeOfLastMipMap = maximum(sizeOfLastMipMap, 16u); // block size of 16
           break;
        default: break;
   }

   // notify(INFO)<<"sizeOfLastMipMap="<<sizeOfLastMipMap<<"\ts="<<s<<"\tt="<<t<<"\tr"<<r<<std::endl;                  

   return maxValue+sizeOfLastMipMap;

}


void Image::setInternalTextureFormat(GLint internalFormat)
{
    // won't do any sanity checking right now, leave it to 
    // OpenGL to make the call.
    _internalTextureFormat = internalFormat;
}

void Image::setPixelFormat(GLenum pixelFormat)
{
    if (_pixelFormat==pixelFormat) return; // do nothing if the same.

    if (_pixelFormat==0 || computeNumComponents(_pixelFormat)==computeNumComponents(pixelFormat))
    {
       // if the two formats have the same number of componets then
       // we can do a straight swap.
        _pixelFormat = pixelFormat;
    }
    else
    {
        notify(WARN)<<"Image::setPixelFormat(..) - warning, attempt to reset the pixel format with a different number of components."<<std::endl;
    }
}

void Image::setDataType(GLenum dataType)
{
    if (_dataType==dataType) return; // do nothing if the same.

    if (_dataType==0)
    {
        // setting the datatype for the first time
        _dataType = dataType;
    }
    else
    {
        notify(WARN)<<"Image::setDataType(..) - warning, attempt to reset the data type not permitted."<<std::endl;
    }
}


void Image::allocateImage(int s,int t,int r,
                        GLenum format,GLenum type,
                        int packing)
{
    _mipmapData.clear();

    unsigned int previousTotalSize = 0;
    
    if (_data) previousTotalSize = computeRowWidthInBytes(_s,_pixelFormat,_dataType,_packing)*_t*_r;
    
    unsigned int newTotalSize = computeRowWidthInBytes(s,format,type,packing)*t*r;

    if (newTotalSize!=previousTotalSize)
    {
        if (newTotalSize)
            setData(new unsigned char [newTotalSize],USE_NEW_DELETE);
        else
            deallocateData(); // and sets it to NULL.
    }

    if (_data)
    {
        _s = s;
        _t = t;
        _r = r;
        _pixelFormat = format;
        _dataType = type;
        _packing = packing;
        
        // preserve internalTextureFormat if already set, otherwise
        // use the pixelFormat as the source for the format.
        if (_internalTextureFormat==0) _internalTextureFormat = format;
    }
    else
    {
    
        // throw exception?? not for now, will simply set values to 0.
        _s = 0;
        _t = 0;
        _r = 0;
        _pixelFormat = 0;
        _dataType = 0;
        _packing = 0;
        
        // commenting out reset of _internalTextureFormat as we are changing
        // policy so that allocateImage honours previous settings of _internalTextureFormat.
        //_internalTextureFormat = 0;
    }
    
    dirty();
}

void Image::setImage(int s,int t,int r,
                     GLint internalTextureFormat,
                     GLenum format,GLenum type,
                     unsigned char *data,
                     AllocationMode mode,
                     int packing)
{
    _mipmapData.clear();

    _s = s;
    _t = t;
    _r = r;

    _internalTextureFormat = internalTextureFormat;
    _pixelFormat    = format;
    _dataType       = type;

    setData(data,mode);

    _packing = packing;
        
    dirty();

}

void Image::readPixels(int x,int y,int width,int height,
                       GLenum format,GLenum type)
{
    allocateImage(width,height,1,format,type);

    glPixelStorei(GL_PACK_ALIGNMENT,_packing);

    glReadPixels(x,y,width,height,format,type,_data);
}


void Image::readImageFromCurrentTexture(unsigned int contextID, bool copyMipMapsIfAvailable, GLenum type)
{
    // osg::notify(osg::NOTICE)<<"Image::readImageFromCurrentTexture()"<<std::endl;

    const osg::Texture::Extensions* extensions = osg::Texture::getExtensions(contextID,true);
    const osg::Texture3D::Extensions* extensions3D = osg::Texture3D::getExtensions(contextID,true);
    const osg::Texture2DArray::Extensions* extensions2DArray = osg::Texture2DArray::getExtensions(contextID,true);

    
    GLboolean binding1D, binding2D, binding3D, binding2DArray;
    glGetBooleanv(GL_TEXTURE_BINDING_1D, &binding1D);
    glGetBooleanv(GL_TEXTURE_BINDING_2D, &binding2D);
    glGetBooleanv(GL_TEXTURE_BINDING_3D, &binding3D);
    
    
    if (extensions2DArray->isTexture2DArraySupported())
    {
        glGetBooleanv(GL_TEXTURE_BINDING_2D_ARRAY_EXT, &binding2DArray);
    }
    else
    {
        binding2DArray = GL_FALSE;
    }

    GLenum textureMode = binding1D ? GL_TEXTURE_1D : binding2D ? GL_TEXTURE_2D : binding3D ? GL_TEXTURE_3D : binding2DArray ? GL_TEXTURE_2D_ARRAY_EXT : 0;
    
    if (textureMode==0) return;

    GLint internalformat;
    GLint width;
    GLint height;
    GLint depth;
    GLint packing;

    GLint numMipMaps = 0;
    if (copyMipMapsIfAvailable)
    {
        for(;numMipMaps<20;++numMipMaps)
        {
            glGetTexLevelParameteriv(textureMode, numMipMaps, GL_TEXTURE_WIDTH, &width);
            glGetTexLevelParameteriv(textureMode, numMipMaps, GL_TEXTURE_HEIGHT, &height);
            glGetTexLevelParameteriv(textureMode, numMipMaps, GL_TEXTURE_DEPTH, &depth);
            // osg::notify(osg::NOTICE)<<"   numMipMaps="<<numMipMaps<<" width="<<width<<" height="<<height<<" depth="<<depth<<std::endl;
            if (width==0 || height==0 || depth==0) break;
        }
    }
    else
    {
        numMipMaps = 1;
    }
    
    // osg::notify(osg::NOTICE)<<"Image::readImageFromCurrentTexture() : numMipMaps = "<<numMipMaps<<std::endl;

        
    GLint compressed = 0;

    if (textureMode==GL_TEXTURE_2D)
    {
        if (extensions->isCompressedTexImage2DSupported())
        {
            glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_COMPRESSED_ARB,&compressed);
        }
    }
    else if (textureMode==GL_TEXTURE_3D)
    {
        if (extensions3D->isCompressedTexImage3DSupported())
        {
            glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_COMPRESSED_ARB,&compressed);
        }
    }
    else if (textureMode==GL_TEXTURE_2D_ARRAY_EXT)
    {
        if (extensions2DArray->isCompressedTexImage3DSupported())
        {
            glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_COMPRESSED_ARB,&compressed);
        }
    }
    
    
        
    /* if the compression has been successful */
    if (compressed == GL_TRUE)
    {

        MipmapDataType mipMapData;
        
        unsigned int total_size = 0;
        GLint i;
        for(i=0;i<numMipMaps;++i)
        {
            if (i>0) mipMapData.push_back(total_size);
            
            GLint compressed_size;
            glGetTexLevelParameteriv(textureMode, i, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &compressed_size);
            
            total_size += compressed_size;
        }
        
        
        unsigned char* data = new unsigned char[total_size];
        if (!data)
        {
            osg::notify(osg::WARN)<<"Warning: Image::readImageFromCurrentTexture(..) out of memory, now image read."<<std::endl;
            return; 
        }

        deallocateData(); // and sets it to NULL.

        glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalformat);
        glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_HEIGHT, &height);
        glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_DEPTH, &depth);
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &packing);
        glPixelStorei(GL_PACK_ALIGNMENT, packing);

        _data = data;
        _s = width;
        _t = height;
        _r = depth;
        
        _pixelFormat = internalformat;
        _dataType = type;
        _internalTextureFormat = internalformat;
        _mipmapData = mipMapData;
        _allocationMode=USE_NEW_DELETE;
        _packing = packing;
        
        for(i=0;i<numMipMaps;++i)
        {
            extensions->glGetCompressedTexImage(textureMode, i, getMipmapData(i));
        }

        dirty();
    
    }
    else
    {
        MipmapDataType mipMapData;

        // Get the internal texture format and packing value from OpenGL,
        // instead of using possibly outdated values from the class.
        glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalformat);
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &packing);
        glPixelStorei(GL_PACK_ALIGNMENT, packing);

        unsigned int total_size = 0;
        GLint i;
        for(i=0;i<numMipMaps;++i)
        {
            if (i>0) mipMapData.push_back(total_size);
            
            glGetTexLevelParameteriv(textureMode, i, GL_TEXTURE_WIDTH, &width);
            glGetTexLevelParameteriv(textureMode, i, GL_TEXTURE_HEIGHT, &height);
            glGetTexLevelParameteriv(textureMode, i, GL_TEXTURE_DEPTH, &depth);
            
            unsigned int level_size = computeRowWidthInBytes(width,internalformat,type,packing)*height*depth;

            total_size += level_size;
        }
        
        
        unsigned char* data = new unsigned char[total_size];
        if (!data)
        {
            osg::notify(osg::WARN)<<"Warning: Image::readImageFromCurrentTexture(..) out of memory, now image read."<<std::endl;
            return; 
        }

        deallocateData(); // and sets it to NULL.

        glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_HEIGHT, &height);
        glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_DEPTH, &depth);

        _data = data;
        _s = width;
        _t = height;
        _r = depth;
        
        _pixelFormat = computePixelFormat(internalformat);
        _dataType = type;
        _internalTextureFormat = internalformat;
        _mipmapData = mipMapData;
        _allocationMode=USE_NEW_DELETE;
        _packing = packing;
        
        for(i=0;i<numMipMaps;++i)
        {
            glGetTexImage(textureMode,i,_pixelFormat,_dataType,getMipmapData(i));
        }

        dirty();
    }    
}


void Image::scaleImage(int s,int t,int r, GLenum newDataType)
{
    if (_s==s && _t==t && _r==r) return;

    if (_data==NULL)
    {
        notify(WARN) << "Error Image::scaleImage() do not succeed : cannot scale NULL image."<<std::endl;
        return;
    }

    if (_r!=1 || r!=1)
    {
        notify(WARN) << "Error Image::scaleImage() do not succeed : scaling of volumes not implemented."<<std::endl;
        return;
    }

    

    unsigned int newTotalSize = computeRowWidthInBytes(s,_pixelFormat,newDataType,_packing)*t;

    // need to sort out what size to really use...
    unsigned char* newData = new unsigned char [newTotalSize];
    if (!newData)
    {
        // should we throw an exception???  Just return for time being.
        notify(FATAL) << "Error Image::scaleImage() do not succeed : out of memory."<<newTotalSize<<std::endl;
        return;
    }

    glPixelStorei(GL_PACK_ALIGNMENT,_packing);
    glPixelStorei(GL_UNPACK_ALIGNMENT,_packing);

    GLint status = gluScaleImage(_pixelFormat,
        _s,
        _t,
        _dataType,
        _data,
        s,
        t,
        newDataType,
        newData);

    if (status==0)
    {

        // free old image.
        _s = s;
        _t = t;
        _dataType = newDataType;
        setData(newData,USE_NEW_DELETE);
    }
    else
    {
       delete [] newData;

        notify(WARN) << "Error Image::scaleImage() did not succeed : errorString = "<<gluErrorString((GLenum)status)<<std::endl;
    }
    
    dirty();
}

void Image::copySubImage(int s_offset, int t_offset, int r_offset, const osg::Image* source)
{
    if (!source) return;
    if (s_offset<0 || t_offset<0 || r_offset<0) 
    {
        notify(WARN)<<"Warning: negative offsets passed to Image::copySubImage(..) not supported, operation ignored."<<std::endl;
        return;
    }
    
    if (!_data)
    {
        notify(INFO)<<"allocating image"<<endl;
        allocateImage(s_offset+source->s(),t_offset+source->t(),r_offset+source->r(),
                    source->getPixelFormat(),source->getDataType(),
                    source->getPacking());
    }
    
    if (s_offset>=_s || t_offset>=_t  || r_offset>=_r)
    {
        notify(WARN)<<"Warning: offsets passed to Image::copySubImage(..) outside destination image, operation ignored."<<std::endl;
        return;
    }
    
    
    if (_pixelFormat != source->getPixelFormat())
    {
        notify(WARN)<<"Warning: image with an incompatible pixel formats passed to Image::copySubImage(..), operation ignored."<<std::endl;
        return;
    }

    void* data_destination = data(s_offset,t_offset,r_offset);
    
    glPixelStorei(GL_PACK_ALIGNMENT,source->getPacking());
    glPixelStorei(GL_PACK_ROW_LENGTH,_s);

    glPixelStorei(GL_UNPACK_ALIGNMENT,_packing);
    
    GLint status = gluScaleImage(_pixelFormat,
        source->s(),
        source->t(),
        source->getDataType(),
        source->data(),
        source->s(),
        source->t(),
        _dataType,
        data_destination);

    glPixelStorei(GL_PACK_ROW_LENGTH,0);

    if (status!=0)
    {
        notify(WARN) << "Error Image::scaleImage() do not succeed : errorString = "<<gluErrorString((GLenum)status)<<std::endl;
    }

}


void Image::flipHorizontal()
{
    if (_data==NULL)
    {
        notify(WARN) << "Error Image::flipHorizontal() did not succeed : cannot flip NULL image."<<std::endl;
        return;
    }

    unsigned int elemSize = getPixelSizeInBits()/8;

    if (_mipmapData.empty())
    {

        for(int r=0;r<_r;++r)
        {
            for (int t=0; t<_t; ++t)
            {
                unsigned char* rowData = _data+t*getRowSizeInBytes()+r*getImageSizeInBytes();
                unsigned char* left  = rowData ;
                unsigned char* right = rowData + ((_s-1)*getPixelSizeInBits())/8;

                while (left < right)
                {
                    char tmp[32];  // max elem size is four floats
                    memcpy(tmp, left, elemSize);
                    memcpy(left, right, elemSize);
                    memcpy(right, tmp, elemSize);
                    left  += elemSize;
                    right -= elemSize;
                }
            }
        }
    }
    else
    {
        notify(WARN) << "Error Image::flipHorizontal() did not succeed : cannot flip mipmapped image."<<std::endl;
        return;
    }
        
    dirty();
}

void flipImageVertical(unsigned char* top, unsigned char* bottom, unsigned int rowSize)
{
    while(top<bottom)
    {
        for(unsigned int i=0;i<rowSize;++i, ++top,++bottom)
        {
            unsigned char temp=*top;
            *top = *bottom;
            *bottom = temp;
        }
        bottom -= 2*rowSize;
    }
}


void Image::flipVertical()
{
    if (_data==NULL)
    {
        notify(WARN) << "Error Image::flipVertical() do not succeed : cannot flip NULL image."<<std::endl;
        return;
    }

    if (!_mipmapData.empty() && _r>1)
    {
        notify(WARN) << "Error Image::flipVertical() do not succeed : flipping of mipmap 3d textures not yet supported."<<std::endl;
        return;
    }

    if (_mipmapData.empty())
    {
        // no mipmaps,
        // so we can safely handle 3d textures
        for(int r=0;r<_r;++r)
        {
            if (!dxtc_tool::VerticalFlip(_s,_t,_pixelFormat,data(0,0,r)))
            {
                // its not a compressed image, so implement flip oursleves.
                
                unsigned int rowSize = computeRowWidthInBytes(_s,_pixelFormat,_dataType,_packing);
                unsigned char* top = data(0,0,r);
                unsigned char* bottom = top + (_t-1)*rowSize;
                    
                flipImageVertical(top, bottom, rowSize);
            }
        }
    }
    else if (_r==1)
    {
        if (!dxtc_tool::VerticalFlip(_s,_t,_pixelFormat,_data))
        {
            // its not a compressed image, so implement flip oursleves.
            unsigned int rowSize = computeRowWidthInBytes(_s,_pixelFormat,_dataType,_packing);
            unsigned char* top = data(0,0,0);
            unsigned char* bottom = top + (_t-1)*rowSize;

            flipImageVertical(top, bottom, rowSize);
        }

        int s = _s;
        int t = _t;
        //int r = _r;

        for(unsigned int i=0;i<_mipmapData.size() && _mipmapData[i];++i)
        {
            s >>= 1;
            t >>= 1;
            if (s==0) s=1;
            if (t==0) t=1;
            if (!dxtc_tool::VerticalFlip(s,t,_pixelFormat,_data+_mipmapData[i]))
            {
                // its not a compressed image, so implement flip oursleves.
                unsigned int rowSize = computeRowWidthInBytes(s,_pixelFormat,_dataType,_packing);
                unsigned char* top = _data+_mipmapData[i];
                unsigned char* bottom = top + (t-1)*rowSize;

                flipImageVertical(top, bottom, rowSize);
            }
       }
    }   

    dirty();
}



void Image::ensureValidSizeForTexturing(GLint maxTextureSize)
{
    int new_s = computeNearestPowerOfTwo(_s);
    int new_t = computeNearestPowerOfTwo(_t);
    
    if (new_s>maxTextureSize) new_s = maxTextureSize;
    if (new_t>maxTextureSize) new_t = maxTextureSize;
    
    if (new_s!=_s || new_t!=_t)
    {
        if (!_fileName.empty()) notify(NOTICE) << "Scaling image '"<<_fileName<<"' from ("<<_s<<","<<_t<<") to ("<<new_s<<","<<new_t<<")"<<std::endl;
        else notify(NOTICE) << "Scaling image from ("<<_s<<","<<_t<<") to ("<<new_s<<","<<new_t<<")"<<std::endl;

        scaleImage(new_s,new_t,_r);
    }
}


template <typename T>    
bool _findLowerAlphaValueInRow(unsigned int num, T* data,T value, unsigned int delta)
{
    for(unsigned int i=0;i<num;++i)
    {
        if (*data<value) return true;
        data += delta;
    }
    return false;
}

template <typename T>    
bool _maskedFindLowerAlphaValueInRow(unsigned int num, T* data,T value, T mask, unsigned int delta)
{
    for(unsigned int i=0;i<num;++i)
    {
        if ((*data & mask)<value) return true;
        data += delta;
    }
    return false;
}

bool Image::isImageTranslucent() const
{
    unsigned int offset = 0;
    unsigned int delta = 1;
    switch(_pixelFormat)
    {
        case(GL_ALPHA):
            offset = 0;
            delta = 1;
            break;
        case(GL_LUMINANCE_ALPHA):
            offset = 1;
            delta = 2;
            break;
        case(GL_RGBA):
            offset = 3;
            delta = 4;
            break;
        case(GL_BGRA):
            offset = 3;
            delta = 4;
            break;
        default:
            return false;
    }

    for(int ir=0;ir<r();++ir)
    {
        for(int it=0;it<t();++it)
        {
            const unsigned char* d = data(0,it,ir);
            switch(_dataType)
            {
                case(GL_BYTE):
                    if (_findLowerAlphaValueInRow(s(), (char*)d +offset, (char)127, delta))
                        return true;
                    break;
                case(GL_UNSIGNED_BYTE):
                    if (_findLowerAlphaValueInRow(s(), (unsigned char*)d + offset, (unsigned char)255, delta))
                        return true;
                    break;
                case(GL_SHORT):
                    if (_findLowerAlphaValueInRow(s(), (short*)d + offset, (short)32767, delta))
                        return true;
                    break;
                case(GL_UNSIGNED_SHORT):
                    if (_findLowerAlphaValueInRow(s(), (unsigned short*)d + offset, (unsigned short)65535, delta))
                        return true;
                    break;
                case(GL_INT):
                    if (_findLowerAlphaValueInRow(s(), (int*)d + offset, (int)2147483647, delta))
                        return true;
                    break;
                case(GL_UNSIGNED_INT):
                    if (_findLowerAlphaValueInRow(s(), (unsigned int*)d + offset, 4294967295u, delta))
                        return true;
                    break;
                case(GL_FLOAT):
                    if (_findLowerAlphaValueInRow(s(), (float*)d + offset, 1.0f, delta))
                        return true;
                    break;
                case(GL_UNSIGNED_SHORT_5_5_5_1):
                    if (_maskedFindLowerAlphaValueInRow(s(), (unsigned short*)d,
                                                        (unsigned short)0x0001,
                                                        (unsigned short)0x0001, 1))
                        return true;
                    break;
                case(GL_UNSIGNED_SHORT_1_5_5_5_REV):
                    if (_maskedFindLowerAlphaValueInRow(s(), (unsigned short*)d,
                                                        (unsigned short)0x8000,
                                                        (unsigned short)0x8000, 1))
                        return true;
                    break;
                case(GL_UNSIGNED_SHORT_4_4_4_4):
                    if (_maskedFindLowerAlphaValueInRow(s(), (unsigned short*)d,
                                                        (unsigned short)0x000f,
                                                        (unsigned short)0x000f, 1))
                        return true;
                    break;
                case(GL_UNSIGNED_SHORT_4_4_4_4_REV):
                    if (_maskedFindLowerAlphaValueInRow(s(), (unsigned short*)d,
                                                        (unsigned short)0xf000,
                                                        (unsigned short)0xf000, 1))
                        return true;
                    break;
                case(GL_UNSIGNED_INT_10_10_10_2):
                    if (_maskedFindLowerAlphaValueInRow(s(), (unsigned int*)d,
                                                        0x00000003u,
                                                        0x00000003u, 1))
                        return true;
                    break;                    
                case(GL_UNSIGNED_INT_2_10_10_10_REV):
                    if (_maskedFindLowerAlphaValueInRow(s(), (unsigned int*)d,
                                                        0xc0000000u,
                                                        0xc0000000u, 1))
                        return true;
                    break;
                case(GL_HALF_FLOAT_NV):
                    if (_findLowerAlphaValueInRow(s(), (unsigned short*)d + offset,
                                                  (unsigned short)0x3c00, delta))
                        return true;
                    break;
            }
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////


Geode* osg::createGeodeForImage(osg::Image* image)
{
    return createGeodeForImage(image,image->s(),image->t());
}


#include <osg/TextureRectangle> 


Geode* osg::createGeodeForImage(osg::Image* image,float s,float t)
{
    if (image)
    {
        if (s>0 && t>0)
        {

            float y = 1.0;
            float x = y*(s/t);

            // set up the texture.

#if 0
            osg::TextureRectangle* texture = new osg::TextureRectangle;
            texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
            texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
            //texture->setResizeNonPowerOfTwoHint(false);
            float texcoord_x = image->s();
            float texcoord_y = image->t();
#else
            osg::Texture2D* texture = new osg::Texture2D;
            texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
            texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
            texture->setResizeNonPowerOfTwoHint(false);
            float texcoord_x = 1.0f;
            float texcoord_y = 1.0f;
#endif
            texture->setImage(image);

            // set up the drawstate.
            osg::StateSet* dstate = new osg::StateSet;
            dstate->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
            dstate->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
            dstate->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);

            // set up the geoset.
            Geometry* geom = new Geometry;
            geom->setStateSet(dstate);

            Vec3Array* coords = new Vec3Array(4);
            (*coords)[0].set(-x,0.0f,y);
            (*coords)[1].set(-x,0.0f,-y);
            (*coords)[2].set(x,0.0f,-y);
            (*coords)[3].set(x,0.0f,y);
            geom->setVertexArray(coords);

            Vec2Array* tcoords = new Vec2Array(4);
            (*tcoords)[0].set(0.0f*texcoord_x,1.0f*texcoord_y);
            (*tcoords)[1].set(0.0f*texcoord_x,0.0f*texcoord_y);
            (*tcoords)[2].set(1.0f*texcoord_x,0.0f*texcoord_y);
            (*tcoords)[3].set(1.0f*texcoord_x,1.0f*texcoord_y);
            geom->setTexCoordArray(0,tcoords);

            osg::Vec4Array* colours = new osg::Vec4Array(1);
            (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
            geom->setColorArray(colours);
            geom->setColorBinding(Geometry::BIND_OVERALL);

            geom->addPrimitiveSet(new DrawArrays(PrimitiveSet::QUADS,0,4));

            // set up the geode.
            osg::Geode* geode = new osg::Geode;
            geode->addDrawable(geom);

            return geode;

        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

template <typename T>    
Vec4 _readColor(GLenum pixelFormat, T* data,float scale)
{
    switch(pixelFormat)
    {
        case(GL_LUMINANCE):         { float l = float(*data++)*scale; return Vec4(l, l, l, 1.0f); }
        case(GL_ALPHA):             { float a = float(*data++)*scale; return Vec4(1.0f, 1.0f, 1.0f, a); }
        case(GL_LUMINANCE_ALPHA):   { float l = float(*data++)*scale; float a = float(*data++)*scale; return Vec4(l,l,l,a); }
        case(GL_RGB):               { float r = float(*data++)*scale; float g = float(*data++)*scale; float b = float(*data++)*scale; return Vec4(r,g,b,1.0f); }
        case(GL_RGBA):              { float r = float(*data++)*scale; float g = float(*data++)*scale; float b = float(*data++)*scale; float a = float(*data++)*scale; return Vec4(r,g,b,a); }
        case(GL_BGR):               { float b = float(*data++)*scale; float g = float(*data++)*scale; float r = float(*data++)*scale; return Vec4(r,g,b,1.0f); }
        case(GL_BGRA):              { float b = float(*data++)*scale; float g = float(*data++)*scale; float r = float(*data++)*scale; float a = float(*data++)*scale; return Vec4(r,g,b,a); }
    }
    return Vec4(1.0f,1.0f,1.0f,1.0f);
}

Vec4 Image::getColor(unsigned int s,unsigned t,unsigned r) const
{
    const unsigned char* ptr = data(s,t,r);

    switch(_dataType)
    {
        case(GL_BYTE):              return _readColor(_pixelFormat, (char*)ptr,             1.0f/128.0f);
        case(GL_UNSIGNED_BYTE):     return _readColor(_pixelFormat, (unsigned char*)ptr,    1.0f/255.0f);
        case(GL_SHORT):             return _readColor(_pixelFormat, (short*)ptr,            1.0f/32768.0f);
        case(GL_UNSIGNED_SHORT):    return _readColor(_pixelFormat, (unsigned short*)ptr,   1.0f/65535.0f);
        case(GL_INT):               return _readColor(_pixelFormat, (int*)ptr,              1.0f/2147483648.0f);
        case(GL_UNSIGNED_INT):      return _readColor(_pixelFormat, (unsigned int*)ptr,     1.0f/4294967295.0f);
        case(GL_FLOAT):             return _readColor(_pixelFormat, (float*)ptr,            1.0f);
    }
    return Vec4(1.0f,1.0f,1.0f,1.0f);
}

Vec4 Image::getColor(const Vec3& texcoord) const
{
    int s = int(texcoord.x()*float(_s-1)) % _s;
    int t = int(texcoord.y()*float(_t-1)) % _t;
    int r = int(texcoord.z()*float(_r-1)) % _r;
    //osg::notify(osg::NOTICE)<<"getColor("<<texcoord<<")="<<getColor(s,t,r)<<std::endl;
    return getColor(s,t,r);
}
