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

#include <stdio.h>
#include <string.h>

#include <osg/Notify>
#include <osg/Object>
#include <osg/Image>
#include <osg/Shader>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/ApplicationUsage>
#include <osg/Version>
#include <osg/Timer>

#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/fstream>
#include <osgDB/Archive>

#include <algorithm>
#include <set>
#include <memory>

#include <stdlib.h>

#if defined(__sgi)
    #include <ctype.h>
#elif defined(__GNUC__) || !defined(WIN32) || defined(__MWERKS__)
    #include <cctype>
    using std::tolower;
#endif

#ifdef OSG_LIBRARY_POSTFIX
    #define OSG_LIBRARY_POSTFIX_WITH_QUOTES ADDQUOTES(OSG_LIBRARY_POSTFIX)
#else
    #define OSG_LIBRARY_POSTFIX_WITH_QUOTES ""
#endif

using namespace osg;
using namespace osgDB;

#if !defined(WIN32) || defined(__CYGWIN__)
static osg::ApplicationUsageProxy Registry_e0(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_FILE_PATH <path>[:path]..","Paths for locating datafiles");
static osg::ApplicationUsageProxy Registry_e1(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_LIBRARY_PATH <path>[:path]..","Paths for locating libraries/ plugins");
#else
static osg::ApplicationUsageProxy Registry_e0(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_FILE_PATH <path>[;path]..","Paths for locating datafiles");
static osg::ApplicationUsageProxy Registry_e1(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_LIBRARY_PATH <path>[;path]..","Paths for locating libraries/ plugins");
#endif

static osg::ApplicationUsageProxy Registry_e2(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_BUILD_KDTREES on/off","Enable/disable the automatic building of KdTrees for each loaded Geometry.");


// from MimeTypes.cpp
extern const char* builtinMimeTypeExtMappings[];


class Registry::AvailableReaderWriterIterator
{
public:
    AvailableReaderWriterIterator(Registry::ReaderWriterList& rwList, OpenThreads::ReentrantMutex& pluginMutex):
        _rwList(rwList),
        _pluginMutex(pluginMutex) {}


    ReaderWriter& operator * () { return *get(); }
    ReaderWriter* operator -> () { return get(); }

    bool valid() { return get()!=0; }

    void operator ++()
    {
        _rwUsed.insert(get());
    }


protected:

    AvailableReaderWriterIterator& operator = (const AvailableReaderWriterIterator&) { return *this; }

    Registry::ReaderWriterList&     _rwList;
    OpenThreads::ReentrantMutex&    _pluginMutex;

    std::set<ReaderWriter*>         _rwUsed;

    ReaderWriter* get()
    {
        OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_pluginMutex);
        Registry::ReaderWriterList::iterator itr=_rwList.begin();
        for(;itr!=_rwList.end();++itr)
        {
            if (_rwUsed.find(itr->get())==_rwUsed.end())
            {
                return itr->get();
            }
        }
        return 0;
    }

};

class Registry::AvailableArchiveIterator
{
public:
    AvailableArchiveIterator(Registry::ArchiveCache& archives, OpenThreads::ReentrantMutex& mutex):
        _archives(archives),
        _mutex(mutex) {}


    Archive& operator * () { return *get(); }
    Archive* operator -> () { return get(); }

    bool valid() { return get()!=0; }

    void operator ++()
    {
        _archivesUsed.insert(get());
    }


protected:

    AvailableArchiveIterator& operator = (const AvailableArchiveIterator&) { return *this; }

    Registry::ArchiveCache&         _archives;
    OpenThreads::ReentrantMutex&    _mutex;

    std::set<Archive*>              _archivesUsed;

    Archive* get()
    {
        OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_mutex);
        Registry::ArchiveCache::iterator itr=_archives.begin();
        for(;itr!=_archives.end();++itr)
        {
            if (_archivesUsed.find(itr->second.get())==_archivesUsed.end())
            {
                return itr->second.get();
            }
        }
        return 0;
    }

};

#if 0
    // temporary test of autoregistering, not compiled by default.
    enum Methods
    {
        SET_1,
        SET_2,
        END
    };


    typedef std::pair<Methods,std::string> MethodPair;

    class Proxy
    {
    public:
        Proxy(MethodPair* methods)
        {
            std::cout<<"methods "<<methods<<std::endl;
            for(int i=0;methods[i].first!=END;++i)
            {
                std::cout<<"\t"<<methods[i].first<<"\t"<<methods[i].second<<std::endl;
            }
        }
    };


    static MethodPair methods[] =
    {
        MethodPair(SET_1,"SET_1"),
        MethodPair(SET_2,"SET_2"),
        MethodPair(END,"")
    };

    Proxy myproxy(methods);

#endif

void PrintFilePathList(std::ostream& stream,const FilePathList& filepath)
{
    for(FilePathList::const_iterator itr=filepath.begin();
        itr!=filepath.end();
        ++itr)
    {
        stream << "    "<< *itr<<std::endl;
    }
}

Registry* Registry::instance(bool erase)
{
    static ref_ptr<Registry> s_registry = new Registry;
    if (erase)
    {
        s_registry->destruct();
        s_registry = 0;
    }
    return s_registry.get(); // will return NULL on erase
}

OSG_INIT_SINGLETON_PROXY(ProxyInitRegistry, Registry::instance())


// definition of the Registry
Registry::Registry()
{
    // comment out because it was causing problems under OSX - causing it to crash osgconv when constructing ostream in osg::notify().
    // OSG_INFO << "Constructing osg::Registry"<<std::endl;

    _buildKdTreesHint = Options::NO_PREFERENCE;
    _kdTreeBuilder = new osg::KdTreeBuilder;

    const char* kdtree_str = getenv("OSG_BUILD_KDTREES");
    if (kdtree_str)
    {
        bool switchOff = (strcmp(kdtree_str, "off")==0 || strcmp(kdtree_str, "OFF")==0 || strcmp(kdtree_str, "Off")==0 );
        if (switchOff) _buildKdTreesHint = Options::DO_NOT_BUILD_KDTREES;
        else _buildKdTreesHint = Options::BUILD_KDTREES;
    }

    const char* ptr=0;

    _expiryDelay = 10.0;
    if( (ptr = getenv("OSG_EXPIRY_DELAY")) != 0)
    {
        _expiryDelay = osg::asciiToDouble(ptr);
        OSG_INFO<<"Registry : Expiry delay = "<<_expiryDelay<<std::endl;
    }

    const char* fileCachePath = getenv("OSG_FILE_CACHE");
    if (fileCachePath)
    {
        _fileCache = new FileCache(fileCachePath);
    }

    // assign ObjectCache.
    _objectCache = new ObjectCache;

    _createNodeFromImage = false;
    _openingLibrary = false;

    // add default osga archive extension
    _archiveExtList.push_back("osga");
    _archiveExtList.push_back("zip");

    initFilePathLists();



    // register file extension alias.
    const char* flt_str = getenv("OSG_OPEN_FLIGHT_PLUGIN");
    if (flt_str)
    {
        if (strcmp(flt_str, "new")==0)
        {
            addFileExtensionAlias("flt", "OpenFlight");
        }
    }
    else
    {
    #ifndef COMPILE_WITH_OLD_OPENFLIGHT_PLUGIN_AS_DEFAULT
        addFileExtensionAlias("flt", "OpenFlight");
    #endif
    }

    addFileExtensionAlias("osgs", "osg");
    addFileExtensionAlias("osgt", "osg");
    addFileExtensionAlias("osgb", "osg");
    addFileExtensionAlias("osgx", "osg");

    addFileExtensionAlias("shadow",  "osgshadow");
    addFileExtensionAlias("terrain", "osgterrain");
    addFileExtensionAlias("view",  "osgviewer");

    //addFileExtensionAlias("vsga", "vsg");
    //addFileExtensionAlias("vsgb", "vsg");

    addFileExtensionAlias("sgi",  "rgb");
    addFileExtensionAlias("rgba", "rgb");
    addFileExtensionAlias("int",  "rgb");
    addFileExtensionAlias("inta", "rgb");
    addFileExtensionAlias("bw",   "rgb");

    addFileExtensionAlias("ivz",  "gz");
    addFileExtensionAlias("ozg",  "gz");

    addFileExtensionAlias("mag",  "dicom");
    addFileExtensionAlias("ph",   "dicom");
    addFileExtensionAlias("ima",  "dicom");
    addFileExtensionAlias("dcm",  "dicom");
    addFileExtensionAlias("dic",  "dicom");

    addFileExtensionAlias("gl",   "glsl");
    addFileExtensionAlias("vert", "glsl");
    addFileExtensionAlias("frag", "glsl");
    addFileExtensionAlias("geom", "glsl");
    addFileExtensionAlias("tctrl", "glsl");
    addFileExtensionAlias("teval", "glsl");
    addFileExtensionAlias("compute", "glsl");
    addFileExtensionAlias("vs", "glsl");
    addFileExtensionAlias("fs", "glsl");
    addFileExtensionAlias("cs", "glsl");
    addFileExtensionAlias("gs", "glsl");

    addFileExtensionAlias("js", "V8");

#if defined(DARWIN_IMAGEIO)
    addFileExtensionAlias("jpg",  "imageio");
    addFileExtensionAlias("jpe",  "imageio");
    addFileExtensionAlias("jpeg", "imageio");
    addFileExtensionAlias("tif",  "imageio");
    addFileExtensionAlias("tiff", "imageio");
    addFileExtensionAlias("gif",  "imageio");
    addFileExtensionAlias("png",  "imageio");
    addFileExtensionAlias("psd",  "imageio");
    addFileExtensionAlias("tga",  "imageio");

#endif

#if defined(USE_QTKIT)
    addFileExtensionAlias("mov",  "QTKit");
    addFileExtensionAlias("mp4",  "QTKit");
    addFileExtensionAlias("mov",  "QTKit");
    addFileExtensionAlias("mpg",  "QTKit");
    addFileExtensionAlias("mpeg",  "QTKit");
    addFileExtensionAlias("mpv",  "QTKit");
    addFileExtensionAlias("m4v",  "QTKit");
    addFileExtensionAlias("3gp",  "QTKit");
    // Requires Perian
    addFileExtensionAlias("avi",  "QTKit");
    addFileExtensionAlias("xvid",  "QTKit");
    // Requires Flip4Mac
    addFileExtensionAlias("wmv",  "QTKit");
#endif

#if defined(DARWIN_QUICKTIME)

    addFileExtensionAlias("jpg",  "qt");
    addFileExtensionAlias("jpe",  "qt");
    addFileExtensionAlias("jpeg", "qt");
    addFileExtensionAlias("tif",  "qt");
    addFileExtensionAlias("tiff", "qt");
    addFileExtensionAlias("gif",  "qt");
    addFileExtensionAlias("png",  "qt");
    addFileExtensionAlias("psd",  "qt");
    addFileExtensionAlias("tga",  "qt");
    addFileExtensionAlias("flv",  "qt");
    addFileExtensionAlias("dv",   "qt");

    #if !defined(USE_QTKIT)
        addFileExtensionAlias("mov",  "qt");
        addFileExtensionAlias("avi",  "qt");
        addFileExtensionAlias("mpg",  "qt");
        addFileExtensionAlias("mpv",  "qt");
        addFileExtensionAlias("mp4",  "qt");
        addFileExtensionAlias("m4v",  "qt");
        addFileExtensionAlias("3gp",  "qt");
        // Add QuickTime live support for OSX
        addFileExtensionAlias("live", "qt");
    #endif
#else
    addFileExtensionAlias("jpg",  "jpeg");
    addFileExtensionAlias("jpe",  "jpeg");
    addFileExtensionAlias("tif",  "tiff");

    // really need to decide this at runtime...
    #if defined(USE_FFMPEG)

        addFileExtensionAlias("mov",  "ffmpeg");
        addFileExtensionAlias("mpg",  "ffmpeg");
        addFileExtensionAlias("ogv",  "ffmpeg");
        addFileExtensionAlias("mpv",  "ffmpeg");
        addFileExtensionAlias("dv",   "ffmpeg");
        addFileExtensionAlias("avi",  "ffmpeg");
        addFileExtensionAlias("wmv",  "ffmpeg");
        addFileExtensionAlias("flv",  "ffmpeg");
    #endif

    // support QuickTime for Windows
    // Logic error here. It is possible for Apple to not define Quicktime and end up in
    // this Quicktime for Windows block. So add an extra check to avoid QTKit clashes.
    #if defined(USE_QUICKTIME) && !defined(USE_QTKIT)

        addFileExtensionAlias("mov",  "qt");
        addFileExtensionAlias("live", "qt");
        addFileExtensionAlias("mpg",  "qt");
        addFileExtensionAlias("avi",  "qt");
    #endif
#endif

    #if defined(USE_AV_FOUNDATION)
        addFileExtensionAlias("mov",  "AVFoundation");
        addFileExtensionAlias("mpg",  "AVFoundation");
        addFileExtensionAlias("mpv",  "AVFoundation");
        addFileExtensionAlias("mp4",  "AVFoundation");
        addFileExtensionAlias("m4v",  "AVFoundation");
    #endif

    // remove geo to lwo alias as the new Carbon Graphics GEO format
    // also uses the .geo. It is still possible to load light wave .geo
    // files via loading the lwo plugin explicitly and then doing a readNodeFile.
    //addFileExtensionAlias("geo",  "lwo");
    addFileExtensionAlias("lw",   "lwo");

    #if defined(USE_VRML)
        addFileExtensionAlias("wrl",   "vrml");
    #elif defined(USE_INVENTOR)
        addFileExtensionAlias("wrl",   "iv");
    #endif

    // add alias for the text/freetype plugin.
    addFileExtensionAlias("ttf",    "freetype");  // true type
    addFileExtensionAlias("ttc",    "freetype");  // true type
    addFileExtensionAlias("cid",    "freetype");  // Postscript CID-Fonts
    addFileExtensionAlias("cff",    "freetype");  // OpenType
    addFileExtensionAlias("cef",    "freetype");  // OpenType
    addFileExtensionAlias("fon",    "freetype");  // Windows bitmap fonts
    addFileExtensionAlias("fnt",    "freetype");  // Windows bitmap fonts
    addFileExtensionAlias("text3d", "freetype"); // use 3D Font instead of 2D Font

    // wont't add type1 and type2 until resolve extension collision with Performer binary and ascii files.
    // addFileExtensionAlias("pfb",   "freetype");  // type1 binary
    // addFileExtensionAlias("pfa",   "freetype");  // type2 ascii


    // TransferFunction head
    addFileExtensionAlias("tf-255", "tf");

    // portable bitmap, greyscale and colour/pixmap image formats
    addFileExtensionAlias("pbm", "pnm");
    addFileExtensionAlias("pgm", "pnm");
    addFileExtensionAlias("ppm", "pnm");


    // add revision file mappings
    addFileExtensionAlias("added",    "revisions");
    addFileExtensionAlias("removed",  "revisions");
    addFileExtensionAlias("modified", "revisions");


    // STEP/IGES file mappings
    addFileExtensionAlias("stp",  "opencascade");
    addFileExtensionAlias("step", "opencascade");
    addFileExtensionAlias("igs",  "opencascade");
    addFileExtensionAlias("iges", "opencascade");


    // add built-in mime-type extension mappings
    for( int i=0; ; i+=2 )
    {
        std::string mimeType = builtinMimeTypeExtMappings[i];
        if ( mimeType.length() == 0 )
            break;
        addMimeTypeExtensionMapping( mimeType, builtinMimeTypeExtMappings[i+1] );
    }

    // register server protocols, so the curl can handle it, if necessary
    registerProtocol("http");
    registerProtocol("https");
    registerProtocol("ftp");
    registerProtocol("ftps");

    _objectWrapperManager = new ObjectWrapperManager;
    _deprecatedDotOsgWrapperManager = new DeprecatedDotOsgWrapperManager;
}


Registry::~Registry()
{
    destruct();
}

void Registry::destruct()
{
    // OSG_NOTICE<<"Registry::destruct()"<<std::endl;

    // clean up the SharedStateManager
    _sharedStateManager = 0;


    // clean up the FileCache
    _fileCache = 0;


    // object cache clear needed here to prevent crash in unref() of
    // the objects it contains when running the TXP plugin.
    // Not sure why, but perhaps there is is something in a TXP plugin
    // which deletes the data before its ref count hits zero, perhaps
    // even some issue with objects be allocated by a plugin that is
    // maintained after that plugin is deleted...  Robert Osfield, Jan 2004.
    clearObjectCache();
    _fileCache = 0;

    clearArchiveCache();


    // unload all the plugin before we finally destruct.
    closeAllLibraries();
}

#include <iostream>

void Registry::initDataFilePathList()
{
    FilePathList filepath;
    //
    // set up data file paths
    //
    char *ptr;

    if( (ptr = getenv( "OSG_FILE_PATH" )) )
    {
        //OSG_NOTIFY(DEBUG_INFO) << "OSG_FILE_PATH("<<ptr<<")"<<std::endl;
        convertStringPathIntoFilePathList(ptr, filepath);
    }
    else if( (ptr = getenv( "OSGFILEPATH" )) )
    {
        //OSG_NOTIFY(DEBUG_INFO) << "OSGFILEPATH("<<ptr<<")"<<std::endl;
        convertStringPathIntoFilePathList(ptr, filepath);
    }

    osgDB::appendPlatformSpecificResourceFilePaths(filepath);
    setDataFilePathList(filepath);

}

void Registry::setDataFilePathList(const std::string& paths)
{
    _dataFilePath.clear();
    convertStringPathIntoFilePathList(paths,_dataFilePath);
}

void Registry::setLibraryFilePathList(const std::string& paths) { _libraryFilePath.clear(); convertStringPathIntoFilePathList(paths,_libraryFilePath); }



void Registry::initLibraryFilePathList()
{
    //
    // set up library paths
    //
    char* ptr;
    if( (ptr = getenv( "OSG_LIBRARY_PATH")) )
    {
        //OSG_NOTIFY(DEBUG_INFO) << "OSG_LIBRARY_PATH("<<ptr<<")"<<std::endl;
        setLibraryFilePathList(ptr);
    }
    else if( (ptr = getenv( "OSG_LD_LIBRARY_PATH")) )
    {
        //OSG_NOTIFY(DEBUG_INFO) << "OSG_LD_LIBRARY_PATH("<<ptr<<")"<<std::endl;
        setLibraryFilePathList(ptr);
    }

    appendPlatformSpecificLibraryFilePaths(_libraryFilePath);

}


void Registry::readCommandLine(osg::ArgumentParser& arguments)
{
    // report the usage options.
    if (arguments.getApplicationUsage())
    {
        arguments.getApplicationUsage()->addCommandLineOption("-l <library>","Load the plugin");
        arguments.getApplicationUsage()->addCommandLineOption("-e <extension>","Load the plugin associated with handling files with specified extension");
        arguments.getApplicationUsage()->addCommandLineOption("-O <option_string>","Provide an option string to reader/writers used to load databases");
    }

    std::string value;
    while(arguments.read("-l",value))
    {
        if (loadLibrary(value)==NOT_LOADED)
        {
            OSG_NOTICE<<"Unable to load library : "<<value<<std::endl;
        }
    }

    while(arguments.read("-e",value))
    {
        std::string libName = createLibraryNameForExtension(value);
        if (loadLibrary(libName)==NOT_LOADED)
        {
            OSG_NOTICE<<"Unable to load library : "<<libName<<std::endl;
        }
    }

    while(arguments.read("-O",value))
    {
        setOptions(new Options(value));
    }
}

void Registry::addReaderWriter(ReaderWriter* rw)
{
    if (rw==0L) return;

    // OSG_NOTIFY(INFO) << "osg::Registry::addReaderWriter("<<rw->className()<<")"<< std::endl;

    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_pluginMutex);

    _rwList.push_back(rw);

}


void Registry::removeReaderWriter(ReaderWriter* rw)
{
    if (rw==0L) return;

//    OSG_NOTIFY(INFO) << "osg::Registry::removeReaderWriter();"<< std::endl;

    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_pluginMutex);

    ReaderWriterList::iterator rwitr = std::find(_rwList.begin(),_rwList.end(),rw);
    if (rwitr!=_rwList.end())
    {
        _rwList.erase(rwitr);
    }

}

ImageProcessor* Registry::getImageProcessor()
{
    {
        OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_pluginMutex);
        if (!_ipList.empty())
        {
            return _ipList.front().get();
        }
    }
    return getImageProcessorForExtension("nvtt");
}

ImageProcessor* Registry::getImageProcessorForExtension(const std::string& ext)
{
    {
        OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_pluginMutex);
        if (!_ipList.empty())
        {
            return _ipList.front().get();
        }
    }

    std::string libraryName = createLibraryNameForExtension(ext);
    OSG_NOTICE << "Now checking for plug-in "<<libraryName<< std::endl;
    if (loadLibrary(libraryName)==LOADED)
    {
        OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_pluginMutex);
        if (!_ipList.empty())
        {
            OSG_NOTICE << "Loaded plug-in "<<libraryName<<" and located ImageProcessor"<< std::endl;
            return _ipList.front().get();
        }
    }
    return 0;
}

void Registry::addImageProcessor(ImageProcessor* ip)
{
    if (ip==0L) return;

    OSG_NOTIFY(NOTICE) << "osg::Registry::addImageProcessor("<<ip->className()<<")"<< std::endl;

    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_pluginMutex);

    _ipList.push_back(ip);

}


void Registry::removeImageProcessor(ImageProcessor* ip)
{
    if (ip==0L) return;

    OSG_NOTIFY(NOTICE) << "osg::Registry::removeImageProcessor();"<< std::endl;

    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_pluginMutex);

    ImageProcessorList::iterator ipitr = std::find(_ipList.begin(),_ipList.end(),ip);
    if (ipitr!=_ipList.end())
    {
        _ipList.erase(ipitr);
    }

}


void Registry::addFileExtensionAlias(const std::string mapExt, const std::string toExt)
{
    _extAliasMap[mapExt] = toExt;
}

void Registry::addMimeTypeExtensionMapping(const std::string fromMimeType, const std::string toExt)
{
    _mimeTypeExtMap[fromMimeType] = toExt;
}

bool Registry::readPluginAliasConfigurationFile( const std::string& file )
{
    std::string fileName = osgDB::findDataFile( file );
    if (fileName.empty())
    {
        OSG_NOTIFY( osg::WARN) << "Can't find plugin alias config file \"" << file << "\"." << std::endl;
        return false;
    }

    osgDB::ifstream ifs;
    ifs.open( fileName.c_str() );
    if (!ifs.good())
    {
        OSG_NOTIFY( osg::WARN) << "Can't open plugin alias config file \"" << fileName << "\"." << std::endl;
        return false;
    }

    int lineNum( 0 );
    while (ifs.good())
    {
        std::string raw;
        ++lineNum;
        std::getline( ifs, raw );
        std::string ln = trim( raw );
        if (ln.empty()) continue;
        if (ln[0] == '#') continue;

        std::string::size_type spIdx = ln.find_first_of( " \t" );
        if (spIdx == ln.npos)
        {
            // mapExt and toExt must be on the same line, separated by a space.
            OSG_NOTIFY( osg::WARN) << file << ", line " << lineNum << ": Syntax error: missing space in \"" << raw << "\"." << std::endl;
            continue;
        }

        const std::string mapExt = trim( ln.substr( 0, spIdx ) );
        const std::string toExt = trim( ln.substr( spIdx+1 ) );
        addFileExtensionAlias( mapExt, toExt );
    }
    return true;
}

std::string Registry::trim( const std::string& str )
{
    if (!str.size()) return str;
    std::string::size_type first = str.find_first_not_of( " \t" );
    std::string::size_type last = str.find_last_not_of( "  \t\r\n" );
    if ((first==str.npos) || (last==str.npos)) return std::string( "" );
    return str.substr( first, last-first+1 );
}


std::string Registry::createLibraryNameForFile(const std::string& fileName)
{
    return createLibraryNameForExtension(getFileExtension(fileName));
}

std::string Registry::createLibraryNameForExtension(const std::string& ext)
{
    std::string lowercase_ext;
    for(std::string::const_iterator sitr=ext.begin();
        sitr!=ext.end();
        ++sitr)
    {
        lowercase_ext.push_back(tolower(*sitr));
    }

    ExtensionAliasMap::iterator itr=_extAliasMap.find(lowercase_ext);
    if (itr!=_extAliasMap.end() && ext != itr->second) return createLibraryNameForExtension(itr->second);

    std::string prepend = std::string("osgPlugins-")+std::string(osgGetVersion())+std::string("/");

#if defined(__CYGWIN__)
    return prepend+"cygwin_"+"osgdb_"+lowercase_ext+OSG_LIBRARY_POSTFIX_WITH_QUOTES+".dll";
#elif defined(__MINGW32__)
    return prepend+"mingw_"+"osgdb_"+lowercase_ext+OSG_LIBRARY_POSTFIX_WITH_QUOTES+".dll";
#elif defined(WIN32)
    return prepend+"osgdb_"+lowercase_ext+OSG_LIBRARY_POSTFIX_WITH_QUOTES+".dll";
#elif macintosh
    return prepend+"osgdb_"+lowercase_ext+OSG_LIBRARY_POSTFIX_WITH_QUOTES;
#else
    return prepend+"osgdb_"+lowercase_ext+OSG_LIBRARY_POSTFIX_WITH_QUOTES+ADDQUOTES(OSG_PLUGIN_EXTENSION);
#endif

}

std::string Registry::createLibraryNameForNodeKit(const std::string& name)
{
#if defined(__CYGWIN__)
    return "cyg"+name+OSG_LIBRARY_POSTFIX_WITH_QUOTES+".dll";
#elif defined(__MINGW32__)
    return "lib"+name+OSG_LIBRARY_POSTFIX_WITH_QUOTES+".dll";
#elif defined(WIN32)
    return name+OSG_LIBRARY_POSTFIX_WITH_QUOTES+".dll";
#elif macintosh
    return name+OSG_LIBRARY_POSTFIX_WITH_QUOTES;
#else
    return "lib"+name+OSG_LIBRARY_POSTFIX_WITH_QUOTES + ADDQUOTES(OSG_PLUGIN_EXTENSION);
#endif
}

Registry::LoadStatus Registry::loadLibrary(const std::string& fileName)
{
    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_pluginMutex);

    DynamicLibraryList::iterator ditr = getLibraryItr(fileName);
    if (ditr!=_dlList.end()) return PREVIOUSLY_LOADED;

    _openingLibrary=true;

    DynamicLibrary* dl = DynamicLibrary::loadLibrary(fileName);
    _openingLibrary=false;

    if (dl)
    {
        _dlList.push_back(dl);
        return LOADED;
    }
    return NOT_LOADED;
}


bool Registry::closeLibrary(const std::string& fileName)
{
    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_pluginMutex);
    DynamicLibraryList::iterator ditr = getLibraryItr(fileName);
    if (ditr!=_dlList.end())
    {
        _dlList.erase(ditr);
        return true;
    }
    return false;
}

void Registry::closeAllLibraries()
{
    // OSG_NOTICE<<"Registry::closeAllLibraries()"<<std::endl;
    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_pluginMutex);
    _dlList.clear();
}

Registry::DynamicLibraryList::iterator Registry::getLibraryItr(const std::string& fileName)
{
    DynamicLibraryList::iterator ditr = _dlList.begin();
    for(;ditr!=_dlList.end();++ditr)
    {
        if ((*ditr)->getName()==fileName) return ditr;
    }
    return _dlList.end();
}

DynamicLibrary* Registry::getLibrary(const std::string& fileName)
{
    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_pluginMutex);
    DynamicLibraryList::iterator ditr = getLibraryItr(fileName);
    if (ditr!=_dlList.end()) return ditr->get();
    else return NULL;
}

ReaderWriter* Registry::getReaderWriterForExtension(const std::string& ext)
{
    // record the existing reader writer.
    std::set<ReaderWriter*> rwOriginal;

    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_pluginMutex);

    // first attempt one of the installed loaders
    for(ReaderWriterList::iterator itr=_rwList.begin();
        itr!=_rwList.end();
        ++itr)
    {
        rwOriginal.insert(itr->get());
        if((*itr)->acceptsExtension(ext)) return (*itr).get();
    }

    // now look for a plug-in to load the file.
    std::string libraryName = createLibraryNameForExtension(ext);
    OSG_NOTIFY(INFO) << "Now checking for plug-in "<<libraryName<< std::endl;
    if (loadLibrary(libraryName)==LOADED)
    {
        for(ReaderWriterList::iterator itr=_rwList.begin();
            itr!=_rwList.end();
            ++itr)
        {
            if (rwOriginal.find(itr->get())==rwOriginal.end())
            {
                if((*itr)->acceptsExtension(ext)) return (*itr).get();
            }
        }
    }

    return NULL;

}

ReaderWriter* Registry::getReaderWriterForMimeType(const std::string& mimeType)
{
    MimeTypeExtensionMap::const_iterator i = _mimeTypeExtMap.find( mimeType );
    return i != _mimeTypeExtMap.end()?
        getReaderWriterForExtension( i->second ) :
        NULL;
}

#if 0
struct concrete_wrapper: basic_type_wrapper
{
    virtual ~concrete_wrapper() {}
    concrete_wrapper(const osg::Object *myobj) : myobj_(myobj) {}
    bool matches(const osg::Object *proto) const
    {
        return myobj_->isSameKindAs(proto);
    }
    const osg::Object *myobj_;
};
#endif



struct Registry::ReadObjectFunctor : public Registry::ReadFunctor
{
    ReadObjectFunctor(const std::string& filename, const Options* options):ReadFunctor(filename,options) {}

    virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw) const { return rw.readObject(_filename, _options); }
    virtual bool isValid(ReaderWriter::ReadResult& readResult) const { return readResult.validObject(); }
    virtual bool isValid(osg::Object* object) const { return object!=0;  }

    virtual ReadFunctor* cloneType(const std::string& filename, const Options* options) const { return new ReadObjectFunctor(filename, options); }
};

struct Registry::ReadImageFunctor : public Registry::ReadFunctor
{
    ReadImageFunctor(const std::string& filename, const Options* options):ReadFunctor(filename,options) {}

    virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw)const  { return rw.readImage(_filename, _options); }
    virtual bool isValid(ReaderWriter::ReadResult& readResult) const { return readResult.validImage(); }
    virtual bool isValid(osg::Object* object) const { return dynamic_cast<osg::Image*>(object)!=0;  }

    virtual ReadFunctor* cloneType(const std::string& filename, const Options* options) const { return new ReadImageFunctor(filename, options); }
};

struct Registry::ReadHeightFieldFunctor : public Registry::ReadFunctor
{
    ReadHeightFieldFunctor(const std::string& filename, const Options* options):ReadFunctor(filename,options) {}

    virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw) const { return rw.readHeightField(_filename, _options); }
    virtual bool isValid(ReaderWriter::ReadResult& readResult) const { return readResult.validHeightField(); }
    virtual bool isValid(osg::Object* object) const { return dynamic_cast<osg::HeightField*>(object)!=0;  }

    virtual ReadFunctor* cloneType(const std::string& filename, const Options* options) const { return new ReadHeightFieldFunctor(filename, options); }
};

struct Registry::ReadNodeFunctor : public Registry::ReadFunctor
{
    ReadNodeFunctor(const std::string& filename, const Options* options):ReadFunctor(filename,options) {}

    virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw) const { return rw.readNode(_filename, _options); }
    virtual bool isValid(ReaderWriter::ReadResult& readResult) const { return readResult.validNode(); }
    virtual bool isValid(osg::Object* object) const { return dynamic_cast<osg::Node*>(object)!=0;  }

    virtual ReadFunctor* cloneType(const std::string& filename, const Options* options) const { return new ReadNodeFunctor(filename, options); }
};

struct Registry::ReadArchiveFunctor : public Registry::ReadFunctor
{
    ReadArchiveFunctor(const std::string& filename, ReaderWriter::ArchiveStatus status, unsigned int indexBlockSizeHint, const Options* options):
        ReadFunctor(filename,options),
        _status(status),
        _indexBlockSizeHint(indexBlockSizeHint) {}

    ReaderWriter::ArchiveStatus _status;
    unsigned int _indexBlockSizeHint;

    virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw) const { return rw.openArchive(_filename, _status, _indexBlockSizeHint, _options); }
    virtual bool isValid(ReaderWriter::ReadResult& readResult) const { return readResult.validArchive(); }
    virtual bool isValid(osg::Object* object) const { return dynamic_cast<osgDB::Archive*>(object)!=0;  }

    virtual ReadFunctor* cloneType(const std::string& filename, const Options* options) const { return new ReadArchiveFunctor(filename, _status, _indexBlockSizeHint, options); }
};

struct Registry::ReadShaderFunctor : public Registry::ReadFunctor
{
    ReadShaderFunctor(const std::string& filename, const Options* options):ReadFunctor(filename,options) {}

    virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw)const  { return rw.readShader(_filename, _options); }
    virtual bool isValid(ReaderWriter::ReadResult& readResult) const { return readResult.validShader(); }
    virtual bool isValid(osg::Object* object) const { return dynamic_cast<osg::Shader*>(object)!=0;  }

    virtual ReadFunctor* cloneType(const std::string& filename, const Options* options) const { return new ReadShaderFunctor(filename, options); }
};

struct Registry::ReadScriptFunctor : public Registry::ReadFunctor
{
    ReadScriptFunctor(const std::string& filename, const Options* options):ReadFunctor(filename,options) {}

    virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw)const  { return rw.readScript(_filename, _options); }
    virtual bool isValid(ReaderWriter::ReadResult& readResult) const { return readResult.validScript(); }
    virtual bool isValid(osg::Object* object) const { return dynamic_cast<osg::Script*>(object)!=0;  }

    virtual ReadFunctor* cloneType(const std::string& filename, const Options* options) const { return new ReadScriptFunctor(filename, options); }
};


void Registry::addArchiveExtension(const std::string ext)
{
    for(ArchiveExtensionList::iterator aitr=_archiveExtList.begin();
        aitr!=_archiveExtList.end();
        ++aitr)
    {
        if ( (*aitr) == ext)   // extension already in archive extension list
            return;
    }
    _archiveExtList.push_back(ext);
}

std::string Registry::findDataFileImplementation(const std::string& filename, const Options* options, CaseSensitivity caseSensitivity)
{
    if (filename.empty()) return filename;

    // if data file contains a server address then we can't find it in local directories so return empty string.
    if (containsServerAddress(filename)) return std::string();

    bool absolutePath = osgDB::isAbsolutePath(filename);

    if (absolutePath && fileExists(filename))
    {
        OSG_DEBUG << "FindFileInPath(" << filename << "): returning " << filename << std::endl;
        return filename;
    }

    std::string fileFound;
    bool pathsContainsCurrentWorkingDirectory = false;

    if (options && !options->getDatabasePathList().empty())
    {
        fileFound = findFileInPath(filename, options->getDatabasePathList(), caseSensitivity);
        if (!fileFound.empty()) return fileFound;

        if (osgDB::containsCurrentWorkingDirectoryReference(options->getDatabasePathList()))
        {
            pathsContainsCurrentWorkingDirectory = true;
        }

    }

    const FilePathList& filepaths = Registry::instance()->getDataFilePathList();
    if (!filepaths.empty())
    {
        fileFound = findFileInPath(filename, filepaths, caseSensitivity);
        if (!fileFound.empty()) return fileFound;

        if (!pathsContainsCurrentWorkingDirectory && osgDB::containsCurrentWorkingDirectoryReference(filepaths))
        {
            pathsContainsCurrentWorkingDirectory = true;
        }
    }

    if (!absolutePath && !pathsContainsCurrentWorkingDirectory)
    {
        // check current working directory
        if (fileExists(filename))
        {
            return filename;
        }
    }


    // if a directory is included in the filename, get just the (simple) filename itself and try that
    std::string simpleFileName = getSimpleFileName(filename);
    if (simpleFileName!=filename)
    {

        if(fileExists(simpleFileName))
        {
            OSG_DEBUG << "FindFileInPath(" << filename << "): returning " << simpleFileName << std::endl;
            return simpleFileName;
        }

        if (options && !options->getDatabasePathList().empty())
        {
            fileFound = findFileInPath(simpleFileName, options->getDatabasePathList(), caseSensitivity);
            if (!fileFound.empty()) return fileFound;
        }

        if (!filepaths.empty())
        {
            fileFound = findFileInPath(simpleFileName, filepaths,caseSensitivity);
            if (!fileFound.empty()) return fileFound;
        }

    }

    // return empty string.
    return std::string();
}

std::string Registry::findLibraryFileImplementation(const std::string& filename, const Options* /*options*/, CaseSensitivity caseSensitivity)
{
    if (filename.empty())
        return filename;

    const FilePathList& filepath = Registry::instance()->getLibraryFilePathList();


    std::string fileFound = findFileInPath(filename, filepath,caseSensitivity);
    if (!fileFound.empty())
        return fileFound;

    if(fileExists(filename))
    {
        OSG_DEBUG << "FindFileInPath(" << filename << "): returning " << filename << std::endl;
        return filename;
    }

    // if a directory is included in the filename, get just the (simple) filename itself and try that
    std::string simpleFileName = getSimpleFileName(filename);
    if (simpleFileName!=filename)
    {
        fileFound = findFileInPath(simpleFileName, filepath,caseSensitivity);
        if (!fileFound.empty()) return fileFound;
    }

    // failed return empty string.
    return std::string();
}



ReaderWriter::ReadResult Registry::read(const ReadFunctor& readFunctor)
{
    for(ArchiveExtensionList::iterator aitr=_archiveExtList.begin();
        aitr!=_archiveExtList.end();
        ++aitr)
    {
        std::string archiveExtension = "." + (*aitr);

        std::string::size_type positionArchive = readFunctor._filename.find(archiveExtension+'/');
        if (positionArchive==std::string::npos) positionArchive = readFunctor._filename.find(archiveExtension+'\\');
        if (positionArchive!=std::string::npos)
        {
            std::string::size_type endArchive = positionArchive + archiveExtension.length();
            std::string archiveName( readFunctor._filename.substr(0,endArchive));
            std::string fileName(readFunctor._filename.substr(endArchive+1,std::string::npos));
            OSG_INFO<<"Contains archive : "<<readFunctor._filename<<std::endl;
            OSG_INFO<<"         archive : "<<archiveName<<std::endl;
            OSG_INFO<<"         filename : "<<fileName<<std::endl;

            ReaderWriter::ReadResult result = openArchiveImplementation(archiveName,ReaderWriter::READ, 4096, readFunctor._options);

            if (!result.validArchive()) return result;

            osgDB::Archive* archive = result.getArchive();

            //if valid options were passed through the read functor clone them
            //otherwise make new options
            osg::ref_ptr<osgDB::ReaderWriter::Options> options = readFunctor._options ?
                readFunctor._options->cloneOptions() :
                new osgDB::ReaderWriter::Options;

            options->setDatabasePath(archiveName);

            osg::ref_ptr<ReadFunctor> rf(readFunctor.cloneType(fileName, options.get()));

            result = rf->doRead(*archive);

            if (rf->isValid(result))
            {
                OSG_INFO<<"Read object from archive"<<std::endl;
                return result;
            }
            OSG_INFO<<"Failed to read object from archive"<<std::endl;
        }
    }

    // record the errors reported by readerwriters.
    typedef std::vector<ReaderWriter::ReadResult> Results;
    Results results;

    // first attempt to load the file from existing ReaderWriter's
    AvailableReaderWriterIterator itr(_rwList, _pluginMutex);
    for(;itr.valid();++itr)
    {
        ReaderWriter::ReadResult rr = readFunctor.doRead(*itr);
        if (readFunctor.isValid(rr)) return rr;
        else results.push_back(rr);
    }

    // check loaded archives.
    AvailableArchiveIterator aaitr(_archiveCache, _archiveCacheMutex);
    for(;aaitr.valid();++aaitr)
    {
        ReaderWriter::ReadResult rr = readFunctor.doRead(*aaitr);
        if (readFunctor.isValid(rr)) return rr;
        else
        {
            // don't pass on FILE_NOT_FOUND results as we don't want to prevent non archive plugins that haven't been
            // loaded yet from getting a chance to test for the presence of the file.
            if (rr.status()!=ReaderWriter::ReadResult::FILE_NOT_FOUND) results.push_back(rr);
        }
    }

    // now look for a plug-in to load the file.
    std::string libraryName = createLibraryNameForFile(readFunctor._filename);
    if (loadLibrary(libraryName)!=NOT_LOADED)
    {
        for(;itr.valid();++itr)
        {
            ReaderWriter::ReadResult rr = readFunctor.doRead(*itr);
            if (readFunctor.isValid(rr)) return rr;
            else results.push_back(rr);
        }
    }

    //If the filename contains a server address and wasn't loaded by any of the plugins, try to find a plugin which supports the server
    //protocol and supports wildcards. If not successfully use curl as a last fallback
    if (containsServerAddress(readFunctor._filename))
    {
        ReaderWriter* rw = getReaderWriterForProtocolAndExtension(
            osgDB::getServerProtocol(readFunctor._filename),
            osgDB::getFileExtension(readFunctor._filename)
        );

        if (rw)
        {
            return readFunctor.doRead(*rw);
        }
        else
        {
            return  ReaderWriter::ReadResult("Could not find the .curl plugin to read from server.");
        }
    }

    if (results.empty())
    {
        return ReaderWriter::ReadResult("Could not find plugin to read objects from file \""+readFunctor._filename+"\".");
    }

    // sort the results so the most relevant (i.e. ERROR_IN_READING_FILE is more relevant than FILE_NOT_FOUND) results get placed at the end of the results list.
    std::sort(results.begin(), results.end());
    ReaderWriter::ReadResult result = results.back();

    return result;
}

ReaderWriter::ReadResult Registry::readImplementation(const ReadFunctor& readFunctor,Options::CacheHintOptions cacheHint)
{
    std::string file(readFunctor._filename);

    bool useObjectCache = false;
    const Options* options = readFunctor._options;
    ObjectCache* optionsCache = options ? options->getObjectCache() : 0;

    //Note CACHE_ARCHIVES has a different object that it caches to so it will never be used here
    if ((optionsCache || _objectCache.valid()) && cacheHint!=Options::CACHE_ARCHIVES)
    {
        useObjectCache= options ? (options->getObjectCacheHint()&cacheHint)!=0: false;
    }

    if (useObjectCache)
    {
        // search for entry in the object cache.
        osg::ref_ptr<osg::Object> object = optionsCache ? optionsCache->getRefFromObjectCache(file, options) : 0;

        if (!object && _objectCache.valid()) object = _objectCache->getRefFromObjectCache(file, options);

        if (object.valid())
        {
            if (readFunctor.isValid(object.get())) return ReaderWriter::ReadResult(object.get(), ReaderWriter::ReadResult::FILE_LOADED_FROM_CACHE);
            else return ReaderWriter::ReadResult("Error file does not contain an osg::Object");
        }

        ReaderWriter::ReadResult rr = read(readFunctor);
        if (rr.validObject())
        {
            // search AGAIN for entry in the object cache.
            object = _objectCache->getRefFromObjectCache(file, options);
            if (object.valid())
            {
                if (readFunctor.isValid(object.get())) return ReaderWriter::ReadResult(object.get(), ReaderWriter::ReadResult::FILE_LOADED_FROM_CACHE);
                else
                {
                    return ReaderWriter::ReadResult("Error file does not contain an osg::Object");
                }
            }

            // update cache with new entry.
            if (optionsCache) optionsCache->addEntryToObjectCache(file, rr.getObject(), 0.0, options);
            else if (_objectCache.valid()) _objectCache->addEntryToObjectCache(file, rr.getObject(), 0.0, options);
        }
        else
        {
            OSG_INFO<<"No valid object found for "<<file<<std::endl;
        }

        return rr;

    }
    else
    {
        ReaderWriter::ReadResult rr = read(readFunctor);
        return rr;
    }
}


ReaderWriter::ReadResult Registry::openArchiveImplementation(const std::string& fileName, ReaderWriter::ArchiveStatus status, unsigned int indexBlockSizeHint, const Options* options)
{
    osg::ref_ptr<osgDB::Archive> archive = getRefFromArchiveCache(fileName);
    if (archive.valid()) return archive.get();

    ReaderWriter::ReadResult result = readImplementation(ReadArchiveFunctor(fileName, status, indexBlockSizeHint, options),Options::CACHE_ARCHIVES);

    // default to using caching archive if no options structure provided, but if options are provided use archives
    // only if supplied.
    if (result.validArchive() &&
        (!options || (options->getObjectCacheHint() & Options::CACHE_ARCHIVES)) )
    {
        addToArchiveCache(fileName,result.getArchive());
    }
    return result;
}


ReaderWriter::ReadResult Registry::readObjectImplementation(const std::string& fileName,const Options* options)
{
    return readImplementation(ReadObjectFunctor(fileName, options),Options::CACHE_OBJECTS);
}

ReaderWriter::WriteResult Registry::writeObjectImplementation(const Object& obj,const std::string& fileName,const Options* options)
{
    // record the errors reported by readerwriters.
    typedef std::vector<ReaderWriter::WriteResult> Results;
    Results results;

    // first attempt to load the file from existing ReaderWriter's
    AvailableReaderWriterIterator itr(_rwList, _pluginMutex);
    for(;itr.valid();++itr)
    {
        ReaderWriter::WriteResult rr = itr->writeObject(obj,fileName,options);
        if (rr.success()) return rr;
        else results.push_back(rr);
    }

    // now look for a plug-in to save the file.
    std::string libraryName = createLibraryNameForFile(fileName);
    if (loadLibrary(libraryName)==LOADED)
    {
        for(;itr.valid();++itr)
        {
            ReaderWriter::WriteResult rr = itr->writeObject(obj,fileName,options);
            if (rr.success()) return rr;
            else results.push_back(rr);
        }
    }

    if (results.empty())
    {
        return ReaderWriter::WriteResult("Could not find plugin to write objects to file \""+fileName+"\".");
    }

    // sort the results so the most relevant (i.e. ERROR_IN_WRITING_FILE is more relevant than FILE_NOT_FOUND) results get placed at the end of the results list.
    std::sort(results.begin(), results.end());
    ReaderWriter::WriteResult result = results.back();

    return result;
}



ReaderWriter::ReadResult Registry::readImageImplementation(const std::string& fileName,const Options* options)
{
    return readImplementation(ReadImageFunctor(fileName, options),Options::CACHE_IMAGES);
}

ReaderWriter::WriteResult Registry::writeImageImplementation(const Image& image,const std::string& fileName,const Options* options)
{
    // record the errors reported by readerwriters.
    typedef std::vector<ReaderWriter::WriteResult> Results;
    Results results;

    // first attempt to load the file from existing ReaderWriter's
    AvailableReaderWriterIterator itr(_rwList, _pluginMutex);
    for(;itr.valid();++itr)
    {
        ReaderWriter::WriteResult rr = itr->writeImage(image,fileName,options);
        if (rr.success()) return rr;
        else results.push_back(rr);
    }

    // now look for a plug-in to save the file.
    std::string libraryName = createLibraryNameForFile(fileName);
    if (loadLibrary(libraryName)==LOADED)
    {
        for(;itr.valid();++itr)
        {
            ReaderWriter::WriteResult rr = itr->writeImage(image,fileName,options);
            if (rr.success()) return rr;
            else results.push_back(rr);
        }
    }

    if (results.empty())
    {
        return ReaderWriter::WriteResult("Could not find plugin to write image to file \""+fileName+"\".");
    }

    // sort the results so the most relevant (i.e. ERROR_IN_WRITING_FILE is more relevant than FILE_NOT_FOUND) results get placed at the end of the results list.
    std::sort(results.begin(), results.end());
    ReaderWriter::WriteResult result = results.back();

    return result;
}


ReaderWriter::ReadResult Registry::readHeightFieldImplementation(const std::string& fileName,const Options* options)
{
    return readImplementation(ReadHeightFieldFunctor(fileName, options),Options::CACHE_HEIGHTFIELDS);
}

ReaderWriter::WriteResult Registry::writeHeightFieldImplementation(const HeightField& HeightField,const std::string& fileName,const Options* options)
{
    // record the errors reported by readerwriters.
    typedef std::vector<ReaderWriter::WriteResult> Results;
    Results results;

    // first attempt to load the file from existing ReaderWriter's
    AvailableReaderWriterIterator itr(_rwList, _pluginMutex);
    for(;itr.valid();++itr)
    {
        ReaderWriter::WriteResult rr = itr->writeHeightField(HeightField,fileName,options);
        if (rr.success()) return rr;
        else results.push_back(rr);
    }

    // now look for a plug-in to save the file.
    std::string libraryName = createLibraryNameForFile(fileName);
    if (loadLibrary(libraryName)==LOADED)
    {
        for(;itr.valid();++itr)
        {
            ReaderWriter::WriteResult rr = itr->writeHeightField(HeightField,fileName,options);
            if (rr.success()) return rr;
            else results.push_back(rr);
        }
    }

    if (results.empty())
    {
        return ReaderWriter::WriteResult("Could not find plugin to write HeightField to file \""+fileName+"\".");
    }

    // sort the results so the most relevant (i.e. ERROR_IN_WRITING_FILE is more relevant than FILE_NOT_FOUND) results get placed at the end of the results list.
    std::sort(results.begin(), results.end());
    ReaderWriter::WriteResult result = results.back();

    return result;
}


ReaderWriter::ReadResult Registry::readNodeImplementation(const std::string& fileName,const Options* options)
{
#if 0

    osg::Timer_t startTick = osg::Timer::instance()->tick();
    ReaderWriter::ReadResult result = readImplementation(ReadNodeFunctor(fileName, options),Options::CACHE_NODES);
    osg::Timer_t endTick = osg::Timer::instance()->tick();
    OSG_NOTICE<<"time to load "<<fileName<<" "<<osg::Timer::instance()->delta_m(startTick, endTick)<<"ms"<<std::endl;
    return result;

#else

    return readImplementation(ReadNodeFunctor(fileName, options),Options::CACHE_NODES);

#endif
}

ReaderWriter::WriteResult Registry::writeNodeImplementation(const Node& node,const std::string& fileName,const Options* options)
{
    // record the errors reported by readerwriters.
    typedef std::vector<ReaderWriter::WriteResult> Results;
    Results results;

    // first attempt to write the file from existing ReaderWriter's
    AvailableReaderWriterIterator itr(_rwList, _pluginMutex);
    for(;itr.valid();++itr)
    {
        ReaderWriter::WriteResult rr = itr->writeNode(node,fileName,options);
        if (rr.success()) return rr;
        else results.push_back(rr);
    }

    // now look for a plug-in to save the file.
    std::string libraryName = createLibraryNameForFile(fileName);

    if (loadLibrary(libraryName)==LOADED)
    {
        for(;itr.valid();++itr)
        {
            ReaderWriter::WriteResult rr = itr->writeNode(node,fileName,options);

            if (rr.success()) return rr;
            else results.push_back(rr);
        }
    }

    if (results.empty())
    {
        return ReaderWriter::WriteResult("Could not find plugin to write nodes to file \""+fileName+"\".");
    }

    // sort the results so the most relevant (i.e. ERROR_IN_WRITING_FILE is more relevant than FILE_NOT_FOUND) results get placed at the end of the results list.
    std::sort(results.begin(), results.end());
    ReaderWriter::WriteResult result = results.back();

    return result;
}

ReaderWriter::ReadResult Registry::readShaderImplementation(const std::string& fileName,const Options* options)
{
    return readImplementation(ReadShaderFunctor(fileName, options),Options::CACHE_SHADERS);
}

ReaderWriter::WriteResult Registry::writeShaderImplementation(const Shader& shader,const std::string& fileName,const Options* options)
{
    // record the errors reported by readerwriters.
    typedef std::vector<ReaderWriter::WriteResult> Results;
    Results results;

    // first attempt to load the file from existing ReaderWriter's
    AvailableReaderWriterIterator itr(_rwList, _pluginMutex);
    for(;itr.valid();++itr)
    {
        ReaderWriter::WriteResult rr = itr->writeShader(shader,fileName,options);
        if (rr.success()) return rr;
        else results.push_back(rr);
    }

    results.clear();

    // now look for a plug-in to save the file.
    std::string libraryName = createLibraryNameForFile(fileName);
    if (loadLibrary(libraryName)==LOADED)
    {
        for(;itr.valid();++itr)
        {
            ReaderWriter::WriteResult rr = itr->writeShader(shader,fileName,options);
            if (rr.success()) return rr;
            else results.push_back(rr);
        }
    }

    if (results.empty())
    {
        return ReaderWriter::WriteResult("Could not find plugin to write shader to file \""+fileName+"\".");
    }

    // sort the results so the most relevant (i.e. ERROR_IN_WRITING_FILE is more relevant than FILE_NOT_FOUND) results get placed at the end of the results list.
    std::sort(results.begin(), results.end());
    ReaderWriter::WriteResult result = results.back();


    return result;
}

ReaderWriter::ReadResult Registry::readScriptImplementation(const std::string& fileName,const Options* options)
{
    return readImplementation(ReadScriptFunctor(fileName, options),Options::CACHE_IMAGES);
}

ReaderWriter::WriteResult Registry::writeScriptImplementation(const Script& image,const std::string& fileName,const Options* options)
{
    // record the errors reported by readerwriters.
    typedef std::vector<ReaderWriter::WriteResult> Results;
    Results results;

    // first attempt to load the file from existing ReaderWriter's
    AvailableReaderWriterIterator itr(_rwList, _pluginMutex);
    for(;itr.valid();++itr)
    {
        ReaderWriter::WriteResult rr = itr->writeScript(image,fileName,options);
        if (rr.success()) return rr;
        else results.push_back(rr);
    }

    // now look for a plug-in to save the file.
    std::string libraryName = createLibraryNameForFile(fileName);
    if (loadLibrary(libraryName)==LOADED)
    {
        for(;itr.valid();++itr)
        {
            ReaderWriter::WriteResult rr = itr->writeScript(image,fileName,options);
            if (rr.success()) return rr;
            else results.push_back(rr);
        }
    }

    if (results.empty())
    {
        return ReaderWriter::WriteResult("Could not find plugin to write image to file \""+fileName+"\".");
    }

    // sort the results so the most relevant (i.e. ERROR_IN_WRITING_FILE is more relevant than FILE_NOT_FOUND) results get placed at the end of the results list.
    std::sort(results.begin(), results.end());
    ReaderWriter::WriteResult result = results.back();

    return result;
}

void Registry::addEntryToObjectCache(const std::string& filename, osg::Object* object, double timestamp, Options *options)
{
    if (_objectCache.valid()) _objectCache->addEntryToObjectCache(filename, object, timestamp, options);
}

osg::Object* Registry::getFromObjectCache(const std::string& filename, Options *options)
{
    return _objectCache.valid() ? _objectCache->getFromObjectCache(filename, options) : 0;
}

osg::ref_ptr<osg::Object> Registry::getRefFromObjectCache(const std::string& filename, Options *options)
{
    return _objectCache.valid() ? _objectCache->getRefFromObjectCache(filename, options) : 0;
}

void Registry::updateTimeStampOfObjectsInCacheWithExternalReferences(const osg::FrameStamp& frameStamp)
{
    if (_objectCache.valid()) _objectCache->updateTimeStampOfObjectsInCacheWithExternalReferences(frameStamp.getReferenceTime());
}

void Registry::removeExpiredObjectsInCache(const osg::FrameStamp& frameStamp)
{
    double expiryTime = frameStamp.getReferenceTime() - _expiryDelay;
    if (_objectCache.valid()) _objectCache->removeExpiredObjectsInCache(expiryTime);
}

void Registry::removeFromObjectCache(const std::string& filename, Options *options)
{
    if (_objectCache.valid()) _objectCache->removeFromObjectCache(filename, options);
}

void Registry::clearObjectCache()
{
    if (_objectCache.valid()) _objectCache->clear();
}

void Registry::addToArchiveCache(const std::string& fileName, osgDB::Archive* archive)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_archiveCacheMutex);
    _archiveCache[fileName] = archive;
}

/** Remove archive from cache.*/
void Registry::removeFromArchiveCache(const std::string& fileName)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_archiveCacheMutex);
    ArchiveCache::iterator itr = _archiveCache.find(fileName);
    if (itr!=_archiveCache.end())
    {
        _archiveCache.erase(itr);
    }
}

osgDB::Archive* Registry::getFromArchiveCache(const std::string& fileName)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_archiveCacheMutex);
    ArchiveCache::iterator itr = _archiveCache.find(fileName);
    if (itr!=_archiveCache.end()) return itr->second.get();
    else return 0;
}

osg::ref_ptr<osgDB::Archive> Registry::getRefFromArchiveCache(const std::string& fileName)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_archiveCacheMutex);
    ArchiveCache::iterator itr = _archiveCache.find(fileName);
    if (itr!=_archiveCache.end()) return itr->second;
    else return 0;
}

void Registry::clearArchiveCache()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_archiveCacheMutex);
    _archiveCache.clear();
}

void Registry::releaseGLObjects(osg::State* state)
{
    if (_objectCache.valid()) _objectCache->releaseGLObjects( state );
    if (_sharedStateManager.valid()) _sharedStateManager->releaseGLObjects( state );
}

SharedStateManager* Registry::getOrCreateSharedStateManager()
{
    if (!_sharedStateManager) _sharedStateManager = new SharedStateManager;

    return _sharedStateManager.get();
}


void Registry::registerProtocol(const std::string& protocol)
{
    _registeredProtocols.insert( convertToLowerCase(protocol) );
}

bool Registry::isProtocolRegistered(const std::string& protocol)
{
    return (_registeredProtocols.find( convertToLowerCase(protocol) ) != _registeredProtocols.end());
}

void Registry::getReaderWriterListForProtocol(const std::string& protocol, ReaderWriterList& results) const
{
    for(ReaderWriterList::const_iterator i = _rwList.begin(); i != _rwList.end(); ++i)
    {        if ((*i)->acceptsProtocol(protocol))
            results.push_back(*i);
    }
}


ReaderWriter* Registry::getReaderWriterForProtocolAndExtension(const std::string& protocol, const std::string& extension)
{
    // try first the registered ReaderWriter
    ReaderWriter* result = getReaderWriterForExtension(extension);
    if (result && result->acceptsProtocol(protocol))
        return result;

    result = NULL;
    ReaderWriterList results;
    getReaderWriterListForProtocol(protocol, results);

    for(ReaderWriterList::const_iterator i = results.begin(); i != results.end(); ++i)
    {
        // if we have a readerwriter which supports wildcards, save it as a fallback
        if ((*i)->acceptsExtension("*"))
            result = i->get();
        else if ((*i)->acceptsExtension(extension))
            return i->get();
    }

    return result ? result : getReaderWriterForExtension("curl");
}
