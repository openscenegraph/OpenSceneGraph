#include <osg/Notify>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include "OSGA_Archive.h"


class ReaderWriterOSGA : public osgDB::ReaderWriter
{
public:
    ReaderWriterOSGA() { }

    virtual const char* className() const { return "OpenSceneGraph Archive Reader/Writer"; }
    virtual bool acceptsExtension(const std::string& extension) const
    {
        return osgDB::equalCaseInsensitive(extension,"osga");
    }

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

    virtual ReadResult readImage(const std::string& file,const Options*) const
    {
        ReadResult result = openArchive(file,osgDB::Archive::READ);
        
        if (!result.validArchive()) return result;


        osg::ref_ptr<osgDB::ReaderWriter::Options> local_options = new osgDB::ReaderWriter::Options;
        local_options->setDatabasePath(file);

        ReadResult result_2 = result.getArchive()->readImage(result.getArchive()->getMasterFileName(),local_options.get());
        

        // register the archive so that it is cached for future use.
        osgDB::Registry::instance()->addToArchiveCache(file, result.getArchive());


        return result_2;
    }

    virtual ReadResult readNode(const std::string& file,const Options*) const
    {
        ReadResult result = openArchive(file,osgDB::Archive::READ);
        
        if (!result.validArchive()) return result;


        osg::ref_ptr<osgDB::ReaderWriter::Options> local_options = new osgDB::ReaderWriter::Options;
        local_options->setDatabasePath(file);

        ReadResult result_2 = result.getArchive()->readNode(result.getArchive()->getMasterFileName(),local_options.get());
        

        // register the archive so that it is cached for future use.
        osgDB::Registry::instance()->addToArchiveCache(file, result.getArchive());


        return result_2;
    }

protected:

    
};


// register with Registry to instantiate the above reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterOSGA> g_osgaReaderWriterProxy;
