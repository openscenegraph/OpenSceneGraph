#ifdef WIN32
#include <Io.h>
#include <Windows.h>
#include <Winbase.h>
#elif !defined(macintosh) // UNIX
#include <unistd.h>
#include <dlfcn.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <osg/Notify>
#include <osg/Node>
#include <osg/Geode>
#include <osg/Group>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

using namespace osg;
using namespace osgDB;

// follows is definition of strdup for compatibility under mac,
// However, I'd prefer to migrate all the findFindInPath tools to use
// std::string's and then be able to remove the following definition.
// My objective is to minimize the number of platform #ifdef's as they
// are source of potential bugs and developer confusion.
#ifdef macintosh
#ifndef strdup
inline static char* strdup(const char *src);

inline static char* strdup(const char *src)
{
	char *ret = (char *) std::malloc(std::strlen(src) +1);
	if(!ret) return NULL;
	
	return std::strcpy(ret, src);
}
#endif
#endif

#ifdef WIN32
char *PathDelimitor = ";";
static const char *s_default_file_path = ".;";
static const char *s_default_dso_path = "C:/Windows/System/;";
static char *s_filePath = ".;";
#elif macintosh
char *PathDelimitor = " ";
static const char *s_default_file_path = ":";
static const char *s_default_dso_path = ":";
static char *s_filePath = ":";
#elif __sgi
char *PathDelimitor = ":";
static const char *s_default_file_path = ".:";
static const char *s_default_dso_path = "/usr/lib32/:/usr/local/lib32/";
static char *s_filePath = ".:";
#else
char *PathDelimitor = ":";
static const char *s_default_file_path = ".:";
static const char *s_default_dso_path = "/usr/lib/:/usr/local/lib/:";
static char *s_filePath = ".:";
#endif

#if defined (WIN32)
#define F_OK 4
#endif

static bool s_filePathInitialized = false;

void osgDB::initFilePath( void )
{
    char *ptr;
    if( (ptr = getenv( "OSGFILEPATH" ))  )
    {
        notify(DEBUG_INFO) << "osgDB::Init("<<ptr<<")"<<std::endl;
        setFilePath( ptr );
    }
    else
    {
        notify(DEBUG_INFO) << "osgDB::Init(NULL)"<<std::endl;
    }
    s_filePathInitialized = true;
}


void osgDB::setFilePath( const char *_path )
{
    char buff[1024];

    notify(DEBUG_INFO) << "In osgDB::setFilePath("<<_path<<")"<<std::endl;

    buff[0] = 0;

    if( s_filePath != s_default_file_path )
    {
        strcpy( buff, s_filePath );
    }

    strcat( buff, PathDelimitor );
    strcat( buff, _path );

    s_filePath = strdup( buff );

    s_filePathInitialized = true;
}


const char* osgDB::getFilePath()
{
    return s_filePath;
}

char *osgDB::findFileInPath( const char *_file, const char * filePath )
{
#ifdef macintosh
    return (char *)_file;
#else



    char pathbuff[1024];
    char *tptr, *tmppath;
    char *path = 0L;

    notify(DEBUG_INFO) << "FindFileInPath() : trying " << _file << " ...\n";
    if( access( _file, F_OK ) == 0 ) 
    {
        return strdup(_file);
    }

    tptr    = strdup( filePath );
    tmppath = strtok(  tptr, PathDelimitor );

    do
    {
        sprintf( pathbuff, "%s/%s", tmppath, _file );
        notify(DEBUG_INFO) << "FindFileInPath() : trying " << pathbuff << " ...\n";
        if( access( pathbuff, F_OK ) == 0 ) break;
    } while( (tmppath = strtok( 0, PathDelimitor )) );

    if( tmppath != (char *)0L )
        path = strdup( pathbuff );

    ::free(tptr);

    if (path) notify( DEBUG_INFO ) << "FindFileInPath() : returning " << path << std::endl;
	else notify( DEBUG_INFO ) << "FindFileInPath() : returning NULL" << std::endl;

    return path;
#endif
}


char *osgDB::findFile( const char *file )
{
    if (!file) return NULL;
    
    if (!s_filePathInitialized) initFilePath();

    char* newFileName = findFileInPath( file, s_filePath );
    if (newFileName) return newFileName;
    

    // need to check here to see if file has a path on it.
    
    // now strip the file of an previous path if one exists.
    std::string simpleFileName = getSimpleFileName(file);
    newFileName = findFileInPath( simpleFileName.c_str(), s_filePath );
    return newFileName;
}

/*

Order of precedence for

Under UNIX.

  ./
  OSG_LD_LIBRARY_PATH
  s_default_dso_path
  LD_LIBRARY*_PATH

Under Windows

  ./
  OSG_LD_LIBRARY_PATH
  s_default_dso_path
  PATH

*/

char *osgDB::findDSO( const char *name )
{
#ifndef macintosh

    #ifdef __linux
    if( access( name, F_OK ) == 0 ) 
    {
        if (name[0]!='/')
        {
            char pathbuff[1024];
            sprintf( pathbuff,"./%s", name );
            return (char *)strdup(pathbuff);
        }
        else
        {
            return (char *)strdup(name);
        }
    }
    #endif

    char path[1024];
    char *ptr;

    strcpy( path, "./" );

    if((ptr = getenv( "OSG_LD_LIBRARY_PATH" )))
    {
        strcat( path, PathDelimitor );
        strcat( path, ptr );
    }

    strcat( path, PathDelimitor );
    strcat( path, s_default_dso_path );

    #ifdef __sgi

    // bloody mess see rld(1) man page
    #if (_MIPS_SIM == _MIPS_SIM_ABI32)

    if( (ptr = getenv( "LD_LIBRARY_PATH" )))
    {
        strcat( path, PathDelimitor );
        strcat( path, ptr );
    }

    #elif (_MIPS_SIM == _MIPS_SIM_NABI32)

    if( !(ptr = getenv( "LD_LIBRARYN32_PATH" )))
        ptr = getenv( "LD_LIBRARY_PATH" );

    if( ptr )
    {
        strcat( path, PathDelimitor );
        strcat( path, ptr );
    }

    #elif (_MIPS_SIM == _MIPS_SIM_ABI64)

    if( !(ptr = getenv( "LD_LIBRARYN32_PATH" )))
        ptr = getenv( "LD_LIBRARY_PATH" );

    if( ptr )
    {
        strcat( path, PathDelimitor );
        strcat( path, ptr );
    }
    #endif

#elif WIN32

    if ((ptr = getenv( "PATH" )))
    {
        notify(DEBUG_INFO) << "PATH = "<<ptr<<std::endl;
        strcat( path, PathDelimitor );
        strcat( path, ptr );
    }

    #else
    if( (ptr = getenv( "LD_LIBRARY_PATH" )))
    {
        strcat( path, PathDelimitor );
        strcat( path, ptr );
    }
    #endif

    // check existance of dso assembled direct paths.
    char* fileFound = NULL;
    fileFound = findFileInPath( name , path );
    if (fileFound) return fileFound;

    // failed with direct paths,
    // now try prepending the filename with "osgPlugins/"
    char* prependosgPlugins = new char[strlen(name)+12];
    strcpy(prependosgPlugins,"osgPlugins/");
    strcat(prependosgPlugins,name);

    fileFound = findFileInPath( prependosgPlugins , path );
    delete prependosgPlugins;

    return fileFound;
#else // defined macintosh
    return NULL;
#endif
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


#ifdef WIN32

#include <io.h>
#include <direct.h>

osgDB::DirectoryContents osgDB::getDirectoryContents(const std::string& dirName)
{
    osgDB::DirectoryContents contents;

    struct _finddata_t data;
    long handle = _findfirst((dirName + "\\*").c_str(), &data);
    if (handle != -1)
    {
        do
        {
            contents.push_back(data.name);
        }
        while (_findnext(handle, &data) != -1);

        _findclose(handle);
    }

    return contents;
}

#elif !defined(macintosh) // UNIX

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
#endif
