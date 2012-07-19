#ifndef OSGDB_ZIPARCHIVE
#define OSGDB_ZIPARCHIVE 1

#include <osgDB/ReaderWriter>
#include <osgDB/FileUtils>

#include <osgDB/Archive>
#include <OpenThreads/Mutex>

#include "unzip.h"


class ZipArchive : public osgDB::Archive
{
    public:
        ZipArchive();
        virtual ~ZipArchive();

        virtual const char* libraryName() const { return "osgDB"; }

        virtual const char* className() const { return "ZipArchive"; }

        virtual bool acceptsExtension(const std::string& /*extension*/) const { return true; }

        /** close the archive.*/
        virtual void close();

        /** open the archive.*/
        virtual bool open(const std::string& filename, ArchiveStatus status, const osgDB::ReaderWriter::Options* options);

        /** open the archive for reading.*/
        virtual bool open(std::istream& fin, const osgDB::ReaderWriter::Options* options);

        /** return true if file exists in archive.*/
        virtual bool fileExists(const std::string& filename) const;

        /** Get the filename that refers to the archived file on disk*/
        virtual std::string getArchiveFileName() const;

        /** Get the file name which represents the master file recorded in the Archive.*/
        virtual std::string getMasterFileName() const;

        /** Get the full list of file names available in the archive.*/
        virtual bool getFileNames(osgDB::Archive::FileNameList& fileNameList) const;

        /** return type of file. */
        virtual osgDB::FileType getFileType(const std::string& filename) const;

        /** return the contents of a directory.
        * returns an empty array on any error.*/
        virtual osgDB::DirectoryContents getDirectoryContents(const std::string& dirName) const;

        virtual osgDB::ReaderWriter::ReadResult readObject(const std::string& /*fileName*/, const osgDB::ReaderWriter::Options* =NULL) const;
        virtual osgDB::ReaderWriter::ReadResult readImage(const std::string& /*fileName*/,const osgDB::ReaderWriter::Options* =NULL) const;
        virtual osgDB::ReaderWriter::ReadResult readHeightField(const std::string& /*fileName*/,const osgDB::ReaderWriter::Options* =NULL) const;
        virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& /*fileName*/, const osgDB::ReaderWriter::Options* =NULL) const;
        virtual osgDB::ReaderWriter::ReadResult readShader(const std::string& /*fileName*/, const osgDB::ReaderWriter::Options* =NULL) const;

        virtual osgDB::ReaderWriter::WriteResult writeObject(const osg::Object& /*obj*/, const std::string& /*fileName*/,const osgDB::ReaderWriter::Options* =NULL) const;
        virtual osgDB::ReaderWriter::WriteResult writeImage(const osg::Image& /*image*/, const std::string& /*fileName*/,const osgDB::ReaderWriter::Options* =NULL) const;
        virtual osgDB::ReaderWriter::WriteResult writeHeightField(const osg::HeightField& /*heightField*/, const std::string& /*fileName*/,const osgDB::ReaderWriter::Options* =NULL) const;
        virtual osgDB::ReaderWriter::WriteResult writeNode(const osg::Node& /*node*/, const std::string& /*fileName*/,const osgDB::ReaderWriter::Options* =NULL) const;
        virtual osgDB::ReaderWriter::WriteResult writeShader(const osg::Shader& /*shader*/, const std::string& /*fileName*/,const osgDB::ReaderWriter::Options* =NULL) const;

    protected:

        osgDB::ReaderWriter* ReadFromZipEntry(const ZIPENTRY* ze, const osgDB::ReaderWriter::Options* options, std::stringstream& streamIn) const;

        void IndexZipFiles(HZIP hz);
        const ZIPENTRY* GetZipEntry(const std::string& filename) const;
        ZIPENTRY* GetZipEntry(const std::string& filename);

        std::string ReadPassword(const osgDB::ReaderWriter::Options* options) const;
        bool CheckZipErrorCode(ZRESULT result) const;

    private:


        typedef std::pair<std::string, ZIPENTRY*> ZipEntryMapping;
        typedef std::map<std::string, ZIPENTRY*> ZipEntryMap;

        std::string _filename, _password, _membuffer;

        OpenThreads::Mutex _zipMutex;
        bool               _zipLoaded;
        ZipEntryMap        _zipIndex;
        ZIPENTRY           _mainRecord;

        struct PerThreadData {
            HZIP _zipHandle;
        };

        typedef std::map<OpenThreads::Thread*, PerThreadData> PerThreadDataMap;
        PerThreadDataMap _perThreadData;

        const PerThreadData& getData() const;
        const PerThreadData& getDataNoLock() const;
};


#endif //OSGDB_ZIPARCHIVE

