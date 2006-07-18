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

#ifndef NEWTEXT_DEFAULTFONT_H
#define NEWTEXT_DEFAULTFONT_H 1

#include <map>

#include <osg/ref_ptr>

#include <osgText/Font>

namespace osgText {

class DefaultFont : public Font
{
public:

    static DefaultFont* instance();

    virtual std::string getFileName() const { return ""; }

    /** NOP with DefaultFont since it only supports a single fixed sized font. */
    virtual void setSize(unsigned int width, unsigned int height);

    virtual Font::Glyph* getGlyph(unsigned int charcode);
    
    virtual osg::Vec2 getKerning(unsigned int leftcharcode,unsigned int rightcharcode, KerningType kerningType);
    
    virtual bool hasVertical() const;

protected:

    DefaultFont();
    virtual ~DefaultFont();
    
    void constructGlyphs();
    
    
};

}

#endif
