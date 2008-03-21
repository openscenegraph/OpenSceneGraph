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
            if (!osgDB::containsServerAddress(fullFileName)) 
            {
                osg::notify(osg::NOTICE)<<"File '"<<fullFileName<<"' does not contain server address, cannort load with libcurl plugin."<<std::endl;
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
                osg::notify(osg::NOTICE)<<"No ReaderWriter for file "<<fileName<<std::endl;
                return ReadResult::FILE_NOT_HANDLED;
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

                if (local_opt.valid() && local_opt->getDatabasePathList().empty())
                {
                    local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));
                }

                ReadResult result = readFile(objectType, reader, buffer, local_opt.get() );
                
                local_opt->getDatabasePathList().pop_front();
                
                return result;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"Read error, file="<<fileName<<" libcurl result = "<<res<<std::endl;
                return ReadResult::FILE_NOT_HANDLED;
            }
        }
        
    protected:
        
        CURL* _curl;
        
};



// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(rgb, ReaderWriterCURL)
