#ifndef WIN32
#include <unistd.h>
#include <dlfcn.h>
#else
#include <Io.h>
#include <Windows.h>
#include <Winbase.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "osg/Notify"
#include "osg/OSG"
#include "osg/Node"
#include "osg/Geode"
#include "osg/Group"
#include "osg/Input"
#include "osg/Output"
#include "osg/Registry"

using namespace osg;

#ifdef WIN32
char *PathDelimitor = ";";
static const char *s_default_file_path = ".;D:/OpenSceneGraph/Data;D:/OpenSceneGraph/Data/Images;D:/OpenSceneGraph/lib";
//static char *s_filePath = (char *)s_default_file_path;
static char *s_filePath = ".;";
#else
char *PathDelimitor = ":";
static const char *s_default_file_path = ".:";
#ifdef __sgi
static const char *s_default_dso_path = "/usr/lib32/osgPlugins/";
#else
static const char *s_default_dso_path = "/usr/lib/osgPlugins/";
#endif
static char *s_filePath = ".:";
//static char *s_filePath = s_default_file_path;
#endif


#include <vector>
#include <algorithm>

using std::vector;
using std::find_if;

void osg::Init( void )
{
    char *ptr;
    if( (ptr = getenv( "OSGFILEPATH" ))  )
    {
        notify(DEBUG) << "osg::Init("<<ptr<<")"<<endl;
        SetFilePath( ptr );
    }
    else
    {
        notify(DEBUG) << "osg::Init(NULL)"<<endl;
    }
}


void osg::SetFilePath( const char *_path )
{
    notify(DEBUG) << "In osg::SetFilePath("<<s_filePath<<")"<<endl;

    if( s_filePath != s_default_file_path )
          delete s_filePath;

    s_filePath = strdup( _path );

    notify(DEBUG) << "Out osg::SetFilePath("<<s_filePath<<")"<<endl;
}


static char *FindFileInPath( const char *_file, const char * filePath )
{
    char pathbuff[1024];
    char *tptr, *tmppath;
    char *path = 0L;

    notify(DEBUG) << "FindFileInPath() : trying " << _file << " ...\n";
#ifdef WIN32
    if( _access( _file, 4 ) == 0 ) return (char *)_file;
#else
    if( access( _file, F_OK ) == 0 ) return (char *)_file;
#endif

    tptr    = strdup( filePath );
    tmppath = strtok(  tptr, PathDelimitor );

    do
    {
        sprintf( pathbuff, "%s/%s", tmppath, _file );
    notify(DEBUG) << "FindFileInPath() : trying " << pathbuff << " ...\n";
#ifdef WIN32
        if( _access( pathbuff, 4 ) == 0 ) break;
#else
        if( access( pathbuff, F_OK ) == 0 ) break;
#endif
    } while( (tmppath = strtok( 0, PathDelimitor )) );

    if( tmppath != (char *)0L )
        path = strdup( pathbuff );

    ::free(tptr);

    notify( DEBUG ) << "FindFileInPath() : returning " << path << "\n";

    return path;
}


char *osg::FindFile( const char *file )
{
    return FindFileInPath( file, s_filePath );
}

/*

Order of precedence for 

  ./                  
  OSG_LD_LIBRARY_PATH 
  s_default_dso_path
  LD_LIBRARY*_PATH
 */

char *osg::findDSO( const char *name )
{
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

#ifdef __sgi // [

// bloody mess see rld(1) man page
#   if (_MIPS_SIM == _MIPS_SIM_ABI32) // [

    if( (ptr = getenv( "LD_LIBRARY_PATH" )))
    {
        strcat( path, PathDelimitor );
        strcat( path, ptr );
    }

#   elif (_MIPS_SIM == _MIPS_SIM_NABI32) // ][

    if( !(ptr = getenv( "LD_LIBRARYN32_PATH" )))
        ptr = getenv( "LD_LIBRARY_PATH" );

    if( ptr )
    {
        strcat( path, PathDelimitor );
        strcat( path, ptr );
    }

#   elif (_MIPS_SIM == _MIPS_SIM_ABI64) // ][

    if( !(ptr = getenv( "LD_LIBRARYN32_PATH" )))
        ptr = getenv( "LD_LIBRARY_PATH" );

    if( ptr )
    {
        strcat( path, PathDelimitor );
        strcat( path, ptr );
    }
#   endif // ]

#else // ][

#	ifdef WIN32 // [
    if ((ptr = getenv( "PATH" ))) 
    {
        notify(DEBUG) << "PATH = "<<ptr<<endl;
        strcat( path, PathDelimitor );
        strcat( path, ptr );
    }
#	else // ][
    if( (ptr = getenv( "LD_LIBRARY_PATH" )))
    {
        strcat( path, PathDelimitor );
        strcat( path, ptr );
    }
#endif // ]
#endif // ]

#ifndef WIN32

//     #ifdef __sgi
//     strcat( path, PathDelimitor );
//     strcat( path, "/usr/lib32/osgPlugins" );
//     #else
//     strcat( path, PathDelimitor );
//     strcat( path, "/usr/lib/osgPlugins" );
//     #endif

#endif

    strcat( path ,PathDelimitor );
    strcat( path, s_filePath );

    char* fileFound = NULL;
    fileFound = FindFileInPath( name , path );
    if (fileFound) return fileFound;
    
    // now try prepending the filename with "osgPlugins/"
    char* prependosgPlugins = new char[strlen(name)+12];
    strcpy(prependosgPlugins,"osgPlugins/");
    strcat(prependosgPlugins,name);
    
    fileFound = FindFileInPath( prependosgPlugins , path );    
    
    delete prependosgPlugins;
    
    return fileFound;
}


