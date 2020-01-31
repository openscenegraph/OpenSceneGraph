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

#include <osgText/Font>
#include <osgText/Text>

#include <osg/State>
#include <osg/Notify>
#include <osg/ApplicationUsage>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osg/GLU>

#include <string.h>

#include <OpenThreads/ReentrantMutex>

#ifdef WITH_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

#include "DefaultFont.h"

using namespace osgText;
using namespace std;

osg::ref_ptr<Font> Font::getDefaultFont()
{
    static OpenThreads::Mutex s_DefaultFontMutex;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_DefaultFontMutex);

    osg::ref_ptr<osg::Object> object = osgDB::Registry::instance()->getObjectCache()->getFromObjectCache("DefaultFont");
    osg::ref_ptr<osgText::Font> font = dynamic_cast<osgText::Font*>(object.get());
    if (!font)
    {
        font = new DefaultFont;
        osgDB::Registry::instance()->getObjectCache()->addEntryToObjectCache("DefaultFont", font.get());
        return font;
    }
    else
    {
        return font;
    }
}

static OpenThreads::ReentrantMutex& getFontFileMutex()
{
    static OpenThreads::ReentrantMutex s_FontFileMutex;
    return s_FontFileMutex;
}

#ifdef WITH_FONTCONFIG
static FcConfig* getFontConfig()
{
    static FcConfig* s_fontConfig = FcInitLoadConfigAndFonts();
    return s_fontConfig;
}

static bool getFilePathFromFontconfig(const std::string &fontName, std::string &fontPath)
{
    FcPattern* pat = FcNameParse((FcChar8*)fontName.c_str());
    FcConfigSubstitute(getFontConfig(), pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    FcResult result = FcResultNoMatch;
    FcPattern* font = FcFontMatch(getFontConfig(), pat, &result);
    if (font)
    {
        FcChar8* file = NULL;
        if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch)
        {
            fontPath = (char *)file;
        }
        FcPatternDestroy(font);
    }
    FcPatternDestroy(pat);
    return result == FcResultMatch;
}
#endif

std::string osgText::findFontFile(const std::string& str)
{
    // try looking in OSGFILEPATH etc first for fonts.
    std::string filename = osgDB::findDataFile(str);
    if (!filename.empty()) return filename;

    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(getFontFileMutex());

    static osgDB::FilePathList s_FontFilePath;
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;
    #if defined(WIN32)
        osgDB::convertStringPathIntoFilePathList(
            ".;C:/winnt/fonts;C:/windows/fonts",
            s_FontFilePath);

        char *ptr;
        if ((ptr = getenv( "windir" )))
        {
            std::string winFontPath = ptr;
            winFontPath += "\\fonts";
            s_FontFilePath.push_back(winFontPath);
        }
    #elif defined(__APPLE__)
      osgDB::convertStringPathIntoFilePathList(
        ".:/usr/share/fonts/ttf:/usr/share/fonts/ttf/western:/usr/share/fonts/ttf/decoratives:/Library/Fonts:/System/Library/Fonts",
        s_FontFilePath);
    #else
      osgDB::convertStringPathIntoFilePathList(
        ".:/usr/share/fonts/ttf:/usr/share/fonts/truetype:/usr/share/fonts/ttf/western:/usr/share/fonts/ttf/decoratives",
        s_FontFilePath);
    #endif
    }

#ifdef WITH_FONTCONFIG
    std::string fontPath;
    if (getFilePathFromFontconfig(str, fontPath))
        return fontPath;
#endif

    filename = osgDB::findFileInPath(str,s_FontFilePath);
    if (!filename.empty()) return filename;

    // Try filename without pathname, if it has a path
    filename = osgDB::getSimpleFileName(str);
    if(filename!=str)
    {
        filename = osgDB::findFileInPath(filename,s_FontFilePath);
        if (!filename.empty()) return filename;
    }
    else
    {
        filename = osgText::findFontFile(std::string("fonts/")+filename);
        if (!filename.empty()) return filename;
    }

    // Not found, return empty string
    OSG_INFO<<"Warning: font file \""<<str<<"\" not found."<<std::endl;
    return std::string();
}

#ifdef OSG_PROVIDE_READFILE
osgText::Font* osgText::readFontFile(const std::string& filename, const osgDB::ReaderWriter::Options* userOptions)
{
    if (filename.empty()) return 0;

    std::string foundFile = findFontFile(filename);
    if (foundFile.empty())
        foundFile = filename;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getFontFileMutex());

    osg::ref_ptr<osgDB::ReaderWriter::Options> localOptions;
    if (!userOptions)
    {
        localOptions = new osgDB::ReaderWriter::Options;
        localOptions->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_OBJECTS);
    }

    osg::Object* object = osgDB::readObjectFile(foundFile, userOptions ? userOptions : localOptions.get());

    // if the object is a font then return it.
    osgText::Font* font = dynamic_cast<osgText::Font*>(object);
    if (font) return font;

    // otherwise if the object has zero references then delete it by doing another unref().
    if (object && object->referenceCount()==0) object->unref();
    return 0;
}

osgText::Font* osgText::readFontStream(std::istream& stream, const osgDB::ReaderWriter::Options* userOptions)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getFontFileMutex());

    osg::ref_ptr<osgDB::ReaderWriter::Options> localOptions;
    if (!userOptions)
    {
        localOptions = new osgDB::ReaderWriter::Options;
        localOptions->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_OBJECTS);
    }

    // there should be a better way to get the FreeType ReaderWriter by name...
    osgDB::ReaderWriter *reader = osgDB::Registry::instance()->getReaderWriterForExtension("ttf");
    if (reader == 0) return 0;
    osgDB::ReaderWriter::ReadResult rr = reader->readObject(stream, userOptions ? userOptions : localOptions.get());
    if (!rr.success())
    {
        OSG_WARN << rr.statusMessage() << std::endl;
        return 0;
    }
    if (!rr.validObject()) return 0;

    osg::Object *object = rr.takeObject();

    // if the object is a font then return it.
    osgText::Font* font = dynamic_cast<osgText::Font*>(object);
    if (font) return font;

    // otherwise if the object has zero references then delete it by doing another unref().
    if (object && object->referenceCount()==0) object->unref();
    return 0;
}
#endif

osg::ref_ptr<Font> osgText::readRefFontFile(const std::string& filename, const osgDB::ReaderWriter::Options* userOptions)
{
    if (filename.empty()) return 0;

    std::string foundFile = findFontFile(filename);
    if (foundFile.empty())
        foundFile = filename;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getFontFileMutex());

    osg::ref_ptr<osgDB::ReaderWriter::Options> localOptions;
    if (!userOptions)
    {
        localOptions = new osgDB::ReaderWriter::Options;
        localOptions->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_OBJECTS);
    }

    osg::ref_ptr<osg::Object> object = osgDB::readRefObjectFile(foundFile, userOptions ? userOptions : localOptions.get());

    // if the object is a font then return it.
    osgText::Font* font = dynamic_cast<osgText::Font*>(object.get());
    if (font) return osg::ref_ptr<Font>(font);

    return 0;
}

osg::ref_ptr<Font> osgText::readRefFontStream(std::istream& stream, const osgDB::ReaderWriter::Options* userOptions)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getFontFileMutex());

    osg::ref_ptr<osgDB::ReaderWriter::Options> localOptions;
    if (!userOptions)
    {
        localOptions = new osgDB::ReaderWriter::Options;
        localOptions->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_OBJECTS);
    }

    // there should be a better way to get the FreeType ReaderWriter by name...
    osgDB::ReaderWriter *reader = osgDB::Registry::instance()->getReaderWriterForExtension("ttf");
    if (reader == 0) return 0;
    osgDB::ReaderWriter::ReadResult rr = reader->readObject(stream, userOptions ? userOptions : localOptions.get());
    if (!rr.success())
    {
        OSG_WARN << rr.statusMessage() << std::endl;
        return 0;
    }
    if (!rr.validObject()) return 0;

    // if the object is a font then return it.
    osgText::Font* font = dynamic_cast<osgText::Font*>(rr.getObject());
    if (font) return osg::ref_ptr<Font>(font);

    return 0;
}

Font::Font(FontImplementation* implementation):
    osg::Object(true),
    _textureWidthHint(1024),
    _textureHeightHint(1024),
    _minFilterHint(osg::Texture::LINEAR_MIPMAP_LINEAR),
    _magFilterHint(osg::Texture::LINEAR),
    _maxAnisotropy(16),
    _depth(1),
    _numCurveSamples(10)
{
    setImplementation(implementation);

    char *ptr;
    if ((ptr = getenv("OSG_MAX_TEXTURE_SIZE")) != 0)
    {
        unsigned int osg_max_size = atoi(ptr);

        if (osg_max_size<_textureWidthHint) _textureWidthHint = osg_max_size;
        if (osg_max_size<_textureHeightHint) _textureHeightHint = osg_max_size;
    }
}

Font::~Font()
{
    if (_implementation.valid()) _implementation->_facade = 0;
}

void Font::setImplementation(FontImplementation* implementation)
{
    if (_implementation.valid()) _implementation->_facade = 0;
    _implementation = implementation;
    if (_implementation.valid()) _implementation->_facade = this;
}

Font::FontImplementation* Font::getImplementation()
{
    return _implementation.get();
}

const Font::FontImplementation* Font::getImplementation() const
{
    return _implementation.get();
}

std::string Font::getFileName() const
{
    if (_implementation.valid()) return _implementation->getFileName();
    return std::string();
}

void Font::setTextureSizeHint(unsigned int width,unsigned int height)
{
    _textureWidthHint = width;
    _textureHeightHint = height;

    char *ptr;
    if( (ptr = getenv("OSG_MAX_TEXTURE_SIZE")) != 0)
    {
        unsigned int osg_max_size = atoi(ptr);

        if (osg_max_size<_textureWidthHint) _textureWidthHint = osg_max_size;
        if (osg_max_size<_textureHeightHint) _textureHeightHint = osg_max_size;
    }
}

unsigned int Font::getTextureWidthHint() const
{
    return _textureWidthHint;
}

unsigned int Font::getTextureHeightHint() const
{
    return _textureHeightHint;
}


void Font::setMinFilterHint(osg::Texture::FilterMode mode)
{
    _minFilterHint = mode;
}

osg::Texture::FilterMode Font::getMinFilterHint() const
{
    return _minFilterHint;
}

/** Set the magnification texture filter to use when creating the texture to store the glyph images when rendering.
  * Note, this doesn't affect already created Texture Glhph's.*/
void Font::setMagFilterHint(osg::Texture::FilterMode mode)
{
    _magFilterHint = mode;
}

osg::Texture::FilterMode Font::getMagFilterHint() const
{
    return _magFilterHint;
}


Glyph* Font::getGlyph(const FontResolution& fontRes, unsigned int charcode)
{
    if (!_implementation) return 0;

    FontResolution fontResUsed(0,0);
    if (_implementation->supportsMultipleFontResolutions()) fontResUsed = fontRes;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_glyphMapMutex);
    FontSizeGlyphMap::iterator itr = _sizeGlyphMap.find(fontResUsed);
    if (itr!=_sizeGlyphMap.end())
    {
        GlyphMap& glyphmap = itr->second;
        GlyphMap::iterator gitr = glyphmap.find(charcode);
        if (gitr!=glyphmap.end()) return gitr->second.get();
    }

    Glyph* glyph = _implementation->getGlyph(fontResUsed, charcode);
    if (glyph)
    {
        _sizeGlyphMap[fontResUsed][charcode] = glyph;
        return glyph;
    }
    else return 0;
}

Glyph3D* Font::getGlyph3D(const FontResolution &fontRes, unsigned int charcode)
{
    if (!_implementation) return 0;

    FontResolution fontResUsed(0,0);
    if (_implementation->supportsMultipleFontResolutions()) fontResUsed = fontRes;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_glyphMapMutex);
    FontSizeGlyph3DMap::iterator itr = _sizeGlyph3DMap.find(fontResUsed);
    if (itr!=_sizeGlyph3DMap.end())
    {
        Glyph3DMap& glyphmap = itr->second;
        Glyph3DMap::iterator gitr = glyphmap.find(charcode);
        if (gitr!=glyphmap.end()) return gitr->second.get();
    }

    Glyph3D* glyph = _implementation->getGlyph3D(fontResUsed, charcode);
    if (glyph)
    {
        _sizeGlyph3DMap[fontResUsed][charcode] = glyph;
        return glyph;
    }
    else return 0;
}

void Font::setThreadSafeRefUnref(bool threadSafe)
{
   osg::Object::setThreadSafeRefUnref(threadSafe);

    for(GlyphTextureList::const_iterator itr=_glyphTextureList.begin();
        itr!=_glyphTextureList.end();
        ++itr)
    {
        (*itr)->setThreadSafeRefUnref(threadSafe);
    }
}

void Font::resizeGLObjectBuffers(unsigned int maxSize)
{
    for(StateSets::iterator itr = _statesets.begin();
        itr != _statesets.end();
        ++itr)
    {
        (*itr)->resizeGLObjectBuffers(maxSize);
    }

    for(GlyphTextureList::const_iterator itr=_glyphTextureList.begin();
        itr!=_glyphTextureList.end();
        ++itr)
    {
        (*itr)->resizeGLObjectBuffers(maxSize);
    }
}

void Font::releaseGLObjects(osg::State* state) const
{
    for(StateSets::const_iterator itr = _statesets.begin();
        itr != _statesets.end();
        ++itr)
    {
        (*itr)->releaseGLObjects(state);
    }

    for(GlyphTextureList::const_iterator itr=_glyphTextureList.begin();
        itr!=_glyphTextureList.end();
        ++itr)
    {
        (*itr)->releaseGLObjects(state);
    }

    // const_cast<Font*>(this)->_glyphTextureList.clear();
    // const_cast<Font*>(this)->_sizeGlyphMap.clear();
}

osg::Vec2 Font::getKerning(const FontResolution& fontRes, unsigned int leftcharcode, unsigned int rightcharcode, KerningType kerningType)
{
    if (_implementation.valid()) return _implementation->getKerning(fontRes, leftcharcode, rightcharcode, kerningType);
    else return osg::Vec2(0.0f,0.0f);
}

bool Font::hasVertical() const
{
    if (_implementation.valid()) return _implementation->hasVertical();
    else return false;
}



void Font::addGlyph(const FontResolution& fontRes, unsigned int charcode, Glyph* glyph)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_glyphMapMutex);

    _sizeGlyphMap[fontRes][charcode]=glyph;

}

void Font::assignGlyphToGlyphTexture(Glyph* glyph, ShaderTechnique shaderTechnique)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_glyphMapMutex);

    int posX=0,posY=0;

    GlyphTexture* glyphTexture = 0;
    for(GlyphTextureList::iterator itr=_glyphTextureList.begin();
        itr!=_glyphTextureList.end() && !glyphTexture;
        ++itr)
    {
        if ((*itr)->getShaderTechnique()==shaderTechnique && (*itr)->getSpaceForGlyph(glyph,posX,posY)) glyphTexture = itr->get();
    }

    if (glyphTexture)
    {
        //cout << "    Font::assignGlyphToGlyphTexture() found space for texture "<<glyphTexture<<" posX="<<posX<<" posY="<<posY<<endl;
    }

    if (!glyphTexture)
    {

        glyphTexture = new GlyphTexture;

        static int numberOfTexturesAllocated = 0;
        ++numberOfTexturesAllocated;

        OSG_INFO<< "   Font " << this<< ", numberOfTexturesAllocated "<<numberOfTexturesAllocated<<std::endl;

        // reserve enough space for the glyphs.
        glyphTexture->setShaderTechnique(shaderTechnique);
        glyphTexture->setTextureSize(_textureWidthHint,_textureHeightHint);
        glyphTexture->setFilter(osg::Texture::MIN_FILTER,_minFilterHint);
        glyphTexture->setFilter(osg::Texture::MAG_FILTER,_magFilterHint);
        glyphTexture->setMaxAnisotropy(_maxAnisotropy);

        _glyphTextureList.push_back(glyphTexture);

        if (!glyphTexture->getSpaceForGlyph(glyph,posX,posY))
        {
            OSG_WARN<<"Warning: unable to allocate texture big enough for glyph"<<std::endl;
            return;
        }

    }

    // add the glyph into the texture.
    glyphTexture->addGlyph(glyph,posX,posY);
}
