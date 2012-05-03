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
//#include <ft2build.h>

//#define PRINT_OUT_FONT_DETAILS
#ifdef PRINT_OUT_FONT_DETAILS
    #include <freetype/ftsnames.h>
#endif

#include FT_TRUETYPE_IDS_H

FreeTypeLibrary::FreeTypeLibrary()
{
    OSG_INFO << "FreeTypeLibrary::FreeTypeLibrary()" << std::endl;
    FT_Error error = FT_Init_FreeType( &_ftlibrary );
    if (error)
    {
        OSG_WARN<<"Warning: an error occurred during FT_Init_FreeType(..) initialisation, error code = "<<std::hex<<error<<std::dec<<std::endl;
    }

}

FreeTypeLibrary::~FreeTypeLibrary()
{
    // need to remove the implementations from the Fonts here
    // just in case the Fonts continue to have external references
    // to them, otherwise they will try to point to an object thats
    // definition has been unloaded along with the unload of the FreeType
    // plugin.
    while(!_fontImplementationSet.empty())
    {
        FreeTypeFont* fontImplementation = *_fontImplementationSet.begin();
        _fontImplementationSet.erase(_fontImplementationSet.begin());
        osgText::Font* font = fontImplementation->_facade;
        if (font) font->setImplementation(0);
        else fontImplementation->_facade = 0;
    }

    FT_Done_FreeType( _ftlibrary);
}

FreeTypeLibrary* FreeTypeLibrary::instance()
{
    static osg::ref_ptr<FreeTypeLibrary> s_library = new FreeTypeLibrary;
    return s_library.get();
}

bool FreeTypeLibrary::getFace(const std::string& fontfile,unsigned int index, FT_Face & face)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getMutex());

    FT_Error error = FT_New_Face( _ftlibrary, fontfile.c_str(), index, &face );
    if (error == FT_Err_Unknown_File_Format)
    {
        OSG_WARN<<" .... the font file could be opened and read, but it appears"<<std::endl;
        OSG_WARN<<" .... that its font format is unsupported"<<std::endl;
        return false;
    }
    else if (error)
    {
        OSG_WARN<<" .... another error code means that the font file could not"<<std::endl;
        OSG_WARN<<" .... be opened, read or simply that it is broken.."<<std::endl;
        return false;
    }

#ifdef PRINT_OUT_FONT_DETAILS

    OSG_NOTICE<<"Face"<<face<<std::endl;
    unsigned int count = FT_Get_Sfnt_Name_Count(face);
    for(unsigned int i=0; i<count; ++i)
    {
        FT_SfntName names;
        FT_Error error = FT_Get_Sfnt_Name(face, i, &names);

        std::string name((char*)names.string, (char*)names.string + names.string_len);

        OSG_NOTICE<<"names "<<name<<std::endl;
    }

    OSG_NOTICE<<std::endl;
#endif

    //
    // GT: Fix to handle symbol fonts in MS Windows
    //
    verifyCharacterMap(face);

    return true;
}

FT_Byte* FreeTypeLibrary::getFace(std::istream& fontstream, unsigned int index, FT_Face & face)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getMutex());

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
        OSG_WARN<<" .... the font file could not be read from its stream"<<std::endl;
        if (buffer) delete [] buffer;
        return 0;
    }
    args.flags = FT_OPEN_MEMORY;
    args.memory_base = buffer;
    args.memory_size = length;

    FT_Error error = FT_Open_Face( _ftlibrary, &args, index, &face );

    if (error == FT_Err_Unknown_File_Format)
    {
        OSG_WARN<<" .... the font file could be opened and read, but it appears"<<std::endl;
        OSG_WARN<<" .... that its font format is unsupported"<<std::endl;
        return 0;
    }
    else if (error)
    {
        OSG_WARN<<" .... another error code means that the font file could not"<<std::endl;
        OSG_WARN<<" .... be opened, read or simply that it is broken..."<<std::endl;
        return 0;
    }

    //
    // GT: Fix to handle symbol fonts in MS Windows
    //
    verifyCharacterMap(face);

    return buffer;
}


osgText::Font* FreeTypeLibrary::getFont(const std::string& fontfile, unsigned int index, unsigned int flags)
{
    FT_Face face;
    if (getFace(fontfile, index, face) == false) return (0);

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getMutex());

    FreeTypeFont* fontImp = new FreeTypeFont(fontfile,face,flags);
    osgText::Font* font = new osgText::Font(fontImp);

    _fontImplementationSet.insert(fontImp);

    return font;
}
osgText::Font* FreeTypeLibrary::getFont(std::istream& fontstream, unsigned int index, unsigned int flags)
{
    FT_Face face = 0;
    FT_Byte * buffer = getFace(fontstream, index, face);
    if (face == 0) return (0);


    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getMutex());

    FreeTypeFont* fontImp = new FreeTypeFont(buffer,face,flags);
    osgText::Font* font = new osgText::Font(fontImp);

    _fontImplementationSet.insert(fontImp);

    return font;
}

void FreeTypeLibrary::verifyCharacterMap(FT_Face face)
{
    //
    // GT: Verify the correct character mapping for MS windows
    // as symbol fonts were being returned incorrectly
    //
    FT_CharMap charmap;
    if (face->charmap == NULL)
    {
        for (int n = 0; n < face->num_charmaps; n++)
        {
            charmap = face->charmaps[n];
            if (charmap->platform_id == TT_PLATFORM_MICROSOFT)
            {
                FT_Set_Charmap(face, charmap);
                break;
            }
        }
    }
}
