/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield 
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

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/Registry>

#include <iostream>
#include <sstream>
#include <fstream>

#include <curl/curl.h>
#include <curl/types.h>

#include "ReaderWriterCURL.h"

using namespace osg_curl;

//
//  StreamObject
//    
EasyCurl::StreamObject::StreamObject(std::ostream* stream1, const std::string& cacheFileName):
    _stream1(stream1),
    _cacheFileName(cacheFileName)
{
    _foutOpened = false;
}

void EasyCurl::StreamObject::write(const char* ptr, size_t realsize)
{
    if (_stream1) _stream1->write(ptr, realsize);

    if (!_cacheFileName.empty())
    {
        if (!_foutOpened)
        {
            osg::notify(osg::INFO)<<"Writing to cache: "<<_cacheFileName<<std::endl;
            _fout.open(_cacheFileName.c_str(), std::ios::out | std::ios::binary);
            _foutOpened = true;
        }

        if (_fout)
        {
            _fout.write(ptr, realsize);
        }
    }
}
    
size_t EasyCurl::StreamMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    StreamObject* sp = (StreamObject*)data;

    sp->write((const char*)ptr, realsize);

    return realsize;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  EasyCurl
//
EasyCurl::EasyCurl()
{
    osg::notify(osg::INFO)<<"EasyCurl::EasyCurl()"<<std::endl;

    _curl = curl_easy_init();

    curl_easy_setopt(_curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");            
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, StreamMemoryCallback);
}

EasyCurl::~EasyCurl()
{
    osg::notify(osg::INFO)<<"EasyCurl::~EasyCurl()"<<std::endl;

    if (_curl) curl_easy_cleanup(_curl);

    _curl = 0;
}


osgDB::ReaderWriter::ReadResult EasyCurl::read(const std::string& proxyAddress, const std::string& fileName, StreamObject& sp)
{
    if(!proxyAddress.empty())
    {
        osg::notify(osg::NOTICE)<<"Setting proxy: "<<proxyAddress<<std::endl;
        curl_easy_setopt(_curl, CURLOPT_PROXY, proxyAddress.c_str()); //Sets proxy address and port on libcurl
    }

    curl_easy_setopt(_curl, CURLOPT_URL, fileName.c_str());
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, (void *)&sp);

    CURLcode res = curl_easy_perform(_curl);

    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, (void *)0);

    if (res==0)
    {
        long code;
        if(!proxyAddress.empty())
        {
            curl_easy_getinfo(_curl, CURLINFO_HTTP_CONNECTCODE, &code);
        }
        else
        {
            curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &code);                    
        }

        if (code>=400)
        {
            osg::notify(osg::NOTICE)<<"Error: libcurl read error, file="<<fileName<<", error code = "<<code<<std::endl;
            return osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;
        }

        return osgDB::ReaderWriter::ReadResult::FILE_LOADED;

    }
    else
    {
        osg::notify(osg::NOTICE)<<"Error: libcurl read error, file="<<fileName<<" error = "<<curl_easy_strerror(res)<<std::endl;
        return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ReaderWriterCURL
//

ReaderWriterCURL::ReaderWriterCURL()
{
    supportsProtocol("http","Read from http port using libcurl.");
    supportsExtension("curl","Psuedo file extension, used to select curl plugin.");
    supportsExtension("*","Passes all read files to other plugins to handle actual model loading.");
    supportsOption("OSG_CURL_PROXY","Specify the http proxy.");
    supportsOption("OSG_CURL_PROXYPORT","Specify the http proxy oirt.");
}

ReaderWriterCURL::~ReaderWriterCURL()
{
    //osg::notify(osg::NOTICE)<<"ReaderWriterCURL::~ReaderWriterCURL()"<<std::endl;
}

osgDB::ReaderWriter::ReadResult ReaderWriterCURL::readFile(ObjectType objectType, osgDB::ReaderWriter* rw, std::istream& fin, const osgDB::ReaderWriter::Options *options) const
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

osgDB::ReaderWriter::ReadResult ReaderWriterCURL::readFile(ObjectType objectType, const std::string& fullFileName, const osgDB::ReaderWriter::Options *options) const
{


    if (!osgDB::containsServerAddress(fullFileName)) 
    {
        if (options && !options->getDatabasePathList().empty())
        {        
            if (osgDB::containsServerAddress(options->getDatabasePathList().front()))
            {
                std::string newFileName = options->getDatabasePathList().front() + "/" + fullFileName;

                return readFile(objectType, newFileName,options);
            }
        }

        return ReadResult::FILE_NOT_HANDLED;
    }

    osg::notify(osg::INFO)<<"ReaderWriterCURL::readFile("<<fullFileName<<")"<<std::endl;

    std::string proxyAddress, optProxy, optProxyPort;

    if (options)
    {
        std::istringstream iss(options->getOptionString());
        std::string opt;
        while (iss >> opt) 
        {
            int index = opt.find( "=" );
            if( opt.substr( 0, index ) == "OSG_CURL_PROXY" )
                optProxy = opt.substr( index+1 );
            else if( opt.substr( 0, index ) == "OSG_CURL_PROXYPORT" )
                optProxyPort = opt.substr( index+1 );
        }

        //Setting Proxy by OSG Options
        if(!optProxy.empty())
            if(!optProxyPort.empty())
                proxyAddress = optProxy + ":" + optProxyPort;
            else
                proxyAddress = optProxy + ":8080"; //Port not found, using default
    }

    std::string fileName;
    std::string ext = osgDB::getFileExtension(fullFileName);
    if (ext=="curl")
    {
        fileName = osgDB::getNameLessExtension(fullFileName);
    }
    else
    {
        fileName = fullFileName;
    }


    osgDB::ReaderWriter *reader = 
        osgDB::Registry::instance()->getReaderWriterForExtension( osgDB::getFileExtension(fileName));

    if (!reader)
    {
        osg::notify(osg::NOTICE)<<"Error: No ReaderWriter for file "<<fileName<<std::endl;
        return ReadResult::FILE_NOT_HANDLED;
    }

    const char* proxyEnvAddress = getenv("OSG_CURL_PROXY");
    if (proxyEnvAddress) //Env Proxy Settings
    {
        const char* proxyEnvPort = getenv("OSG_CURL_PROXYPORT"); //Searching Proxy Port on Env

        if(proxyEnvPort)
            proxyAddress = std::string(proxyEnvAddress) + ":" + std::string(proxyEnvPort);
        else
            proxyAddress = std::string(proxyEnvAddress) + ":8080"; //Default
    }
    
    std::stringstream buffer;

    EasyCurl::StreamObject sp(&buffer, std::string());

    ReadResult curlResult = getEasyCurl().read(proxyAddress, fileName, sp);

    if (curlResult.status()==ReadResult::FILE_LOADED)
    {
        osg::ref_ptr<Options> local_opt = const_cast<Options*>(options);
        if (!local_opt) local_opt = new Options;

        local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));

        ReadResult readResult = readFile(objectType, reader, buffer, local_opt.get() );

        local_opt->getDatabasePathList().pop_front();

        return readResult;
    }
    else
    {
        return curlResult;
    }
}

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(curl, ReaderWriterCURL)
