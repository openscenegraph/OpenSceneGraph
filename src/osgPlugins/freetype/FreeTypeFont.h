/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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

#ifndef FREETYPE_FONT
#define FREETYPE_FONT 1

#include <osgText/Font>

#include <ft2build.h>
#include FT_FREETYPE_H

class FreeTypeFont : public osgText::Font
{
// declare the interface to a font.
public:

    FreeTypeFont(const std::string& filename, FT_Face face);

    virtual std::string getFileName() const { return _filename; }

    virtual void setSize(unsigned int width, unsigned int height);

    virtual osgText::Font::Glyph* getGlyph(unsigned int charcode);
    
    virtual osg::Vec2 getKerning(unsigned int leftcharcode,unsigned int rightcharcode);
    
    virtual bool hasVertical() const;

protected:

    std::string     _filename;
    FT_Face         _face;

};

#endif
