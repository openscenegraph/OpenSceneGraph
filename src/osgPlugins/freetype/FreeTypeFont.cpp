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

#include "FreeTypeFont.h"
#include "FreeTypeLibrary.h"

#include <osg/Notify>  
#include <osgDB/WriteFile>

FreeTypeFont::FreeTypeFont(const std::string& filename, FT_Face face, unsigned int flags):
    _currentRes(osgText::FontResolution(0,0)),
    _filename(filename),
    _buffer(0),
    _face(face),
    _flags(flags)
{
}

FreeTypeFont::FreeTypeFont(FT_Byte* buffer, FT_Face face, unsigned int flags):
    _currentRes(osgText::FontResolution(0,0)),
    _filename(""),
    _buffer(buffer),
    _face(face),
    _flags(flags)
{
}

FreeTypeFont::~FreeTypeFont()
{
    if(_face)
    {
        FreeTypeLibrary* freeTypeLibrary = FreeTypeLibrary::instance();
        if (freeTypeLibrary)
        {
            // remove myself from the local registry to ensure that
            // not dangling pointers remain
            freeTypeLibrary->removeFontImplmentation(this);
        
            // free the freetype font face itself
            FT_Done_Face(_face);
            _face = 0;

            // release memory held for FT_Face to work
            if (_buffer)
            {
                delete [] _buffer;
                _buffer = 0;
            }
        }
    }
}

void FreeTypeFont::setFontResolution(const osgText::FontResolution& fontSize)
{
    if (fontSize==_currentRes) return;

    int width = fontSize.first;
    int height = fontSize.second;
    int maxAxis = std::max(width, height);
    int margin = _facade->getGlyphImageMargin() + (int)((float)maxAxis * _facade->getGlyphImageMarginRatio());

    if ((unsigned int)(width+2*margin) > _facade->getTextureWidthHint() ||
        (unsigned int)(width+2*margin) > _facade->getTextureHeightHint())
    {
        osg::notify(osg::WARN)<<"Warning: FreeTypeFont::setSize("<<width<<","<<height<<") sizes too large,"<<std::endl;
    
        width = _facade->getTextureWidthHint()-2*margin;
        height = _facade->getTextureHeightHint()-2*margin;

        osg::notify(osg::WARN)<<"         sizes capped ("<<width<<","<<height<<") to fit int current glyph texture size."<<std::endl;
    }

    FT_Error error = FT_Set_Pixel_Sizes( _face,      /* handle to face object  */
                                         width,      /* pixel_width            */
                                         height );   /* pixel_height            */

    if (error)
    {
        osg::notify(osg::WARN)<<"FT_Set_Pixel_Sizes() - error 0x"<<std::hex<<error<<std::dec<<std::endl;
    }
    else
    {
        _currentRes = fontSize;
    }

}

osgText::Font::Glyph* FreeTypeFont::getGlyph(const osgText::FontResolution& fontRes, unsigned int charcode)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(FreeTypeLibrary::instance()->getMutex());

    setFontResolution(fontRes);

    //
    // GT: fix for symbol fonts (i.e. the Webdings font) as the wrong character are being  
    // returned, for symbol fonts in windows (FT_ENCONDING_MS_SYMBOL in freetype) the correct 
    // values are from 0xF000 to 0xF0FF not from 0x000 to 0x00FF (0 to 255) as you would expect.  
    // Microsoft uses a private field for its symbol fonts
    //
    unsigned int charindex = charcode;
    if (_face->charmap != NULL)
    {
        if (_face->charmap->encoding == FT_ENCODING_MS_SYMBOL)
        {
            charindex |= 0xF000;
        }
    }

    FT_Error error = FT_Load_Char( _face, charindex, FT_LOAD_RENDER|FT_LOAD_NO_BITMAP|_flags );
    if (error)
    {
        osg::notify(osg::WARN) << "FT_Load_Char(...) error 0x"<<std::hex<<error<<std::dec<<std::endl;
        return 0;
    }


    FT_GlyphSlot glyphslot = _face->glyph;

    int pitch = glyphslot->bitmap.pitch;
    unsigned char* buffer = glyphslot->bitmap.buffer;

    unsigned int sourceWidth = glyphslot->bitmap.width;;
    unsigned int sourceHeight = glyphslot->bitmap.rows;
    
    unsigned int width = sourceWidth;
    unsigned int height = sourceHeight;

    osg::ref_ptr<osgText::Font::Glyph> glyph = new osgText::Font::Glyph;
    
    unsigned int dataSize = width*height;
    unsigned char* data = new unsigned char[dataSize];
    

    // clear the image to zeros.
    for(unsigned char* p=data;p<data+dataSize;) { *p++ = 0; }

    glyph->setImage(width,height,1,
                    GL_ALPHA,
                    GL_ALPHA,GL_UNSIGNED_BYTE,
                    data,
                    osg::Image::USE_NEW_DELETE,
                    1);

    glyph->setInternalTextureFormat(GL_ALPHA);

    // copy image across to osgText::Glyph image.     
    switch(glyphslot->bitmap.pixel_mode)
    {
        case FT_PIXEL_MODE_MONO:
            for(int r=sourceHeight-1;r>=0;--r)
            {
                unsigned char* ptr = buffer+r*pitch;
                for(unsigned int c=0;c<sourceWidth;++c)
                {
                    (*data++)= (ptr[c >> 3] & (1 << (~c & 7))) ? 255 : 0;
                }
            }
            break;

        
        case FT_PIXEL_MODE_GRAY:
            for(int r=sourceHeight-1;r>=0;--r)
            {
                unsigned char* ptr = buffer+r*pitch;
                for(unsigned int c=0;c<sourceWidth;++c,++ptr)
                {
                    (*data++)=*ptr;
                }
            }
            break;
            
        default:
            osg::notify(osg::WARN) << "FT_Load_Char(...) returned bitmap with unknown pixel_mode " << glyphslot->bitmap.pixel_mode << std::endl;
    }


    FT_Glyph_Metrics* metrics = &(glyphslot->metrics);

    glyph->setHorizontalBearing(osg::Vec2((float)metrics->horiBearingX/64.0f,(float)(metrics->horiBearingY-metrics->height)/64.0f)); // bottom left.
    glyph->setHorizontalAdvance((float)metrics->horiAdvance/64.0f);
    glyph->setVerticalBearing(osg::Vec2((float)metrics->vertBearingX/64.0f,(float)(metrics->vertBearingY-metrics->height)/64.0f)); // top middle.
    glyph->setVerticalAdvance((float)metrics->vertAdvance/64.0f);

    addGlyph(fontRes,charcode,glyph.get());
    
//    cout << "      in getGlyph() implementation="<<this<<"  "<<_filename<<"  facade="<<_facade<<endl;

    return glyph.get();

}

osg::Vec2 FreeTypeFont::getKerning(const osgText::FontResolution& fontRes, unsigned int leftcharcode,unsigned int rightcharcode, osgText::KerningType kerningType)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(FreeTypeLibrary::instance()->getMutex());

    if (!FT_HAS_KERNING(_face) || (kerningType == osgText::KERNING_NONE)) return osg::Vec2(0.0f,0.0f);

    setFontResolution(fontRes);

    FT_Kerning_Mode mode = (kerningType==osgText::KERNING_DEFAULT) ? ft_kerning_default : ft_kerning_unfitted;

    // convert character code to glyph index
    FT_UInt left = FT_Get_Char_Index( _face, leftcharcode );
    FT_UInt right = FT_Get_Char_Index( _face, rightcharcode );
    
    // get the kerning distances.   
    FT_Vector  kerning;

    FT_Error error = FT_Get_Kerning( _face,                     // handle to face object
                                     left,                      // left glyph index
                                     right,                     // right glyph index
                                     mode,                      // kerning mode
                                     &kerning );                // target vector

    if (error)
    {
        osg::notify(osg::WARN) << "FT_Get_Kerning(...) returned error code " <<std::hex<<error<<std::dec<< std::endl;
        return osg::Vec2(0.0f,0.0f);
    }

    return osg::Vec2((float)kerning.x/64.0f,(float)kerning.y/64.0f);
}

bool FreeTypeFont::hasVertical() const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(FreeTypeLibrary::instance()->getMutex());
    return FT_HAS_VERTICAL(_face)!=0;
}
