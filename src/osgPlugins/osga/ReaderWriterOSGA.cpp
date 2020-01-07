#include <osg/Notify>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include "OSGA_Archive.h"


class ReaderWriterOSGA : public osgDB::ReaderWriter
{
public:
    ReaderWriterOSGA()
    {
        supportsExtension("osga","OpenSceneGraph Archive format");
    }

    virtual const char* className() const { return "OpenSceneGraph Archive Reader/Writer"; }

    virtual ReadResult openArchive(const std::string& file,ArchiveStatus status, unsigned int indexBlockSize = 4096, const Options* options=NULL) const
    {

        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        std::string fileName = osgDB::findDataFile( file, options );
        if (fileName.empty())
        {
            if (status==READ) return ReadResult::FILE_NOT_FOUND;
            fileName = file;
        }

        osg::ref_ptr<OSGA_Archive> archive = new OSGA_Archive;
        if (!archive->open(fileName, status, indexBlockSize))
        {
            return ReadResult(ReadResult::FILE_NOT_HANDLED);
        }

        return archive.get();
    }

    /** open an archive for reading.*/
    virtual ReadResult openArchive(std::istream& fin,const Options*) const
    {
        osg::ref_ptr<OSGA_Archive> archive = new OSGA_Archive;
        if (!archive->open(fin))
        {
            return ReadResult(ReadResult::FILE_NOT_HANDLED);
        }

        return archive.get();
    }

    enum ReadType
    {
        READ_OBJECT,
        READ_IMAGE,
        READ_HEIGHT_FIELD,
        READ_NODE,
        READ_SHADER
    };

    virtual ReadResult readMasterFile(ReadType type, const std::string& file, const Options* options) const
    {
        ReadResult result = openArchive(file, osgDB::Archive::READ);

        if (!result.validArchive()) return result;

        if (!options || (options->getObjectCacheHint() & osgDB::ReaderWriter::Options::CACHE_ARCHIVES))
        {
            // register the archive so that it is cached for future use.
            osgDB::Registry::instance()->addToArchiveCache(file, result.getArchive());
        }

        // copy the incoming options if possible so that plugin options can be applied to files
        // inside the archive
        osg::ref_ptr<osgDB::ReaderWriter::Options> local_options =
            options ?
            new osgDB::ReaderWriter::Options(*options) :
            new osgDB::ReaderWriter::Options;

        local_options->setDatabasePath(file);

        switch (type) {
        default:
        case READ_OBJECT:
            return result.getArchive()->readObject(result.getArchive()->getMasterFileName(), local_options.get());
        case READ_IMAGE:
            return result.getArchive()->readImage(result.getArchive()->getMasterFileName(), local_options.get());
        case READ_HEIGHT_FIELD:
            return result.getArchive()->readHeightField(result.getArchive()->getMasterFileName(), local_options.get());
        case READ_NODE:
            return result.getArchive()->readNode(result.getArchive()->getMasterFileName(), local_options.get());
        case READ_SHADER:
            return result.getArchive()->readShader(result.getArchive()->getMasterFileName(), local_options.get());
        }
    }
    virtual ReadResult readObject(const std::string& file, const Options* options) const
    {
        return readMasterFile(READ_OBJECT, file, options);
    }

    virtual ReadResult readImage(const std::string& file, const Options* options) const
    {
        return readMasterFile(READ_IMAGE, file, options);
    }

    virtual ReadResult readHeightField(const std::string& file, const Options* options) const
    {
        return readMasterFile(READ_HEIGHT_FIELD, file, options);
    }

    virtual ReadResult readNode(const std::string& file, const Options* options) const
    {
        return readMasterFile(READ_NODE, file, options);
    }

    virtual ReadResult readShader(const std::string& file, const Options* options) const
    {
        return readMasterFile(READ_SHADER, file, options);
    }

protected:


};


// register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(osga, ReaderWriterOSGA)
