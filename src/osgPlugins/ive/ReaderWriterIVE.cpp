#include "MatrixTransform.h"
#include "Group.h"

#include <osg/Notify>

#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

using namespace osg;
using namespace osgDB;

class IVEReaderWriter : public ReaderWriter
{
    public:
        virtual const char* className() { return "IVE Reader/Writer"; }

        virtual bool acceptsExtension(const std::string& extension)
        {
            return equalCaseInsensitive(extension,"ive");
        }

        virtual ReadResult readNode(const std::string& fileName, const Options* options)
        {
            std::string ext = getFileExtension(fileName);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;
    
            std::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
            return readNode(istream,options);
        }
        
        virtual ReadResult readNode(std::istream& fin, const Options*)
        {
        #define IVE_CATCH_EXCEPTIONS
        #ifdef IVE_CATCH_EXCEPTIONS
        try{
        #endif
                // Create datainputstream.
                ive::DataInputStream in(&fin);

                return in.readNode();
        #ifdef IVE_CATCH_EXCEPTIONS
        }
        catch(ive::Exception e)
            {
            std::cout<<"Error reading file: "<< e.getError()<<std::endl;
            return ReadResult::FILE_NOT_HANDLED;
        }
            return 0;
        #endif
        }

        virtual WriteResult writeNode(const Node& node,const std::string& fileName, const osgDB::ReaderWriter::Options* options)
        {
            std::string ext = getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

            std::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
            WriteResult result = writeNode(node, fout, options);
            fout.close();
            return result;
        }
        
        virtual WriteResult writeNode(const Node& node,std::ostream& fout, const osgDB::ReaderWriter::Options* options)
        {
            try
            {
                ive::DataOutputStream out(&fout);

                if (options)
                {
                    out.setIncludeImageData(options->getOptionString().find("noTexturesInIVEFile")==std::string::npos);
                    osg::notify(osg::DEBUG_INFO) << "ive::DataOutpouStream.setIncludeImageData()=" << out.getIncludeImageData() << std::endl;
                }
                
                out.writeNode(const_cast<osg::Node*>(&node));
                return WriteResult::FILE_SAVED;
            }
            catch(ive::Exception e)
            {
            osg::notify(osg::WARN)<<"Error parsing OSG file: "<< e.getError() << std::endl;
            }
            return WriteResult::FILE_NOT_HANDLED;

        }

};

// now register with Registry to instantiate the above
// reader/writer.
RegisterReaderWriterProxy<IVEReaderWriter> g_IVEReaderWriterProxy;
