/* -*-c++-*- VirtualPlanetBuilder - Copyright (C) 1998-2007 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/


#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>

#include <iostream>
#include <sstream>

#include <curl/curl.h>
#include <curl/types.h>

class EasyCurl : public osg::Referenced
{
    public:
    
        struct StreamPair
        {
            StreamPair(std::ostream* stream1, std::ostream* stream2=0):
                _stream1(stream1),
                _stream2(stream2) {}
        
            std::ostream* _stream1;
            std::ostream* _stream2;
        };
    
        static size_t StreamMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
        {
            size_t realsize = size * nmemb;
            StreamPair* sp = (StreamPair*)data;

            if (sp->_stream1) sp->_stream1->write((const char*)ptr, realsize);
            if (sp->_stream2) sp->_stream2->write((const char*)ptr, realsize);

            return realsize;
        }


        EasyCurl()
        {
            osg::notify(osg::INFO)<<"EasyCurl::EasyCurl()"<<std::endl;

            _curl = curl_easy_init();
            
            curl_easy_setopt(_curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");            
            curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, StreamMemoryCallback);
        }

        ~EasyCurl()
        {
            osg::notify(osg::INFO)<<"EasyCurl::~EasyCurl()"<<std::endl;

            if (_curl) curl_easy_cleanup(_curl);
            
            _curl = 0;
        }


        osgDB::ReaderWriter::ReadResult read(const std::string& proxyAddress, const std::string& fileName, StreamPair& sp)
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

    protected:

        // disallow copying
        EasyCurl(const EasyCurl& rhs):_curl(rhs._curl) {}
        EasyCurl& operator = (const EasyCurl&) { return *this; }

        
        CURL* _curl;
};


class ReaderWriterCURL : public osgDB::ReaderWriter
{
    public:
    
        ReaderWriterCURL()
        {
        }
      
        ~ReaderWriterCURL()
        {
        }

        virtual const char* className() const { return "HTTP Protocol Model Reader"; }
                                                                                            
        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension,"curl");
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

        virtual ReadResult readFile(ObjectType objectType, const std::string& fullFileName, const Options *options) const
        {
            std::string cacheFilePath, cacheFileName;
            std::string proxyAddress, optProxy, optProxyPort;

            if (options)
            {
                std::istringstream iss(options->getOptionString());
                std::string opt;
                while (iss >> opt) 
                {
                    int index = opt.find( "=" );
                    if( opt.substr( 0, index ) == "OSG_FILE_CACHE" )
                        cacheFilePath = opt.substr( index+1 ); //Setting Cache Directory by OSG Options
                    else if( opt.substr( 0, index ) == "OSG_CURL_PROXY" )
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

            if (!osgDB::containsServerAddress(fullFileName)) 
            {
                if (options && !(options->getDatabasePathList().empty()))
                {
                    if (osgDB::containsServerAddress(options->getDatabasePathList().front()))
                    {
                        std::string newFileName = options->getDatabasePathList().front() + "/" + fullFileName;
                        
                        return readFile(objectType, newFileName,options);
                    }
                }

                return ReadResult::FILE_NOT_HANDLED;
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

            //Getting CURL Environment Variables (If found rewrite OSG Options)
            const char* fileCachePath = getenv("OSG_FILE_CACHE");
            if (fileCachePath) //Env Cache Directory
                cacheFilePath = std::string(fileCachePath);

            if (!cacheFilePath.empty())
            {
                cacheFileName = cacheFilePath + "/" + 
                                osgDB::getServerAddress(fileName) + "/" + 
                                osgDB::getServerFileName(fileName);
            }
                                                        
            if (!cacheFilePath.empty() && osgDB::fileExists(cacheFileName))
            {
                osg::notify(osg::INFO) << "Reading cache file " << cacheFileName << std::endl;
                ReadResult result = osgDB::Registry::instance()->readObject(cacheFileName,options);
                return result;                
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

            EasyCurl::StreamPair sp(&buffer);
            
            //ReadResult result = _easyCurl.read(proxyAddress, fileName, sp);
            ReadResult result = getEasyCurl().read(proxyAddress, fileName, sp);

            if (result.status()==ReadResult::FILE_LOADED)
            {
                osg::ref_ptr<Options> local_opt = const_cast<Options*>(options);
                if (!local_opt) local_opt = new Options;

                local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));

                ReadResult readResult = readFile(objectType, reader, buffer, local_opt.get() );

                local_opt->getDatabasePathList().pop_front();

                if (!cacheFilePath.empty() && readResult.validObject())
                {
                    osg::notify(osg::INFO)<<"Writing cache file "<<cacheFileName<<std::endl;
                    
                    std::string filePath = osgDB::getFilePath(cacheFileName);
                    if (osgDB::fileExists(filePath) || osgDB::makeDirectory(filePath))
                    {
                        switch(objectType)
                        {
                        case(OBJECT): osgDB::writeObjectFile( *(readResult.getObject()), cacheFileName ); break;
                        case(IMAGE): osgDB::writeImageFile( *(readResult.getImage()), cacheFileName ); break;
                        case(HEIGHTFIELD): osgDB::writeHeightFieldFile( *(readResult.getHeightField()), cacheFileName ); break;
                        case(NODE): osgDB::writeNodeFile( *(readResult.getNode()), cacheFileName ); break;
                        default: break;
                        }
                    }
                    else
                    {
                        osg::notify(osg::NOTICE)<<"Error: Failed to created directory "<<filePath<<std::endl;
                    }

                }

                return readResult;
            }
            else
            {
                return result;
            }
        }
        
    protected:
    
        typedef std::map<OpenThreads::Thread*, osg::ref_ptr<EasyCurl> > ThreadCurlMap;
        
        EasyCurl& getEasyCurl() const
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex>  lock(_threadCurlMapMutex);

            osg::ref_ptr<EasyCurl>& ec = _threadCurlMap[OpenThreads::Thread::CurrentThread()];
            if (!ec) ec = new EasyCurl;
            
            return *ec;
        }
        
        mutable OpenThreads::Mutex  _threadCurlMapMutex;
        mutable ThreadCurlMap       _threadCurlMap;
};



// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(rgb, ReaderWriterCURL)
