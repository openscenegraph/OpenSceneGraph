//
// This should move to osgDB::FileUtils
//

#include <iostream>
#include <stdio.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>
#include <string>
#include <stack>
#include <osgDB/FileUtils>
#include "makeDir.h"

namespace TemporaryFileUtils {

bool makeDirectory( const std::string &path )
{
    char *cpath = new char[path.length()+1];
    strcpy( cpath, path.c_str());
    char *p = dirname(cpath);
    struct stat stbuf;

    if( stat( p, &stbuf ) == 0 )
    {
        if( S_ISDIR(stbuf.st_mode))
            return true;
        else
        {
            std::cerr << "TemporaryFileUtils::makeDirectory() - "<< p
                << " already exists and is not a directory!" << std::endl;
            return false;
        }
    }

    std::stack<std::string> paths;
    while( true )
    {
        if( p[0] == '.' )
            break;

        if( stat( p, &stbuf ) < 0 )
        {
            switch( errno )
            {
                case ENOENT:
                case ENOTDIR:
                    paths.push( std::string(p) );
                    break;

                default:
                    perror( p );
                    return false;
            }
        }
        p = dirname(p);
    } ;

    while( !paths.empty() )
    {
        std::string dir = paths.top();

        if( mkdir( dir.c_str(), 0755 )< 0 )
        {
            perror( dir.c_str() );
            return false;
        }

        paths.pop();
    }
    return true;
}

}
