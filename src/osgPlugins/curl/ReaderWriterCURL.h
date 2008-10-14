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

#ifndef READERWRITERCURL_H
#define READERWRITERCURL_H 1

#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>

namespace osg_curl
{

enum ObjectType
{
    OBJECT,
    ARCHIVE,
    IMAGE,
    HEIGHTFIELD,
    NODE
};
                                                                                            
class EasyCurl : public osg::Referenced
{
    public:
    
        struct StreamObject
        {
            StreamObject(std::ostream* stream1, const std::string& cacheFileName);
            
            void write(const char* ptr, size_t realsize);
        
            std::ostream*   _stream1;
            
            bool            _foutOpened;
            std::string     _cacheFileName;
            std::ofstream   _fout;
        };
    
        static size_t StreamMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data);

        EasyCurl();

        osgDB::ReaderWriter::ReadResult read(const std::string& proxyAddress, const std::string& fileName, StreamObject& sp, const osgDB::ReaderWriter::Options *options);

    protected:

        virtual ~EasyCurl();

        // disallow copying
        EasyCurl(const EasyCurl& rhs):_curl(rhs._curl) {}
        EasyCurl& operator = (const EasyCurl&) { return *this; }

        
        CURL* _curl;
        
        std::string     _previousPassword;
        long            _previousHttpAuthentication;
};


class ReaderWriterCURL : public osgDB::ReaderWriter
{
    public:
    
        ReaderWriterCURL();
      
        ~ReaderWriterCURL();

        virtual const char* className() const { return "HTTP Protocol Model Reader"; }
                                                                                            
        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension,"curl");
        }

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

        ReadResult readFile(ObjectType objectType, osgDB::ReaderWriter* rw, std::istream& fin, const Options *options) const;
        
        virtual ReadResult readFile(ObjectType objectType, const std::string& fullFileName, const Options *options) const;
        
        EasyCurl& getEasyCurl() const
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex>  lock(_threadCurlMapMutex);

            osg::ref_ptr<EasyCurl>& ec = _threadCurlMap[OpenThreads::Thread::CurrentThread()];
            if (!ec) ec = new EasyCurl;
            
            return *ec;
        }

        bool read(std::istream& fin, std::string& destination) const;

    protected:
    
        typedef std::map< OpenThreads::Thread*, osg::ref_ptr<EasyCurl> >    ThreadCurlMap;
        
        mutable OpenThreads::Mutex          _threadCurlMapMutex;
        mutable ThreadCurlMap               _threadCurlMap;
};

}

#endif
