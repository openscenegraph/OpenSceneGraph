#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>
#include <osg/os_utils>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <direct.h>
#else
#include <unistd.h>
#endif

using namespace osg;

class sgReaderWriterOSGTGZ : public osgDB::ReaderWriter
{
    public:

        sgReaderWriterOSGTGZ()
        {
            supportsExtension("osgtgz","OpenSceneGraph tar gzid'd archive format");
        }

        virtual const char* className() const { return "OSGTGZ Database Reader/Writer"; }

        virtual ReadResult readObject(const std::string& filename, const osgDB::ReaderWriter::Options* options) const
        {
            return readNode(filename, options);
        }

        virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file,options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            OSG_INFO<<"sgReaderWriterOSGTGZ::readNode( "<<fileName.c_str()<<" )\n";

            char dirname[128];
            char command[1024];

        #if defined(_WIN32) && !defined(__CYGWIN__)
            sprintf( dirname, "C:/Windows/Temp/.osgdb_osgtgz");
            // note, the following C option under windows does not seem to work...
            // will pursue an better tar.exe later. RO.
            sprintf( command,
                "tar xfCz %s %s",
                fileName.c_str(), dirname );
            mkdir( dirname);
        #endif

        #if defined(__linux) || defined(__CYGWIN__)
            sprintf( dirname, "/tmp/.osg%06d", getpid());
            sprintf( command,
                "tar xfCz %s %s",
                fileName.c_str(), dirname );
            if (mkdir( dirname, 0700 )<0)
            {
                return ReadResult::ERROR_IN_READING_FILE;
            }
        #endif

        #ifdef __sgi
            sprintf( dirname, "/tmp/.osg%06d", getpid());
            sprintf( command,
                "cp %s %s; cd %s;"
                "gzcat %s | tar xf -",
                fileName.c_str(), dirname, dirname,
                fileName.c_str() );
            mkdir( dirname, 0700 );
        #endif

            if (osg_system( command ) ) {
                return ReadResult::FILE_NOT_HANDLED;
            }

            osg::ref_ptr<osg::Group> grp = new osg::Group;

            osg::ref_ptr<osgDB::ReaderWriter::Options> local_options = options ? static_cast<osgDB::ReaderWriter::Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new osgDB::ReaderWriter::Options;
            local_options->getDatabasePathList().push_front(dirname);

            osgDB::DirectoryContents contents = osgDB::getDirectoryContents(dirname);
            for(osgDB::DirectoryContents::iterator itr = contents.begin();
                itr != contents.end();
                ++itr)
            {
                std::string file_ext = osgDB::getLowerCaseFileExtension(*itr);
                if (osgDB::equalCaseInsensitive(file_ext,"osg"))
                {
                    osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFile( *itr, local_options.get() );
                    grp->addChild( node );
                }
            }

        #if defined(_WIN32) && !defined(__CYGWIN__)
            // note, is this the right command for windows?
            // is there any way of overriding the Y/N option? RO.
            sprintf( command, "erase %s", dirname );
        #else

            sprintf( command, "rm -rf %s", dirname );
        #endif
            if (osg_system( command ) ) {
                return ReadResult::FILE_NOT_HANDLED;
            }

            if( grp->getNumChildren() == 0 )
            {
                grp->unref();
                return ReadResult::FILE_NOT_HANDLED;
            }

            return grp.get();

        }

};

// now register with sgRegistry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(osgtgz, sgReaderWriterOSGTGZ)
