#include <osg/Group>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgDB/Options>

#include <sstream>
#include "unzip.h"

class ReaderWriterZIP : public osgDB::ReaderWriter
{
    public:

        ReaderWriterZIP()
        {
            supportsExtension("zip","Zip archive format");
        }

        virtual const char* className() const { return "ZIP Database Reader/Writer"; }

        virtual ReadResult readNode(const std::string& file, const osgDB::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osg::notify(osg::INFO)<<"ReaderWriterZIP::readNode( "<<fileName.c_str()<<" )\n";

            ReadResult rresult = ReadResult::FILE_NOT_HANDLED;

            // First open file as stream
            std::ifstream srcFileStrm(fileName.c_str(),std::ios::in|std::ios::binary);
            if (!srcFileStrm.fail())
            {
                // Now read entire zip file into stream buffer
                std::stringstream tmpStrmBuffer;
                srcFileStrm.seekg(0,std::ios_base::beg);
                tmpStrmBuffer.operator <<(srcFileStrm.rdbuf());
                srcFileStrm.close();

                // Setup appropriate options
                osg::ref_ptr<Options> local_opt = options ?
                    static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) :
                    new Options;

                // minor issue associated with database path list, as in context of zip file it
                // doesn't make sense. Need to set to empty path for other plugins to access
                local_opt->getDatabasePathList().push_front(osgDB::getFilePath(file));

                //    Now pass through to memory zip handler
                rresult = readNode(tmpStrmBuffer,local_opt);

                // Clean up options
                local_opt->getDatabasePathList().pop_front();
            }

            return rresult;
        }

        virtual ReadResult readNode(std::istream& fin,const osgDB::Options* options =NULL) const
        {
            ReadResult result = ReadResult(ReadResult::FILE_NOT_HANDLED);
            std::stringstream buffer;
    
            if (!fin.fail())
            {
                unsigned int ulzipFileLength;
                fin.seekg(0,std::ios_base::end);
                ulzipFileLength = fin.tellg();
                fin.seekg(0,std::ios_base::beg);

                // Decompress stream to standard stream
                HZIP hz;

                // Need to decouple stream content as I can't see any other way to get access to a byte array
                // containing the content in the stream. One saving grace here is that we know that the
                // stream has already been fully read in, hence no need to concern ourselves with asynchronous
                // reads.

                //void * pMemBuffer = malloc(ulzipFileLength);
                char * pMemBuffer = new char [ulzipFileLength];
                if (pMemBuffer)
                {
                    fin.read(pMemBuffer,ulzipFileLength);
                    if ((unsigned int)fin.gcount()==ulzipFileLength)
                    {
                        hz = OpenZip(pMemBuffer, ulzipFileLength, "");  SetUnzipBaseDir(hz,_T("\\"));
    
                        ZIPENTRY ze;
                        GetZipItem(hz,-1,&ze);
                        int numitems=ze.index;

                        // Initialise top level group
                        osg::ref_ptr<osg::Group> grp = new osg::Group;
                        if (grp.valid())
                        {
                            // Now loop through each file in zip
                            for (int i=0; i<numitems; i++)
                            {
                                GetZipItem(hz,i,&ze);
                                std::string StreamName = ze.name;
    
                                char *ibuf = new char[ze.unc_size];
                                if (ibuf)
                                {
                                    UnzipItem(hz,i, ibuf, ze.unc_size);
                                    buffer.write(ibuf,ze.unc_size);
                                    delete[] ibuf;
                                    // Now ready to read node //

                                    std::string file_ext = osgDB::getFileExtension(StreamName);

                                    ReaderWriter* rw = osgDB::Registry::instance()->getReaderWriterForExtension(file_ext);
                                    if (rw)
                                    {
                                        // Setup appropriate options
                                        osg::ref_ptr<Options> local_opt = options ?
                                            static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) :
                                            new Options;

                                        local_opt->setPluginStringData("STREAM_FILENAME",osgDB::getSimpleFileName(StreamName));

                                        result = rw->readNode(buffer,local_opt);
                                        if (result.validNode())
                                        {
                                            grp->addChild( result.takeNode() );
                                        }
                                    }
                                }
                            }
                            if( grp->getNumChildren() == 0 )
                                result = ReadResult(ReadResult::FILE_NOT_HANDLED);
                            else
                                result = grp.get();
                        }
                        else
                            result = ReadResult(ReadResult::FILE_NOT_HANDLED);
                    }
                    else
                        result = ReadResult(ReadResult::FILE_NOT_HANDLED);
                    delete [] pMemBuffer;
                }
                else
                    result = ReadResult(ReadResult::FILE_NOT_HANDLED);

            }
            return result;
        }

        virtual ReadResult original_readNode(const std::string& file, const osgDB::Options* options) const
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
