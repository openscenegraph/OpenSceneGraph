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

#include "FreeTypeLibrary.h"


FreeTypeLibrary::FreeTypeLibrary()
{
    FT_Error error = FT_Init_FreeType( &_ftlibrary );
    if (error)
    {
        std::cout<<"Warning: an error occured during FT_Init_FreeType(..) initialisation .. "<<std::endl;
    }

}

FreeTypeLibrary::~FreeTypeLibrary()
{
    FT_Done_FreeType( _ftlibrary);
}

FreeTypeLibrary* FreeTypeLibrary::instance()
{
    static FreeTypeLibrary s_library;
    return &s_library;
}

FreeTypeFont* FreeTypeLibrary::getFont(const std::string& fontfile,unsigned int index)
{
    FT_Face face;      /* handle to face object */
    FT_Error error = FT_New_Face( _ftlibrary, fontfile.c_str(), index, &face );
    if (error == FT_Err_Unknown_File_Format)
    {
        std::cout<<" .... the font file could be opened and read, but it appears"<<std::endl;
        std::cout<<" .... that its font format is unsupported"<<std::endl;
        return 0;
    }
    else if (error)
    {
        std::cout<<" .... another error code means that the font file could notd"<<std::endl;
        std::cout<<" .... be opened, read or simply that it is broken..d"<<std::endl;
        return 0;
    }
    
    return new FreeTypeFont(fontfile,face);

}
