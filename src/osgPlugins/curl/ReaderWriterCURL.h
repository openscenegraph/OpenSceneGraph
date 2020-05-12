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
            StreamObject(std::ostream* outputStream, std::istream* inputStream, const std::string& cacheFileName);

            void write(const char* ptr, size_t realsize);
            size_t read(char* ptr, size_t maxsize);

            std::ostream*   _outputStream;
            std::istream*   _inputStream;

            bool            _foutOpened;
            std::string     _cacheFileName;
            std::ofstream   _fout;
            std::string     _resultMimeType;
        };

        static size_t StreamMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data);

        EasyCurl();

        // Added this function to set the desired connection timeout if needed (in case someone needs to try to connect
        // to offline servers without freezing the process for a very long time) [even if, as stated on curl website,
        // some normal transfer may be timed out this way].
        inline void setConnectionTimeout(long val) { _connectTimeout = val; }

        // the timeout variable is used to limit the whole transfer duration instead of the connection phase only.
        inline void setTimeout(long val) { _timeout = val; }

        inline void setSSLVerifyPeer(long verifyPeer) { _sslVerifyPeer = verifyPeer; }

        // Perform HTTP GET to download data from web server.
        osgDB::ReaderWriter::ReadResult read(const std::string& proxyAddress, const std::string& fileName, StreamObject& sp, const osgDB::ReaderWriter::Options *options);

        // Perform HTTP POST to upload data using "multipart/form-data" encoding to web server.
        osgDB::ReaderWriter::WriteResult write(const std::string& proxyAddress, const std::string& fileName, StreamObject& sp, const osgDB::ReaderWriter::Options *options);

        /** Returns the mime type of the data retrieved with the provided stream object on a
          * previous call to EasyCurl::read(). */
        std::string getResultMimeType(const StreamObject& sp) const;

        std::string getMimeTypeForExtension(const std::string& ext) const;
        static std::string getFileNameFromURL(const std::string& url);

    protected:

        virtual ~EasyCurl();

        // disallow copying
        EasyCurl(const EasyCurl& rhs) : osg::Referenced(rhs), _curl(rhs._curl) {}
        EasyCurl& operator = (const EasyCurl&) { return *this; }

        void setOptions(const std::string& proxyAddress, const std::string& fileName, StreamObject& sp, const osgDB::ReaderWriter::Options *options);
        osgDB::ReaderWriter::ReadResult processResponse(CURLcode responseCode, const std::string& proxyAddress, const std::string& fileName, StreamObject& sp);

        CURL* _curl;

        std::string     _previousPassword;
        long            _previousHttpAuthentication;
        long            _connectTimeout;
        long            _timeout;
        long            _sslVerifyPeer;
};


class ReaderWriterCURL : public osgDB::ReaderWriter
{
    public:

        ReaderWriterCURL();

        ~ReaderWriterCURL();

        virtual const char* className() const { return "HTTP Protocol Model Reader"; }

        virtual bool fileExists(const std::string& filename, const osgDB::Options* options) const;

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

        virtual WriteResult writeObject(const osg::Object& obj, const std::string& fileName, const Options* options) const
        {
            return writeFile(obj,fileName,options);
        }

        virtual WriteResult writeImage(const osg::Image& image, const std::string& fileName, const Options* options) const
        {
            return writeFile(image,fileName,options);
        }

        virtual WriteResult writeHeightField(const osg::HeightField& heightField, const std::string& fileName, const Options* options) const
        {
            return writeFile(heightField,fileName,options);
        }

        virtual WriteResult writeNode(const osg::Node& node, const std::string& fileName, const Options* options) const
        {
            return writeFile(node,fileName,options);
        }

        ReadResult readFile(ObjectType objectType, osgDB::ReaderWriter* rw, std::istream& fin, const Options *options) const;
        WriteResult writeFile(const osg::Object& obj, osgDB::ReaderWriter* rw, std::ostream& fout, const Options *options) const;

        virtual ReadResult readFile(ObjectType objectType, const std::string& fullFileName, const Options *options) const;
        virtual WriteResult writeFile(const osg::Object& obj, const std::string& fullFileName, const Options *options) const;

        EasyCurl& getEasyCurl() const
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex>  lock(_threadCurlMapMutex);

            osg::ref_ptr<EasyCurl>& ec = _threadCurlMap[OpenThreads::Thread::CurrentThreadId()];
            if (!ec) ec = new EasyCurl;

            return *ec;
        }

        bool read(std::istream& fin, std::string& destination) const;

    protected:
        void getConnectionOptions(const osgDB::ReaderWriter::Options *options, std::string& proxyAddress, long& connectTimeout, long& timeout, long& sslVerifyPeer) const;

        typedef std::map< size_t, osg::ref_ptr<EasyCurl> >    ThreadCurlMap;

        mutable OpenThreads::Mutex          _threadCurlMapMutex;
        mutable ThreadCurlMap               _threadCurlMap;
};

}

#endif
