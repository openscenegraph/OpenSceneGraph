#include <iostream>
#include <sstream>

#include <osg/Notify>

#include <osgDB/Input>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/MatrixTransform>
#include <osg/Group>

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
            std::string server_prefix;
            int port = 80;

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
                             opt.substr( 0, index ) == "server_prefix" )
                    {
                        server_prefix = opt.substr(index+1);
                    }
                }
            }

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

            iosockinet  sio (sockbuf::sock_stream);
            try {
                sio->connect( hostname.c_str(), port );
            }
            catch( sockerr e )
            {
                osg::notify(osg::WARN) << "osgPlugin .net reader: Unable to connect to host " << hostname << std::endl;
                return ReadResult::FILE_NOT_FOUND;
            }

            if( !server_prefix.empty() )
                fileName = server_prefix + '/' + fileName;

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

            // Invoke the reader corresponding to the extension
            osgDB::ReaderWriter *reader = 
                    osgDB::Registry::instance()->getReaderWriterForExtension( osgDB::getFileExtension(fileName));

            ReadResult readResult = ReadResult::FILE_NOT_HANDLED;
            if( reader != 0L )
                readResult = reader->readNode( sio );

            return readResult;
        }
};

osgDB::RegisterReaderWriterProxy<NetReader> g_netReader_Proxy;

                


