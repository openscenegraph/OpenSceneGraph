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

#ifndef FREETYPE_FONT3D
#define FREETYPE_FONT3D 1

#include <osgText/Font3D>

#include <ft2build.h>
#include FT_FREETYPE_H

class FreeTypeFont3D : public osgText::Font3D::Font3DImplementation
{
// declare the interface to a font.
public:

    FreeTypeFont3D(const std::string& filename, FT_Face face, unsigned int flags);
    FreeTypeFont3D(FT_Byte* buffer, FT_Face face, unsigned int flags);

    virtual std::string getFileName() const { return _filename; }

//    virtual void setFontResolution(unsigned int width, unsigned int height, unsigned int depth);

    virtual osgText::Font3D::Glyph3D * getGlyph(unsigned int charcode);
        
    virtual osg::Vec2 getKerning(unsigned int leftcharcode,unsigned int rightcharcode, osgText::KerningType _kerningType);
    
    virtual bool hasVertical() const;
    
    virtual float getScale() const;

protected:

    void init();
    
    long ft_round( long x ) { return (( x + 32 ) & -64); }
    long ft_floor( long x ) { return (x & -64); }
    long ft_ceiling( long x ){ return (( x + 63 ) & -64); }
    
    virtual ~FreeTypeFont3D();
    
    std::string     _filename;
    FT_Byte*        _buffer;
    FT_Face         _face;
    unsigned int    _flags;
    
    
    double  _scale;
    double  _shiftY;
    double  _shiftX;
    double  _charScale;
};

#endif
