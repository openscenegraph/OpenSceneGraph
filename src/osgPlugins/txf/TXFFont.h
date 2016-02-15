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

#ifndef TXF_FONT
#define TXF_FONT

#include <iosfwd>
#include <string>
#include <map>
#include <osgText/Font>

class TXFFont : public osgText::Font::FontImplementation
{
public:
    TXFFont(const std::string& filename);

    virtual ~TXFFont();

    virtual std::string getFileName() const;

    virtual bool supportsMultipleFontResolutions() const { return false; }

    virtual osgText::Glyph* getGlyph(const osgText::FontResolution& fontRes, unsigned int charcode);

    virtual osgText::Glyph3D* getGlyph3D(const osgText::FontResolution&, unsigned int) { return 0; }

    virtual bool hasVertical() const;

    virtual osg::Vec2 getKerning(const osgText::FontResolution& fontRes, unsigned int leftcharcode, unsigned int rightcharcode, osgText::KerningType kerningType);

    bool loadFont(std::istream& stream);

protected:
    typedef std::map<unsigned int, osg::ref_ptr<osgText::Glyph> > GlyphMap;

    std::string _filename;
    GlyphMap _chars;
};

#endif
