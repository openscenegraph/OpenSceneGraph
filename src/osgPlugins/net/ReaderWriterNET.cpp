#include <iostream>
#include <fstream>
#include <sstream>

#include <osg/Notify>

#include <osgDB/Input>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osg/MatrixTransform>
#include <osg/Group>
#include "sockinet.h"
#include "makeDir.h"

 /*
  *  Semantics:
  *    Two methods for using the .net loader.
  *      1) Add a hostname prefix and a '.net' suffix on a model when passing 
  *          to osgDB::readNodeFile() 
  *          e.g:    osgDB::readNodeFile( "openscenegraph.org:cow.osg.net" );
  *
  *     2) Explicitely load the .net plugin and pass the plugin options including
  *           hostname=<hostname>
  *
  *    Method #1 takes precedence.  SO, if the hostname option is passed the
  *    plugin, but the name also contains a hostname prefix, the hostname
  *    prefix on the file name will override the option
  *
  *  Plugin options:
  *           hostname=<hostname>     - Specify the host where the data file is to 
  *                                     be fetched from.
  *
  *           prefix=<prefix>         - Specify a server directory to prefix the 
  *                                     file name with.
  *            
  *           local_cache_dir=<dir>   - Specify a directory in which to cache files
  *                                     on the local machine once they've been fetched.  
  *                                     This directory is also searched before fetching
  *                                     the file from the server when Read mode is 
  *                                     enabled on the cache.
  *
  *           cache_mode=<mode>       - Set the mode for the local cache if local_cache
  *                                     was specified.  If local_cache was not specified
  *                                     this directive is ignored.  <mode> may
  *                                     be specified with ReadOnly, WriteOnly, or
  *                                     ReadWrite.  Behavior for the different modes is
  *                                     defined as:
  *
  *                                       ReadOnly   - When retrieving files, cache is 
  *                                                    searched first, and if the file is
  *                                                    not present, it is fetched from the
  *                                                    server.  If it is fetched from the
  *                                                    server it is not stored in local cache
  *
  *                                       WriteOnly  - When retrieving files, cache is not
  *                                                    searched, file is always retrieved
  *                                                    from the server and always written to
  *                                                    cache.
  *
  *                                       ReadWrite  - (the default).  When retrieving files
  *                                                    cache is searched first, if file is
  *                                                    not present in cache, it is fetched from
  *                                                    the server.  If fetched, it is written
  *                                                    to cache.
  *
  */


class NetReader : public osgDB::ReaderWriter
{
    public:
        NetReader() {}
                                                                                            
        virtual const char* className() { return "HTTP Protocol Model Reader"; }
                                                                                            
        virtual bool acceptsExtension(const std::string& extension)
        {
            return osgDB::equalCaseInsensitive(extension,"net");
        }
                                                                                            
        virtual ReadResult readObject(const std::string& fileName, const Options* opt)
        { return readNode(fileName,opt); }
                                                                                            
        virtual ReadResult readNode(const std::string& inFileName, const Options *options )
        {
            std::string hostname;
            std::string serverPrefix;
            std::string localCacheDir;
            int port = 80;

            enum CacheMode {
                Read      = 1,
                Write     = 2,
                ReadWrite = 3
            };

            CacheMode cacheMode = ReadWrite;

            if (options)
            {
                std::istringstream iss(options->getOptionString());
                std::string opt;
                while (iss >> opt) 
                {
                    int index = opt.find( "=" );
                    if( opt.substr( 0, index ) == "hostname" ||
                        opt.substr( 0, index ) == "HOSTNAME" )
                    {
                        hostname = opt.substr( index+1 );
                    }
                    else if( opt.substr( 0, index ) == "port" ||
                             opt.substr( 0, index ) == "PORT" )
                    {
                        port = atoi( opt.substr(index+1).c_str() );
                    }
                    else if( opt.substr( 0, index ) == "server_prefix" ||
                             opt.substr( 0, index ) == "SERVER_PREFIX" ||
                             opt.substr( 0, index ) == "prefix" ||
                             opt.substr( 0, index ) == "PREFIX" )
                    {
                        serverPrefix = opt.substr(index+1);
                    }
                    else if( opt.substr( 0, index ) == "local_cache_dir" ||
                             opt.substr( 0, index ) == "LOCAL_CACHE_DIR" )
                    {
                        localCacheDir = opt.substr(index+1);
                    }
                    else if( opt.substr( 0, index ) == "cache_mode" ||
                             opt.substr( 0, index ) == "CACHE_MODE" )
                    {
                        if( opt.substr(index+1) == "ReadOnly" )
                            cacheMode = Read;
                        else if(  opt.substr(index+1) == "WriteOnly" )
                            cacheMode = Write;
                        else if(  opt.substr(index+1) == "ReadWrite" )
                            cacheMode = ReadWrite;
                        else
                            osg::notify(osg::WARN) << 
                                "NET plug-in warning:  cache_mode " << opt.substr(index+1) << 
                                " not understood.  Defaulting to ReadWrite." << std::endl;
                    }
                }
            }

            ReadResult readResult = ReadResult::FILE_NOT_HANDLED;

            /* * we accept all extensions
            std::string ext = osgDB::getFileExtension(inFileName);
            if (!acceptsExtension(ext))
                return ReadResult::FILE_NOT_HANDLED;
                */

            std::string fileName;
            int index = inFileName.find(":");
            // If we haven't been given a hostname as an option
            // and it hasn't been prefixed to the name, we fail
            if( index != -1 )
            {
                hostname = inFileName.substr( 0, index);
                // need to strip the inFileName of the hostname prefix
                fileName = inFileName.substr( index+1 );
            }
            else
            {
                if( hostname.empty() )
                    return ReadResult::FILE_NOT_HANDLED;
                else
                    fileName = inFileName;
            }

            // Let's also strip the possible .net extension
            if( osgDB::getFileExtension( fileName ) == "net" )
            {
                int rindex = fileName.rfind( "." );
                fileName = fileName.substr( 0, rindex );
            }

            if( !serverPrefix.empty() )
                fileName = serverPrefix + '/' + fileName;

            // Invoke the reader corresponding to the extension
            osgDB::ReaderWriter *reader = 
                    osgDB::Registry::instance()->getReaderWriterForExtension( osgDB::getFileExtension(fileName));
            if( reader == 0L )
                return ReadResult::FILE_NOT_HANDLED;

            // Before we go to the network, lets see if it is in local cache, if cache
            // was specified and Read bit is set
            if( !localCacheDir.empty() && (cacheMode & Read) )
            {
                std::string cacheFile = localCacheDir + '/' + fileName;
                if( osgDB::fileExists( cacheFile ))
                {
                    std::ifstream  in(cacheFile.c_str());
                    readResult = reader->readNode( in );
                    in.close();
                    osg::notify(osg::DEBUG_INFO) << "osgPlugin .net: " << fileName << 
                                         " fetched from local cache." << std::endl;
                    return readResult;
                }
            }

            // Fetch from the network
            iosockinet  sio (sockbuf::sock_stream);
            try {
                sio->connect( hostname.c_str(), port );
            }
            catch( sockerr e )
            {
                osg::notify(osg::WARN) << "osgPlugin .net reader: Unable to connect to host " << hostname << std::endl;
                return ReadResult::FILE_NOT_FOUND;
            }

            sio << "GET /" << fileName << " HTTP/1.1\n" << "Host:\n\n";
            sio.flush();
                                                                                                           
            char linebuff[256];
            do
            {
                sio.getline( linebuff, sizeof( linebuff ));

                std::istringstream iss(linebuff);
                std::string directive;
                iss >> directive;
                if( directive.substr(0,4) == "HTTP" )
                {
                    iss >> directive;
                    // Code 200.  We be ok.
                    if( directive == "200" )
                        ;
                    // Code 400 Bad Request
                    else if( directive == "400" )
                    {
                        osg::notify(osg::WARN) << 
                            "osgPluing .net: http server response 400 - Bad Request" << std::endl;
                        return ReadResult::FILE_NOT_FOUND;
                    }
                    // Code 401 Bad Request
                    else if( directive == "401" )
                    {
                        osg::notify(osg::WARN) << 
                            "osgPluing .net: http server response 401 - Unauthorized Access" << std::endl;
                        return ReadResult::FILE_NOT_FOUND;
                    }
                    // Code 403 Bad Request
                    else if( directive == "403" )
                    {
                        osg::notify(osg::WARN) << 
                            "osgPluing .net: http server response 403 - Access Forbidden" << std::endl;
                        return ReadResult::FILE_NOT_FOUND;
                    }
                    // Code 404 File not found
                    else if( directive == "404" )
                    {
                        osg::notify(osg::WARN) << 
                            "osgPluing .net: http server response 404 - File Not Found" << std::endl;
                        return ReadResult::FILE_NOT_FOUND;
                    }
                    // Code 405 Method not allowed
                    else if( directive == "405" )
                    {
                        osg::notify(osg::WARN) << 
                            "osgPluing .net: http server response 405 - Method Not Allowed" << std::endl;
                        return ReadResult::FILE_NOT_FOUND;
                    }
                    // There's more....
                }

            } while( linebuff[0] != '\r' );

            /*
            // Invoke the reader corresponding to the extension
            osgDB::ReaderWriter *reader = 
                    osgDB::Registry::instance()->getReaderWriterForExtension( osgDB::getFileExtension(fileName));
                    */

            osg::notify(osg::DEBUG_INFO) << "osgPlugin .net: " << fileName << 
                                         " fetched from server." << std::endl;

            if( reader != 0L )
                readResult = reader->readNode( sio );

            if( !localCacheDir.empty() && cacheMode & Write )
            {
                std::string cacheFile = localCacheDir + '/' + fileName;
                if( TemporaryFileUtils::makeDirectory( cacheFile ) )
                {
                    osgDB::writeNodeFile( *(readResult.getNode()), cacheFile );
                    osg::notify(osg::DEBUG_INFO) << "osgPlugin .net: " << fileName << 
                                         " stored to local cache." << std::endl;
                }
            }

            return readResult;
        }
};

osgDB::RegisterReaderWriterProxy<NetReader> g_netReader_Proxy;

                


