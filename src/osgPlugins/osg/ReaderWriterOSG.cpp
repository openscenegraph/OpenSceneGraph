#include "osg/Image"
#include "osg/Group"

#include "osgDB/FileNameUtils"
#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

class OSGReaderWriter : public ReaderWriter
{
    public:
        virtual const char* className() { return "OSG Reader/Writer"; }

        virtual bool acceptsExtension(const std::string& extension)
        {
            return equalCaseInsensitive(extension,"osg");
        }

        virtual ReadResult readObject(const std::string& fileName, const Options* opt) { return readNode(fileName,opt); }

        virtual ReadResult readNode(const std::string& fileName, const Options* opt)
        {
            std::string ext = getFileExtension(fileName);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::ifstream fin(fileName.c_str());
            if (fin)
            {
                return readNode(fin, opt);
            }
            return 0L;
                        
        }
        
        virtual ReadResult readNode(std::istream& fin, const Options*)
        {

            Input fr;
            fr.attach(&fin);

            typedef std::vector<osg::Node*> NodeList;
            NodeList nodeList;

            // load all nodes in file, placing them in a group.
            while(!fr.eof())
            {
                Node *node = fr.readNode();
                if (node) nodeList.push_back(node);
                else fr.advanceOverCurrentFieldOrBlock();
            }

            if  (nodeList.empty())
            {
                return ReadResult("No data loaded");
            }
            else if (nodeList.size()==1)
            {
                return nodeList.front();
            }
            else
            {
                Group* group = new Group;
                group->setName("import group");
                for(NodeList::iterator itr=nodeList.begin();
                    itr!=nodeList.end();
                    ++itr)
                {
                    group->addChild(*itr);
                }
                return group;
            }

        }

        virtual WriteResult writeObject(const Object& obj,const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {
            Output fout;
            fout.open(fileName.c_str());
            if (fout)
            {
                fout.writeObject(obj);
                fout.close();
                return WriteResult::FILE_SAVED;
            }
            return WriteResult("Unable to open file for output");
        }

        virtual WriteResult writeNode(const Node& node,const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {
            std::string ext = getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

            Output fout(fileName.c_str());
            if (fout)
            {
                fout.writeObject(node);
                fout.close();
                return WriteResult::FILE_SAVED;
            }
            return WriteResult("Unable to open file for output");
        }

};

// now register with Registry to instantiate the above
// reader/writer.
RegisterReaderWriterProxy<OSGReaderWriter> g_OSGReaderWriterProxy;
