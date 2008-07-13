#include <iostream>
#include <fstream>
#include <sstream>

#include <osg/Notify>

#include <osgDB/Input>
#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/Archive>
#include <osg/MatrixTransform>
#include <osg/Group>
#include <osg/Timer>
#include "sockinet.h"

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
  *            proxy_host=<hostname>   - Specify the name of a Proxy host.  Proxies are not 
  *                                      used by default
  *
  *            proxy_port=<portnumber> - When using a Proxy host, specify the port.  The 
  *                                      internal variable defaults to 8080.
  *
  *      Environmental Variables:
  *
  *               OSG_PROXY_HOST        Specifies the name of a Proxy host.  Overrides options.
  *
  *               OSG_PROXY_PORT        When using a Proxy, sets the proxy port deliberately.  
  *                                     The internal variable defaults to 8080.
  */


class NetReader : public osgDB::ReaderWriter
{
    public:
        NetReader()
        {
            supportsProtocol("http","HTTP Protocol");
            supportsExtension("net","Psuedo loader extension for selecting NET plugin");
            supportsExtension("*","Passes all file loading onto other plugins");
        }
                                                                                            
        virtual const char* className() const { return "HTTP Protocol Model Reader"; }
                                                                                            
        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension,"net");
        }

        enum ObjectType
        {
            OBJECT,
            ARCHIVE,
            IMAGE,
            HEIGHTFIELD,
            NODE
        };
                                                                                            
        virtual ReadResult openArchive(const std::string& fileName,ArchiveStatus status, unsigned int , const Options* options) const
        {
            if (status!=READ) return ReadResult(ReadResult::FILE_NOT_HANDLED);
            else return readFile(ARCHIVE,fileName,options);
        }

        virtual ReadResult readObject(const std::string& fileName, const Options* options) const
        {
            return readFile(OBJECT,fileName,options);
        }
                                                                                            
        virtual ReadResult readImage(const std::string& fileName, const Options *options) const
        {
            return readFile(IMAGE,fileName,options);
        }

        virtual ReadResult readHeightField(const std::string& fileName, const Options *options) const
        {
            return readFile(HEIGHTFIELD,fileName,options);
        }

        virtual ReadResult readNode(const std::string& fileName, const Options *options) const
        {
            return readFile(NODE,fileName,options);
        }

        ReadResult readFile(ObjectType objectType, osgDB::ReaderWriter* rw, std::istream& fin, const Options *options) const
        {
            switch(objectType)
            {
            case(OBJECT): return rw->readObject(fin,options);
            case(ARCHIVE): return rw->openArchive(fin,options);
            case(IMAGE): return rw->readImage(fin,options);
            case(HEIGHTFIELD): return rw->readHeightField(fin,options);
            case(NODE): return rw->readNode(fin,options);
            default: break;
            }
            return ReadResult::FILE_NOT_HANDLED;
        }

        virtual ReadResult readFile(ObjectType objectType, const std::string& inFileName, const Options *options) const
        {
            osg::Timer_t start = osg::Timer::instance()->tick();

            osg::notify(osg::NOTICE) << "osgPlugin .net: start load" << inFileName << std::endl;


            std::string hostname;
            std::string serverPrefix;
            std::string localCacheDir;
            int port = 80;

            //CARLO: proxy modf 
            std::string proxyHost;
            int proxyPort = 8080;

            enum CacheMode {
                Read      = 1,
                Write     = 2,
                ReadWrite = 3
            };

            std::string cacheFile;
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
                    else if( opt.substr( 0, index ) == "proxy_hostname" ||
                             opt.substr( 0, index ) == "PROXY_HOSTNAME" ) //CARLO: proxy modf
                    {
                        proxyHost = opt.substr(index+1);
                        osg::notify(osg::WARN) << "proxy host " << proxyHost << std::endl;
                    }
                    else if( opt.substr( 0, index ) == "proxy_port" ||
                             opt.substr( 0, index ) == "PROXY_PORT" )
                    {
                        proxyPort = atoi( opt.substr(index+1).c_str() );
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
                        osg::notify(osg::INFO)<<"LOCAL_CACHE_DIR="<<localCacheDir<<std::endl;
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

            // Env variables should override plug-in options.
            {
                char * env_proxyHost = getenv("OSG_PROXY_HOST"); //Checking proxy environment variables 
                char * env_proxyPort = getenv("OSG_PROXY_PORT");

                if( env_proxyHost )
                {
                    proxyHost = std::string(env_proxyHost);
                    if( env_proxyPort )
                        proxyPort = atoi(std::string(env_proxyPort).c_str());
                }
            }

            ReadResult readResult = ReadResult::FILE_NOT_HANDLED;

            /* * we accept all extensions
            std::string ext = osgDB::getFileExtension(inFileName);
            if (!acceptsExtension(ext))
                return ReadResult::FILE_NOT_HANDLED;
                */


            std::string fileName;
            int index = inFileName.find_last_of(":");
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

            //Find the Port Number specified in URL
            index = hostname.find_last_of(":");
            if( index != -1 )
            {
                port = atoi( hostname.substr( index + 1 ).c_str() );
                hostname = hostname.substr( 0, index );            
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
                    readResult = readFile(objectType, reader, in, options );

                    in.close();
                    osg::notify(osg::NOTICE) << "osgPlugin .net: " << fileName << 
                                         " fetched from local cache." << std::endl;
                    return readResult;
                }
                else
                {
                    osg::notify(osg::NOTICE) << "osgPlugin .net: " << fileName << 
                                         " could not be found in local cache." << std::endl;
                }
            
            }

            // Fetch from the network
            osg::ref_ptr<iosockinet> sio = new iosockinet(sockbuf::sock_stream);

            std::string requestAdd;

            if(!proxyHost.empty())
            {
                try {
                    sio->rdbuf()->connect( proxyHost.c_str(), proxyPort );
                    osg::notify(osg::DEBUG_FP) << "osgPlugin .net reader: connected to Proxy " << proxyHost << " with Port " << proxyPort <<  std::endl;
                }
                catch( sockerr e )
                {
                    osg::notify(osg::WARN) << "osgPlugin .net reader: Unable to connect to Proxy " << proxyHost << std::endl;
                    return ReadResult::FILE_NOT_FOUND;
                }
                
                if(port != 80) //Default HTTP Port
                {
                    std::ostringstream portstream;
                    portstream << port << std::flush; 

                    requestAdd = std::string("http://") + hostname + std::string(":") + portstream.str() + std::string("/") + fileName;
                }
                else 
                    requestAdd = std::string("http://") + hostname + std::string("/") + fileName;
            }
            else
            {
                try {
                    sio->rdbuf()->connect( hostname.c_str(), port );
                    osg::notify(osg::DEBUG_FP) << "osgPlugin .net reader: connected to Hostname " << hostname << " with Port " << port <<  std::endl;
                }
                catch( sockerr e )
                {
                    osg::notify(osg::WARN) << "osgPlugin .net reader: Unable to connect to host " << hostname << std::endl;
                    return ReadResult::FILE_NOT_FOUND;
                }
                requestAdd = "/" + fileName; 
            }

            *sio << 
                    "GET " << requestAdd << " HTTP/1.1\n" 
                    "User-Agent: OSGNetPlugin/1.0\n"
                    "Accept: */*\n"
                    "Host: " << hostname << "\n"
                    "Connection: close\n"
                    "\n";    

            sio->flush();
                                                                                                           
            char linebuff[512];
            do
            {
                sio->getline( linebuff, sizeof( linebuff ));

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
                            "osgPlugin .net: http server response 400 - Bad Request : "<<requestAdd << std::endl;
                        return ReadResult::FILE_NOT_FOUND;
                    }
                    // Code 401 Bad Request
                    else if( directive == "401" )
                    {
                        osg::notify(osg::WARN) << 
                            "osgPlugin .net: http server response 401 - Unauthorized Access : "<<requestAdd << std::endl;
                        return ReadResult::FILE_NOT_FOUND;
                    }
                    // Code 403 Bad Request
                    else if( directive == "403" )
                    {
                        osg::notify(osg::WARN) << 
                            "osgPlugin .net: http server response 403 - Access forbidden : "<<requestAdd << std::endl;
                        return ReadResult::FILE_NOT_FOUND;
                    }
                    // Code 404 File not found
                    else if( directive == "404" )
                    {
                        osg::notify(osg::WARN) << 
                            "osgPlugin .net: http server response 404 - File not found : "<<requestAdd<< std::endl;
                        return ReadResult::FILE_NOT_FOUND;
                    }
                    // Code 405 Method not allowed
                    else if( directive == "405" )
                    {
                        osg::notify(osg::WARN) << 
                            "osgPlugin .net: http server response 405 - Method Not Allowed : " <<requestAdd<< std::endl;
                        return ReadResult::FILE_NOT_FOUND;
                    }
                    // There's more....
                }

            } while( linebuff[0] != '\r' );

            // code for setting up the database path so that any paged
            // databases can be automatically located. 
            osg::ref_ptr<Options> local_opt = const_cast<Options*>(options);
            if (!local_opt) local_opt = new Options;

            if (local_opt.valid() && local_opt->getDatabasePathList().empty())
            {
                local_opt->setDatabasePath(osgDB::getFilePath(inFileName));
            }
            
            


            if( reader != 0L )
                readResult = readFile(objectType, reader, *sio, local_opt.get() );

            double ms = osg::Timer::instance()->delta_m(start,osg::Timer::instance()->tick());

            osg::notify(osg::NOTICE) << "osgPlugin .net: " << fileName << 
                                         " fetched from server. in " << ms <<" ms"<< std::endl;

            if (objectType==ARCHIVE && readResult.validArchive())
            {
                // attach the socket istream to the archive to keep it alive.
                osgDB::Archive* archive = readResult.getArchive();
                archive->setUserData(sio.get());
            }

            if( !localCacheDir.empty() && cacheMode & Write )
            {
                std::string cacheFile = localCacheDir + '/' + fileName;
                if( osgDB::makeDirectoryForFile( cacheFile ) )
                {
 
                    switch(objectType)
                    {
                    case(OBJECT): osgDB::writeObjectFile( *(readResult.getObject()), cacheFile ); break;
                    case(IMAGE): osgDB::writeImageFile( *(readResult.getImage()), cacheFile ); break;
                    case(HEIGHTFIELD): osgDB::writeHeightFieldFile( *(readResult.getHeightField()), cacheFile ); break;
                    case(NODE): osgDB::writeNodeFile( *(readResult.getNode()), cacheFile );  break;
                    default: break;
                    }
 
                     
                    osg::notify(osg::NOTICE) << "osgPlugin .net: " << fileName << 
                                         " stored to local cache." << std::endl;
                }
            }

            return readResult;
        }
        
};

REGISTER_OSGPLUGIN(net, NetReader)
