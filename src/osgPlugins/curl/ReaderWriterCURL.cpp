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

#include <string.h>

#include <curl/curl.h>

#if LIBCURL_VERSION_NUM < 0x071503
    // types.h has been removed in 7.21.7 so have to protect with version guard
    // it may be possible to remove it completely but can't yet work out when
    // types.h was deprecated so will assume it's still needed in older libcurl versions
    // that OSG users are using.
    #include <curl/types.h>
#endif

#include "ReaderWriterCURL.h"

using namespace osg_curl;


//
//  StreamObject
//
EasyCurl::StreamObject::StreamObject(std::ostream* outputStream, std::istream* inputStream, const std::string& cacheFileName):
    _outputStream(outputStream),
    _inputStream(inputStream),
    _cacheFileName(cacheFileName)
{
    _foutOpened = false;
}

void EasyCurl::StreamObject::write(const char* ptr, size_t realsize)
{
    if (_outputStream) _outputStream->write(ptr, realsize);

    if (!_cacheFileName.empty())
    {
        if (!_foutOpened)
        {
            OSG_INFO<<"Writing to cache: "<<_cacheFileName<<std::endl;
            _fout.open(_cacheFileName.c_str(), std::ios::out | std::ios::binary);
            _foutOpened = true;
        }

        if (_fout)
        {
            _fout.write(ptr, realsize);
        }
    }
}

size_t EasyCurl::StreamObject::read(char* ptr, size_t maxsize)
{
    if (!_inputStream) return 0;
    _inputStream->read(ptr, maxsize);
    size_t realsize = _inputStream->gcount();
    return realsize;
}

std::string EasyCurl::getResultMimeType(const StreamObject& sp) const
{
    return sp._resultMimeType;
}

std::string EasyCurl::getMimeTypeForExtension(const std::string& ext) const
{
    const osgDB::Registry::MimeTypeExtensionMap& mimeMap = osgDB::Registry::instance()->getMimeTypeExtensionMap();
    osgDB::Registry::MimeTypeExtensionMap::const_iterator i;
    for (i = mimeMap.begin(); i != mimeMap.end(); ++i)
    {
        if (ext == i->second) return i->first;
    }
    return "application/octet-stream"; // unknown mime type
}

std::string EasyCurl::getFileNameFromURL(const std::string& url)
{
    // If the URL has query parameter "filename", return its value,
    // otherwise just return url assuming it has a filename at the end.
    // Typically, uploading will require cooperation with a server side
    // script that requires parameters such as filename and/or session
    // and/or authentication information, so in general the filename
    // can not be assumed to be at the tail of the URL.
    std::string::size_type pos = url.find('?');
    if (pos == std::string::npos) return url;
    std::string params = url.substr(pos + 1);
    const char* filenameKey = "filename=";
    pos = params.find(filenameKey);
    if (pos == std::string::npos)
    {
        // No filename param, so just chop off parameters on the url.
        return url.substr(0, url.find('?'));
    }
    std::string fileName = params.substr(pos + strlen(filenameKey));
    pos = fileName.find("&");
    if (pos != std::string::npos)
    {
        // Chop off next url parameter
        fileName = fileName.substr(0, pos);
    }
    return fileName;
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
    OSG_INFO<<"EasyCurl::EasyCurl()"<<std::endl;

    _previousHttpAuthentication = 0;
    _connectTimeout = 0; // no timeout by default.
    _timeout = 0;

    _curl = curl_easy_init();

    curl_easy_setopt(_curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, StreamMemoryCallback);
    curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1L);
}

EasyCurl::~EasyCurl()
{
    OSG_INFO<<"EasyCurl::~EasyCurl()"<<std::endl;

    if (_curl) curl_easy_cleanup(_curl);

    _curl = 0;
}

osgDB::ReaderWriter::ReadResult EasyCurl::read(const std::string& proxyAddress, const std::string& fileName, StreamObject& sp, const osgDB::ReaderWriter::Options *options)
{
    setOptions(proxyAddress, fileName, sp, options);

    CURLcode responseCode = curl_easy_perform(_curl);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, (void *)0);

    return processResponse(responseCode, proxyAddress, fileName, sp);
}

osgDB::ReaderWriter::WriteResult EasyCurl::write(const std::string& proxyAddress, const std::string& fileName, StreamObject& sp, const osgDB::ReaderWriter::Options *options)
{
    setOptions(proxyAddress, fileName, sp, options);

    char* postedContent = NULL;

    // Copy data from istream into buffer.
    long contentLength = 0;
    const int bufferSize = 4096;
    while(true)
    {
        postedContent = (char *)realloc(postedContent, contentLength + bufferSize);
        size_t gotBytes = sp.read(postedContent + contentLength, bufferSize);
        if (gotBytes == 0) break;
        contentLength += gotBytes;
    };

    // Extract name and mime type of buffer to upload.
    std::string uploadFileName = getFileNameFromURL(fileName);
    std::string ext = osgDB::getLowerCaseFileExtension(uploadFileName);
    std::string mimeType = getMimeTypeForExtension(ext);

    // Construct "multipart/form-data" (RFC 1867) form elements for file upload.
    struct curl_httppost* post = NULL;
    struct curl_httppost* last = NULL;
    curl_formadd(&post, &last,
        CURLFORM_COPYNAME, "upload",
        CURLFORM_CONTENTTYPE, mimeType.c_str(),
        CURLFORM_BUFFER, uploadFileName.c_str(),
        CURLFORM_BUFFERPTR, postedContent,
        CURLFORM_BUFFERLENGTH, contentLength,
        CURLFORM_END);

    // Tell curl to use HTTP POST to send the form data.
    curl_easy_setopt(_curl, CURLOPT_HTTPPOST, post);

    CURLcode responseCode = curl_easy_perform(_curl);

    if (post) curl_formfree(post);
    if (postedContent) free(postedContent);
    curl_easy_setopt(_curl, CURLOPT_HTTPPOST, (void *)0);
    curl_easy_setopt(_curl, CURLOPT_HTTPGET, 1L);

    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, (void *)0);

    if (processResponse(responseCode, proxyAddress, fileName, sp).success())
    {
        osgDB::ReaderWriter::WriteResult result(osgDB::ReaderWriter::WriteResult::FILE_SAVED);
        std::stringstream* ss = dynamic_cast<std::stringstream*>(sp._outputStream);
        if (ss)
        {
            // Put the server response in the message part of the result object.
            result.message() = ss->str();
        }
        return result;
    }
    else
    {
        return osgDB::ReaderWriter::WriteResult::ERROR_IN_WRITING_FILE;
    }
}

void EasyCurl::setOptions(const std::string& proxyAddress, const std::string& fileName, StreamObject& sp, const osgDB::ReaderWriter::Options *options)
{
    const osgDB::AuthenticationMap* authenticationMap = (options && options->getAuthenticationMap()) ?
            options->getAuthenticationMap() :
            osgDB::Registry::instance()->getAuthenticationMap();

    // Set the timeout value here:
    // Note that this has an effect only in a connection phase.
    // WARNING: here we make the assumption that if someone starts using the timeouts settings
    // he will not try to disable them afterwards (a value must be provided or the previous value is used).
    if(_connectTimeout > 0)
        curl_easy_setopt(_curl, CURLOPT_CONNECTTIMEOUT, _connectTimeout);
    if(_timeout > 0)
        curl_easy_setopt(_curl, CURLOPT_TIMEOUT, _timeout);

    if(!proxyAddress.empty())
    {
        OSG_INFO<<"Setting proxy: "<<proxyAddress<<std::endl;
        curl_easy_setopt(_curl, CURLOPT_PROXY, proxyAddress.c_str()); //Sets proxy address and port on libcurl
    }

    const osgDB::AuthenticationDetails* details = authenticationMap ?
        authenticationMap->getAuthenticationDetails(fileName) :
        0;

    // configure/reset authentication if required.
    if (details)
    {
        const std::string colon(":");
        std::string password(details->username + colon + details->password);
        curl_easy_setopt(_curl, CURLOPT_USERPWD, password.c_str());
        _previousPassword = password;

        // use for https.
        // curl_easy_setopt(_curl, CURLOPT_KEYPASSWD, password.c_str());

#if LIBCURL_VERSION_NUM >= 0x070a07
        if (details->httpAuthentication != _previousHttpAuthentication)
        {
            curl_easy_setopt(_curl, CURLOPT_HTTPAUTH, details->httpAuthentication);
            _previousHttpAuthentication = details->httpAuthentication;
        }
#endif

    }
    else
    {
        if (!_previousPassword.empty())
        {
            curl_easy_setopt(_curl, CURLOPT_USERPWD, 0);
            _previousPassword.clear();
        }

#if LIBCURL_VERSION_NUM >= 0x070a07
        // need to reset if previously set.
        if (_previousHttpAuthentication!=0)
        {
            curl_easy_setopt(_curl, CURLOPT_HTTPAUTH, 0);
            _previousHttpAuthentication = 0;
        }
#endif
    }

    curl_easy_setopt(_curl, CURLOPT_URL, fileName.c_str());
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, (void *)&sp);
}

osgDB::ReaderWriter::ReadResult EasyCurl::processResponse(CURLcode res, const std::string& proxyAddress, const std::string& fileName, StreamObject& sp)
{
    if (res==0)
    {

#if LIBCURL_VERSION_NUM >= 0x070a07
        long code;
        if(!proxyAddress.empty())
        {
            curl_easy_getinfo(_curl, CURLINFO_HTTP_CONNECTCODE, &code);
        }
        else
        {
            curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &code);
        }

        //If the code is greater than 400, there was an error
        if (code >= 400)
        {
            osgDB::ReaderWriter::ReadResult::ReadStatus status;

            //Distinguish between a client error and a server error
            if (code < 500)
            {
                //A 400 level error indicates a client error
                status = osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;
            }
            else
            {
                //A 500 level error indicates a server error
                status = osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
            }

            osgDB::ReaderWriter::ReadResult rr(status);

            //Add the error code to the ReadResult
            std::stringstream message;
            message << "error code = " << code;

            rr.message() = message.str();

            return rr;
        }
#endif

        // Store the mime-type, if any. (Note: CURL manages the buffer returned by
        // this call.)
        char* ctbuf = NULL;
        if ( curl_easy_getinfo(_curl, CURLINFO_CONTENT_TYPE, &ctbuf) == 0 && ctbuf )
        {
            sp._resultMimeType = ctbuf;
        }


        return osgDB::ReaderWriter::ReadResult::FILE_LOADED;

    }
    else
    {

#if LIBCURL_VERSION_NUM >= 0x070c00
         OSG_NOTICE<<"Error: libcurl read error, file="<<fileName<<" error = "<<curl_easy_strerror(res)<<std::endl;
#else
         OSG_NOTICE<<"Error: libcurl read error, file="<<fileName<<" error no = "<<res<<std::endl;
#endif
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
    supportsProtocol("https","Read from https port using libcurl.");
    supportsProtocol("ftp","Read from ftp port using libcurl.");
    supportsProtocol("ftps","Read from ftps port using libcurl.");
    
    supportsExtension("curl","Psuedo file extension, used to select curl plugin.");
    supportsExtension("*","Passes all read files to other plugins to handle actual model loading.");
    supportsOption("OSG_CURL_PROXY","Specify the http proxy.");
    supportsOption("OSG_CURL_PROXYPORT","Specify the http proxy port.");
    supportsOption("OSG_CURL_CONNECTTIMEOUT","Specify the connection timeout duration in seconds [default = 0 = not set].");
    supportsOption("OSG_CURL_TIMEOUT","Specify the timeout duration of the whole transfer in seconds [default = 0 = not set].");
}

ReaderWriterCURL::~ReaderWriterCURL()
{
    //OSG_NOTICE<<"ReaderWriterCURL::~ReaderWriterCURL()"<<std::endl;
}

osgDB::ReaderWriter::WriteResult ReaderWriterCURL::writeFile(const osg::Object& obj, osgDB::ReaderWriter* rw, std::ostream& fout, const osgDB::ReaderWriter::Options *options) const
{
    const osg::HeightField* heightField = dynamic_cast<const osg::HeightField*>(&obj);
    if (heightField) return rw->writeHeightField(*heightField, fout, options);

    const osg::Node* node = dynamic_cast<const osg::Node*>(&obj);
    if (node) return rw->writeNode(*node, fout, options);

    const osg::Image* image = dynamic_cast<const osg::Image*>(&obj);
    if (image) return rw->writeImage(*image, fout, options);

    return rw->writeObject(obj, fout, options);
}

osgDB::ReaderWriter::WriteResult ReaderWriterCURL::writeFile(const osg::Object& obj, const std::string& fullFileName, const Options *options) const
{
    if (!osgDB::containsServerAddress(fullFileName))
    {
        return WriteResult::FILE_NOT_HANDLED;
    }

    std::stringstream requestBuffer; // Buffer to be filled then output via http request.
    std::stringstream responseBuffer; // Buffer to contain content of http response.

    // Serialize obj into an std::stringstream buffer which will be uploaded via HTTP post request.
    std::string fileName = EasyCurl::getFileNameFromURL(fullFileName);
    std::string ext = osgDB::getLowerCaseFileExtension(fileName);
    osgDB::ReaderWriter* writer = osgDB::Registry::instance()->getReaderWriterForExtension(ext);
    if (!writer) return WriteResult::FILE_NOT_HANDLED;
    osgDB::ReaderWriter::WriteResult result = writeFile(obj, writer, requestBuffer, options);
    if (!result.success()) return result;

    // Configure curl connection options.
    std::string proxyAddress;
    long connectTimeout = 0;
    long timeout = 0;
    getConnectionOptions(options, proxyAddress, connectTimeout, timeout);
    EasyCurl::StreamObject sp(&responseBuffer, &requestBuffer, std::string());
    EasyCurl& easyCurl = getEasyCurl();
    easyCurl.setConnectionTimeout(connectTimeout);
    easyCurl.setTimeout(timeout);

    // Output requestBuffer via curl, and return responseBuffer in message of result.
    return easyCurl.write(proxyAddress, fullFileName, sp, options);
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

void ReaderWriterCURL::getConnectionOptions(const osgDB::ReaderWriter::Options *options, std::string& proxyAddress, long& connectTimeout, long& timeout) const
{
    if (options)
    {
        std::istringstream iss(options->getOptionString());
        std::string opt, optProxy, optProxyPort;
        while (iss >> opt)
        {
            int index = opt.find( "=" );
            if( opt.substr( 0, index ) == "OSG_CURL_PROXY" )
                optProxy = opt.substr( index+1 );
            else if( opt.substr( 0, index ) == "OSG_CURL_PROXYPORT" )
                optProxyPort = opt.substr( index+1 );
            else if( opt.substr( 0, index ) == "OSG_CURL_CONNECTTIMEOUT" )
                connectTimeout = atol(opt.substr( index+1 ).c_str()); // this will return 0 in case of improper format.
            else if( opt.substr( 0, index ) == "OSG_CURL_TIMEOUT" )
                timeout = atol(opt.substr( index+1 ).c_str()); // this will return 0 in case of improper format.
        }

        //Setting Proxy by OSG Options
        if(!optProxy.empty())
        {
            if(!optProxyPort.empty())
                proxyAddress = optProxy + ":" + optProxyPort;
            else
                proxyAddress = optProxy + ":8080"; //Port not found, using default
        }
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

}

osgDB::ReaderWriter::ReadResult ReaderWriterCURL::readFile(ObjectType objectType, const std::string& fullFileName, const osgDB::ReaderWriter::Options *options) const
{
    std::string fileName(fullFileName);
    std::string ext = osgDB::getFileExtension(fullFileName);
    bool curl_ext = ext=="curl";
    if (curl_ext)
    {
        fileName = osgDB::getNameLessExtension(fullFileName);
        ext = osgDB::getFileExtension(fileName);
    }

    if (!osgDB::containsServerAddress(fileName))
    {
        if (options && !options->getDatabasePathList().empty())
        {
            if (osgDB::containsServerAddress(options->getDatabasePathList().front()))
            {
                std::string newFileName = options->getDatabasePathList().front() + "/" + fileName;

                return readFile(objectType, newFileName,options);
            }
        }

        // if user has explictly specified curl then we don't about at this point,
        // instead assume the curl can read it any way, if it doesn't explictly
        // specify curl then we assume that the file is a local file and not appropriate
        // for the curl plugin to load.
        if (!curl_ext) return ReadResult::FILE_NOT_HANDLED;
    }

    OSG_INFO<<"ReaderWriterCURL::readFile("<<fullFileName<<")"<<std::endl;

    std::string proxyAddress;
    long connectTimeout = 0;
    long timeout = 0;
    getConnectionOptions(options, proxyAddress, connectTimeout, timeout);

    bool uncompress = false;

    if (ext=="gz" || ext=="osgz" || ext=="ivez")
    {
        OSG_INFO<<"CURL: Compressed file type "<<ext<<std::endl;

        #ifndef USE_ZLIB
            // don't have zlib so can't compile compressed formats
            return ReadResult::FILE_NOT_HANDLED;
        #endif

        uncompress = true;

        if (ext=="gz")
        {
            ext = osgDB::getFileExtension(osgDB::getNameLessExtension(fileName));
        }
        else if (ext=="osgz")
        {
            ext = "osg";
        }
        else if (ext=="ivez")
        {
            ext = "ive";
        }

        OSG_INFO<<"CURL: assuming file type "<<ext<<std::endl;
    }

    std::stringstream buffer;

    EasyCurl::StreamObject sp(&buffer, NULL, std::string());
    EasyCurl& easyCurl = getEasyCurl();

    // setup the timeouts:
    easyCurl.setConnectionTimeout(connectTimeout);
    easyCurl.setTimeout(timeout);

    ReadResult curlResult = easyCurl.read(proxyAddress, fileName, sp, options);

    if (curlResult.status()==ReadResult::FILE_LOADED)
    {
        OSG_INFO<<"CURL: ReadResult::FILE_LOADED "<<std::endl;

        // Try to find a reader by file extension. If this fails, we will fetch the file
        // anyway and try to get a reader via mime-type.
        osgDB::ReaderWriter *reader =
            osgDB::Registry::instance()->getReaderWriterForExtension( ext );

        // If we do not already have a ReaderWriter, try to find one based on the
        // mime-type:
        if ( !reader )
        {
            std::string mimeType = easyCurl.getResultMimeType(sp);
            OSG_INFO << "CURL: Looking up extension for mime-type " << mimeType << std::endl;
            if ( mimeType.length() > 0 )
            {
                reader = osgDB::Registry::instance()->getReaderWriterForMimeType(mimeType);
            }
        }

        // If there is still no reader, fail.
        if ( !reader )
        {
            OSG_NOTICE<<"Error: No ReaderWriter for file "<<fileName<<std::endl;
            return ReadResult::FILE_NOT_HANDLED;
        }

        osg::ref_ptr<Options> local_opt = options ?
            static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) :
            new Options;

        local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));
        local_opt->setPluginStringData("STREAM_FILENAME",osgDB::getSimpleFileName(fileName));
        local_opt->setPluginStringData("filename",fileName);

        if (uncompress)
        {
            OSG_INFO<<"Curl:: plugin uncompressing "<<fileName<<std::endl;

            std::string uncompressed;
            if (!read(buffer, uncompressed))
            {
                return ReadResult::FILE_NOT_HANDLED;
            }

            buffer.str(uncompressed);
        }

        ReadResult readResult = readFile(objectType, reader, buffer, local_opt.get() );

        local_opt->getDatabasePathList().pop_front();

        return readResult;
    }
    else
    {
        OSG_INFO<<"CURL: not loading successfully "<<std::endl;
        return curlResult;
    }
}

#ifdef USE_ZLIB

#include <zlib.h>

bool ReaderWriterCURL::read(std::istream& fin, std::string& destination) const
{
    #define CHUNK 16384

    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm,
                       15 + 32 // autodected zlib or gzip header
                       );
    if (ret != Z_OK)
        return false;

    /* decompress until deflate stream ends or end of file */
    do {

        strm.avail_in = fin.readsome((char*)in, CHUNK);

        if (fin.fail())
        {
            (void)inflateEnd(&strm);
            return false;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);

            switch (ret) {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return false;
            }
            have = CHUNK - strm.avail_out;

            destination.append((char*)out, have);

        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? true : false;
}
#else
bool ReaderWriterCURL::read(std::istream& fin, std::string& destination) const
{
    return false;
}
#endif

size_t empty_write_data(const char *buffer, size_t size, size_t nmemb, char
*userp)
{
    return size*nmemb;
}

bool ReaderWriterCURL::fileExists(const std::string& filename, const osgDB::Options* options) const
{
    if (osgDB::containsServerAddress(filename))
    {
        std::string data;
        OSG_NOTICE<<"Checking if file exists using curl plugin: "<<filename<<std::endl;
        CURL* curl_handle = curl_easy_init();
        curl_easy_setopt(curl_handle, CURLOPT_URL, filename.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1);
        curl_easy_setopt(curl_handle, CURLOPT_HEADER, 1);
        curl_easy_setopt(curl_handle, CURLOPT_FILETIME, 1);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, &empty_write_data);

        int result = curl_easy_perform(curl_handle);
        
        long http_return_code(0);
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_return_code);
        
        curl_easy_cleanup(curl_handle);
        
        return ((result == 0) && ((http_return_code == 200) || (http_return_code == 0)));
    }
    else
    {
        return ReaderWriter::fileExists(filename, options);
    }
}


// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(curl, ReaderWriterCURL)
