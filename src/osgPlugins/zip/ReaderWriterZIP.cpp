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

#if defined(WIN32) && !defined(__CYGWIN__)
#include <direct.h>
#else
#include <unistd.h>
#endif

class ReaderWriterZIP : public osgDB::ReaderWriter
{
    public:
    
        ReaderWriterZIP()
        {
            supportsExtension("zip","Zip archive format");
        }
        
        virtual const char* className() const { return "ZIP Database Reader/Writer"; }

        virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {

            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osg::notify(osg::INFO)<<"ReaderWriterZIP::readNode( "<<fileName.c_str()<<" )\n";

            char dirname[128];
            char command[1024];

        #if defined(WIN32) && !defined(__CYGWIN__)
            if ( getenv("TEMP") != NULL ){
               strcpy(dirname, getenv("TEMP"));
            }else{
               //TEMP environment variable not set so pick current directory.
               strcpy(dirname, "./");
            }
            strcat(dirname, "\\.osgdb_zip");

            mkdir(dirname);
            // Using unzip.exe from http://www.info-zip.org/pub/infozip/UnZip.html
            // unzip.exe must be in your path.  (PATH environment variable).

            // OR - WinRAR

            // Checking for WinRAR
            std::string winrar = std::string( getenv( "ProgramFiles" ) ) + "/WinRAR/winrar.exe";
            if ( osgDB::fileExists(winrar) ) {
                sprintf( command,
                    "%s x -o+ \"%s\" \"%s\"", winrar.c_str(),
                    fileName.c_str(), dirname);
            } else {
                sprintf( command,
                    "unzip -o -qq \"%s\" -d \"%s\"",
                    fileName.c_str(), dirname);
            }

        #else
            sprintf( dirname, "/tmp/.zip%06d", getpid());
            mkdir( dirname, 0700 );

            sprintf( command,
                "unzip %s -d %s",
                fileName.c_str(), dirname);

        #endif

            osg::notify(osg::INFO)<<"Running command '"<<command<<"'"<<std::endl;
            if ( system( command ) ) {
                return ReadResult::FILE_NOT_HANDLED;
            }

            osg::ref_ptr<osg::Group> grp = new osg::Group;
 
            osg::ref_ptr<osgDB::ReaderWriter::Options> local_options = options ? static_cast<osgDB::ReaderWriter::Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new osgDB::ReaderWriter::Options;
            local_options->getDatabasePathList().push_front(dirname);

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
                    osg::Node *node = osgDB::readNodeFile( *itr, local_options.get() );
                    grp->addChild( node );
                }
            }

            osgDB::Registry::instance()->setCreateNodeFromImage(prevCreateNodeFromImage);

        #if defined(WIN32) && !defined(__CYGWIN__)
            // note, is this the right command for windows?
            // is there any way of overiding the Y/N option? RO.
            sprintf( command, "erase /S /Q \"%s\"", dirname );
            int result = system( command );
        #else

            sprintf( command, "rm -rf %s", dirname );
            int result = system( command );
        #endif
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
REGISTER_OSGPLUGIN(zip, ReaderWriterZIP)
