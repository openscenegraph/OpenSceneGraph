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

#ifndef FREETYPE_LIBRARY
#define FREETYPE_LIBRARY

#include "FreeTypeFont.h"
#include "FreeTypeFont3D.h"

#include <set>
#include <istream>

#include <osgText/Font>
#include <osgText/Font3D>

#include <ft2build.h>

class FreeTypeLibrary : public osg::Referenced
{
public:

    /** protected destrcutor to prevent inappropriate deletion.*/
    virtual ~FreeTypeLibrary();

    /** get the singleton instance.*/
    static FreeTypeLibrary* instance();

    OpenThreads::Mutex& getMutex() { return _mutex; }

    osgText::Font* getFont(const std::string& fontfile,unsigned int index=0, unsigned int flags=0);
    osgText::Font* getFont(std::istream& fontstream, unsigned int index=0, unsigned int flags=0);
    
    osgText::Font3D* getFont3D(const std::string& fontfile, unsigned int index=0, unsigned int flags=0);
    osgText::Font3D* getFont3D(std::istream& fontstream, unsigned int index=0, unsigned int flags=0);
    
    void removeFontImplmentation(FreeTypeFont* fontImpl) { _fontImplementationSet.erase(fontImpl); }
    void removeFont3DImplmentation(FreeTypeFont3D* font3DImpl) { _font3DImplementationSet.erase(font3DImpl); }

protected:

    /** common method to load a FT_Face from a file*/
    bool getFace(const std::string& fontfile,unsigned int index, FT_Face & face);
    /** common method to load a FT_Face from a stream */
    FT_Byte* getFace(std::istream& fontstream, unsigned int index, FT_Face & face);
    
    /** Verify the correct character mapping for MS windows */
    void  verifyCharacterMap(FT_Face face);

    /** protected constructor to ensure the only way to create the 
      * library is via the singleton instance method.*/
    FreeTypeLibrary();

    typedef std::set< FreeTypeFont* > FontImplementationSet;
    typedef std::set< FreeTypeFont3D* > Font3DImplementationSet;

    mutable OpenThreads::Mutex  _mutex;
    FT_Library                  _ftlibrary;
    FontImplementationSet       _fontImplementationSet;
    Font3DImplementationSet     _font3DImplementationSet;

};


#endif
