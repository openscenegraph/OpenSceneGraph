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


class ReaderWriterCURL : public osgDB::ReaderWriter
{
    public:
    
        static size_t StreamMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
        {
            size_t realsize = size * nmemb;
            std::ostream* buffer = (std::ostream*)data;

            buffer->write((const char*)ptr, realsize);

            return realsize;
        }


        ReaderWriterCURL()
        {
            _curl = curl_easy_init();
            
            curl_easy_setopt(_curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");            
            curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, StreamMemoryCallback);
        }
      
        ~ReaderWriterCURL()
        {
            if (_curl) curl_easy_cleanup(_curl);
            
            _curl = 0;
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

            osgDB::ReaderWriter *reader = 
                osgDB::Registry::instance()->getReaderWriterForExtension( osgDB::getFileExtension(fileName));
                
            if (!reader)
            {
                osg::notify(osg::NOTICE)<<"Error: No ReaderWriter for file "<<fileName<<std::endl;
                return ReadResult::FILE_NOT_HANDLED;
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
                osg::notify(osg::NOTICE) << "Reading cache file " << cacheFileName << std::endl;
                return osgDB::Registry::instance()->readObject(cacheFileName,options);
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

            if(!proxyAddress.empty())
            {
                osg::notify(osg::NOTICE)<<"Setting proxy: "<<proxyAddress<<std::endl;
                curl_easy_setopt(_curl, CURLOPT_PROXY, proxyAddress.c_str()); //Sets proxy address and port on libcurl
            }

            std::stringstream buffer;

            curl_easy_setopt(_curl, CURLOPT_URL, fileName.c_str());
            curl_easy_setopt(_curl, CURLOPT_WRITEDATA, (void *)&buffer);

            CURLcode res = curl_easy_perform(_curl);
            

            curl_easy_setopt(_curl, CURLOPT_WRITEDATA, (void *)0);
        
            if (res==0)
            {


                osg::ref_ptr<Options> local_opt = const_cast<Options*>(options);
                if (!local_opt) local_opt = new Options;

                local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));

                ReadResult readResult = readFile(objectType, reader, buffer, local_opt.get() );
                
                local_opt->getDatabasePathList().pop_front();

                if (!cacheFilePath.empty() && readResult.validObject())
                {
                    osg::notify(osg::NOTICE)<<"Writing cache file "<<cacheFileName<<std::endl;
                    
                    std::string filePath = osgDB::getFilePath(cacheFileName);
                    if (!osgDB::fileExists(filePath))
                    {
                        osg::notify(osg::NOTICE)<<"Creating directory "<<filePath<<std::endl;
                        if (osgDB::makeDirectory(filePath))
                        {
                            osg::notify(osg::NOTICE)<<"   Created directory"<<std::endl;
                        }
                        else
                        {
                            osg::notify(osg::NOTICE)<<"   Failed"<<std::endl;
                        }
                    }
                    
                    switch(objectType)
                    {
                    case(OBJECT): osgDB::writeObjectFile( *(readResult.getObject()), cacheFileName ); break;
                    case(IMAGE): osgDB::writeImageFile( *(readResult.getImage()), cacheFileName ); break;
                    case(HEIGHTFIELD): osgDB::writeHeightFieldFile( *(readResult.getHeightField()), cacheFileName ); break;
                    case(NODE): osgDB::writeNodeFile( *(readResult.getNode()), cacheFileName ); break;
                    default: break;
                    }

                }

                return readResult;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"Error: libcurl read error, file="<<fileName<<" result = "<<res<<std::endl;
                return ReadResult::FILE_NOT_HANDLED;
            }
        }
        
    protected:
        
        CURL* _curl;
        
};



// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(rgb, ReaderWriterCURL)
