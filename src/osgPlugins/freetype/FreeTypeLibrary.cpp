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
    // need to remove the implementations from the Fonts here
    // just in case the Fonts continue to have external references
    // to them, otherwise they will try to point to an object thats
    // definiation has been unloaded along with the unload of the FreeType
    // plugin.
    while(!_fontImplementationSet.empty())
    {
        FreeTypeFont* fontImplementation = *_fontImplementationSet.begin();
        _fontImplementationSet.erase(_fontImplementationSet.begin());
        osgText::Font* font = fontImplementation->_facade;
        font->setImplementation(0);
    }
    
    FT_Done_FreeType( _ftlibrary);
}

FreeTypeLibrary* FreeTypeLibrary::instance()
{
    static osg::ref_ptr<FreeTypeLibrary> s_library = new FreeTypeLibrary;
    return s_library.get();
}

osgText::Font* FreeTypeLibrary::getFont(const std::string& fontfile,unsigned int index)
{

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

    _fontImplementationSet.insert(fontImp);
    
    return font;

}

osgText::Font* FreeTypeLibrary::getFont(std::istream& fontstream, unsigned int index)
{
    FT_Face face;    /* handle to face object */
    FT_Open_Args args;

    std::streampos start = fontstream.tellg();
    fontstream.seekg(0, std::ios::end);
    std::streampos end = fontstream.tellg();
    fontstream.seekg(start, std::ios::beg);
    std::streampos length = end - start;

    /* empty stream into memory, open that, and keep the pointer in a FreeTypeFont for cleanup */
    FT_Byte *buffer = new FT_Byte[length];
    fontstream.read(reinterpret_cast<char*>(buffer), length);
    if (!fontstream || (static_cast<std::streampos>(fontstream.gcount()) != length))
    {
        osg::notify(osg::WARN)<<" .... the font file could not be read from its stream"<<std::endl;
        return 0;
    }
    args.flags = FT_OPEN_MEMORY;
    args.memory_base = buffer;
    args.memory_size = length;

    FT_Error error = FT_Open_Face( _ftlibrary, &args, index, &face );

    if (error == FT_Err_Unknown_File_Format)
    {
        osg::notify(osg::WARN)<<" .... the font file could be opened and read, but it appears"<<std::endl;
        osg::notify(osg::WARN)<<" .... that its font format is unsupported"<<std::endl;
        return 0;
    }
    else if (error)
    {
        osg::notify(osg::WARN)<<" .... another error code means that the font file could not"<<std::endl;
        osg::notify(osg::WARN)<<" .... be opened, read or simply that it is broken..."<<std::endl;
        return 0;
    }
    
    FreeTypeFont* fontImp = new FreeTypeFont(buffer,face);
    osgText::Font* font = new osgText::Font(fontImp);
    
    _fontImplementationSet.insert(fontImp);

    return font;
}
