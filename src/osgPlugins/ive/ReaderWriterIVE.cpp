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
	    try{
                // Create datainputstream.
                ive::DataInputStream* in = new ive::DataInputStream(&fin);

                // Which object is written first in the stream.
                int id = in->peekInt();

                if(id==IVEMATRIXTRANSFORM)
                {
	                osg::MatrixTransform* rootNode = new osg::MatrixTransform;
	                ((ive::MatrixTransform*)(rootNode))->read(in);
                        return rootNode;
                }
                else if(id==IVEGROUP)
                {
	                osg::Group* rootNode = new osg::Group;
	                ((ive::Group*)(rootNode))->read(in);
                        return rootNode;
                }
                else{
                    std::cout <<"Unknown class identification in file "<< id << std::endl;
                }
	    }
	    catch(ive::Exception e)
            {
	        std::cout<<"Error reading file: "<< e.getError()<<std::endl;
	        return ReadResult::FILE_NOT_HANDLED;
	    }
            return 0;
        }

        virtual WriteResult writeNode(const Node& node,const std::string& fileName, const osgDB::ReaderWriter::Options* options)
        {
            std::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
            WriteResult result = writeNode(node, fout, options);
            fout.close();
            return result;
        }
        
        virtual WriteResult writeNode(const Node& node,std::ostream& fout, const osgDB::ReaderWriter::Options*)
        {
            try
            {
                ive::DataOutputStream* out = new ive::DataOutputStream(&fout);
                // write ive file.
                if(dynamic_cast<const osg::MatrixTransform*>(&node))
                    const_cast<ive::MatrixTransform*>(static_cast<const ive::MatrixTransform*>(&node))->write(out);
                else if(dynamic_cast<const osg::Group*>(&node))
                    const_cast<ive::Group*>(static_cast<const ive::Group*>(&node))->write(out);
                else
                    std::cout<<"File must start with a MatrixTransform or Group "<<std::endl;
                fout.flush();
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
