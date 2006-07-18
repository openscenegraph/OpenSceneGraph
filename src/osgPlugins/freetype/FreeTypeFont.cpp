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

FreeTypeFont::FreeTypeFont(const std::string& filename, FT_Face face):
    _filename(filename),
    _buffer(0),
    _face(face)
{
}

FreeTypeFont::FreeTypeFont(FT_Byte* buffer, FT_Face face):
    _filename(""),
    _buffer(buffer),
    _face(face)
{
}

FreeTypeFont::~FreeTypeFont()
{
    if(_face)
    {
        FreeTypeLibrary* freeTypeLibrary = FreeTypeLibrary::instance();
        if (freeTypeLibrary)
        {
            // remove myself from the local regsitry to ensure that
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

void FreeTypeFont::setFontResolution(unsigned int width, unsigned int height)
{
    if (width+2*_facade->getGlyphImageMargin()>_facade->getTextureWidthHint() ||
        height+2*_facade->getGlyphImageMargin()>_facade->getTextureHeightHint())
    {
        osg::notify(osg::WARN)<<"Warning: FreeTypeFont::setSize("<<width<<","<<height<<") sizes too large,"<<std::endl;
    
        width = _facade->getTextureWidthHint()-2*_facade->getGlyphImageMargin();
        height = _facade->getTextureHeightHint()-2*_facade->getGlyphImageMargin();

        osg::notify(osg::WARN)<<"         sizes capped ("<<width<<","<<height<<") to fit int current glyph texture size."<<std::endl;
    }

    FT_Error error = FT_Set_Pixel_Sizes( _face,   /* handle to face object            */
                                         width,      /* pixel_width                      */
                                         height );   /* pixel_height */

    if (error)
    {
        osg::notify(osg::WARN)<<"FT_Set_Pixel_Sizes() - error "<<error<<std::endl;
    }
    else
    {
        setFontWidth(width);
        setFontHeight(height);
    }

}

osgText::Font::Glyph* FreeTypeFont::getGlyph(unsigned int charcode)
{
    FT_Error error = FT_Load_Char( _face, charcode, FT_LOAD_RENDER|FT_LOAD_NO_BITMAP );
    if (error)
    {
        osg::notify(osg::WARN) << "FT_Load_Char(...) error "<<error<<std::endl;
        return 0;
    }


    FT_GlyphSlot glyphslot = _face->glyph;

    int pitch = glyphslot->bitmap.pitch;
    unsigned char* buffer = glyphslot->bitmap.buffer;

    unsigned int sourceWidth = glyphslot->bitmap.width;;
    unsigned int sourceHeight = glyphslot->bitmap.rows;
    
    unsigned int margin = _facade->getGlyphImageMargin();
    unsigned int width = sourceWidth+2*margin;
    unsigned int height = sourceHeight+2*margin;

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

    // skip the top margin        
    data += (margin*width);

    // copy image across to osgText::Glyph image.     
    for(int r=sourceHeight-1;r>=0;--r)
    {
        data+=margin; // skip the left margin

        unsigned char* ptr = buffer+r*pitch;
        for(unsigned int c=0;c<sourceWidth;++c,++ptr)
        {
            (*data++)=*ptr;
        }
        data+=margin; // skip the right margin.
    }


    FT_Glyph_Metrics* metrics = &(glyphslot->metrics);

    glyph->setHorizontalBearing(osg::Vec2((float)metrics->horiBearingX/64.0f,(float)(metrics->horiBearingY-metrics->height)/64.0f)); // bottom left.
    glyph->setHorizontalAdvance((float)metrics->horiAdvance/64.0f);
    glyph->setVerticalBearing(osg::Vec2((float)metrics->vertBearingX/64.0f,(float)(metrics->vertBearingY-metrics->height)/64.0f)); // top middle.
    glyph->setVerticalAdvance((float)metrics->vertAdvance/64.0f);

    addGlyph(_facade->getFontWidth(),_facade->getFontHeight(),charcode,glyph.get());
    
//    cout << "      in getGlyph() implementation="<<this<<"  "<<_filename<<"  facade="<<_facade<<endl;

    return glyph.get();

}

osg::Vec2 FreeTypeFont::getKerning(unsigned int leftcharcode,unsigned int rightcharcode, osgText::KerningType kerningType)
{
    if (!FT_HAS_KERNING(_face) || (kerningType == osgText::KERNING_NONE)) return osg::Vec2(0.0f,0.0f);

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
        return osg::Vec2(0.0f,0.0f);
    }

    return osg::Vec2((float)kerning.x/64.0f,(float)kerning.y/64.0f);
}

bool FreeTypeFont::hasVertical() const
{
    return FT_HAS_VERTICAL(_face)!=0;
}
