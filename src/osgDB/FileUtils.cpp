#ifdef TARGET_API_MAC_CARBON
    #include "FileUtils_Mac.cpp"
#else

// implementations for Windows and all Unix's except Mac when TARGET_API_MAC_CARBON defined.    

#include <osg/Notify>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#if defined(WIN32) && !defined(__CYGWIN__)
    #include <Io.h>
    #include <Windows.h>
    #include <Winbase.h>
    // set up for windows so acts just like unix access().
    #define F_OK 4
#else
    // unix.
    #include <unistd.h>
    #include <dlfcn.h>
#endif

bool osgDB::fileExists(const std::string& filename)
{
    return access( filename.c_str(), F_OK ) == 0;
}

std::string osgDB::findFileInPath(const std::string& filename, const FilePathList& filepath)
{
    if (filename.empty()) return filename;

    if(fileExists(filename)) return filename;

    for(FilePathList::const_iterator itr=filepath.begin();
        itr!=filepath.end();
        ++itr)
    {
        std::string path = *itr + '/'+ filename;
        osg::notify(osg::DEBUG_INFO) << "FindFileInPath() : trying " << path << " ...\n";
        if(fileExists(path)) return path;
    }

    return std::string();
}

std::string osgDB::findDataFile(const std::string& filename)
{
    if (filename.empty()) return filename;

    const FilePathList& filepath = Registry::instance()->getDataFilePathList();

    std::string fileFound = findFileInPath(filename, filepath);
    if (!fileFound.empty()) return fileFound;

    // if a directory is included in the filename, get just the (simple) filename itself and try that
    std::string simpleFileName = getSimpleFileName(filename);
    if (simpleFileName!=filename)
    {
        std::string fileFound = findFileInPath(simpleFileName, filepath);
        if (!fileFound.empty()) return fileFound;
    }

    // return empty string.
    return std::string();
}

std::string osgDB::findLibraryFile(const std::string& filename)
{
    if (filename.empty()) return filename;

    const FilePathList& filepath = Registry::instance()->getLibraryFilePathList();

    std::string fileFound = findFileInPath(filename, filepath);
    if (!fileFound.empty()) return fileFound;

    // if a directory is included in the filename, get just the (simple) filename itself and try that
    std::string simpleFileName = getSimpleFileName(filename);
    if (simpleFileName!=filename)
    {
        std::string fileFound = findFileInPath(simpleFileName, filepath);
        if (!fileFound.empty()) return fileFound;
    }

    // failed with direct paths,
    // now try prepending the filename with "osgPlugins/"
    return findFileInPath("osgPlugins/"+simpleFileName,filepath);
}

std::string osgDB::findFileInDirectory(const std::string& fileName,const std::string& dirName,bool caseInsensitive)
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
        if ((caseInsensitive && osgDB::equalCaseInsensitive(fileName,*itr)) ||
            (fileName==*itr))
        {
            if (!needDirectoryName) return *itr;
            else if (needFollowingBackslash) return dirName+'/'+*itr;
            else return dirName+*itr;
        }
    }
    return "";
}

#if defined(WIN32) && !defined(__CYGWIN)

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
