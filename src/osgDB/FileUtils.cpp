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
#if 0 // defined(__DARWIN_OSX__)
#include "FileUtils_Mac.cpp" // this is not functional yet -- fix!
#else

// currently this impl is for _all_ platforms, execpt as defined.
// the mac version will change soon to reflect the path scheme under osx, but
// for now, the above include is commented out, and the below code takes precedence.

#if defined(WIN32) && !defined(__CYGWIN__)
    #include <Io.h>
    #include <Windows.h>
    #include <Winbase.h>
    // set up for windows so acts just like unix access().
    #define F_OK 4
#else // unix
    #include <unistd.h>
#endif

#include <osg/Notify>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>


bool osgDB::fileExists(const std::string& filename)
{
    // hack for getting TXP plugin to utilise PagedLOD.
    if (getLowerCaseFileExtension(filename)=="txp") return true;

    return access( filename.c_str(), F_OK ) == 0;
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
