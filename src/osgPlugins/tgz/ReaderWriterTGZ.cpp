#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <direct.h>
#else
#include <unistd.h>
#endif

using namespace osg;

class ReaderWriterTGZ : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() const { return "TGZ Database Reader/Writer"; }

        ReaderWriterTGZ()
        {
            supportsExtension("tgz","Tar gzip'd archive format");
        }

        virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
             std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            OSG_NOTICE<<"file="<<file<<std::endl;

            std::string fileName = osgDB::findDataFile( file, options );

            OSG_NOTICE<<"fileName="<<fileName<<std::endl;

            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            OSG_INFO<<   "ReaderWriterTGZ::readNode( "<<fileName.c_str()<<" )\n";

            char dirname[128];
            char command[1024];

        #if defined(_WIN32) && !defined(__CYGWIN__)
            if ( getenv("TEMP") != NULL ){
               strcpy(dirname, getenv("TEMP"));
            }else{
               //TEMP environment variable not set so pick current directory.
               strcpy(dirname, "./");
            }
            strcat(dirname, ".osgdb_tgz");
            mkdir( dirname);
            // Using tar.exe from http://www.cygwin.com/
            // tar.exe must be in your path.  (PATH environment variable).
            sprintf( command,
                "tar xfCz \"%s\" \"%s\"",
                fileName.c_str(), dirname );
        #endif

        #if defined(__linux) || defined(__CYGWIN__)
            sprintf( dirname, "/tmp/.tgz%06d", getpid());
            mkdir( dirname, 0700 );
            sprintf( command,
                "tar xfCz %s %s",
                fileName.c_str(), dirname );
        #endif
        #ifdef __sgi
            sprintf( dirname, "/tmp/.tgz%06d", getpid());
            mkdir( dirname, 0700 );
            sprintf( command,
                "cp %s %s; cd %s;"
                "gzcat %s | tar xf -",
                fileName.c_str(), dirname, dirname,
                fileName.c_str());
        #endif

            OSG_NOTICE<<"Running command '"<<command<<"'"<<std::endl;

            int result = system( command );
            if (result!=0) return ReadResult::ERROR_IN_READING_FILE;


            osg::ref_ptr<osg::Group> grp = new osg::Group;

            OSG_NOTICE<<"Done"<<std::endl;

            osg::ref_ptr<osgDB::ReaderWriter::Options> local_options = options ? static_cast<osgDB::ReaderWriter::Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new osgDB::ReaderWriter::Options;
            local_options->getDatabasePathList().push_front(dirname);

            OSG_NOTICE<<"local_options->getDatabasePathList().="<<local_options->getDatabasePathList().front()<<std::endl;
            OSG_NOTICE<<"dirname="<<dirname<<std::endl;

            // deactivate the automatic generation of images to geode's.
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
                    osg::Node *node = osgDB::readNodeFile(*itr, local_options.get());
                    grp->addChild( node );
                }
            }

            // restorre original state of the automatic generation of images to geode's.
            osgDB::Registry::instance()->setCreateNodeFromImage(prevCreateNodeFromImage);

        #if defined(_WIN32) && !defined(__CYGWIN__)
            sprintf( command, "erase /F /Q /S \"%s\"", dirname );
        #else
            sprintf( command, "rm -rf %s", dirname );
        #endif
            OSG_NOTICE<<"Running command '"<<command<<"'"<<std::endl;

            result = system( command );
            if (result!=0) return ReadResult::ERROR_IN_READING_FILE;

            if( grp->getNumChildren() == 0 )
            {
                return ReadResult::FILE_NOT_HANDLED;
            }

            return grp.get();

        }

};

// now register with sgRegistry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(tgz, ReaderWriterTGZ)
