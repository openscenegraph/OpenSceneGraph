#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

class ReaderWriterZIP : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() { return "ZIP Database Reader/Writer"; }

        virtual bool acceptsExtension(const std::string& extension)
        {
            return osgDB::equalCaseInsensitive(extension,"zip");
        }

        virtual osg::Node* readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {

            std::string ext = osgDB::getLowerCaseFileExtension(fileName);
            if (!acceptsExtension(ext)) return NULL;

            osg::notify(osg::INFO)<<"ReaderWriterZIP::readNode( "<<fileName.c_str()<<" )\n";

            char dirname[128];
            char command[1024];

        #ifdef _WIN32
            strcpy(dirname, "C:/Windows/Temp/.osgdb_zip");
            mkdir(dirname);
            sprintf( command,
                "unzip %s -d %s",
                fileName.c_str(), dirname);

            system( command );

        #else
            sprintf( dirname, "/tmp/.zip%06d", getpid());
            mkdir( dirname, 0700 );

            sprintf( command,
                "unzip %s -d %s",
                fileName.c_str(), dirname);

            system( command );
        #endif

            osg::Group *grp = new osg::Group;
            osgDB::setFilePath( dirname );

            bool prevCreateNodeFromImage = osgDB::Registry::instance()->getCreateNodeFromImage();
            osgDB::Registry::instance()->setCreateNodeFromImage(false);

            osgDB::DirectoryContents contents = osgDB::getDirectoryContents(dirname);
            for(osgDB::DirectoryContents::iterator itr = contents.begin();
                itr != contents.end();
                ++itr)
            {
                std::string file_ext = osgDB::getFileExtension(*itr);
                if (!acceptsExtension(file_ext) && 
                    *itr!=std::string(".") && 
                    *itr!=std::string(".."))
                {
                    osg::Node *node = osgDB::readNodeFile( *itr );
                    grp->addChild( node );
                }
            }

            osgDB::Registry::instance()->setCreateNodeFromImage(prevCreateNodeFromImage);

        #ifdef _WIN32
            // note, is this the right command for windows?
            // is there any way of overiding the Y/N option? RO.
            sprintf( command, "erase %s", dirname );
            system( command );
        #else

            sprintf( command, "rm -rf %s", dirname );
            system( command );
        #endif

            if( grp->getNumChildren() == 0 )
            {
                grp->unref();
                return NULL;
            }

            return grp;
        }

};

// now register with sgRegistry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterZIP> g_readerWriter_ZIP_Proxy;
