#include <stdio.h>

#include <iostream>
#include <fstream>
#include <osgDB/Input>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/MatrixTransform>
#include <osg/Group>

#include "sockinet.h"

extern bool goGetTheFile(const std::string &host, const std::string & file, const std::string &saveDir );

class NetReader : public osgDB::ReaderWriter
{
    public:
        NetReader() {}
                                                                                            
        virtual const char* className() { return "Network Reader"; }
                                                                                            
        virtual bool acceptsExtension(const std::string& extension)
        {
            return osgDB::equalCaseInsensitive(extension,"net");
        }
                                                                                            
        virtual ReadResult readObject(const std::string& fileName, const Options* opt)
        { return readNode(fileName,opt); }
                                                                                            
        virtual ReadResult readNode(const std::string& fileName, const Options* )
        {
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext))
                return ReadResult::FILE_NOT_HANDLED;

            int index = fileName.find(":");
            if( index == -1 )
                return ReadResult::FILE_NOT_HANDLED;

            std::string host = fileName.substr( 0, index);
            int rindex = fileName.rfind( "." );
            std::string file = fileName.substr( index+1, rindex-index-1 );

            iosockinet  sio (sockbuf::sock_stream);
            sio->connect( host.c_str(), 80 );
                                                                                                           
            sio << "GET /" << file << " HTTP/1.1\n" << "Host:\n\n";
            sio.flush();
                                                                                                           
            char linebuff[256];
            do
            {
                sio.getline( linebuff, sizeof( linebuff ));
            } while( linebuff[0] != '\r' );
                                                                                                           
            osgDB::ReaderWriter *reader = 
                    osgDB::Registry::instance()->getReaderWriterForExtension( osgDB::getFileExtension(file));

            ReadResult readResult = ReadResult::FILE_NOT_HANDLED;
            if( reader == 0L )
                return ReadResult::FILE_NOT_HANDLED;
            else
                readResult = reader->readNode( sio );

            return readResult;
        }
};

osgDB::RegisterReaderWriterProxy<NetReader> g_netReader_Proxy;

                


