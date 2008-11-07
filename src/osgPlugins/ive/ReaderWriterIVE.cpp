#include "MatrixTransform.h"
#include "Group.h"

#include <osg/Notify>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>
#include <osgDB/Registry>

using namespace osg;
using namespace osgDB;

class ReaderWriterIVE : public ReaderWriter
{
    public:
    
        ReaderWriterIVE()
        {
            supportsExtension("ive","OpenSceneGraph native binary format");

            supportsOption("noTexturesInIVEFile","Export option");
            supportsOption("includeImageFileInIVEFile","Export option");
            supportsOption("compressImageData","Export option");
            supportsOption("inlineExternalReferencesInIVEFile","Export option");
            supportsOption("noWriteExternalReferenceFiles","Export option");
            supportsOption("useOriginalExternalReferences","Export option");
            supportsOption("TerrainMaximumErrorToSizeRatio=value","Export option that controls error matric used to determine terrain HieghtField storage precision.");
            supportsOption("noLoadExternalReferenceFiles","Import option");
        }
    
        virtual const char* className() const { return "IVE Reader/Writer"; }

        virtual bool acceptsExtension(const std::string& extension) const
        {
            return equalCaseInsensitive(extension,"ive");
        }

        virtual ReadResult readObject(const std::string& file, const Options* options) const
        {
            return readNode(file, options);
        }

        virtual ReadResult readImage(const std::string& file, const Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;
            std::string fileName = osgDB::findDataFile(file, options);
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            // code for setting up the database path so that internally referenced files are searched for on relative paths.
            osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
            local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));

            osgDB::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
            return readImage(istream, local_opt.get());
        }
        
        virtual ReadResult readNode(const std::string& file, const Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            // code for setting up the database path so that internally referenced file are searched for on relative paths. 
            osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
            local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));
            
            osgDB::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
            return readNode(istream,local_opt.get());
        }
        
        virtual ReadResult readObject(std::istream& fin, const Options* options) const
        {
            return readNode(fin, options);
        }

        virtual ReadResult readImage(std::istream& fin, const Options* options) const
        {
            try{
                ive::DataInputStream in(&fin, options);
                return in.readImage(ive::IMAGE_INCLUDE_DATA);
            }
            catch(ive::Exception e)
            {
                return e.getError();
            }
        }
        
        virtual ReadResult readNode(std::istream& fin, const Options* options) const
        {
            try{
                // Create datainputstream.
                ive::DataInputStream in(&fin, options);

                return in.readNode();
            }
            catch(ive::Exception e)
            {
                return e.getError();
            }
        }

        

        virtual WriteResult writeObject(const Object& object,const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            const Node* node = dynamic_cast<const Node*>(&object);
            if (node) return writeNode( *node, fileName, options );
            const Image* image = dynamic_cast<const Image*>(&object);
            if (image) return writeImage(*image, fileName, options);
            return WriteResult::FILE_NOT_HANDLED;
        }

        virtual WriteResult writeImage(const Image& image,const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;
            // code for setting up the database path so that internally referenced file are searched for on relative paths. 
            osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
            if(local_opt->getDatabasePathList().empty())
                local_opt->setDatabasePath(osgDB::getFilePath(fileName));
            osgDB::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
            if (!fout) return WriteResult::ERROR_IN_WRITING_FILE;
            WriteResult result = writeImage(image, fout, local_opt.get());
            fout.close();
            return result;
        }

        virtual WriteResult writeNode(const Node& node,const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

            // code for setting up the database path so that internally referenced file are searched for on relative paths. 
            osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
            if(local_opt->getDatabasePathList().empty())
                local_opt->setDatabasePath(osgDB::getFilePath(fileName));

            osgDB::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
            if (!fout) return WriteResult::ERROR_IN_WRITING_FILE;
    
            WriteResult result = writeNode(node, fout, local_opt.get());
            fout.close();
            return result;
        }
        
        virtual WriteResult writeObject(const Object& object,std::ostream& fout, const osgDB::ReaderWriter::Options* options) const
        {
            const Node* node = dynamic_cast<const Node*>(&object);
            if (node) return writeNode( *node, fout, options );
            const Image* image = dynamic_cast<const Image*>(&object);
            if (image) return writeImage(*image, fout, options);
            return WriteResult::FILE_NOT_HANDLED;
        }

        virtual WriteResult writeImage(const Image& image,std::ostream& fout, const osgDB::ReaderWriter::Options* options) const
        {
            try
            {
                ive::DataOutputStream out(&fout, options);
                out.writeImage(ive::IMAGE_INCLUDE_DATA, const_cast<osg::Image*>(&image));
                if (fout.fail()) return WriteResult::ERROR_IN_WRITING_FILE;
                return WriteResult::FILE_SAVED;
            }
            catch(ive::Exception e)
            {
                osg::notify(osg::WARN)<<"Error writing IVE image: "<< e.getError() << std::endl;
            }
            return WriteResult::FILE_NOT_HANDLED;
        }

        virtual WriteResult writeNode(const Node& node,std::ostream& fout, const osgDB::ReaderWriter::Options* options) const
        {
            try
            {
                ive::DataOutputStream out(&fout, options);

                out.writeNode(const_cast<osg::Node*>(&node));

                if ( fout.fail() ) return WriteResult::ERROR_IN_WRITING_FILE;

                return WriteResult::FILE_SAVED;
            }
            catch(ive::Exception e)
            {
                osg::notify(osg::WARN)<<"Error writing IVE file: "<< e.getError() << std::endl;
            }
            return WriteResult::FILE_NOT_HANDLED;

        }

};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(ive, ReaderWriterIVE)
