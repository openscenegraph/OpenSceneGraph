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
#include <osgText/Font>
#include <set>
#include <istream>

class FreeTypeLibrary : public osg::Referenced
{
public:

    /** protected destrcutor to prevent inappropriate deletion.*/
    virtual ~FreeTypeLibrary();

    /** get the singleton instance.*/
    static FreeTypeLibrary* instance();

    osgText::Font* getFont(const std::string& fontfile,unsigned int index=0);
    osgText::Font* getFont(std::istream& fontstream, unsigned int index=0);
    
    void removeFontImplmentation(FreeTypeFont* fontImpl) { _fontImplementationSet.erase(fontImpl); }

protected:

    /** protected constructor to ensure the only way to create the 
      * library is via the singleton instance method.*/
    FreeTypeLibrary();

    typedef std::set< FreeTypeFont* > FontImplementationSet;


    FT_Library              _ftlibrary;
    FontImplementationSet   _fontImplementationSet;

};


#endif
