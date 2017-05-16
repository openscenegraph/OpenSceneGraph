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

#include "DefaultFont.h"

using namespace osgText;
using namespace std;

static osg::ApplicationUsageProxy Font_e0(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_TEXT_INCREMENTAL_SUBLOADING <type>","ON | OFF");

#if (!defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GLES3_AVAILABLE))
    #define GLSL_VERSION_STR "330 core"
    #define GLYPH_CMP "r"
#else
    #define GLSL_VERSION_STR "300 es"
    #define GLYPH_CMP "a"
#endif

osg::ref_ptr<Font>& Font::getDefaultFont()
{
    static OpenThreads::Mutex s_DefaultFontMutex;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_DefaultFontMutex);

    static osg::ref_ptr<Font> s_defaultFont = new DefaultFont;
    return s_defaultFont;
}

static OpenThreads::ReentrantMutex& getFontFileMutex()
{
    static OpenThreads::ReentrantMutex s_FontFileMutex;
    return s_FontFileMutex;
}

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
        ".:/usr/share/fonts/ttf:/usr/share/fonts/ttf/western:/usr/share/fonts/ttf/decoratives",
        s_FontFilePath);
    #endif
    }

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
    if (rr.error())
    {
        OSG_WARN << rr.message() << std::endl;
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
    if (rr.error())
    {
        OSG_WARN << rr.message() << std::endl;
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
    _margin(1),
    _marginRatio(0.02),
    _textureWidthHint(1024),
    _textureHeightHint(1024),
    _minFilterHint(osg::Texture::LINEAR_MIPMAP_LINEAR),
    _magFilterHint(osg::Texture::LINEAR),
    _depth(1),
    _numCurveSamples(10)
{
    setImplementation(implementation);

    _texenv = new osg::TexEnv;
    _stateset = new osg::StateSet;

    _stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    _stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    _stateset->setMode(GL_BLEND, osg::StateAttribute::ON);

#if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)

    OSG_INFO<<"Font::Font() Fixed function pipeline"<<std::endl;

    _stateset->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::ON);
#endif

#if defined(OSG_GL3_AVAILABLE) || defined(OSG_GLES3_AVAILABLE)
    {

        OSG_INFO<<"Font::Font() Setting up GL3 compatible shaders"<<std::endl;

        static const char* gl3_TextVertexShader = {
            "#version " GLSL_VERSION_STR "\n"
            "// gl3_TextVertexShader\n"
            "#ifdef GL_ES\n"
            "    precision highp float;\n"
            "#endif\n"
            "in vec4 osg_Vertex;\n"
            "in vec4 osg_Color;\n"
            "in vec4 osg_MultiTexCoord0;\n"
            "uniform mat4 osg_ModelViewProjectionMatrix;\n"
            "out vec2 texCoord;\n"
            "out vec4 vertexColor;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
            "    texCoord = osg_MultiTexCoord0.xy;\n"
            "    vertexColor = osg_Color; \n"
            "}\n"
        };

        static const char* gl3_TextFragmentShader = {
            "#version " GLSL_VERSION_STR "\n"
            "// gl3_TextFragmentShader\n"
            "#ifdef GL_ES\n"
            "    precision highp float;\n"
            "#endif\n"
            "uniform sampler2D glyphTexture;\n"
            "in vec2 texCoord;\n"
            "in vec4 vertexColor;\n"
            "out vec4 color;\n"
            "void main(void)\n"
            "{\n"
            "    if (texCoord.x>=0.0) color = vertexColor * vec4(1.0, 1.0, 1.0, texture(glyphTexture, texCoord)." GLYPH_CMP ");\n"
            "    else color = vertexColor;\n"
            "}\n"
        };

        osg::ref_ptr<osg::Program> program = new osg::Program;
        program->addShader(new osg::Shader(osg::Shader::VERTEX, gl3_TextVertexShader));
        program->addShader(new osg::Shader(osg::Shader::FRAGMENT, gl3_TextFragmentShader));
        _stateset->setAttributeAndModes(program.get());
        _stateset->addUniform(new osg::Uniform("glyphTexture", 0));

    }
#elif defined(OSG_GL2_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
    {
        OSG_INFO<<"Font::Font() Setting up GL2 compatible shaders"<<std::endl;

        static const char* gl2_TextVertexShader = {
            "// gl2_TextVertexShader\n"
            "#ifdef GL_ES\n"
            "    precision highp float;\n"
            "#endif\n"
            "varying vec2 texCoord;\n"
            "varying vec4 vertexColor;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
            "    texCoord = gl_MultiTexCoord0.xy;\n"
            "    vertexColor = gl_Color; \n"
            "}\n"
        };

        static const char* gl2_TextFragmentShader = {
            "// gl2_TextFragmentShader\n"
            "#ifdef GL_ES\n"
            "    precision highp float;\n"
            "#endif\n"
            "uniform sampler2D glyphTexture;\n"
            "varying vec2 texCoord;\n"
            "varying vec4 vertexColor;\n"
            "void main(void)\n"
            "{\n"
            "    if (texCoord.x>=0.0) gl_FragColor = vertexColor * vec4(1.0, 1.0, 1.0, texture2D(glyphTexture, texCoord).a);\n"
            "    else gl_FragColor = vertexColor;\n"
            "}\n"
        };

        osg::ref_ptr<osg::Program> program = new osg::Program;
        program->addShader(new osg::Shader(osg::Shader::VERTEX, gl2_TextVertexShader));
        program->addShader(new osg::Shader(osg::Shader::FRAGMENT, gl2_TextFragmentShader));
        _stateset->setAttributeAndModes(program.get());
        _stateset->addUniform(new osg::Uniform("glyphTexture", 0));

    }
#endif

    char *ptr;
    if( (ptr = getenv("OSG_MAX_TEXTURE_SIZE")) != 0)
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

void Font::setGlyphImageMargin(unsigned int margin)
{
    _margin = margin;
}

unsigned int Font::getGlyphImageMargin() const
{
    return _margin;
}

void Font::setGlyphImageMarginRatio(float ratio)
{
    _marginRatio = ratio;
}

float Font::getGlyphImageMarginRatio() const
{
    return _marginRatio;
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

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_glyphMapMutex);
        FontSizeGlyphMap::iterator itr = _sizeGlyphMap.find(fontResUsed);
        if (itr!=_sizeGlyphMap.end())
        {
            GlyphMap& glyphmap = itr->second;
            GlyphMap::iterator gitr = glyphmap.find(charcode);
            if (gitr!=glyphmap.end()) return gitr->second.get();
        }
    }

    Glyph* glyph = _implementation->getGlyph(fontResUsed, charcode);
    if (glyph)
    {
        addGlyph(fontResUsed, charcode, glyph);
        return glyph;
    }
    else return 0;
}

Glyph3D* Font::getGlyph3D(unsigned int charcode)
{
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_glyphMapMutex);
        Glyph3DMap::iterator itr = _glyph3DMap.find(charcode);
        if (itr!=_glyph3DMap.end()) return itr->second.get();
    }

    Glyph3D* glyph = _implementation.valid() ? _implementation->getGlyph3D(charcode) : 0;
    if (glyph)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_glyphMapMutex);
        _glyph3DMap[charcode] = glyph;
        return glyph;
    }
    return 0;
}

void Font::setThreadSafeRefUnref(bool threadSafe)
{
   osg::Object::setThreadSafeRefUnref(threadSafe);

    if (_texenv.valid()) _texenv->setThreadSafeRefUnref(threadSafe);
    if (_stateset.valid()) _stateset->setThreadSafeRefUnref(threadSafe);

    for(GlyphTextureList::const_iterator itr=_glyphTextureList.begin();
        itr!=_glyphTextureList.end();
        ++itr)
    {
        (*itr)->setThreadSafeRefUnref(threadSafe);
    }
}

void Font::resizeGLObjectBuffers(unsigned int maxSize)
{
    if (_stateset.valid()) _stateset->resizeGLObjectBuffers(maxSize);

    for(GlyphTextureList::const_iterator itr=_glyphTextureList.begin();
        itr!=_glyphTextureList.end();
        ++itr)
    {
        (*itr)->resizeGLObjectBuffers(maxSize);
    }
}

void Font::releaseGLObjects(osg::State* state) const
{
    if (_stateset.valid()) _stateset->releaseGLObjects(state);

    for(GlyphTextureList::const_iterator itr=_glyphTextureList.begin();
        itr!=_glyphTextureList.end();
        ++itr)
    {
        (*itr)->releaseGLObjects(state);
    }

    // const_cast<Font*>(this)->_glyphTextureList.clear();
    // const_cast<Font*>(this)->_sizeGlyphMap.clear();
}

osg::Vec2 Font::getKerning(unsigned int leftcharcode,unsigned int rightcharcode, KerningType kerningType)
{
    if (_implementation.valid()) return _implementation->getKerning(leftcharcode,rightcharcode,kerningType);
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

    int posX=0,posY=0;

    GlyphTexture* glyphTexture = 0;
    for(GlyphTextureList::iterator itr=_glyphTextureList.begin();
        itr!=_glyphTextureList.end() && !glyphTexture;
        ++itr)
    {
        if ((*itr)->getSpaceForGlyph(glyph,posX,posY)) glyphTexture = itr->get();
    }

    if (glyphTexture)
    {
        //cout << "    found space for texture "<<glyphTexture<<" posX="<<posX<<" posY="<<posY<<endl;
    }

    if (!glyphTexture)
    {

        glyphTexture = new GlyphTexture;

        static int numberOfTexturesAllocated = 0;
        ++numberOfTexturesAllocated;

        OSG_INFO<< "   Font " << this<< ", numberOfTexturesAllocated "<<numberOfTexturesAllocated<<std::endl;

        // reserve enough space for the glyphs.
        glyphTexture->setGlyphImageMargin(_margin);
        glyphTexture->setGlyphImageMarginRatio(_marginRatio);
        glyphTexture->setTextureSize(_textureWidthHint,_textureHeightHint);
        glyphTexture->setFilter(osg::Texture::MIN_FILTER,_minFilterHint);
        glyphTexture->setFilter(osg::Texture::MAG_FILTER,_magFilterHint);
        glyphTexture->setMaxAnisotropy(8);

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
