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



#include <osgText/Font3D>
#include <osgText/Text>

//#include <osg/State>
#include <osg/Notify>
//#include <osg/ApplicationUsage>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
//#include <osg/GLU>

#include <OpenThreads/ReentrantMutex>


using namespace std;

//static osg::ApplicationUsageProxy Font3D_e0(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_TEXT3D_INCREMENTAL_SUBLOADING <type>","ON | OFF");

static OpenThreads::ReentrantMutex s_Font3DFileMutex;

namespace osgText
{

std::string findFont3DFile(const std::string& str)
{
    // try looking in OSGFILEPATH etc first for fonts.
    std::string filename = osgDB::findDataFile(str);
    if (!filename.empty()) return filename;

    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(s_Font3DFileMutex);

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
        filename = osgText::findFont3DFile(std::string("fonts/")+filename);
        if (!filename.empty()) return filename;
    }

    // Not found, return empty string
    osg::notify(osg::WARN)<<"Warning: font file \""<<str<<"\" not found."<<std::endl;    
    return std::string();
}

osgText::Font3D* readFont3DFile(const std::string& filename, const osgDB::ReaderWriter::Options* userOptions)
{
    if (filename=="") return 0;

    std::string foundFile = findFont3DFile(filename);
    if (foundFile.empty()) return 0;
    
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_Font3DFileMutex);

    osg::ref_ptr<osgDB::ReaderWriter::Options> localOptions;
    if (!userOptions)
    {
        localOptions = new osgDB::ReaderWriter::Options;
        localOptions->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_OBJECTS);
        // ** HACK to load Font3D instead of Font
        localOptions->setPluginData("3D", (void*) 1);
    }
    else
    {
        userOptions->setPluginData("3D", (void*) 1);
    }

    osg::Object* object = osgDB::readObjectFile(foundFile, userOptions ? userOptions : localOptions.get());
    
    // if the object is a font then return it.
    osgText::Font3D* font3D = dynamic_cast<osgText::Font3D*>(object);
    if (font3D) return font3D;

    // otherwise if the object has zero references then delete it by doing another unref().
    if (object && object->referenceCount()==0) object->unref();
    return 0;
}

osgText::Font3D* readFont3DStream(std::istream& stream, const osgDB::ReaderWriter::Options* userOptions)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_Font3DFileMutex);

    osg::ref_ptr<osgDB::ReaderWriter::Options> localOptions;
    if (!userOptions)
    {
        localOptions = new osgDB::ReaderWriter::Options;
        localOptions->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_OBJECTS);
        localOptions->setPluginData("3D", (void*) 1);
    }
    else
    {
        userOptions->setPluginData("3D", (void*) 1);
    }

    // there should be a better way to get the FreeType ReaderWriter by name...
    osgDB::ReaderWriter *reader = osgDB::Registry::instance()->getReaderWriterForExtension("ttf");
    if (reader == 0) return 0;
    
    osgDB::ReaderWriter::ReadResult rr = reader->readObject(stream, userOptions ? userOptions : localOptions.get());
    if (rr.error())
    {
        osg::notify(osg::WARN) << rr.message() << std::endl;
        return 0;
    }
    if (!rr.validObject()) return 0;
    
    osg::Object *object = rr.takeObject();

    // if the object is a font then return it.
    osgText::Font3D* font3D = dynamic_cast<osgText::Font3D*>(object);
    if (font3D) return font3D;

    // otherwise if the object has zero references then delete it by doing another unref().
    if (object && object->referenceCount()==0) object->unref();
    return 0;
}

osg::ref_ptr<Font3D> readRefFont3DFile(const std::string& filename, const osgDB::ReaderWriter::Options* userOptions)
{
    if (filename=="") return 0;

    std::string foundFile = findFont3DFile(filename);
    if (foundFile.empty()) return 0;
    
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_Font3DFileMutex);

    osg::ref_ptr<osgDB::ReaderWriter::Options> localOptions;
    if (!userOptions)
    {
        localOptions = new osgDB::ReaderWriter::Options;
        localOptions->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_OBJECTS);
        // ** HACK to load Font3D instead of Font
        localOptions->setPluginData("3D", (void*) 1);
    }
    else
    {
        userOptions->setPluginData("3D", (void*) 1);
    }

    osg::ref_ptr<osg::Object> object = osgDB::readRefObjectFile(foundFile, userOptions ? userOptions : localOptions.get());
    
    // if the object is a font then return it.
    osgText::Font3D* font3D = dynamic_cast<osgText::Font3D*>(object.get());
    if (font3D) return osg::ref_ptr<Font3D>(font3D);

    return 0;
}

osg::ref_ptr<Font3D> readRefFont3DStream(std::istream& stream, const osgDB::ReaderWriter::Options* userOptions)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_Font3DFileMutex);

    osg::ref_ptr<osgDB::ReaderWriter::Options> localOptions;
    if (!userOptions)
    {
        localOptions = new osgDB::ReaderWriter::Options;
        localOptions->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_OBJECTS);
        localOptions->setPluginData("3D", (void*) 1);
    }
    else
    {
        userOptions->setPluginData("3D", (void*) 1);
    }

    // there should be a better way to get the FreeType ReaderWriter by name...
    osgDB::ReaderWriter *reader = osgDB::Registry::instance()->getReaderWriterForExtension("ttf");
    if (reader == 0) return 0;
    
    osgDB::ReaderWriter::ReadResult rr = reader->readObject(stream, userOptions ? userOptions : localOptions.get());
    if (rr.error())
    {
        osg::notify(osg::WARN) << rr.message() << std::endl;
        return 0;
    }
    if (!rr.validObject()) return 0;

    // if the object is a font then return it.
    osgText::Font3D* font3D = dynamic_cast<osgText::Font3D*>(rr.getObject());
    if (font3D) return osg::ref_ptr<Font3D>(font3D);

    return 0;
}

Font3D::Font3D(Font3DImplementation* implementation):
    osg::Object(true),
    _depth(1),
    _width(64),
    _height(64)
{
    setImplementation(implementation);
}

Font3D::~Font3D()
{
    if (_implementation.valid()) _implementation->_facade = 0;
}

void Font3D::setImplementation(Font3DImplementation* implementation)
{
    if (_implementation.valid()) _implementation->_facade = 0;
    _implementation = implementation;
    if (_implementation.valid()) _implementation->_facade = this;
}

Font3D::Font3DImplementation* Font3D::getImplementation()
{
    return _implementation.get();
}

const Font3D::Font3DImplementation* Font3D::getImplementation() const
{
    return _implementation.get();
}

std::string Font3D::getFileName() const
{
    if (_implementation.valid()) return _implementation->getFileName();
    return "";
}

Font3D::Glyph3D* Font3D::getGlyph(unsigned int charcode)
{
    Glyph3D * glyph3D = NULL; 
    
    Glyph3DMap::iterator itr = _glyph3DMap.find(charcode);
    if (itr!=_glyph3DMap.end()) glyph3D = itr->second.get();
    
    else if (_implementation.valid())
    {
        glyph3D = _implementation->getGlyph(charcode);
        if (glyph3D) _glyph3DMap[charcode] = glyph3D;
    }
    
    return glyph3D;
}

void Font3D::setThreadSafeRefUnref(bool threadSafe)
{
    Glyph3DMap::iterator it,end = _glyph3DMap.end();
    
    for (it=_glyph3DMap.begin(); it!=end; ++it)
        it->second->setThreadSafeRefUnref(threadSafe);
}

osg::Vec2 Font3D::getKerning(unsigned int leftcharcode,unsigned int rightcharcode, KerningType kerningType)
{
    if (_implementation.valid()) return _implementation->getKerning(leftcharcode,rightcharcode,kerningType);
    else return osg::Vec2(0.0f,0.0f);
}
bool Font3D::hasVertical() const
{
    if (_implementation.valid()) return _implementation->hasVertical();
    else return false;
}

void Font3D::Glyph3D::setThreadSafeRefUnref(bool threadSafe)
{
    if (_vertexArray.valid()) _vertexArray->setThreadSafeRefUnref(threadSafe);
    if (_normalArray.valid()) _normalArray->setThreadSafeRefUnref(threadSafe);
}
}
