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
#include <osg/Notify>

FreeTypeLibrary::FreeTypeLibrary()
{
    FT_Error error = FT_Init_FreeType( &_ftlibrary );
    if (error)
    {
        osg::notify(osg::WARN)<<"Warning: an error occured during FT_Init_FreeType(..) initialisation .. "<<std::endl;
    }

}

FreeTypeLibrary::~FreeTypeLibrary()
{
#ifdef USE_LOCAL_CACHE
    // need to remove the implementations from the Fonts here
    // just in case the Fonts continue to have external references
    // to them, otherwise they will try to point to an object thats
    // definiation has been unloaded along with the unload of the FreeType
    // plugin.
    for(FontMap::iterator itr=_fontMap.begin();
        itr!=_fontMap.end();
        ++itr)
    {
        osgText::Font* font = itr->second.get();
        font->setImplementation(0);
    }
#endif
    
    FT_Done_FreeType( _ftlibrary);
}

FreeTypeLibrary* FreeTypeLibrary::instance()
{
    static FreeTypeLibrary s_library;
    return &s_library;
}

osgText::Font* FreeTypeLibrary::getFont(const std::string& fontfile,unsigned int index)
{

#ifdef USE_LOCAL_CACHE
    FontMap::iterator itr = _fontMap.find(fontfile);
    if (itr!=_fontMap.end()) return itr->second.get();
#endif

    FT_Face face;      /* handle to face object */
    FT_Error error = FT_New_Face( _ftlibrary, fontfile.c_str(), index, &face );
    if (error == FT_Err_Unknown_File_Format)
    {
        osg::notify(osg::WARN)<<" .... the font file could be opened and read, but it appears"<<std::endl;
        osg::notify(osg::WARN)<<" .... that its font format is unsupported"<<std::endl;
        return 0;
    }
    else if (error)
    {
        osg::notify(osg::WARN)<<" .... another error code means that the font file could notd"<<std::endl;
        osg::notify(osg::WARN)<<" .... be opened, read or simply that it is broken..d"<<std::endl;
        return 0;
    }
    
    FreeTypeFont* fontImp = new FreeTypeFont(fontfile,face);
    osgText::Font* font = new osgText::Font(fontImp);

#ifdef USE_LOCAL_CACHE
    _fontMap[fontfile]=font;
#endif
    
    return font;

}
