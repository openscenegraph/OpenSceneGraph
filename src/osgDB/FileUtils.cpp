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
#if 0 // defined(__APPLE__)
#include "FileUtils_Mac.cpp" // this is not functional yet -- fix!
#else

// currently this impl is for _all_ platforms, execpt as defined.
// the mac version will change soon to reflect the path scheme under osx, but
// for now, the above include is commented out, and the below code takes precedence.

#if defined(WIN32) && !defined(__CYGWIN__)
    #include <Io.h>
    #include <Windows.h>
    #include <Winbase.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    // set up for windows so acts just like unix access().
    #define F_OK 4
#else // unix
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
#endif

#include <osg/Notify>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>


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
#if 1

    // proposed single code path, from Bruce Clay.
    struct stat fileStat;
    if ( stat(filename.c_str(), &fileStat) != 0 ) 
    {
        return FILE_NOT_FOUND;
    } // end if

    if ( fileStat.st_mode & S_IFDIR )
        return DIRECTORY;
    else if ( fileStat.st_mode & S_IFREG )
        return REGULAR_FILE;

    return FILE_NOT_FOUND;

#else
    #if defined(WIN32) && !defined(__CYGWIN__)
        struct _stat fileStat;
        if ( _stat(filename.c_str(), &fileStat) != 0 ) 
        {
            return FILE_NOT_FOUND;
        } // end if

        if ( fileStat.st_mode & _S_IFDIR )
            return DIRECTORY;
        else if ( fileStat.st_mode & _S_IFREG )
            return REGULAR_FILE;

        return FILE_NOT_FOUND;

    #else
        struct stat fileStat;

        if(stat(filename.c_str(), &fileStat) == -1)
        {
	    return FILE_NOT_FOUND;
        }

        if(S_ISREG(fileStat.st_mode))
        {
	    return REGULAR_FILE;
        }

        if(S_ISDIR(fileStat.st_mode))
        {
	    return DIRECTORY;
        }

        return FILE_NOT_FOUND;
    #endif
#endif
}

std::string osgDB::findFileInPath(const std::string& filename, const FilePathList& filepath,CaseSensitivity caseSensitivity)
{
    if (filename.empty()) 
        return filename;

    if(fileExists(filename)) 
    {
        osg::notify(osg::DEBUG_INFO) << "FindFileInPath(" << filename << "): returning " << filename << std::endl;
        return filename;
    }

    for(FilePathList::const_iterator itr=filepath.begin();
        itr!=filepath.end();
        ++itr)
    {
        std::string path = *itr + '/'+ filename;
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
    if (filename.empty()) return filename;

    const FilePathList& filepath = Registry::instance()->getDataFilePathList();

    std::string fileFound = findFileInPath(filename, filepath,caseSensitivity);
    if (!fileFound.empty()) return fileFound;

    // if a directory is included in the filename, get just the (simple) filename itself and try that
    std::string simpleFileName = getSimpleFileName(filename);
    if (simpleFileName!=filename)
    {
        std::string fileFound = findFileInPath(simpleFileName, filepath,caseSensitivity);
        if (!fileFound.empty()) return fileFound;
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

#endif // ! target mac carbon



/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation of appendPlatformSpecificLibraryFilePaths(..)
//
#ifdef __sgi

    void osgDB::appendPlatformSpecificLibraryFilePaths(FilePathList& filepath)
    {
        convertStringPathIntoFilePathList("/usr/lib32/:/usr/local/lib32/",filepath);

        // bloody mess see rld(1) man page
        #if (_MIPS_SIM == _MIPS_SIM_ABI32)

        char* ptr;
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

    // #define COMPILE_COCOA_VERSION_WITH_OBJECT-C++
    #ifdef COMPILE_COCOA_VERSION_WITH_OBJECT-C++
    #include <Foundation/Foundation.h>
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

    #else

    #include <CoreServices/CoreServices.h>
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

        const int MAX_OSX_PATH_SIZE = 1024;
        const std::string OSG_PLUGIN_PATH("/OpenSceneGraph/PlugIns");
        char buffer[MAX_OSX_PATH_SIZE];
        char bundlePathBuffer[MAX_OSX_PATH_SIZE];
        CFURLRef  url;
        CFStringRef pathString;
        CFBundleRef myBundle;
        CFStringRef bundlePathString;
        FSRef     f;
        OSErr	errCode;

        // Start with the the Bundle PlugIns directory.
        // Unlike the Cocoa API, it seems that the PlugIn path is relative
        // and not absolute. So I will need to fetch both the bundle path
        // (which is absolute) and the PlugIn path (which is relative),
        // and combine the two to form a full path.

        // Get the bundle first
        myBundle = CFBundleGetMainBundle();
        if(myBundle != NULL)
        {
            // Get the URL to the bundle
            url = CFBundleCopyBundleURL( myBundle );

            // Convert the URL to a CFString that looks like a Unix file path
            bundlePathString = CFURLCopyFileSystemPath( url, kCFURLPOSIXPathStyle );
            // Convert the CFString to a C string
            CFStringGetCString( bundlePathString, bundlePathBuffer, MAX_OSX_PATH_SIZE, kCFStringEncodingUTF8 );

            CFRelease( url );

            // Now find the PlugIns directory
            // Get the URL to the bundle
            url = CFBundleCopyBuiltInPlugInsURL( myBundle );
            //pathString = CFURLCopyPath( url );
            // Convert the URL to a CFString that looks like a Unix file path
            pathString = CFURLCopyFileSystemPath( url, kCFURLPOSIXPathStyle );
            // Convert the CFString to a C string
            CFStringGetCString( pathString, buffer, MAX_OSX_PATH_SIZE, kCFStringEncodingUTF8 );

            // Combine the string and copy it into the FilePath list
            filepath.push_back( 
                std::string(bundlePathBuffer) 
                + std::string("/")
                + std::string(buffer)
            );

            CFRelease( pathString );
            CFRelease( bundlePathString );
            CFRelease( url );

            // I was getting random crashes which I believe were caused by
            // releasing the bundle. The documentation says I'm responsible
            // for retaining and releasing which didn't immediately register
            // in my head. I never retain the bundle, so I don't release it.
            // CFRelease( myBundle );

            pathString = NULL;
            bundlePathString = NULL;
            url = NULL;
            // myBundle = NULL;
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
            // Convert the URL to a CFString that looks like a Unix file path
            pathString = CFURLCopyFileSystemPath( url, kCFURLPOSIXPathStyle );
            // Convert the CFString to a C string
            CFStringGetCString( pathString, buffer, MAX_OSX_PATH_SIZE, kCFStringEncodingUTF8 );

            // Add the directory to the FilePathList
            filepath.push_back( 
                std::string(buffer) 
                + OSG_PLUGIN_PATH
            );

            CFRelease( url );
            CFRelease( pathString );
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
            // Convert the URL to a CFString that looks like a Unix file path
            pathString = CFURLCopyFileSystemPath( url, kCFURLPOSIXPathStyle );
            // Convert the CFString to a C string
            CFStringGetCString( pathString, buffer, MAX_OSX_PATH_SIZE, kCFStringEncodingUTF8 );

            // Add the directory to the FilePathList
            filepath.push_back( 
                std::string(buffer) 
                + OSG_PLUGIN_PATH
            );

            CFRelease( url );
            CFRelease( pathString );
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
            // Convert the URL to a CFString that looks like a Unix file path
            pathString = CFURLCopyFileSystemPath( url, kCFURLPOSIXPathStyle );
            // Convert the CFString to a C string
            CFStringGetCString( pathString, buffer, MAX_OSX_PATH_SIZE, kCFStringEncodingUTF8 );

            // Add the directory to the FilePathList
            filepath.push_back( 
                std::string(buffer) 
                + OSG_PLUGIN_PATH
            );

            CFRelease( url );
            CFRelease( pathString );
        }
        else
        {
            osg::notify( osg::DEBUG_INFO ) << "Couldn't find the Network Application Support Path" << std::endl;
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

        convertStringPathIntoFilePathList("/usr/lib/:/usr/local/lib/",filepath);

    }

#endif










