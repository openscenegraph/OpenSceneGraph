/* -*-c++-*- OpenSceneGraph - Copyright (C) 2006 Mathias Froehlich
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

#include "TXFFont.h"
#include <iostream>
#include <osg/Notify>

#define FNT_BYTE_FORMAT         0
#define FNT_BITMAP_FORMAT       1

struct GlyphData {
    unsigned short ch;
    unsigned char width;
    unsigned char height;
    signed char x_off;
    signed char y_off;
    signed char advance;
    short x;
    short y;
};

static inline void swap(unsigned short& x, bool isSwapped)
{
    if (!isSwapped)
        return;
    x = ((x >> 8) & 0x00FF) | ((x << 8) & 0xFF00);
}

static inline void swap(unsigned& x, bool isSwapped)
{
    if (!isSwapped)
        return;
    x = ((x >> 24) & 0x000000FF) | ((x >> 8) & 0x0000FF00) | ((x << 8) & 0x00FF0000) | ((x << 24) & 0xFF000000);
}

static inline unsigned char readByte(std::istream& stream)
{
    unsigned char x;
    stream.read(reinterpret_cast<std::istream::char_type*>(&x), 1);
    return x;
}

static inline unsigned short readShort(std::istream& stream, bool isSwapped)
{
    unsigned short x;
    stream.read(reinterpret_cast<std::istream::char_type*>(&x), 2);
    swap(x, isSwapped);
    return x;
}

static inline unsigned readInt(std::istream& stream, bool isSwapped)
{
    unsigned x;
    stream.read(reinterpret_cast<std::istream::char_type*>(&x), 4);
    swap(x, isSwapped);
    return x;
}

TXFFont::TXFFont(const std::string& filename):
    _filename(filename)
{
}

TXFFont::~TXFFont()
{
}

std::string
TXFFont::getFileName() const
{
    return _filename;
}

osgText::Font::Glyph*
TXFFont::getGlyph(const osgText::FontResolution&, unsigned int charcode)
{
    GlyphMap::iterator i = _chars.find(charcode);
    if (i != _chars.end())
        return i->second.get();

    // ok, not available, we have an additional chance with some translations
    // That is to make the loader compatible with an other prominent one
    if (charcode >= 'A' && charcode <= 'Z')
    {
        i = _chars.find(charcode - 'A' + 'a');
        if (i != _chars.end())
        {
            _chars[charcode] = i->second;
            addGlyph(osgText::FontResolution(i->second->s(), i->second->t()), charcode, i->second.get());
            return i->second.get();
        }
    }
    else if (charcode >= 'a' && charcode <= 'z')
    {
        i = _chars.find(charcode - 'a' + 'A');
        if (i != _chars.end())
        {
            _chars[charcode] = i->second;
            addGlyph(osgText::FontResolution(i->second->s(), i->second->t()), charcode, i->second.get());
            return i->second.get();
        }
    }

    return 0;
}

bool
TXFFont::hasVertical() const
{
    return true;
}

osg::Vec2
TXFFont::getKerning(const osgText::FontResolution&, unsigned int, unsigned int, osgText::KerningType)
{
    return osg::Vec2(0, 0);
}

bool
TXFFont::loadFont(std::istream& stream)
{
    unsigned char magic[4];
    stream.read(reinterpret_cast<std::istream::char_type*>(&magic), 4);

    if (magic[0] != 0xFF || magic[1] != 't' || magic[2] != 'x'  || magic[3] != 'f' )
    {
        osg::notify(osg::FATAL) << "osgdb_txf: input file \"" << _filename << "\" is not a texture font file!" << std::endl;
        return false;
    }

    // read endianess hint
    bool isSwapped = 0x12345678u != readInt(stream, false);

    unsigned format = readInt(stream, isSwapped);
    unsigned texwidth = readInt(stream, isSwapped);
    unsigned texheight = readInt(stream, isSwapped);
    unsigned maxheight = readInt(stream, isSwapped);
    readInt(stream, isSwapped);
    unsigned num_glyphs = readInt(stream, isSwapped);


    unsigned maxwidth = 0;

    unsigned w = texwidth;
    unsigned h = texheight;
    
    osgText::FontResolution fontResolution(maxheight, maxheight);

    std::vector<GlyphData> glyphs;
    for (unsigned i = 0; i < num_glyphs; ++i)
    {
        GlyphData glyphData;
        glyphData.ch = readShort(stream, isSwapped);
        glyphData.width = readByte(stream);
        glyphData.height = readByte(stream);
        glyphData.x_off = readByte(stream);
        glyphData.y_off = readByte(stream);
        glyphData.advance = readByte(stream);
        readByte(stream);
        glyphData.x = readShort(stream, isSwapped);
        glyphData.y = readShort(stream, isSwapped);

        maxwidth = std::max(maxwidth, (unsigned)glyphData.width);

        glyphs.push_back(glyphData);
    }

    unsigned ntexels = w * h;
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage(w, h, 1, GL_ALPHA, GL_UNSIGNED_BYTE);

    if (format == FNT_BYTE_FORMAT)
    {
        stream.read(reinterpret_cast<std::istream::char_type*>(image->data()), ntexels);
        if (!stream)
        {
            osg::notify(osg::FATAL) << "osgdb_txf: unxpected end of file in txf file \"" << _filename << "\"!" << std::endl;
            return false;
        }
    }
    else if (format == FNT_BITMAP_FORMAT)
    {
        unsigned stride = (w + 7) >> 3;
        unsigned char *texbitmap = new unsigned char[stride*h] ;
        stream.read(reinterpret_cast<std::istream::char_type*>(texbitmap), stride*h);
        if (!stream)
        {
            delete [] texbitmap;
            osg::notify(osg::FATAL) << "osgdb_txf: unxpected end of file in txf file \"" << _filename << "\"!" << std::endl;
            return false;
        }

        for (unsigned i = 0; i < h; i++)
        {
            for (unsigned j = 0; j < w; j++)
            {
                if (texbitmap[i * stride + (j >> 3)] & (1 << (j & 7)))
                {
                    *image->data(j, i) = 255;
                }
                else
                {
                    *image->data(j, i) = 0;
                }
            }
        }

        delete [] texbitmap;
    }
    else
    {
        osg::notify(osg::FATAL) << "osgdb_txf: unxpected txf file!" << std::endl;
        return false;
    }

    for (unsigned i = 0; i < glyphs.size(); ++i)
    {
        // have a special one for that
        if (glyphs[i].ch == ' ')
            continue;

        // add the characters ...
        osgText::Font::Glyph* glyph = new osgText::Font::Glyph;

        unsigned sourceWidth = glyphs[i].width;
        unsigned sourceHeight = glyphs[i].height;

        unsigned width = sourceWidth;
        unsigned height = sourceHeight;

        glyph->allocateImage(width, height, 1, GL_ALPHA, GL_UNSIGNED_BYTE);
        glyph->setInternalTextureFormat(GL_ALPHA);

        for (unsigned k = 0; k < width; ++k)
        {
            for (unsigned l = 0; l < height; ++l)
            {
                *glyph->data(k, l) = 0;
            }
        }

        for (unsigned k = 0; k < glyphs[i].width; ++k)
        {
            for (unsigned l = 0; l < glyphs[i].height; ++l)
            {
                *glyph->data(k, l) = *image->data(glyphs[i].x + k, glyphs[i].y + l);
            }
        }

        float texToVertX = float(glyphs[i].width)/width;
        float texToVertY = float(glyphs[i].height)/height;
        glyph->setHorizontalAdvance(glyphs[i].advance + 0.1f*maxheight);
        glyph->setHorizontalBearing(osg::Vec2((glyphs[i].x_off - 0.5f)*texToVertX,
                                              (glyphs[i].y_off - 0.5f)*texToVertY));
        glyph->setVerticalAdvance(sourceHeight);
        glyph->setVerticalBearing(osg::Vec2(glyphs[i].x_off*texToVertX - 0.5f*glyphs[i].advance*texToVertX,
                                            - glyphs[i].height*texToVertY));

        _chars[glyphs[i].ch] = glyph;
        addGlyph(fontResolution, glyphs[i].ch, glyph);
    }

    // insert a trivial blank character
    osgText::Font::Glyph* glyph = new osgText::Font::Glyph;

    unsigned width = 1;
    unsigned height = 1;
    
    glyph->allocateImage(width, height, 1, GL_ALPHA, GL_UNSIGNED_BYTE);
    glyph->setInternalTextureFormat(GL_ALPHA);

    for (unsigned k = 0; k < width; ++k)
    {
        for (unsigned l = 0; l < height; ++l)
        {
            *glyph->data(k, l) = 0;
        }
    }

    glyph->setHorizontalAdvance(0.5f*float(fontResolution.second));
    glyph->setHorizontalBearing(osg::Vec2(0.0f, 0.0f));
    glyph->setVerticalAdvance(float(fontResolution.second));
    glyph->setVerticalBearing(osg::Vec2(0.0f, 0.0f));
    _chars[' '] = glyph;
    addGlyph(fontResolution, ' ', glyph);

    return true;
}
