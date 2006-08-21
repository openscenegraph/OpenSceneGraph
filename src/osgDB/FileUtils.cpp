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

// currently this impl is for _all_ platforms, execpt as defined.
// the mac version will change soon to reflect the path scheme under osx, but
// for now, the above include is commented out, and the below code takes precedence.

#if defined(WIN32) && !defined(__CYGWIN__)
    #include <Io.h>
    #include <Windows.h>
    #include <Winbase.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <direct.h> // for _mkdir
    
    #define mkdir(x,y) _mkdir((x))
    #define stat64 _stati64

    // set up for windows so acts just like unix access().
    #define F_OK 4


#else // unix

#if defined( __APPLE__ )
    // I'm not sure how we would handle this in raw Darwin
    // without the AvailablilityMacros.
    #include <AvailabilityMacros.h>
    // 10.5 defines stat64 so we can't use this #define
    // By default, MAC_OS_X_VERSION_MAX_ALLOWED is set to the latest
    // system the headers know about. So I will use this as the control
    // variable. (MIN_ALLOWED is set low by default so it is 
    // unhelpful in this case.) 
    // Unfortunately, we can't use the label MAC_OS_X_VERSION_10_4
    // for older OS's like Jaguar, Panther since they are not defined,
    // so I am going to hardcode the number.
    #if (MAC_OS_X_VERSION_MAX_ALLOWED <= 1040)
        #define stat64 stat
    #endif
#elif defined(__CYGWIN__) || defined(__FreeBSD__)
    #define stat64 stat
#endif

    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
#endif

    // set up _S_ISDIR()
#if !defined(S_ISDIR)
#  if defined( _S_IFDIR) && !defined( __S_IFDIR)
#    define __S_IFDIR _S_IFDIR
#  endif
#  define S_ISDIR(mode)    (mode&__S_IFDIR)
#endif

#include <osg/Notify>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include <errno.h>
#include <stack>

bool osgDB::makeDirectory( const std::string &path )
{
    if (path.empty())
    {
        osg::notify(osg::DEBUG_INFO) << "osgDB::makeDirectory(): cannot create an empty directory" << std::endl;
        return false;
    }
    
    struct stat64 stbuf;
    if( stat64( path.c_str(), &stbuf ) == 0 )
    {
        if( S_ISDIR(stbuf.st_mode))
            return true;
        else
        {
            osg::notify(osg::DEBUG_INFO) << "osgDB::makeDirectory(): "  << 
                    path << " already exists and is not a directory!" << std::endl;
            return false;
        }
    }

    std::string dir = path;
    std::stack<std::string> paths;
    while( true )
    {
        if( dir.empty() )
            break;
 
        if( stat64( dir.c_str(), &stbuf ) < 0 )
        {
            switch( errno )
            {
                case ENOENT:
                case ENOTDIR:
                    paths.push( dir );
                    break;
 
                default:
                    osg::notify(osg::DEBUG_INFO) << "osgDB::makeDirectory(): "  << strerror(errno) << std::endl;
                    return false;
            }
        }
        dir = getFilePath(std::string(dir));
    }

    while( !paths.empty() )
    {
        std::string dir = paths.top();
 
        #if defined(WIN32)
            //catch drive name
            if (dir.size() == 2 && dir.c_str()[1] == ':') {
                paths.pop();
                continue;
            }
        #endif

        if( mkdir( dir.c_str(), 0755 )< 0 )
        {
            osg::notify(osg::DEBUG_INFO) << "osgDB::makeDirectory(): "  << strerror(errno) << std::endl;
            return false;
        } 
        paths.pop();
    }
    return true;
}

bool osgDB::makeDirectoryForFile( const std::string &path )
{
    return makeDirectory( getFilePath( path ));
}

void osgDB::convertStringPathIntoFilePathList(const std::string& paths,FilePathList& filepath)
{
#if defined(WIN32) && !defined(__CYGWIN__)
    char delimitor = ';';
#else
    char delimitor = ':';
#endif

    if (!paths.empty())
    {
        std::string::size_type start = 0;
        std::string::size_type end;
        while ((end = paths.find_first_of(delimitor,start))!=std::string::npos)
        {
            filepath.push_back(std::string(paths,start,end-start));
            start = end+1;
        }

        filepath.push_back(std::string(paths,start,std::string::npos));
    }
 
}

bool osgDB::fileExists(const std::string& filename)
{
    return access( filename.c_str(), F_OK ) == 0;
}

osgDB::FileType osgDB::fileType(const std::string& filename)
{
    struct stat64 fileStat;
    if ( stat64(filename.c_str(), &fileStat) != 0 ) 
    {
        return FILE_NOT_FOUND;
    } // end if

    if ( fileStat.st_mode & S_IFDIR )
        return DIRECTORY;
    else if ( fileStat.st_mode & S_IFREG )
        return REGULAR_FILE;

    return FILE_NOT_FOUND;
}

std::string osgDB::findFileInPath(const std::string& filename, const FilePathList& filepath,CaseSensitivity caseSensitivity)
{
    if (filename.empty()) 
        return filename;

    if (!isFileNameNativeStyle(filename)) 
        return findFileInPath(convertFileNameToNativeStyle(filename), filepath, caseSensitivity);


    for(FilePathList::const_iterator itr=filepath.begin();
        itr!=filepath.end();
        ++itr)
    {
        osg::notify(osg::DEBUG_INFO) << "itr='" <<*itr<< "'\n";
        std::string path = itr->empty() ? filename : *itr + '/'+ filename;
        osg::notify(osg::DEBUG_INFO) << "FindFileInPath() : trying " << path << " ...\n";
        if(fileExists(path)) 
        {
            osg::notify(osg::DEBUG_INFO) << "FindFileInPath() : USING " << path << "\n";
            return path;
        }
#ifndef WIN32 
// windows already case insensitive so no need to retry..
        else if (caseSensitivity==CASE_INSENSITIVE) 
        {
            std::string foundfile = findFileInDirectory(filename,*itr,CASE_INSENSITIVE);
            if (!foundfile.empty()) return foundfile;
        }
#endif
            
    }

    return std::string();
}


std::string osgDB::findDataFile(const std::string& filename,CaseSensitivity caseSensitivity)
{
    return findDataFile(filename,static_cast<ReaderWriter::Options*>(0),caseSensitivity);
}

OSGDB_EXPORT std::string osgDB::findDataFile(const std::string& filename,const ReaderWriter::Options* options, CaseSensitivity caseSensitivity)
{
    if (filename.empty()) return filename;
    
    if(fileExists(filename)) 
    {
        osg::notify(osg::DEBUG_INFO) << "FindFileInPath(" << filename << "): returning " << filename << std::endl;
        return filename;
    }

    std::string fileFound;
    
    if (options && !options->getDatabasePathList().empty())
    {
        fileFound = findFileInPath(filename, options->getDatabasePathList(), caseSensitivity);
        if (!fileFound.empty()) return fileFound;
    }

    const FilePathList& filepath = Registry::instance()->getDataFilePathList();
    if (!filepath.empty())
    {
        fileFound = findFileInPath(filename, filepath,caseSensitivity);
        if (!fileFound.empty()) return fileFound;
    }
    

    // if a directory is included in the filename, get just the (simple) filename itself and try that
    std::string simpleFileName = getSimpleFileName(filename);
    if (simpleFileName!=filename)
    {

        if(fileExists(simpleFileName)) 
        {
            osg::notify(osg::DEBUG_INFO) << "FindFileInPath(" << filename << "): returning " << filename << std::endl;
            return simpleFileName;
        }

        if (options && !options->getDatabasePathList().empty())
        {
            fileFound = findFileInPath(simpleFileName, options->getDatabasePathList(), caseSensitivity);
            if (!fileFound.empty()) return fileFound;
        }

        if (!filepath.empty())
        {
            fileFound = findFileInPath(simpleFileName, filepath,caseSensitivity);
            if (!fileFound.empty()) return fileFound;
        }

    }

    // return empty string.
    return std::string();
}

std::string osgDB::findLibraryFile(const std::string& filename,CaseSensitivity caseSensitivity)
{
    if (filename.empty()) 
        return filename; 

    const FilePathList& filepath = Registry::instance()->getLibraryFilePathList();

    std::string fileFound = findFileInPath(filename, filepath,caseSensitivity);
    if (!fileFound.empty()) 
        return fileFound;

    if(fileExists(filename)) 
    {
        osg::notify(osg::DEBUG_INFO) << "FindFileInPath(" << filename << "): returning " << filename << std::endl;
        return filename;
    }

    // if a directory is included in the filename, get just the (simple) filename itself and try that
    std::string simpleFileName = getSimpleFileName(filename);
    if (simpleFileName!=filename)
    {
        std::string fileFound = findFileInPath(simpleFileName, filepath,caseSensitivity);
        if (!fileFound.empty()) return fileFound;
    }

    // failed with direct paths,
    // now try prepending the filename with "osgPlugins/"
    return findFileInPath("osgPlugins/"+simpleFileName,filepath,caseSensitivity);
}

std::string osgDB::findFileInDirectory(const std::string& fileName,const std::string& dirName,CaseSensitivity caseSensitivity)
{
    bool needFollowingBackslash = false;
    bool needDirectoryName = true;
    osgDB::DirectoryContents dc;

    if (dirName.empty())
    {
        dc = osgDB::getDirectoryContents(".");
        needFollowingBackslash = false;
        needDirectoryName = false;
    }
    else if (dirName=="." || dirName=="./" || dirName==".\\")
    {
        dc = osgDB::getDirectoryContents(".");
        needFollowingBackslash = false;
        needDirectoryName = false;
    }
    else
    {
        dc = osgDB::getDirectoryContents(dirName);
        char lastChar = dirName[dirName.size()-1];
        if (lastChar=='/') needFollowingBackslash = false;
        else if (lastChar=='\\') needFollowingBackslash = false;
        else needFollowingBackslash = true;
        needDirectoryName = true;
    }

    for(osgDB::DirectoryContents::iterator itr=dc.begin();
        itr!=dc.end();
        ++itr)
    {
        if ((caseSensitivity==CASE_INSENSITIVE && osgDB::equalCaseInsensitive(fileName,*itr)) ||
            (fileName==*itr))
        {
            if (!needDirectoryName) return *itr;
            else if (needFollowingBackslash) return dirName+'/'+*itr;
            else return dirName+*itr;
        }
    }
    return "";
}

#if defined(WIN32) && !defined(__CYGWIN__)
    #include <io.h>
    #include <direct.h>

    osgDB::DirectoryContents osgDB::getDirectoryContents(const std::string& dirName)
    {
        osgDB::DirectoryContents contents;

        WIN32_FIND_DATA data;
        HANDLE handle = FindFirstFile((dirName + "\\*").c_str(), &data);
        if (handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                contents.push_back(data.cFileName);
            }
            while (FindNextFile(handle, &data) != 0);

            FindClose(handle);
        }
        return contents;
    }

#else

    #include <dirent.h>
    osgDB::DirectoryContents osgDB::getDirectoryContents(const std::string& dirName)
    {
        osgDB::DirectoryContents contents;

        DIR *handle = opendir(dirName.c_str());
        if (handle)
        {
            dirent *rc;
            while((rc = readdir(handle))!=NULL)
            {
                contents.push_back(rc->d_name);
            }
            closedir(handle);
        }

        return contents;
    }

#endif // unix getDirectoryContexts


/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation of appendPlatformSpecificLibraryFilePaths(..)
//
#ifdef __sgi

    void osgDB::appendPlatformSpecificLibraryFilePaths(FilePathList& filepath)
    {
        convertStringPathIntoFilePathList("/usr/lib32/:/usr/local/lib32/",filepath);

        // bloody mess see rld(1) man page
        char* ptr;

        #if (_MIPS_SIM == _MIPS_SIM_ABI32)
        if( (ptr = getenv( "LD_LIBRARY_PATH" )))
        {
            convertStringPathIntoFilePathList(ptr,filepath);
        }

        #elif (_MIPS_SIM == _MIPS_SIM_NABI32)

        if( !(ptr = getenv( "LD_LIBRARYN32_PATH" )))
            ptr = getenv( "LD_LIBRARY_PATH" );

        if( ptr )
        {
            convertStringPathIntoFilePathList(ptr,filepath);
        }

        #elif (_MIPS_SIM == _MIPS_SIM_ABI64)

        if( !(ptr = getenv( "LD_LIBRARY64_PATH" )))
            ptr = getenv( "LD_LIBRARY_PATH" );

        if( ptr )
        {
            convertStringPathIntoFilePathList(ptr,filepath);
        }
        #endif
    }


#elif defined(__CYGWIN__)

    void osgDB::appendPlatformSpecificLibraryFilePaths(FilePathList& filepath)
    {
        char* ptr;
        if ((ptr = getenv( "PATH" )))
        {
            convertStringPathIntoFilePathList(ptr,filepath);
        }

        convertStringPathIntoFilePathList("/usr/bin/:/usr/local/bin/",filepath);

    }
    
#elif defined(WIN32)

    void osgDB::appendPlatformSpecificLibraryFilePaths(FilePathList& filepath)
    {
        char* ptr;
        if ((ptr = getenv( "PATH" )))
        {
            convertStringPathIntoFilePathList(ptr,filepath);
        }

        convertStringPathIntoFilePathList("C:/Windows/System/",filepath);
    }
    
#elif defined(__APPLE__)

    // #define COMPILE_COCOA_VERSION
    #define COMPILE_CARBON_VERSION
    // WARNING: Cocoa version is currently untested.
    #ifdef COMPILE_COCOA_VERSION
        #include <Foundation/Foundation.h>
    #endif
    #ifdef COMPILE_CARBON_VERSION
        #include <CoreServices/CoreServices.h>
        #include <CoreFoundation/CoreFoundation.h>
        #include <Carbon/Carbon.h>
    #endif
    #include <iostream>
    
    // These functions are local to FileUtils.cpp and not exposed to the API
    // returns the path string except for numToShorten directories stripped off the end
    std::string GetShortenedPath(std::string path, int numToShorten)
    {
        unsigned int i = path.length() - 1;
        if(path[i] == '/') i--;
        while(i > 1 && numToShorten)
        {
            if(path[i] == '/')
                numToShorten--;
            i--;
        }
        return path.substr(0,i + 1);
    }

    // returns an absolute (POSIX on MacOS X) path from a CFURLRef
    std::string GetPathFromCFURLRef(CFURLRef urlRef)
    {
        char buffer[1024];
        std::string path;
        if(CFURLGetFileSystemRepresentation(urlRef, true, (UInt8*)buffer, 1024))
            path = std::string(buffer);
        return path;
    }

    // returns the absolute path to the main bundle
    std::string GetApplicationBundlePath(CFBundleRef mainBundle)
    {
        std::string path;
        CFURLRef urlRef = CFBundleCopyBundleURL(mainBundle);
        if(urlRef)
        {
            path = GetPathFromCFURLRef(urlRef);
            CFRelease(urlRef); // docs say we are responsible for releasing CFURLRef
        }
        return path;
        
    }

    std::string GetApplicationParentPath(CFBundleRef mainBundle)
    {
        return GetShortenedPath(GetApplicationBundlePath(mainBundle), 1);
    }

    std::string GetApplicationPluginsPath(CFBundleRef mainBundle)
    {
        std::string path;
        CFURLRef urlRef = CFBundleCopyBuiltInPlugInsURL(mainBundle);
        if(urlRef)
        {
            path = GetPathFromCFURLRef(urlRef);
            CFRelease(urlRef);
        }
        return path;
        
    }

    std::string GetApplicationResourcesPath(CFBundleRef mainBundle)
    {
        std::string path;
        CFURLRef urlRef = CFBundleCopyResourcesDirectoryURL(mainBundle);
        if(urlRef)
        {
            path = GetPathFromCFURLRef(urlRef);
            CFRelease(urlRef);
        }
        return path;
    }

    // The Cocoa version is about 10 lines of code.
    // The Carbon version is noticably longer.
    // Unfortunately, the Cocoa version requires -lobjc to be 
    // linked in when creating an executable. 
    // Rumor is that this will be done autmatically in gcc 3.5/Tiger,
    // but for now, this will cause a lot of headaches for people
    // who aren't familiar with this concept, so the Carbon version 
    // is preferable.
    // But for the curious, both implementations are here.
    // Note that if the Cocoa version is used, the file should be 
    // renamed to use the .mm extension to denote Objective-C++.
    // And of course, you will need to link against Cocoa
    // Update: There is a bug in the Cocoa version. Advanced users can remap 
    // their systems so these paths go somewhere else. The Carbon calls
    // will catch this, but the hardcoded Cocoa code below will not.

    #ifdef COMPILE_COCOA_VERSION
    // OS X has preferred locations for where PlugIns should be located.
    // This function will set this as the order to search:
    // YourProgram.app/Contents/PlugIns
    // ~/Library/Application Support/OpenSceneGraph/PlugIns
    // /Library/Application Support/OpenSceneGraph/PlugIns
    // /Network/Library/Application Support/OpenSceneGraph/PlugIns
    // 
    // As a side effect of this function, if the application is not a 
    // bundle, the first place searched becomes
    // YourProgram/PlugIns
    //
    // In principle, these other directories should be searched:
    // ~/Library/Application Support/YourProgram/PlugIns
    // /Library/Application Support/YourProgram/PlugIns
    // /Network/Library/Application Support/TheProgram/PlugIns 
    // But I'm not going to worry about it for now because the 
    // bundle's PlugIns directory is supposed to be the preferred 
    // place for this anyway.
    //
    // Another directory that might be worth considering is
    // the directory the program resides in,
    // but I'm worried about multiplatform distribution.
    // Because .so is used by other platforms like Linux, we 
    // could end up loading the wrong binary.
    // I'm not sure how robust the current code is for this case.
    // Assuming the program doesn't crash, will OSG move on to the 
    // next search directory, or just give up?
    void osgDB::appendPlatformSpecificLibraryFilePaths(FilePathList& filepath)
    {
        char* ptr;
        if ((ptr = getenv( "DYLD_LIBRARY_PATH" )) )
        {
            convertStringPathIntoFilePathList(ptr, filepath);
        }

        // Since this is currently the only Objective-C code in the
        // library, we need an autoreleasepool for obj-c memory management.
        // If more Obj-C is added, we might move this pool to another 
        // location so it can be shared. Pools seem to be stackable,
        // so I don't think there will be a problem if multiple pools
        // exist at a time.
        NSAutoreleasePool* mypool = [[NSAutoreleasePool alloc] init];

        NSString* myBundlePlugInPath;
        NSString* userSupportDir;

        // This will grab the "official" bundle plug in path.
        // It will be YourProgram.app/Contents/PlugIns (for App bundles)
        // or YourProgram/PlugIns (for Unix executables)
        myBundlePlugInPath = [[NSBundle mainBundle] builtInPlugInsPath];

        // Now setup the other search paths
        // Cocoa has a nice method for tilde expansion.
        // There's probably a better way of getting this directory, but I 
        // can't find the call.
        userSupportDir = [@"~/Library/Application Support/OpenSceneGraph/PlugIns" stringByExpandingTildeInPath];

        // Can setup the remaining directories directly in C++

        // Since Obj-C and C++ objects don't understand each other,
        // the Obj-C strings must be converted down to C strings so
        // C++ can make them into C++ strings.
        filepath.push_back( [myBundlePlugInPath UTF8String] );
        filepath.push_back( [userSupportDir UTF8String] );

        filepath.push_back( "/Library/Application Support/OpenSceneGraph/PlugIns" );
        filepath.push_back( "/Network/Library/Application Support/OpenSceneGraph/PlugIns" );

        // Clean up the autorelease pool
        [mypool release];
    }

    #elif defined(COMPILE_CARBON_VERSION)

    // OS X has preferred locations for where PlugIns should be located.
    // This function will set this as the order to search:
    // YourProgram.app/Contents/PlugIns
    // ~/Library/Application Support/OpenSceneGraph/PlugIns
    // /Library/Application Support/OpenSceneGraph/PlugIns
    // /Network/Library/Application Support/OpenSceneGraph/PlugIns
    //
    // In principle, these other directories should be searched:
    // ~/Library/Application Support/YourProgram/PlugIns
    // /Library/Application Support/YourProgram/PlugIns
    // /Network/Library/Application Support/TheProgram/PlugIns 
    // But I'm not going to worry about it for now because the 
    // bundle's PlugIns directory is supposed to be the preferred 
    // place for this anyway.
    //
    // Another directory that might be worth considering is
    // the directory the program resides in,
    // but I'm worried about multiplatform distribution.
    // Because .so is used by other platforms like Linux, we 
    // could end up loading the wrong binary.
    // I'm not sure how robust the current code is for this case.
    // Assuming the program doesn't crash, will OSG move on to the 
    // next search directory, or just give up?
    void osgDB::appendPlatformSpecificLibraryFilePaths(FilePathList& filepath)
    {
        char* ptr;
        if ((ptr = getenv( "DYLD_LIBRARY_PATH" )) )
        {
            convertStringPathIntoFilePathList(ptr, filepath);
        }

        const std::string OSG_PLUGIN_PATH("/OpenSceneGraph/PlugIns");
        CFURLRef  url;
        CFBundleRef myBundle;
        FSRef     f;
        OSErr    errCode;

        // Start with the the Bundle PlugIns directory.

        // Get the main bundle first. No need to retain or release it since
        //  we are not keeping a reference
        myBundle = CFBundleGetMainBundle();
        
        if(myBundle != NULL)
        {
            // CFBundleGetMainBundle will return a bundle ref even if 
            //  the application isn't part of a bundle, so we need to check
            //  if the path to the bundle ends in ".app" to see if it is a
            //  proper application bundle. If it is, the plugins path is added
            std::string bundlePath = GetApplicationBundlePath(myBundle);
            if( bundlePath.substr(bundlePath.length() - 4, 4) == std::string(".app") )
                filepath.push_back(GetApplicationPluginsPath(myBundle));
        }
        else
        {
            osg::notify( osg::DEBUG_INFO ) << "Couldn't find the Application Bundle" << std::endl;
        }

        // Next, check the User's Application Support folder
        errCode = FSFindFolder( kUserDomain, kApplicationSupportFolderType, kDontCreateFolder, &f );
        if(noErr == errCode)
        {
            // Get the URL
            url = CFURLCreateFromFSRef( 0, &f );
            if(url)
            {
                filepath.push_back(GetPathFromCFURLRef(url) + OSG_PLUGIN_PATH);
                CFRelease( url );
            }
            else
                osg::notify( osg::DEBUG_INFO ) << "Couldn't create CFURLRef for User's application support Path" << std::endl;

            url = NULL;
        }
        else
        {
            osg::notify( osg::DEBUG_INFO ) << "Couldn't find the User's Application Support Path" << std::endl;
        }

        // Next, check the Local System's Application Support Folder
        errCode = FSFindFolder( kLocalDomain, kApplicationSupportFolderType, kDontCreateFolder, &f );
        if(noErr == errCode)
        {
            // Get the URL
            url = CFURLCreateFromFSRef( 0, &f );
            
            if(url)
            {
                filepath.push_back(GetPathFromCFURLRef(url) + OSG_PLUGIN_PATH);
                CFRelease( url );
            }
            else
                osg::notify( osg::DEBUG_INFO ) << "Couldn't create CFURLRef for local System's ApplicationSupport Path" << std::endl;

            url = NULL;
        }
        else
        {
            osg::notify( osg::DEBUG_INFO ) << "Couldn't find the Local System's Application Support Path" << std::endl;
        }

        // Finally, check the Network Application Support Folder
        // This one has a likely chance of not existing so an error
        // may be returned. Don't panic.
        errCode = FSFindFolder( kNetworkDomain, kApplicationSupportFolderType, kDontCreateFolder, &f );
        if(noErr == errCode)
        {
            // Get the URL
            url = CFURLCreateFromFSRef( 0, &f );
            
            if(url)
            {
                filepath.push_back(GetPathFromCFURLRef(url) + OSG_PLUGIN_PATH);
                CFRelease( url );
            }
            else
                osg::notify( osg::DEBUG_INFO ) << "Couldn't create CFURLRef for network Application Support Path" << std::endl;

            url = NULL;
        }
        else
        {
        // had to comment out as it segfauls the OSX app otherwise
            // osg::notify( osg::DEBUG_INFO ) << "Couldn't find the Network Application Support Path" << std::endl;
        }
    }
    #else
    void osgDB::appendPlatformSpecificLibraryFilePaths(FilePathList& filepath)
    {
        char* ptr;
        if ((ptr = getenv( "DYLD_LIBRARY_PATH" )) )
        {
            convertStringPathIntoFilePathList(ptr, filepath);
        }
    }
    #endif
    
#else   

    void osgDB::appendPlatformSpecificLibraryFilePaths(FilePathList& filepath)
    {

       char* ptr;
       if( (ptr = getenv( "LD_LIBRARY_PATH" )) )
        {
            convertStringPathIntoFilePathList(ptr,filepath);
        }


#if defined(__ia64__) || defined(__x86_64__)
        convertStringPathIntoFilePathList("/usr/lib/:/usr/lib64/:/usr/local/lib/:/usr/local/lib64/",filepath);
#else
        convertStringPathIntoFilePathList("/usr/lib/:/usr/local/lib/",filepath);
#endif

    }

#endif




#ifdef __APPLE__
    void osgDB::appendPlatformSpecificResourceFilePaths(FilePathList& filepath)
    {
        // Get the main application bundle
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        
        if (mainBundle != NULL) {
            // Get the parent directory and the resources directory
            std::string bundlePath = GetApplicationBundlePath(mainBundle);
            std::string resourcesPath = GetApplicationResourcesPath(mainBundle);
            
            // check if application is really part of a .app bundle
            if(bundlePath.substr(bundlePath.length() - 4, 4) == std::string(".app"))
            {
                if(resourcesPath != std::string(""))
                    filepath.push_back( resourcesPath );
                
                std::string parentPath = GetShortenedPath(bundlePath, 1);
                if(parentPath != std::string(""))
                    filepath.push_back( parentPath );
            }
        }
        else
        {
            osg::notify( osg::DEBUG_INFO ) << "Couldn't find the Application Bundle." << std::endl;
        }
    }
#else
    void osgDB::appendPlatformSpecificResourceFilePaths(FilePathList& /*filepath*/)
    {
    }
#endif






