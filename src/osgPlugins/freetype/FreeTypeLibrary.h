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

#ifndef OSGTEXT_LIBRARY
#define OSGTEXT_LIBRARY

#include "FreeTypeFont.h"

class FreeTypeLibrary
{
public:

    /** protected destrcutor to prevent inappropriate deletion.*/
    virtual ~FreeTypeLibrary();

    /** get the singleton instance.*/
    static FreeTypeLibrary* instance();

    FreeTypeFont* getFont(const std::string& fontfile,unsigned int index=0);

protected:

    /** protected constructor to ensure the only way to create the 
      * library is via the singleton instance method.*/
    FreeTypeLibrary();


    FT_Library  _ftlibrary;

};


#endif
