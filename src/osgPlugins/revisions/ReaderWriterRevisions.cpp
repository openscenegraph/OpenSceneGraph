#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgDB/DatabaseRevisions>
#include <osg/Notify>

class ReaderWriterRevisions : public osgDB::ReaderWriter
{
    public:
        ReaderWriterRevisions()
        {
            supportsExtension("revisions","list of revision files");
            supportsExtension("added","revision file containing list of added files");
            supportsExtension("removed","revision file containing list of removed files");
            supportsExtension("modified","revision file containing list of modified files");
        }

        virtual const char* className() const { return "Revisions Font Reader/Writer"; }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osgDB::ifstream fin(fileName.c_str());

            if (ext=="revisions") return readRevisions(fin, file, options);
            else return readFileList(fin, file, options);
        }

        virtual ReadResult readObject(std::istream& fin, const osgDB::ReaderWriter::Options* options) const
        {
            std::string fileName = options->getPluginStringData("filename");
            if (fileName.empty())
            {
                OSG_NOTICE<<"Error: ReaderWriterRevision unable to determine stream type, cannot not read file."<<std::endl;
                return ReadResult::FILE_NOT_HANDLED;
            }

            std::string ext = osgDB::getLowerCaseFileExtension(fileName);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            if (ext=="revisions") return readRevisions(fin, fileName, options);
            else return readFileList(fin, fileName, options);
        }

        ReadResult readFileList(std::istream& fin, const std::string& name, const osgDB::ReaderWriter::Options* options) const
        {
            OSG_INFO<<"    readFileList="<<name<<std::endl;

            osg::ref_ptr<osgDB::FileList> fileList = new osgDB::FileList;
            fileList->setName(name);

            while(fin)
            {
                std::string filename;
                fin >> filename;
                OSG_INFO<<"        ="<<filename<<std::endl;

                if (!filename.empty()) fileList->getFileNames().insert(filename);
            }

            return fileList.get();
        }

        ReadResult readRevisions(std::istream& fin, const std::string& name, const osgDB::ReaderWriter::Options* options) const
        {
            osg::ref_ptr<osgDB::DatabaseRevisions> revisions = new osgDB::DatabaseRevisions;
            revisions->setName(name);

            typedef std::map<std::string, osg::ref_ptr<osgDB::DatabaseRevision> > RevisionMap;
            RevisionMap revisionMap;

            std::string revisions_path;
            if (options && !(options->getDatabasePathList().empty())) revisions_path = options->getDatabasePathList().front();
            else revisions_path = osgDB::getFilePath(name);

            revisions->setDatabasePath(revisions_path);

            OSG_INFO<<"readRevisions="<<name<<std::endl;
            OSG_INFO<<"  revisions_path="<<revisions_path<<std::endl;

            bool loadFileLists = false;

            while(fin)
            {
                std::string filename;
                fin >> filename;

                OSG_INFO<<"    filename="<<filename<<std::endl;

                if (!filename.empty())
                {
                    std::string ext = osgDB::getLowerCaseFileExtension(filename);
                    std::string revisionName = osgDB::getNameLessExtension(filename);
                    if (!revisionName.empty())
                    {
                        osg::ref_ptr<osgDB::DatabaseRevision>& dbRevision = revisionMap[revisionName];
                        if (!dbRevision)
                        {
                            dbRevision = new osgDB::DatabaseRevision;
                            dbRevision->setName(revisionName);
                            dbRevision->setDatabasePath(revisions_path);
                        }

                        osg::ref_ptr<osgDB::FileList> fileList;


                        if (loadFileLists)
                        {
                            std::string complete_path = osgDB::concatPaths(revisions_path, filename);
                            OSG_INFO<<"    complete_path="<<complete_path<<std::endl;
                            osg::ref_ptr<osg::Object> object = osgDB::readObjectFile(complete_path, options);
                            fileList = dynamic_cast<osgDB::FileList*>(object.get());
                        }

                        if (!fileList)
                        {
                            fileList = new osgDB::FileList;
                            fileList->setName(filename);
                        }

                        if (ext=="added")
                        {
                            dbRevision->setFilesAdded(fileList.get());
                        }
                        else if (ext=="removed")
                        {
                            dbRevision->setFilesRemoved(fileList.get());
                        }
                        else if (ext=="modified")
                        {
                            dbRevision->setFilesModified(fileList.get());
                        }
                    }
                }
            }

            for(RevisionMap::iterator itr = revisionMap.begin();
                itr != revisionMap.end();
                ++itr)
            {
                revisions->addRevision(itr->second.get());
            }

            return revisions.get();
        }

        virtual WriteResult writeObject(const osg::Object& object, std::ostream& fout,const osgDB::ReaderWriter::Options*) const
        {
            const osgDB::FileList* fileList = dynamic_cast<const osgDB::FileList*>(&object);
            if (fileList)
            {
                const osgDB::FileList::FileNames& fileNames = fileList->getFileNames();
                for(osgDB::FileList::FileNames::const_iterator itr = fileNames.begin();
                    itr != fileNames.end();
                    ++itr)
                {
                    fout<<*itr<<std::endl;
                }
                return WriteResult::FILE_SAVED;
            }

            const osgDB::DatabaseRevisions* revisions = dynamic_cast<const osgDB::DatabaseRevisions*>(&object);
            if (revisions)
            {
                typedef osgDB::DatabaseRevisions::DatabaseRevisionList RevisionList;
                const RevisionList& revisionList  = revisions->getDatabaseRevisionList();
                for(RevisionList::const_iterator itr = revisionList.begin();
                    itr != revisionList.end();
                    ++itr)
                {
                    const osgDB::DatabaseRevision* revision = itr->get();
                    if (revision->getFilesAdded())
                    {
                        if (!(revision->getFilesAdded()->getName().empty())) fout<<revision->getFilesAdded()->getName()<<std::endl;
                        else fout<<"FilesAdded entry had no name assigned."<<std::endl;
                    }

                    if (revision->getFilesRemoved())
                    {
                        if (!(revision->getFilesRemoved()->getName().empty())) fout<<revision->getFilesRemoved()->getName()<<std::endl;
                        else fout<<"FilesAdded entry had no name assigned."<<std::endl;
                    }

                    if (revision->getFilesModified())
                    {
                        if (!(revision->getFilesModified()->getName().empty())) fout<<revision->getFilesModified()->getName()<<std::endl;
                        else fout<<"FilesAdded entry had no name assigned."<<std::endl;
                    }
                }
            }

            return WriteResult::FILE_NOT_HANDLED;
        }

        virtual WriteResult writeObject(const osg::Object& object,const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

            osgDB::ofstream fout(fileName.c_str());
            if(!fout) return WriteResult::ERROR_IN_WRITING_FILE;

            return writeObject(object, fout, options);
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(revisions, ReaderWriterRevisions)
