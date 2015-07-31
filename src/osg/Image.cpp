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
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/Texture2DArray>
#include <osg/TextureCubeMap>
#include <osg/Light>

#include <algorithm>
#include <string.h>
#include <stdlib.h>

#include "dxtctool.h"

using namespace osg;
using namespace std;

void Image::UpdateCallback::operator () (osg::StateAttribute* attr, osg::NodeVisitor* nv)
{
    osg::Texture* texture = attr ? attr->asTexture() : 0;

    // OSG_NOTICE<<"ImageSequence::UpdateCallback::"<<texture<<std::endl;
    if (texture)
    {
        for(unsigned int i=0; i<texture->getNumImages(); ++i)
        {
            texture->getImage(i)->update(nv);
        }
    }
}

Image::DataIterator::DataIterator(const Image* image):
    _image(image),
    _rowNum(0),
    _imageNum(0),
    _mipmapNum(0),
    _currentPtr(0),
    _currentSize(0)
{
    assign();
}

Image::DataIterator::DataIterator(const DataIterator& ri):
    _image(ri._image),
    _rowNum(ri._rowNum),
    _imageNum(ri._imageNum),
    _mipmapNum(ri._mipmapNum),
    _currentPtr(0),
    _currentSize(0)
{
    assign();
}

void Image::DataIterator::operator ++ ()
{
    if (!_image || _image->isDataContiguous())
    {
        // for contiguous image data we never need more than one block of data
        _currentPtr = 0;
        _currentSize = 0;
        return;
    }

    if (_image->isMipmap())
    {
        // advance to next row
        ++_rowNum;

        if (_rowNum>=_image->t())
        {
            // moved over end of current image so move to next
            _rowNum = 0;
            ++_imageNum;

            if (_imageNum>=_image->r())
            {
                // move to next mipmap
                _imageNum = 0;
                ++_mipmapNum;

                if (_mipmapNum>=_image->getNumMipmapLevels())
                {
                    _currentPtr = 0;
                    _currentSize = 0;
                    return;
                }
            }
        }
    }
    else
    {
        // advance to next row
        ++_rowNum;

        if (_rowNum>=_image->t())
        {
            // moved over end of current image so move to next
            _rowNum = 0;
            ++_imageNum;

            if (_imageNum>=_image->r())
            {
                // we've moved off the end of the osg::Image so reset to null
                _currentPtr = 0;
                _currentSize = 0;
                return;
            }
        }
    }

    assign();
}

void Image::DataIterator::assign()
{
    //OSG_NOTICE<<"DataIterator::assign A"<<std::endl;
    if (!_image)
    {
        _currentPtr = 0;
        _currentSize = 0;
        return;
    }

    //OSG_NOTICE<<"DataIterator::assign B"<<std::endl;

    if (_image->isDataContiguous())
    {
        _currentPtr = _image->data();
        _currentSize = _image->getTotalSizeInBytesIncludingMipmaps();

        //OSG_NOTICE<<"   _currentPtr="<<(void*)_currentPtr<<std::endl;
        //OSG_NOTICE<<"   _currentSize="<<_currentSize<<std::endl;

        return;
    }

    //OSG_NOTICE<<"DataIterator::assign C"<<std::endl;

    if (_image->isMipmap())
    {
        //OSG_NOTICE<<"DataIterator::assign D"<<std::endl;

        if (_mipmapNum>=_image->getNumMipmapLevels())
        {
            _currentPtr = 0;
            _currentSize = 0;
            return;
        }
        const unsigned char* ptr = _image->getMipmapData(_mipmapNum);

        int rowLength = _image->getRowLength()>>_mipmapNum;
        if (rowLength==0) rowLength = 1;

        int imageHeight = _image->t()>>_mipmapNum;
        if (imageHeight==0) imageHeight = 1;

        unsigned int rowWidthInBytes = Image::computeRowWidthInBytes(rowLength,_image->getPixelFormat(),_image->getDataType(),_image->getPacking());
        unsigned int imageSizeInBytes = rowWidthInBytes*imageHeight;

        _currentPtr = ptr + rowWidthInBytes*_rowNum + imageSizeInBytes*_imageNum;
        _currentSize = rowWidthInBytes;
    }
    else
    {
        //OSG_NOTICE<<"DataIterator::assign E"<<std::endl;

        if (_imageNum>=_image->r() || _rowNum>=_image->t())
        {
            _currentPtr = 0;
            _currentSize = 0;
            return;
        }

        //OSG_NOTICE<<"DataIterator::assign F"<<std::endl;

        _currentPtr = _image->data(0, _rowNum, _imageNum);
        _currentSize = _image->getRowSizeInBytes();
        return;
    }
}


Image::Image()
    :BufferData(),
    _fileName(""),
    _writeHint(NO_PREFERENCE),
    _origin(BOTTOM_LEFT),
    _s(0), _t(0), _r(0),
    _rowLength(0),
    _internalTextureFormat(0),
    _pixelFormat(0),
    _dataType(0),
    _packing(4),
    _pixelAspectRatio(1.0),
    _allocationMode(USE_NEW_DELETE),
    _data(0L),
    _dimensionsChangedCallbacks()
{
    setDataVariance(STATIC);
}

Image::Image(const Image& image,const CopyOp& copyop):
    BufferData(image,copyop),
    _fileName(image._fileName),
    _writeHint(image._writeHint),
    _origin(image._origin),
    _s(image._s), _t(image._t), _r(image._r),
    _rowLength(0),
    _internalTextureFormat(image._internalTextureFormat),
    _pixelFormat(image._pixelFormat),
    _dataType(image._dataType),
    _packing(image._packing),
    _pixelAspectRatio(image._pixelAspectRatio),
    _allocationMode(USE_NEW_DELETE),
    _data(0L),
    _mipmapData(image._mipmapData),
    _dimensionsChangedCallbacks(image._dimensionsChangedCallbacks)
{
    if (image._data)
    {
        unsigned int size = image.getTotalSizeInBytesIncludingMipmaps();
        setData(new unsigned char [size],USE_NEW_DELETE);
        unsigned char* dest_ptr = _data;
        for(DataIterator itr(&image); itr.valid(); ++itr)
        {
            memcpy(dest_ptr, itr.data(), itr.size());
            dest_ptr += itr.size();
        }
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
    dirty();
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

        case(GL_RGBA8):
        case(GL_RGBA16):
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
            OSG_WARN<<"Image::computePixelFormat("<<std::hex<<format<<std::dec<<") intensity pixel format is not correctly specified, so assume GL_LUMINANCE_INTEGER."<<std::endl;
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
            return GL_RGBA_INTEGER_EXT;

        case(GL_DEPTH_COMPONENT16):
        case(GL_DEPTH_COMPONENT24):
        case(GL_DEPTH_COMPONENT32):
        case(GL_DEPTH_COMPONENT32F):
        case(GL_DEPTH_COMPONENT32F_NV):
            return GL_DEPTH_COMPONENT;

        default:
            return format;
    }
}

GLenum Image::computeFormatDataType(GLenum pixelFormat)
{
    switch (pixelFormat)
    {
        case GL_R32F:
        case GL_RG32F:
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
        case GL_RED:
        case GL_RG:
        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA:
        case GL_ALPHA: return GL_UNSIGNED_BYTE;

        default:
        {
            OSG_WARN<<"error computeFormatType = "<<std::hex<<pixelFormat<<std::dec<<std::endl;
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
        case(GL_COMPRESSED_SIGNED_RED_RGTC1_EXT): return 1;
        case(GL_COMPRESSED_RED_RGTC1_EXT):   return 1;
        case(GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT): return 2;
        case(GL_COMPRESSED_RED_GREEN_RGTC2_EXT): return 2;
        case(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG): return 3;
        case(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG): return 3;
        case(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG): return 4;
        case(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG): return 4;
        case(GL_ETC1_RGB8_OES): return 3;
        case(GL_COMPRESSED_RGB8_ETC2): return 3;
        case(GL_COMPRESSED_SRGB8_ETC2): return 3;
        case(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2): return 4;
        case(GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2): return 4;
        case(GL_COMPRESSED_RGBA8_ETC2_EAC): return 4;
        case(GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC): return 4;
        case(GL_COMPRESSED_R11_EAC): return 1;
        case(GL_COMPRESSED_SIGNED_R11_EAC): return 1;
        case(GL_COMPRESSED_RG11_EAC): return 2;
        case(GL_COMPRESSED_SIGNED_RG11_EAC): return 2;
        case(GL_COLOR_INDEX): return 1;
        case(GL_STENCIL_INDEX): return 1;
        case(GL_DEPTH_COMPONENT): return 1;
        case(GL_DEPTH_COMPONENT16): return 1;
        case(GL_DEPTH_COMPONENT24): return 1;
        case(GL_DEPTH_COMPONENT32): return 1;
        case(GL_DEPTH_COMPONENT32F): return 1;
        case(GL_DEPTH_COMPONENT32F_NV): return 1;
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
        case(GL_R32F): return 1;
        case(GL_RG): return 2;
        case(GL_RG32F): return 2;
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
            OSG_WARN<<"error pixelFormat = "<<std::hex<<pixelFormat<<std::dec<<std::endl;
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
        case(GL_COMPRESSED_SIGNED_RED_RGTC1_EXT): return 4;
        case(GL_COMPRESSED_RED_RGTC1_EXT):   return 4;
        case(GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT): return 8;
        case(GL_COMPRESSED_RED_GREEN_RGTC2_EXT): return 8;
        case(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG): return 4;
        case(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG): return 2;
        case(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG): return 4;
        case(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG): return 2;
        case(GL_ETC1_RGB8_OES): return 4;
        case(GL_COMPRESSED_RGB8_ETC2): return 4;
        case(GL_COMPRESSED_SRGB8_ETC2): return 4;
        case(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2): return 4;
        case(GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2): return 4;
        case(GL_COMPRESSED_RGBA8_ETC2_EAC): return 8;
        case(GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC): return 8;
        case(GL_COMPRESSED_R11_EAC): return 4;
        case(GL_COMPRESSED_SIGNED_R11_EAC): return 4;
        case(GL_COMPRESSED_RG11_EAC): return 8;
        case(GL_COMPRESSED_SIGNED_RG11_EAC): return 8;
        default: break;
    }

    // note, haven't yet added proper handling of the ARB GL_COMPRESSRED_* pathways
    // yet, no clear size for these since its probably implementation dependent
    // which raises the question of how to actually query for these sizes...
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
            OSG_WARN<<"Image::computePixelSizeInBits(format,type) : cannot compute correct size of compressed format ("<<format<<") returning 0."<<std::endl;
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

        case(GL_HALF_FLOAT):
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
            OSG_WARN<<"error type = "<<type<<std::endl;
            return 0;
        }
    }

}

unsigned int Image::computeBlockSize(GLenum pixelFormat, GLenum packing)
{
    switch(pixelFormat)
    {
        case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT):
            return osg::maximum(8u,packing); // block size of 8
        case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT):
        case(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG):
        case(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG):
        case(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG):
        case(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG):
        case(GL_ETC1_RGB8_OES):
            return osg::maximum(16u,packing); // block size of 16
        case(GL_COMPRESSED_SIGNED_RED_RGTC1_EXT):
        case(GL_COMPRESSED_RED_RGTC1_EXT):
            return osg::maximum(8u,packing); // block size of 8
            break;
        case(GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT):
        case(GL_COMPRESSED_RED_GREEN_RGTC2_EXT):
            return osg::maximum(16u,packing); // block size of 16

        case(GL_COMPRESSED_RGB8_ETC2):
        case(GL_COMPRESSED_SRGB8_ETC2):
        case(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2):
        case(GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2):
        case(GL_COMPRESSED_R11_EAC):
        case(GL_COMPRESSED_SIGNED_R11_EAC):
            return osg::maximum(8u,packing); // block size of 8

        case(GL_COMPRESSED_RGBA8_ETC2_EAC):
        case(GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC):
        case(GL_COMPRESSED_RG11_EAC):
        case(GL_COMPRESSED_SIGNED_RG11_EAC):
            return osg::maximum(16u,packing); // block size of 16
        default:
            break;
    }
    return packing;
}

unsigned int Image::computeRowWidthInBytes(int width,GLenum pixelFormat,GLenum type,int packing)
{
    unsigned int pixelSize = computePixelSizeInBits(pixelFormat,type);
    int widthInBits = width*pixelSize;
    int packingInBits = packing!=0 ? packing*8 : 8;
    //OSG_INFO << "width="<<width<<" pixelSize="<<pixelSize<<"  width in bit="<<widthInBits<<" packingInBits="<<packingInBits<<" widthInBits%packingInBits="<<widthInBits%packingInBits<<std::endl;
    return (widthInBits/packingInBits + ((widthInBits%packingInBits)?1:0))*packing;
}

unsigned int Image::computeImageSizeInBytes(int width,int height, int depth, GLenum pixelFormat,GLenum type,int packing, int slice_packing, int image_packing)
{
    if (width<=0 || height<=0 || depth<=0) return 0;

    // Taking advantage of the fact that
    // DXT formats are defined as 4 successive numbers:
    // GL_COMPRESSED_RGB_S3TC_DXT1_EXT         0x83F0
    // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT        0x83F1
    // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT        0x83F2
    // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT        0x83F3
    if( pixelFormat >= GL_COMPRESSED_RGB_S3TC_DXT1_EXT &&
        pixelFormat <= GL_COMPRESSED_RGBA_S3TC_DXT5_EXT )
    {
        width = (width + 3) & ~3;
        height = (height + 3) & ~3;
    }

    // 3dc ATI formats
    // GL_COMPRESSED_RED_RGTC1_EXT                     0x8DBB
    // GL_COMPRESSED_SIGNED_RED_RGTC1_EXT              0x8DBC
    // GL_COMPRESSED_RED_GREEN_RGTC2_EXT               0x8DBD
    // GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT        0x8DBE
    if( pixelFormat >= GL_COMPRESSED_RED_RGTC1_EXT &&
        pixelFormat <= GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT )
    {
        width = (width + 3) & ~3;
        height = (height + 3) & ~3;
    }

    // compute size of one row
    unsigned int size = osg::Image::computeRowWidthInBytes( width, pixelFormat, type, packing );

    // now compute size of slice
    size *= height;
    size += slice_packing - 1;
    size -= size % slice_packing;

    // compute size of whole image
    size *= depth;
    size += image_packing - 1;
    size -= size % image_packing;

    return osg::maximum( size, computeBlockSize(pixelFormat, packing) );
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

    int n = 0;
    while (w >>= 1)
        ++n;
    return n+1;
}

bool Image::isCompressed() const
{
    switch(_pixelFormat)
    {
        case(GL_COMPRESSED_ALPHA_ARB):
        case(GL_COMPRESSED_INTENSITY_ARB):
        case(GL_COMPRESSED_LUMINANCE_ALPHA_ARB):
        case(GL_COMPRESSED_LUMINANCE_ARB):
        case(GL_COMPRESSED_RGBA_ARB):
        case(GL_COMPRESSED_RGB_ARB):
        case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT):
        case(GL_COMPRESSED_SIGNED_RED_RGTC1_EXT):
        case(GL_COMPRESSED_RED_RGTC1_EXT):
        case(GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT):
        case(GL_COMPRESSED_RED_GREEN_RGTC2_EXT):
        case(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG):
        case(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG):
        case(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG):
        case(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG):
        case(GL_ETC1_RGB8_OES):
        case(GL_COMPRESSED_RGB8_ETC2):
        case(GL_COMPRESSED_SRGB8_ETC2):
        case(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2):
        case(GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2):
        case(GL_COMPRESSED_RGBA8_ETC2_EAC):
        case(GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC):
        case(GL_COMPRESSED_R11_EAC):
        case(GL_COMPRESSED_SIGNED_R11_EAC):
        case(GL_COMPRESSED_RG11_EAC):
        case(GL_COMPRESSED_SIGNED_RG11_EAC):
            return true;
        default:
            return false;
    }
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
    unsigned int totalSize = 0;
    for(unsigned int i=0;i<_mipmapData.size()+1;++i)
    {
        totalSize += computeImageSizeInBytes(s, t, r, _pixelFormat, _dataType, _packing);

        s >>= 1;
        t >>= 1;
        r >>= 1;

        if (s<1) s=1;
        if (t<1) t=1;
        if (r<1) r=1;
   }

   return totalSize;
}

void Image::setRowLength(int length)
{
    _rowLength = length;
    #if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
    if (length > 0)
    {
        OSG_WARN << "Image::setRowLength is not supported on this platform, ignoring" << std::endl;
    }
    #endif

}

void Image::setInternalTextureFormat(GLint internalFormat)
{
    // won't do any sanity checking right now, leave it to
    // OpenGL to make the call.
    _internalTextureFormat = internalFormat;
}

void Image::setPixelFormat(GLenum pixelFormat)
{
    _pixelFormat = pixelFormat;
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
        OSG_WARN<<"Image::setDataType(..) - warning, attempt to reset the data type not permitted."<<std::endl;
    }
}


void Image::allocateImage(int s,int t,int r,
                        GLenum format,GLenum type,
                        int packing)
{
    _mipmapData.clear();

    bool callback_needed(false);

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
        callback_needed = (_s != s) || (_t != t) || (_r != r);
        _s = s;
        _t = t;
        _r = r;
        _pixelFormat = format;
        _dataType = type;
        _packing = packing;
        _rowLength = 0;

        // preserve internalTextureFormat if already set, otherwise
        // use the pixelFormat as the source for the format.
        if (_internalTextureFormat==0) _internalTextureFormat = format;
    }
    else
    {
        callback_needed = (_s != 0) || (_t != 0) || (_r != 0);

        // failed to allocate memory, for now, will simply set values to 0.
        _s = 0;
        _t = 0;
        _r = 0;
        _pixelFormat = 0;
        _dataType = 0;
        _packing = 0;
        _rowLength = 0;

        // commenting out reset of _internalTextureFormat as we are changing
        // policy so that allocateImage honours previous settings of _internalTextureFormat.
        //_internalTextureFormat = 0;
    }

    if (callback_needed)
        handleDimensionsChangedCallbacks();

    dirty();
}

void Image::setImage(int s,int t,int r,
                     GLint internalTextureFormat,
                     GLenum format,GLenum type,
                     unsigned char *data,
                     AllocationMode mode,
                     int packing,
                     int rowLength)
{
    _mipmapData.clear();

    bool callback_needed = (_s != s) || (_t != t) || (_r != r);

    _s = s;
    _t = t;
    _r = r;

    _internalTextureFormat = internalTextureFormat;
    _pixelFormat    = format;
    _dataType       = type;

    setData(data,mode);

    _packing = packing;
    _rowLength = rowLength;

    dirty();

    if (callback_needed)
        handleDimensionsChangedCallbacks();

}

void Image::readPixels(int x,int y,int width,int height,
                       GLenum format, GLenum type, int packing)
{
    allocateImage(width,height,1,format,type, packing);

    glPixelStorei(GL_PACK_ALIGNMENT,_packing);
    glPixelStorei(GL_PACK_ROW_LENGTH,_rowLength);

    glReadPixels(x,y,width,height,format,type,_data);
}


void Image::readImageFromCurrentTexture(unsigned int contextID, bool copyMipMapsIfAvailable, GLenum type, unsigned int face)
{
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
    // OSG_NOTICE<<"Image::readImageFromCurrentTexture()"<<std::endl;

    const osg::GLExtensions* extensions = osg::GLExtensions::Get(contextID,true);

    GLboolean binding1D = GL_FALSE, binding2D = GL_FALSE, binding3D = GL_FALSE, binding2DArray = GL_FALSE, bindingCubeMap = GL_FALSE;

    glGetBooleanv(GL_TEXTURE_BINDING_1D, &binding1D);
    glGetBooleanv(GL_TEXTURE_BINDING_2D, &binding2D);
    glGetBooleanv(GL_TEXTURE_BINDING_3D, &binding3D);
    glGetBooleanv(GL_TEXTURE_BINDING_CUBE_MAP, &bindingCubeMap);

    if (extensions->isTexture2DArraySupported)
    {
        glGetBooleanv(GL_TEXTURE_BINDING_2D_ARRAY_EXT, &binding2DArray);
    }

    GLenum textureMode = binding1D ? GL_TEXTURE_1D : binding2D ? GL_TEXTURE_2D : binding3D ? GL_TEXTURE_3D : binding2DArray ? GL_TEXTURE_2D_ARRAY_EXT : 0;
    if (bindingCubeMap)
    {
        switch (face)
        {
            case TextureCubeMap::POSITIVE_X:
                textureMode = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
                break;
            case TextureCubeMap::NEGATIVE_X:
                textureMode = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
                break;
            case TextureCubeMap::POSITIVE_Y:
                textureMode = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
                break;
            case TextureCubeMap::NEGATIVE_Y:
                textureMode = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
                break;
            case TextureCubeMap::POSITIVE_Z:
                textureMode = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
                break;
            case TextureCubeMap::NEGATIVE_Z:
                textureMode = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
                break;
        }
    }

    if (textureMode==0) return;

    GLint internalformat;
    GLint width;
    GLint height;
    GLint depth;
    GLint packing;
    GLint rowLength;

    GLint numMipMaps = 0;
    if (copyMipMapsIfAvailable)
    {
        for(;numMipMaps<20;++numMipMaps)
        {
            glGetTexLevelParameteriv(textureMode, numMipMaps, GL_TEXTURE_WIDTH, &width);
            glGetTexLevelParameteriv(textureMode, numMipMaps, GL_TEXTURE_HEIGHT, &height);
            glGetTexLevelParameteriv(textureMode, numMipMaps, GL_TEXTURE_DEPTH, &depth);
            // OSG_NOTICE<<"   numMipMaps="<<numMipMaps<<" width="<<width<<" height="<<height<<" depth="<<depth<<std::endl;
            if (width==0 || height==0 || depth==0) break;
        }
    }
    else
    {
        numMipMaps = 1;
    }

    // OSG_NOTICE<<"Image::readImageFromCurrentTexture() : numMipMaps = "<<numMipMaps<<std::endl;


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
        if (extensions->isCompressedTexImage3DSupported())
        {
            glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_COMPRESSED_ARB,&compressed);
        }
    }
    else if (textureMode==GL_TEXTURE_2D_ARRAY_EXT)
    {
        if (extensions->isCompressedTexImage3DSupported())
        {
            glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_COMPRESSED_ARB,&compressed);
        }
    }
    else if(bindingCubeMap)
    {
        if (extensions->isCompressedTexImage2DSupported())
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
            OSG_WARN<<"Warning: Image::readImageFromCurrentTexture(..) out of memory, now image read."<<std::endl;
            return;
        }

        deallocateData(); // and sets it to NULL.

        glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalformat);
        glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_HEIGHT, &height);
        glGetTexLevelParameteriv(textureMode, 0, GL_TEXTURE_DEPTH, &depth);
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &packing);
        glPixelStorei(GL_PACK_ALIGNMENT, packing);
        glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowLength);
        glPixelStorei(GL_PACK_ROW_LENGTH, rowLength);

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
        _rowLength = rowLength;

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
        glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowLength);
        glPixelStorei(GL_PACK_ROW_LENGTH, rowLength);

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
            OSG_WARN<<"Warning: Image::readImageFromCurrentTexture(..) out of memory, now image read."<<std::endl;
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
        _rowLength = rowLength;

        for(i=0;i<numMipMaps;++i)
        {
            glGetTexImage(textureMode,i,_pixelFormat,_dataType,getMipmapData(i));
        }

        dirty();
    }
#else
    OSG_NOTICE<<"Warning: Image::readImageFromCurrentTexture() not supported."<<std::endl;
#endif
}

void Image::swap(osg::Image& rhs)
{
    std::swap(_fileName, rhs._fileName);
    std::swap(_writeHint, rhs._writeHint);

    std::swap(_origin, rhs._origin);

    std::swap(_s, rhs._s); std::swap(_t, rhs._t); std::swap(_r, rhs._r);
    std::swap(_rowLength, rhs._rowLength);
    std::swap(_internalTextureFormat, rhs._internalTextureFormat);
    std::swap(_pixelFormat, rhs._pixelFormat);
    std::swap(_dataType, rhs._dataType);
    std::swap(_packing, rhs._packing);
    std::swap(_pixelAspectRatio, rhs._pixelAspectRatio);

    std::swap(_allocationMode, rhs._allocationMode);
    std::swap(_data, rhs._data);

    std::swap(_mipmapData, rhs._mipmapData);

    std::swap(_bufferObject, rhs._bufferObject);

    std::swap(_dimensionsChangedCallbacks, rhs._dimensionsChangedCallbacks);
}


void Image::scaleImage(int s,int t,int r, GLenum newDataType)
{
    if (_s==s && _t==t && _r==r) return;

    if (_data==NULL)
    {
        OSG_WARN << "Error Image::scaleImage() do not succeed : cannot scale NULL image."<<std::endl;
        return;
    }

    if (_r!=1 || r!=1)
    {
        OSG_WARN << "Error Image::scaleImage() do not succeed : scaling of volumes not implemented."<<std::endl;
        return;
    }

    unsigned int newTotalSize = computeRowWidthInBytes(s,_pixelFormat,newDataType,_packing)*t;

    // need to sort out what size to really use...
    unsigned char* newData = new unsigned char [newTotalSize];
    if (!newData)
    {
        // should we throw an exception???  Just return for time being.
        OSG_FATAL << "Error Image::scaleImage() do not succeed : out of memory."<<newTotalSize<<std::endl;
        return;
    }

    PixelStorageModes psm;
    psm.pack_alignment = _packing;
    psm.pack_row_length = _rowLength;
    psm.unpack_alignment = _packing;

    GLint status = gluScaleImage(&psm, _pixelFormat,
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
        _rowLength = 0;
        _dataType = newDataType;
        setData(newData,USE_NEW_DELETE);
    }
    else
    {
        delete [] newData;

        OSG_WARN << "Error Image::scaleImage() did not succeed : errorString = "<< gluErrorString((GLenum)status) << ". The rendering context may be invalid." << std::endl;
    }

    dirty();
}

void Image::copySubImage(int s_offset, int t_offset, int r_offset, const osg::Image* source)
{
    if (!source) return;
    if (s_offset<0 || t_offset<0 || r_offset<0)
    {
        OSG_WARN<<"Warning: negative offsets passed to Image::copySubImage(..) not supported, operation ignored."<<std::endl;
        return;
    }

    if (!_data)
    {
        OSG_INFO<<"allocating image"<<endl;
        allocateImage(s_offset+source->s(),t_offset+source->t(),r_offset+source->r(),
                    source->getPixelFormat(),source->getDataType(),
                    source->getPacking());
    }

    if (s_offset>=_s || t_offset>=_t  || r_offset>=_r)
    {
        OSG_WARN<<"Warning: offsets passed to Image::copySubImage(..) outside destination image, operation ignored."<<std::endl;
        return;
    }


    if (_pixelFormat != source->getPixelFormat())
    {
        OSG_WARN<<"Warning: image with an incompatible pixel formats passed to Image::copySubImage(..), operation ignored."<<std::endl;
        return;
    }

    void* data_destination = data(s_offset,t_offset,r_offset);

    PixelStorageModes psm;
    psm.pack_alignment = _packing;
    psm.pack_row_length = _rowLength!=0 ? _rowLength : _s;
    psm.unpack_alignment = source->getPacking();
    psm.unpack_row_length = source->getRowLength();

    GLint status = gluScaleImage(&psm, _pixelFormat,
        source->s(),
        source->t(),
        source->getDataType(),
        source->data(),
        source->s(),
        source->t(),
        _dataType,
        data_destination);

    if (status!=0)
    {
        OSG_WARN << "Error Image::scaleImage() did not succeed : errorString = "<< gluErrorString((GLenum)status) << ". The rendering context may be invalid." << std::endl;
    }
}

void Image::flipHorizontal()
{
    if (_data==NULL)
    {
        OSG_WARN << "Error Image::flipHorizontal() did not succeed : cannot flip NULL image."<<std::endl;
        return;
    }

    unsigned int elemSize = getPixelSizeInBits()/8;

    if (_mipmapData.empty())
    {
        unsigned int rowStepInBytes = getRowStepInBytes();
        unsigned int imageStepInBytes = getImageStepInBytes();

        for(int r=0;r<_r;++r)
        {
            for (int t=0; t<_t; ++t)
            {
                unsigned char* rowData = _data + t*rowStepInBytes + r*imageStepInBytes;
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
        OSG_WARN << "Error Image::flipHorizontal() did not succeed : cannot flip mipmapped image."<<std::endl;
        return;
    }

    dirty();
}

void flipImageVertical(unsigned char* top, unsigned char* bottom, unsigned int rowSize, unsigned int rowStep)
{
    while(top<bottom)
    {
        unsigned char* t = top;
        unsigned char* b = bottom;
        for(unsigned int i=0;i<rowSize;++i, ++t,++b)
        {
            unsigned char temp=*t;
            *t = *b;
            *b = temp;
        }
        top += rowStep;
        bottom -= rowStep;
    }
}


void Image::flipVertical()
{
    if (_data==NULL)
    {
        OSG_WARN << "Error Image::flipVertical() do not succeed : cannot flip NULL image."<<std::endl;
        return;
    }

    if (!_mipmapData.empty() && _r>1)
    {
        OSG_WARN << "Error Image::flipVertical() do not succeed : flipping of mipmap 3d textures not yet supported."<<std::endl;
        return;
    }

    unsigned int rowSize = getRowSizeInBytes();
    unsigned int rowStep = getRowStepInBytes();

    const bool dxtc(dxtc_tool::isDXTC(_pixelFormat));
    if (_mipmapData.empty())
    {
        // no mipmaps,
        // so we can safely handle 3d textures
        for(int r=0;r<_r;++r)
        {
            if (dxtc)
            {
                if (!dxtc_tool::VerticalFlip(_s,_t,_pixelFormat,data(0,0,r)))
                {
                    OSG_NOTICE << "Notice Image::flipVertical(): Vertical flip do not succeed" << std::endl;
                }
            }
            else
            {
                if (isCompressed()) OSG_NOTICE << "Notice Image::flipVertical(): image is compressed but normal v-flip is used" << std::endl;
                // its not a compressed image, so implement flip oursleves.
                unsigned char* top = data(0,0,r);
                unsigned char* bottom = top + (_t-1)*rowStep;

                flipImageVertical(top, bottom, rowSize, rowStep);
            }
        }
    }
    else if (_r==1)
    {
        if (dxtc)
        {
            if (!dxtc_tool::VerticalFlip(_s,_t,_pixelFormat,_data))
            {
                OSG_NOTICE << "Notice Image::flipVertical(): Vertical flip do not succeed" << std::endl;
            }
        }
        else
        {
            if (isCompressed()) OSG_NOTICE << "Notice Image::flipVertical(): image is compressed but normal v-flip is used" << std::endl;
            // its not a compressed image, so implement flip oursleves.
            unsigned char* top = data(0,0,0);
            unsigned char* bottom = top + (_t-1)*rowStep;

            flipImageVertical(top, bottom, rowSize, rowStep);
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
            if (dxtc)
            {
                if (!dxtc_tool::VerticalFlip(s,t,_pixelFormat,_data+_mipmapData[i]))
                {
                    OSG_NOTICE << "Notice Image::flipVertical(): Vertical flip did not succeed" << std::endl;
                }
            }
            else
            {
                // it's not a compressed image, so implement flip ourselves.
                unsigned int mipRowSize = computeRowWidthInBytes(s, _pixelFormat, _dataType, _packing);
                unsigned int mipRowStep = mipRowSize;
                unsigned char* top = _data+_mipmapData[i];
                unsigned char* bottom = top + (t-1)*mipRowStep;

                flipImageVertical(top, bottom, mipRowSize, mipRowStep);
            }
       }
    }

    dirty();
}

void Image::flipDepth()
{
    if (_data==NULL)
    {
        OSG_WARN << "Error Image::flipVertical() do not succeed : cannot flip NULL image."<<std::endl;
        return;
    }

    if (_r==1)
    {
        return;
    }

    if (!_mipmapData.empty() && _r>1)
    {
        OSG_WARN << "Error Image::flipVertical() do not succeed : flipping of mipmap 3d textures not yet supported."<<std::endl;
        return;
    }

    unsigned int sizeOfRow = getRowSizeInBytes();

    int r_front = 0;
    int r_back = _r-1;
    for(; r_front<r_back; ++r_front,--r_back)
    {
        for(int row=0; row<_t; ++row)
        {
            unsigned char* front = data(0, row, r_front);
            unsigned char* back = data(0, row, r_back);
            for(unsigned int i=0; i<sizeOfRow; ++i, ++front, ++back)
            {
                std::swap(*front, *back);
            }
        }
    }
}


void Image::ensureValidSizeForTexturing(GLint maxTextureSize)
{
    int new_s = computeNearestPowerOfTwo(_s);
    int new_t = computeNearestPowerOfTwo(_t);

    if (new_s>maxTextureSize) new_s = maxTextureSize;
    if (new_t>maxTextureSize) new_t = maxTextureSize;

    if (new_s!=_s || new_t!=_t)
    {
        if (!_fileName.empty()) { OSG_NOTICE << "Scaling image '"<<_fileName<<"' from ("<<_s<<","<<_t<<") to ("<<new_s<<","<<new_t<<")"<<std::endl; }
        else { OSG_NOTICE << "Scaling image from ("<<_s<<","<<_t<<") to ("<<new_s<<","<<new_t<<")"<<std::endl; }

        scaleImage(new_s,new_t,_r);
    }
}

bool Image::supportsTextureSubloading() const
{
    switch(_internalTextureFormat)
    {
        case GL_ETC1_RGB8_OES:
        case(GL_COMPRESSED_RGB8_ETC2):
        case(GL_COMPRESSED_SRGB8_ETC2):
        case(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2):
        case(GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2):
        case(GL_COMPRESSED_RGBA8_ETC2_EAC):
        case(GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC):
        case(GL_COMPRESSED_R11_EAC):
        case(GL_COMPRESSED_SIGNED_R11_EAC):
        case(GL_COMPRESSED_RG11_EAC):
        case(GL_COMPRESSED_SIGNED_RG11_EAC):
            return false;
        default:
            return true;
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
        case(GL_RGB):
            return false;
        case(GL_BGR):
            return false;
        case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT):
            return false;
        case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT):
            return dxtc_tool::CompressedImageTranslucent(_s, _t, _pixelFormat, _data);
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
                case(GL_HALF_FLOAT):
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

            float texcoord_y_b = (image->getOrigin() == osg::Image::BOTTOM_LEFT) ? 0.0f : 1.0f;
            float texcoord_y_t = (image->getOrigin() == osg::Image::BOTTOM_LEFT) ? 1.0f : 0.0f;

            // set up the texture.

#if 0
            osg::TextureRectangle* texture = new osg::TextureRectangle;
            texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
            texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
            //texture->setResizeNonPowerOfTwoHint(false);
            float texcoord_x = image->s();
            texcoord_y_b *= image->t();
            texcoord_y_t *= image->t();
#else
            osg::Texture2D* texture = new osg::Texture2D;
            texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
            texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
            texture->setResizeNonPowerOfTwoHint(false);
            float texcoord_x = 1.0f;
#endif
            texture->setImage(image);

            // set up the drawstate.
            osg::StateSet* dstate = new osg::StateSet;
            dstate->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
            dstate->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
            dstate->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);

            // set up the geoset.                unsigned int rowSize = computeRowWidthInBytes(s,_pixelFormat,_dataType,_packing);

            Geometry* geom = new Geometry;
            geom->setStateSet(dstate);

            Vec3Array* coords = new Vec3Array(4);
            (*coords)[0].set(-x,0.0f,y);
            (*coords)[1].set(-x,0.0f,-y);
            (*coords)[2].set(x,0.0f,-y);
            (*coords)[3].set(x,0.0f,y);
            geom->setVertexArray(coords);

            Vec2Array* tcoords = new Vec2Array(4);
            (*tcoords)[0].set(0.0f*texcoord_x,texcoord_y_t);
            (*tcoords)[1].set(0.0f*texcoord_x,texcoord_y_b);
            (*tcoords)[2].set(1.0f*texcoord_x,texcoord_y_b);
            (*tcoords)[3].set(1.0f*texcoord_x,texcoord_y_t);
            geom->setTexCoordArray(0,tcoords);

            osg::Vec4Array* colours = new osg::Vec4Array(1);
            (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
            geom->setColorArray(colours, osg::Array::BIND_OVERALL);

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
        case(GL_DEPTH_COMPONENT):   //intentionally fall through and execute the code for GL_LUMINANCE
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
        case(GL_DOUBLE):            return _readColor(_pixelFormat, (double*)ptr,           1.0f);
    }
    return Vec4(1.0f,1.0f,1.0f,1.0f);
}

Vec4 Image::getColor(const Vec3& texcoord) const
{
    unsigned int s = osg::clampTo(int(texcoord.x()*float(_s-1)), 0, _s-1);
    unsigned int t = osg::clampTo(int(texcoord.y()*float(_t-1)), 0, _t-1);
    unsigned int r = osg::clampTo(int(texcoord.z()*float(_r-1)), 0, _r-1);
    //OSG_NOTICE<<"getColor("<<texcoord<<")="<<getColor(s,t,r)<<std::endl;
    return getColor(s,t,r);
}


template <typename T>
void _writeColor(GLenum pixelFormat, T* data, float scale, const Vec4& c)
{
    switch(pixelFormat)
    {
    case(GL_DEPTH_COMPONENT):   //intentionally fall through and execute the code for GL_LUMINANCE
    case(GL_LUMINANCE):         { (*data++) = (T)(c[0] * scale); } break;
    case(GL_ALPHA):             { (*data++) = (T)(c[3] * scale); } break;
    case(GL_LUMINANCE_ALPHA):   { (*data++) = (T)(c[0] * scale);  (*data++) = (T)(c[3] * scale); } break;
    case(GL_RGB):               { (*data++) = (T)(c[0] *scale); (*data++) = (T)(c[1] *scale); (*data++) = (T)(c[2] *scale);} break;
    case(GL_RGBA):              { (*data++) = (T)(c[0] *scale); (*data++) = (T)(c[1] *scale); (*data++) = (T)(c[2] *scale); (*data++) = (T)(c[3] *scale);} break;
    case(GL_BGR):               { (*data++) = (T)(c[2] *scale); (*data++) = (T)(c[1] *scale); (*data++) = (T)(c[0] *scale);} break;
    case(GL_BGRA):              { (*data++) = (T)(c[2] *scale); (*data++) = (T)(c[1] *scale); (*data++) = (T)(c[0] *scale); (*data++) = (T)(c[3] *scale);} break;
    }

}


void Image::setColor( const Vec4& color, unsigned int s, unsigned int t/*=0*/, unsigned int r/*=0*/ )
{
    unsigned char* ptr = data(s,t,r);

    switch(getDataType())
    {
    case(GL_BYTE):              return _writeColor(getPixelFormat(), (char*)ptr,             128.0f, color);
    case(GL_UNSIGNED_BYTE):     return _writeColor(getPixelFormat(), (unsigned char*)ptr,    255.0f, color);
    case(GL_SHORT):             return _writeColor(getPixelFormat(), (short*)ptr,            32768.0f, color);
    case(GL_UNSIGNED_SHORT):    return _writeColor(getPixelFormat(), (unsigned short*)ptr,   65535.0f, color);
    case(GL_INT):               return _writeColor(getPixelFormat(), (int*)ptr,              2147483648.0f, color);
    case(GL_UNSIGNED_INT):      return _writeColor(getPixelFormat(), (unsigned int*)ptr,     4294967295.0f, color);
    case(GL_FLOAT):             return _writeColor(getPixelFormat(), (float*)ptr,            1.0f, color);
    case(GL_DOUBLE):            return _writeColor(getPixelFormat(), (double*)ptr,           1.0f, color);
    }
}

void Image::setColor( const Vec4& color, const Vec3& texcoord )
{
    unsigned int s = osg::clampTo(int(texcoord.x()*float(_s-1)), 0, _s-1);
    unsigned int t = osg::clampTo(int(texcoord.y()*float(_t-1)), 0, _t-1);
    unsigned int r = osg::clampTo(int(texcoord.z()*float(_r-1)), 0, _r-1);

    return setColor(color, s,t,r);
}

void Image::addDimensionsChangedCallback(DimensionsChangedCallback* cb)
{
    _dimensionsChangedCallbacks.push_back(cb);
}

void Image::removeDimensionsChangedCallback(DimensionsChangedCallback* cb)
{
    DimensionsChangedCallbackVector::iterator itr = std::find(_dimensionsChangedCallbacks.begin(), _dimensionsChangedCallbacks.end(), cb);
    if (itr!=_dimensionsChangedCallbacks.end()) _dimensionsChangedCallbacks.erase(itr);
}
