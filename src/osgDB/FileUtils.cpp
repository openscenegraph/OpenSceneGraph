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

// handle TCHAR type on various platforms
// #ifndef is inspired by https://svn.apache.org/repos/asf/logging/log4cxx/tags/v0_9_4/include/log4cxx/helpers/tchar.h
// defining type as plain char is from unzip.h, line 64

#ifndef TCHAR
typedef char TCHAR;
#endif


// currently this impl is for _all_ platforms, except as defined.
// the mac version will change soon to reflect the path scheme under osx, but
// for now, the above include is commented out, and the below code takes precedence.

#if defined(WIN32) && !defined(__CYGWIN__)
    #include <io.h>
    #define WINBASE_DECLARE_GET_MODULE_HANDLE_EX
    #include <windows.h>
    #include <winbase.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <direct.h> // for _mkdir

    #define mkdir(x,y) _mkdir((x))
    #define stat64 _stati64

    // set up for windows so acts just like unix access().
#ifndef F_OK
    #define F_OK 4
#endif

#else // unix

#if defined( __APPLE__ )
    // I'm not sure how we would handle this in raw Darwin
    // without the AvailablilityMacros.
    #include <AvailabilityMacros.h>

    //>OSG_IOS
    //IOS includes
    #include "TargetConditionals.h"
    #include <sys/cdefs.h>

    #if (TARGET_OS_IPHONE)
        #include <Availability.h>
        // workaround a bug which appears when compiling for SDK < 4.0 and for the simulator
        #if defined(__IPHONE_4_0) && (__IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_4_0)
            #define stat64 stat
        #else
            #if !TARGET_IPHONE_SIMULATOR
                #define stat64 stat
            #endif
        #endif
    #endif
    //<OSG_IPHONE

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
#elif defined(__CYGWIN__) || defined(__FreeBSD__) || defined(__DragonFly__) || \
      (defined(__hpux) && !defined(_LARGEFILE64_SOURCE))
    #define stat64 stat
#endif

    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>

    #if defined(_DARWIN_FEATURE_64_BIT_INODE)
        #define stat64 stat
    #endif
#endif

    // set up _S_ISDIR()
#if !defined(S_ISDIR)
#  if defined( _S_IFDIR) && !defined( __S_IFDIR)
#    define __S_IFDIR _S_IFDIR
#  endif
#  define S_ISDIR(mode)    (mode&__S_IFDIR)
#endif

#include <osg/Config>
#include <osgDB/ConvertUTF>
#include <osg/Notify>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include <errno.h>
#include <string.h>

#include <stack>

namespace osgDB
{
#ifdef OSG_USE_UTF8_FILENAME
#define OSGDB_STRING_TO_FILENAME(s) osgDB::convertUTF8toUTF16(s)
#define OSGDB_FILENAME_TO_STRING(s) osgDB::convertUTF16toUTF8(s)
#define OSGDB_FILENAME_TEXT(x) L ## x
#define OSGDB_WINDOWS_FUNCT(x) x ## W
#define OSGDB_WINDOWS_FUNCT_STRING(x) #x "W"
typedef wchar_t filenamechar;
typedef std::wstring filenamestring;
#else
#define OSGDB_STRING_TO_FILENAME(s) s
#define OSGDB_FILENAME_TO_STRING(s) s
#define OSGDB_FILENAME_TEXT(x) x
#define OSGDB_WINDOWS_FUNCT(x) x ## A
#define OSGDB_WINDOWS_FUNCT_STRING(x) #x "A"
typedef char filenamechar;
typedef std::string filenamestring;
#endif
}

FILE* osgDB::fopen(const char* filename, const char* mode)
{
#ifdef OSG_USE_UTF8_FILENAME
    return ::_wfopen(convertUTF8toUTF16(filename).c_str(), convertUTF8toUTF16(mode).c_str());
#else
    return ::fopen(filename, mode);
#endif
}

bool osgDB::makeDirectory( const std::string &path )
{
    if (path.empty())
    {
        OSG_DEBUG << "osgDB::makeDirectory(): cannot create an empty directory" << std::endl;
        return false;
    }

    struct stat64 stbuf;
#ifdef OSG_USE_UTF8_FILENAME
    if( _wstat64( OSGDB_STRING_TO_FILENAME(path).c_str(), &stbuf ) == 0 )
#else
    if( stat64( path.c_str(), &stbuf ) == 0 )
#endif
    {
        if( S_ISDIR(stbuf.st_mode))
            return true;
        else
        {
            OSG_DEBUG << "osgDB::makeDirectory(): "  <<
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

#ifdef OSG_USE_UTF8_FILENAME
        if( _wstat64( OSGDB_STRING_TO_FILENAME(dir).c_str(), &stbuf ) < 0 )
#else
        if( stat64( dir.c_str(), &stbuf ) < 0 )
#endif
        {
            switch( errno )
            {
                case ENOENT:
                case ENOTDIR:
                    paths.push( dir );
                    break;

                default:
                    OSG_DEBUG << "osgDB::makeDirectory(): "  << strerror(errno) << std::endl;
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

#ifdef OSG_USE_UTF8_FILENAME
        if ( _wmkdir(OSGDB_STRING_TO_FILENAME(dir).c_str())< 0 )
#else
        if( mkdir( dir.c_str(), 0755 )< 0 )
#endif
        {
            // Only return an error if the directory actually doesn't exist.  It's possible that the directory was created
            // by another thread or process
            if (!osgDB::fileExists(dir))
            {
                OSG_DEBUG << "osgDB::makeDirectory(): "  << strerror(errno) << std::endl;
                return false;
            }
        }
        paths.pop();
    }
    return true;
}

bool osgDB::makeDirectoryForFile( const std::string &path )
{
    return makeDirectory( getFilePath( path ));
}


std::string osgDB::getCurrentWorkingDirectory( void )
{
    // MAX_PATH/cwd inspired by unzip.cpp
#ifndef MAX_PATH
    #define MAX_PATH 1024
#endif
    TCHAR rootdir[MAX_PATH];
    if(getcwd(rootdir,MAX_PATH-1))
    {
        return(rootdir);
    }
    return("");
}// osgDB::getCurrentWorkingDirectory



bool osgDB::setCurrentWorkingDirectory( const std::string &newCurrentWorkingDirectory )
{
    if (newCurrentWorkingDirectory.empty())
    {
        OSG_DEBUG << "osgDB::setCurrentWorkingDirectory(): called with empty string." << std::endl;
        return false;
    }

#ifdef OSG_USE_UTF8_FILENAME
    return _wchdir( OSGDB_STRING_TO_FILENAME(newCurrentWorkingDirectory).c_str()) == 0;
#else
    return chdir( newCurrentWorkingDirectory.c_str()) == 0;
#endif

    return true;
} // osgDB::setCurrentWorkingDirectory



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

        std::string lastPath(paths,start,std::string::npos);
        if (!lastPath.empty())
            filepath.push_back(lastPath);
    }

}

bool osgDB::fileExists(const std::string& filename)
{
#ifdef OSG_USE_UTF8_FILENAME
    return _waccess( OSGDB_STRING_TO_FILENAME(filename).c_str(), F_OK ) == 0;
#else
    return access( filename.c_str(), F_OK ) == 0;
#endif
}

osgDB::FileType osgDB::fileType(const std::string& filename)
{
    struct stat64 fileStat;
#ifdef OSG_USE_UTF8_FILENAME
    if ( _wstat64(OSGDB_STRING_TO_FILENAME(filename).c_str(), &fileStat) != 0 )
#else
    if ( stat64(filename.c_str(), &fileStat) != 0 )
#endif
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
        OSG_DEBUG << "itr='" <<*itr<< "'\n";
        std::string path = itr->empty() ? filename : concatPaths(*itr, filename);

        path = getRealPath(path);

        OSG_DEBUG << "FindFileInPath() : trying " << path << " ...\n";
        if(fileExists(path))
        {
            OSG_DEBUG << "FindFileInPath() : USING " << path << "\n";
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
    return findDataFile(filename,static_cast<Options*>(0),caseSensitivity);
}

OSGDB_EXPORT std::string osgDB::findDataFile(const std::string& filename,const Options* options, CaseSensitivity caseSensitivity)
{
    return Registry::instance()->findDataFile(filename, options, caseSensitivity);
}

std::string osgDB::findLibraryFile(const std::string& filename,CaseSensitivity caseSensitivity)
{
    return Registry::instance()->findLibraryFile(filename, osgDB::Registry::instance()->getOptions(), caseSensitivity);
}

std::string osgDB::findFileInDirectory(const std::string& fileName,const std::string& dirName,CaseSensitivity caseSensitivity)
{
    bool needFollowingBackslash = false;
    bool needDirectoryName = true;
    osgDB::DirectoryContents dc;

    std::string realDirName = dirName;
    std::string realFileName = fileName;

    // Skip case-insensitive recursion if on Windows
    #ifdef WIN32
        bool win32 = true;
    #else
        bool win32 = false;
    #endif

    // If the fileName contains extra path information, make that part of the
    // directory name instead
    if (fileName != getSimpleFileName(fileName))
    {
        // See if we need to add a slash between the directory and file
        if (realDirName.empty())
        {
            realDirName = getFilePath(fileName);
        }
        else if (realDirName=="." || realDirName=="./" || realDirName==".\\")
        {
            realDirName = "./" + getFilePath(fileName);
        }
        else
        {
            char lastChar = dirName[dirName.size()-1];
            if ((lastChar == '/') || (lastChar == '\\'))
                realDirName = dirName + getFilePath(fileName);
            else
                realDirName = dirName + "/" + getFilePath(fileName);
        }

        // Simplify the file name
        realFileName = getSimpleFileName(fileName);
    }

    OSG_DEBUG << "findFileInDirectory() : looking for " << realFileName << " in " << realDirName << "...\n";

    if (realDirName.empty())
    {
        dc = osgDB::getDirectoryContents(".");
        needFollowingBackslash = false;
        needDirectoryName = false;
    }
    else if (realDirName=="." || realDirName=="./" || realDirName==".\\")
    {
        dc = osgDB::getDirectoryContents(".");
        needFollowingBackslash = false;
        needDirectoryName = false;
    }
    else if (realDirName=="/")
    {
        dc = osgDB::getDirectoryContents("/");
        needFollowingBackslash = false;
        needDirectoryName = true;
    }
    else
    {
        // See if we're working in case insensitive mode, and that we're not
        // using Windows (the recursive search is not needed in these
        // cases)
        if ((caseSensitivity == CASE_INSENSITIVE) && (!win32))
        {
            // Split the last path element from the directory name
            std::string parentPath = getFilePath(realDirName);
            std::string lastElement = getSimpleFileName(realDirName);

            // See if we're already at the top level of the filesystem
            if ((parentPath.empty()) && (!lastElement.empty()))
            {
                std::string directoryStringToUse = (realDirName[0]=='/' || realDirName[0]=='\\') ? std::string("/") : std::string(".");

                // Search for the first path element (ignoring case) in
                // the top-level directory
                realDirName = findFileInDirectory(lastElement, directoryStringToUse,
                                                  CASE_INSENSITIVE);

                dc = osgDB::getDirectoryContents(realDirName);
                needFollowingBackslash = true;
                needDirectoryName = true;
            }
            else
            {
                // Recursively search for the last path element (ignoring case)
                // in the parent path
                realDirName = findFileInDirectory(lastElement, parentPath,
                                                  CASE_INSENSITIVE);

                dc = osgDB::getDirectoryContents(realDirName);
                char lastChar = realDirName[realDirName.size()-1];
                if (lastChar=='/') needFollowingBackslash = false;
                else if (lastChar=='\\') needFollowingBackslash = false;
                else needFollowingBackslash = true;
                needDirectoryName = true;
            }
        }
        else
        {
            // No need for recursive search if we're doing an exact comparison
            dc = osgDB::getDirectoryContents(realDirName);
            char lastChar = realDirName[realDirName.size()-1];
            if (lastChar=='/') needFollowingBackslash = false;
            else if (lastChar=='\\') needFollowingBackslash = false;
            else needFollowingBackslash = true;
            needDirectoryName = true;
        }
    }

    for(osgDB::DirectoryContents::iterator itr=dc.begin();
        itr!=dc.end();
        ++itr)
    {
        if ((caseSensitivity==CASE_INSENSITIVE && osgDB::equalCaseInsensitive(realFileName,*itr)) ||
            (realFileName==*itr))
        {
            if (!needDirectoryName) return *itr;
            else if (needFollowingBackslash) return realDirName+'/'+*itr;
            else return realDirName+*itr;
        }
    }
    return "";
}

static void appendInstallationLibraryFilePaths(osgDB::FilePathList& filepath)
{
#ifdef OSG_DEFAULT_LIBRARY_PATH

    // Append the install prefix path to the library search path if configured
    filepath.push_back(OSG_DEFAULT_LIBRARY_PATH);
#endif
}

#if defined(WIN32) && !defined(__CYGWIN__)
    #include <io.h>
    #include <direct.h>

    osgDB::DirectoryContents osgDB::getDirectoryContents(const std::string& dirName)
    {
        osgDB::DirectoryContents contents;

        OSGDB_WINDOWS_FUNCT(WIN32_FIND_DATA) data;
        HANDLE handle = OSGDB_WINDOWS_FUNCT(FindFirstFile)((OSGDB_STRING_TO_FILENAME(dirName) + OSGDB_FILENAME_TEXT("\\*")).c_str(), &data);
        if (handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                contents.push_back(OSGDB_FILENAME_TO_STRING(data.cFileName));
            }
            while (OSGDB_WINDOWS_FUNCT(FindNextFile)(handle, &data) != 0);

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

osgDB::DirectoryContents osgDB::getSortedDirectoryContents(const std::string& dirName)
{
    osgDB::DirectoryContents filenames = osgDB::getDirectoryContents(dirName);
    std::sort(filenames.begin(), filenames.end(), osgDB::FileNameComparator());
    return filenames;
}


osgDB::DirectoryContents osgDB::expandWildcardsInFilename(const std::string& filename)
{
    osgDB::DirectoryContents contents;

    std::string dir = osgDB::getFilePath(filename);
    std::string filenameOnly = dir.empty() ? filename : filename.substr(dir.length()+1, std::string::npos);
    std::string left = filenameOnly.substr(0, filenameOnly.find('*'));
    std::string right = filenameOnly.substr(filenameOnly.find('*')+1, std::string::npos);

    if (dir.empty())
        dir = osgDB::getCurrentWorkingDirectory();

    osgDB::DirectoryContents dirContents = osgDB::getDirectoryContents(dir);
    for (unsigned int i = 0; i < dirContents.size(); ++i)
    {
        std::string filenameInDir = dirContents[i];

        if (filenameInDir == "." ||
            filenameInDir == "..")
        {
            continue;
        }

        if ((filenameInDir.find(left) == 0 || left.empty()) &&
            (filenameInDir.find(right) == filenameInDir.length() - right.length() || right.empty()))
        {
            contents.push_back( dir + osgDB::getNativePathSeparator() + filenameInDir );
        }
    }

    return contents;
}

osgDB::FileOpResult::Value osgDB::copyFile(const std::string & source, const std::string & destination)
{
    if (source.empty() || destination.empty())
    {
        OSG_INFO << "copyFile(): Empty file name." << std::endl;
        return FileOpResult::BAD_ARGUMENT;
    }

    // Check if source and destination are the same
    if (source == destination || osgDB::getRealPath(source) == osgDB::getRealPath(destination))
    {
        OSG_INFO << "copyFile(): Source and destination point to the same file: source=" << source << ", destination=" << destination << std::endl;
        return FileOpResult::SOURCE_EQUALS_DESTINATION;
    }

    // Check if source file exists
    if (!osgDB::fileExists(source))
    {
        OSG_INFO << "copyFile(): Source file does not exist: " << source << std::endl;
        return FileOpResult::SOURCE_MISSING;
    }

    // Open source file
    osgDB::ifstream fin(source.c_str(), std::ios::in | std::ios::binary);
    if (!fin)
    {
        OSG_NOTICE << "copyFile(): Can't read source file: " << source << std::endl;
        return FileOpResult::SOURCE_NOT_OPENED;        // Return success since it's not an output error.
    }

    // Ensure the directory exists or else the FBX SDK will fail
    if (!osgDB::makeDirectoryForFile(destination))
    {
        OSG_INFO << "Can't create directory for file '" << destination << "'. Copy may fail creating the file." << std::endl;
    }

    // Open destination file
    osgDB::ofstream fout(destination.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
    if (!fout)
    {
        OSG_NOTICE << "copyFile(): Can't write destination file: " << destination << std::endl;
        return FileOpResult::DESTINATION_NOT_OPENED;
    }

    // Copy file
    const unsigned int BUFFER_SIZE = 10240;
    osgDB::ifstream::char_type buffer[BUFFER_SIZE];
    for(; fin.good() && fout.good() && !fin.eof(); )
    {
        fin.read(buffer, BUFFER_SIZE);
        fout.write(buffer, fin.gcount());
    }

    if (!fout.good())
    {
        OSG_NOTICE << "copyFile(): Error writing destination file: " << destination << std::endl;
        return FileOpResult::WRITE_ERROR;
    }

    if (!fin.eof())
    {
        OSG_NOTICE << "copyFile(): Error reading source file: " << source << std::endl;
        return FileOpResult::READ_ERROR;
    }

    return FileOpResult::OK;
}


bool osgDB::containsCurrentWorkingDirectoryReference(const FilePathList& paths)
{
    const std::string cwd(".");
    for(FilePathList::const_iterator itr = paths.begin();
        itr != paths.end();
        ++itr)
    {
        if (itr->empty()) return true;
        if (*itr==cwd) return true;
    }
    return false;
}

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

        appendInstallationLibraryFilePaths(filepath);
    }


#elif defined(__CYGWIN__)

    void osgDB::appendPlatformSpecificLibraryFilePaths(FilePathList& filepath)
    {
        char* ptr;
        if ((ptr = getenv( "PATH" )))
        {
            convertStringPathIntoFilePathList(ptr,filepath);
        }

        appendInstallationLibraryFilePaths(filepath);

        convertStringPathIntoFilePathList("/usr/bin/:/usr/local/bin/",filepath);
    }

#elif defined(WIN32)

    void osgDB::appendPlatformSpecificLibraryFilePaths(FilePathList& filepath)
    {
        // See http://msdn2.microsoft.com/en-us/library/ms682586.aspx

        // Safe DLL search mode changes the DLL search order to search for
        // DLLs in the current directory after the system directories, instead
        // of right after the application's directory. According to the article
        // linked above, on Windows XP and Windows 2000, Safe DLL search mode
        // is disabled by default. However, it is a good idea to enable it. We
        // will search as if it was enabled.

        // So if SafeDllSearchMode is enabled, the search order is as follows:

        //   1. The directory from which the application loaded.
        DWORD retval = 0;
        const DWORD size = MAX_PATH;
        filenamechar path[size];
        retval = OSGDB_WINDOWS_FUNCT(GetModuleFileName)(NULL, path, size);
        if (retval != 0 && retval < size)
        {
            filenamestring pathstr(path);
            filenamestring executableDir(pathstr, 0,
                                      pathstr.find_last_of(OSGDB_FILENAME_TEXT("\\/")));
            convertStringPathIntoFilePathList(OSGDB_FILENAME_TO_STRING(executableDir), filepath);
        }
        else
        {
            OSG_WARN << "Could not get application directory "
                "using Win32 API. It will not be searched." << std::endl;
        }

        //   2. The directory that the dll that contains this function is in.
        // For static builds, this will be the executable directory.

        #if defined(_MSC_VER)
            // Requires use of the GetModuleHandleEx() function which is available only on Windows XP or higher.
            // In order to allow execution on older versions, we load the function dynamically from the library and
            // use it only if it's available.
            OSGDB_WINDOWS_FUNCT(PGET_MODULE_HANDLE_EX) pGetModuleHandleEx = reinterpret_cast<OSGDB_WINDOWS_FUNCT(PGET_MODULE_HANDLE_EX)>
                (GetProcAddress( GetModuleHandleA("kernel32.dll"), OSGDB_WINDOWS_FUNCT_STRING(GetModuleHandleEx)));
            if( pGetModuleHandleEx )
            {
                HMODULE thisModule = 0;
                static filenamechar static_variable = 0;    // Variable that is located in DLL address space.

                if( pGetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, &static_variable, &thisModule) )
                {
                    retval = OSGDB_WINDOWS_FUNCT(GetModuleFileName)(thisModule, path, size);
                    if (retval != 0 && retval < size)
                    {
                        filenamestring pathstr(path);
                        filenamestring dllDir(pathstr, 0,
                                                pathstr.find_last_of(OSGDB_FILENAME_TEXT("\\/")));
                        convertStringPathIntoFilePathList(OSGDB_FILENAME_TO_STRING(dllDir), filepath);
                    }
                    else
                    {
                        OSG_WARN << "Could not get dll directory "
                            "using Win32 API. It will not be searched." << std::endl;
                    }
                    FreeLibrary(thisModule);
                }
                else
                {
                    OSG_WARN << "Could not get dll module handle "
                        "using Win32 API. Dll directory will not be searched." << std::endl;
                }
            }
        #endif

        //   3. The system directory. Use the GetSystemDirectory function to
        //      get the path of this directory.
        filenamechar systemDir[(UINT)size];
        retval = OSGDB_WINDOWS_FUNCT(GetSystemDirectory)(systemDir, (UINT)size);

        if (retval != 0 && retval < size)
        {
            convertStringPathIntoFilePathList(OSGDB_FILENAME_TO_STRING(systemDir),
                                              filepath);
        }
        else
        {
            OSG_WARN << "Could not get system directory using "
                "Win32 API, using default directory." << std::endl;
            convertStringPathIntoFilePathList("C:\\Windows\\System32",
                                              filepath);
        }

        //   4. The 16-bit system directory. There is no function that obtains
        //      the path of this directory, but it is searched.
        //   5. The Windows directory. Use the GetWindowsDirectory function to
        //      get the path of this directory.
        filenamechar windowsDir[(UINT)size];
        retval = OSGDB_WINDOWS_FUNCT(GetWindowsDirectory)(windowsDir, (UINT)size);
        if (retval != 0 && retval < size)
        {
            convertStringPathIntoFilePathList(std::string(OSGDB_FILENAME_TO_STRING(windowsDir)) +
                                              "\\System", filepath);
            convertStringPathIntoFilePathList(OSGDB_FILENAME_TO_STRING(windowsDir),
                                              filepath);
        }
        else
        {
            OSG_WARN << "Could not get Windows directory using "
                "Win32 API, using default directory." << std::endl;
            convertStringPathIntoFilePathList("C:\\Windows", filepath);
            convertStringPathIntoFilePathList("C:\\Windows\\System", filepath);
        }


        //   6. The current directory.
        convertStringPathIntoFilePathList(".", filepath);

        //   7. The directories that are listed in the PATH environment
        //      variable. Note that this does not include the per-application
        //      path specified by the App Paths registry key.
        filenamechar* ptr;
#ifdef OSG_USE_UTF8_FILENAME
        if ((ptr = _wgetenv(OSGDB_FILENAME_TEXT("PATH"))))
#else
        if ((ptr = getenv("PATH")))
#endif
        {
            // Note that on any sane Windows system, some of the paths above
            // will also be on the PATH (the values gotten in systemDir and
            // windowsDir), but the DLL search goes sequentially and stops
            // when a DLL is found, so I see no point in removing duplicates.
            convertStringPathIntoFilePathList(OSGDB_FILENAME_TO_STRING(ptr), filepath);
        }

        appendInstallationLibraryFilePaths(filepath);
    }

#elif defined(__APPLE__)
#if (TARGET_OS_IPHONE) || (MAC_OS_X_VERSION_MIN_REQUIRED >= 1080)
    #define COMPILE_COCOA_VERSION
#else
     #define COMPILE_CARBON_VERSION
#endif
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
    // The Carbon version is noticeably longer.
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

        appendInstallationLibraryFilePaths(filepath);
        
        // Since this is currently the only Objective-C code in the
        // library, we need an autoreleasepool for obj-c memory management.
        // If more Obj-C is added, we might move this pool to another
        // location so it can be shared. Pools seem to be stackable,
        // so I don't think there will be a problem if multiple pools
        // exist at a time.
        NSAutoreleasePool* mypool = [[NSAutoreleasePool alloc] init];

        NSString* myBundlePlugInPath;

        // This will grab the "official" bundle plug in path.
        // It will be YourProgram.app/Contents/PlugIns (for App bundles)
        // or YourProgram/PlugIns (for Unix executables)
        myBundlePlugInPath = [[NSBundle mainBundle] builtInPlugInsPath];

        // Since Obj-C and C++ objects don't understand each other,
        // the Obj-C strings must be converted down to C strings so
        // C++ can make them into C++ strings.
        filepath.push_back( [myBundlePlugInPath UTF8String] );
        
        // add all application-support-folders
        NSArray *systemSearchPaths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSAllDomainsMask, YES);

        NSFileManager *fileManager = [NSFileManager defaultManager];

        for (NSString *systemPath in systemSearchPaths) {
            NSString *systemPluginsPath = [systemPath stringByAppendingPathComponent:@"OpenSceneGraph/PlugIns"];
            if ([fileManager fileExistsAtPath:systemPluginsPath]) {
                filepath.push_back( [systemPluginsPath UTF8String] );
            }
        }

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

        appendInstallationLibraryFilePaths(filepath);

        const std::string OSG_PLUGIN_PATH("/OpenSceneGraph/PlugIns");
        CFURLRef  url;
        CFBundleRef myBundle;
        FSRef     f;
        OSErr    errCode;

        // Start with the Bundle PlugIns directory.

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
            OSG_NOTIFY( osg::DEBUG_INFO ) << "Couldn't find the Application Bundle" << std::endl;
        }

        // Next, check the User's Application Support folder
        int domains[3] = { kUserDomain, kLocalDomain, kNetworkDomain };
        
        for(unsigned int domain_ndx = 0; domain_ndx < 3; ++domain_ndx)
        {
            errCode = FSFindFolder( domains[domain_ndx], kApplicationSupportFolderType, kDontCreateFolder, &f );
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
                    OSG_NOTIFY( osg::DEBUG_INFO ) << "Couldn't create CFURLRef for application support Path at ndx " << domain_ndx << std::endl;

                url = NULL;
            }
            else
            {
                OSG_NOTIFY( osg::DEBUG_INFO ) << "Couldn't find the User's Application Support Path at ndx "  << domain_ndx << std::endl;
            }
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

        appendInstallationLibraryFilePaths(filepath);
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

        appendInstallationLibraryFilePaths(filepath);

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
            OSG_NOTIFY( osg::DEBUG_INFO ) << "Couldn't find the Application Bundle." << std::endl;
        }
    }
#else
    void osgDB::appendPlatformSpecificResourceFilePaths(FilePathList& /*filepath*/)
    {
    }
#endif


