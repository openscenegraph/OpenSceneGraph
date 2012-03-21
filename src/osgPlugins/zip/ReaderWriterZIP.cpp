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

        osgDB::ReaderWriter::ReadResult readNodeFromArchive(osgDB::Archive& archive, const osgDB::ReaderWriter::Options* options) const
        {
            osgDB::ReaderWriter::ReadResult result(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);

            if (!archive.getMasterFileName().empty())
            {
                result = archive.readNode(archive.getMasterFileName(), options);
            }
            else
            {
                osgDB::Archive::FileNameList fileNameList;
                if (archive.getFileNames(fileNameList))
                {
                    typedef std::list< osg::ref_ptr<osg::Node> > Nodes;
                    Nodes nodes;
                    for(osgDB::Archive::FileNameList::iterator itr = fileNameList.begin();
                        itr != fileNameList.end();
                        ++itr)
                    {
                        result = archive.readNode(*itr, options);
                        if (result.validNode()) nodes.push_back(result.getNode());
                    }

                    if (!nodes.empty())
                    {
                        if (nodes.size()==1)
                        {
                            result = osgDB::ReaderWriter::ReadResult(nodes.front().get());
                        }
                        else
                        {
                            osg::ref_ptr<osg::Group> group = new osg::Group;
                            for(Nodes::iterator itr = nodes.begin();
                                itr != nodes.end();
                                ++itr)
                            {
                                group->addChild(itr->get());
                            }
                            result = osgDB::ReaderWriter::ReadResult(group.get());
                        }
                    }
                }
            }
            return result;
        }


        virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            osgDB::ReaderWriter::ReadResult result = openArchive(file, osgDB::Archive::READ);

            if (!result.validArchive()) return result;

            osg::ref_ptr<osgDB::Archive> archive = result.getArchive();

            if (!options || (options->getObjectCacheHint() & osgDB::ReaderWriter::Options::CACHE_ARCHIVES))
            {
                // register the archive so that it is cached for future use.
                osgDB::Registry::instance()->addToArchiveCache(file, archive.get());
            }

            // copy the incoming options if possible so that plugin options can be applied to files
            // inside the archive
            osg::ref_ptr<osgDB::ReaderWriter::Options> local_options = options?
                options->cloneOptions() :
                new osgDB::ReaderWriter::Options;

            local_options->setDatabasePath(file);

            return readNodeFromArchive(*archive, local_options.get());
        }

        virtual ReadResult readNode(std::istream& fin,const osgDB::ReaderWriter::Options* options) const
        {
            osgDB::ReaderWriter::ReadResult result = openArchive(fin, options);

            if (!result.validArchive()) return result;

            osg::ref_ptr<osgDB::Archive> archive = result.getArchive();

            // copy the incoming options if possible so that plugin options can be applied to files
            // inside the archive
            osg::ref_ptr<osgDB::ReaderWriter::Options> local_options = options?
                options->cloneOptions() :
                new osgDB::ReaderWriter::Options;

            return readNodeFromArchive(*archive, local_options.get());
        }

        osgDB::ReaderWriter::ReadResult readImageFromArchive(osgDB::Archive& archive, const osgDB::ReaderWriter::Options* options) const
        {
            osgDB::ReaderWriter::ReadResult result(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);

            if (!archive.getMasterFileName().empty())
            {
                result = archive.readImage(archive.getMasterFileName(), options);
            }
            else
            {
                osgDB::Archive::FileNameList fileNameList;
                if (archive.getFileNames(fileNameList))
                {
                    for(osgDB::Archive::FileNameList::iterator itr = fileNameList.begin();
                        itr != fileNameList.end() && !result.validImage();
                        ++itr)
                    {
                        result = archive.readImage(*itr, options);
                    }
                }
            }
            return result;
        }

        virtual ReadResult readImage(const std::string& file,const Options* options) const
        {
            osgDB::ReaderWriter::ReadResult result = openArchive(file, osgDB::Archive::READ);

            if (!result.validArchive()) return result;

            osg::ref_ptr<osgDB::Archive> archive = result.getArchive();

            if (!options || (options->getObjectCacheHint() & osgDB::ReaderWriter::Options::CACHE_ARCHIVES))
            {
                // register the archive so that it is cached for future use.
                osgDB::Registry::instance()->addToArchiveCache(file, archive.get());
            }

            // copy the incoming options if possible so that plugin options can be applied to files
            // inside the archive
            osg::ref_ptr<osgDB::ReaderWriter::Options> local_options = options?
                options->cloneOptions() :
                new osgDB::ReaderWriter::Options;

            local_options->setDatabasePath(file);

            return readImageFromArchive(*archive, local_options.get());
        }

        virtual ReadResult readImage(std::istream& fin,const osgDB::ReaderWriter::Options* options) const
        {
            osgDB::ReaderWriter::ReadResult result = openArchive(fin, options);

            if (!result.validArchive()) return result;

            osg::ref_ptr<osgDB::Archive> archive = result.getArchive();

            // copy the incoming options if possible so that plugin options can be applied to files
            // inside the archive
            osg::ref_ptr<osgDB::ReaderWriter::Options> local_options = options?
                options->cloneOptions() :
                new osgDB::ReaderWriter::Options;

            return readImageFromArchive(*archive, local_options.get());
        }
};

// now register with sgRegistry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(zip, ReaderWriterZIP)
