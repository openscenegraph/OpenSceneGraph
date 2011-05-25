#include <osg/Group>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/Registry>

#include <sstream>
#include "ZipArchive.h"

class ReaderWriterZIP : public osgDB::ReaderWriter
{
    public:

        ReaderWriterZIP()
        {
            supportsExtension("zip","Zip archive format");
            osgDB::Registry::instance()->addArchiveExtension("zip");
        }

        virtual const char* className() const { return "ZIP Database Reader/Writer"; }


        virtual ReadResult openArchive(const std::string& file,ArchiveStatus status, unsigned int indexBlockSize = 4096, const Options* options = NULL) const
        {

            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile(file, options);
            if (fileName.empty())
            {
                //we do not support writing so the file must exist
                return ReadResult::FILE_NOT_FOUND;
            }

            // copy the incoming options if possible so that plugin options can be applied to files
            // inside the archive
            osg::ref_ptr<osgDB::ReaderWriter::Options> local_options =  options?
                options->cloneOptions() :
                new osgDB::ReaderWriter::Options;

            osg::ref_ptr<ZipArchive> archive = new ZipArchive;
            if (!archive->open(fileName, osgDB::ReaderWriter::READ, local_options.get()))
            {
                return ReadResult(ReadResult::FILE_NOT_HANDLED);
            }

            return archive.get();
        }

        /** open an archive for reading.*/ 
        virtual ReadResult openArchive(std::istream& fin, const Options* options) const
        {
            osg::ref_ptr<ZipArchive> archive = new ZipArchive;
            if (!archive->open(fin, options))
            {
                return ReadResult(ReadResult::FILE_NOT_HANDLED);
            }

            return archive.get();
        }


        virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            osgDB::ReaderWriter::ReadResult result = openArchive(file, osgDB::Archive::READ);

            if (!result.validArchive()) return result;

            // copy the incoming options if possible so that plugin options can be applied to files
            // inside the archive
            osg::ref_ptr<osgDB::ReaderWriter::Options> local_options = options?
                options->cloneOptions() :
                new osgDB::ReaderWriter::Options;

            local_options->setDatabasePath(file);

            //todo- what should we read here?
            osgDB::ReaderWriter::ReadResult result_2 = result.getArchive()->readNode(result.getArchive()->getMasterFileName(),local_options.get());

            if (!options || (options->getObjectCacheHint() & osgDB::ReaderWriter::Options::CACHE_ARCHIVES))
            {
                // register the archive so that it is cached for future use.
                osgDB::Registry::instance()->addToArchiveCache(file, result.getArchive());
            }

            return result_2;

        }

        virtual ReadResult readNode(std::istream& fin,const osgDB::ReaderWriter::Options* options) const
        {

            osgDB::ReaderWriter::ReadResult result = openArchive(fin, options);

            if (!result.validArchive()) return result;

            // copy the incoming options if possible so that plugin options can be applied to files
            // inside the archive
            osg::ref_ptr<osgDB::ReaderWriter::Options> local_options = options?
                options->cloneOptions() :
                new osgDB::ReaderWriter::Options;

            //todo- what should the database path be?
            //local_options->setDatabasePath(file);


            //todo- what should we read here?
            osgDB::ReaderWriter::ReadResult result_2 = result.getArchive()->readNode(result.getArchive()->getMasterFileName(),local_options.get());

            //todo- what to do to cache the archive here?
            //if (!options || (options->getObjectCacheHint() & osgDB::ReaderWriter::Options::CACHE_ARCHIVES))
            //{
            //   // register the archive so that it is cached for future use.
            //   osgDB::Registry::instance()->addToArchiveCache(file, result.getArchive());
            //}

            return result_2;
        }

        virtual ReadResult readImage(const std::string& file,const Options* options) const
        {
           ReadResult result = openArchive(file,osgDB::Archive::READ);

           if (!result.validArchive()) return result;


           // copy the incoming options if possible so that plugin options can be applied to files
           // inside the archive
           osg::ref_ptr<osgDB::ReaderWriter::Options> local_options = options?
                options->cloneOptions() :
                new osgDB::ReaderWriter::Options;

           local_options->setDatabasePath(file);

           ReadResult result_2 = result.getArchive()->readImage(result.getArchive()->getMasterFileName(),local_options.get());


           if (!options || (options->getObjectCacheHint() & osgDB::ReaderWriter::Options::CACHE_ARCHIVES))
           {
              // register the archive so that it is cached for future use.
              osgDB::Registry::instance()->addToArchiveCache(file, result.getArchive());
           }

           return result_2;
        }

};

// now register with sgRegistry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(zip, ReaderWriterZIP)
