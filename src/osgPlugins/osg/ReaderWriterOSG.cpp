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

        virtual ReadResult readNode(const std::string& fileName, const Options*)
        {
            std::string ext = getFileExtension(fileName);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

			std::ifstream fin(fileName.c_str());
            if (fin)
            {
                Input fr;
                fr.attach(&fin);

                Group* group = new Group;
                group->setName("import group");

                // load all nodes in file, placing them in a group.
                while(!fr.eof())
                {
                    Node *node = fr.readNode();
                    if (node) group->addChild(node);
                    else fr.advanceOverCurrentFieldOrBlock();
                }

                if  (group->getNumChildren()>1)
                {
                    return group;
                }
                else if (group->getNumChildren()==1)
                {
                    // only one node loaded so just return that one node,
                    // and delete the redundent group.  Note, the
                    // child must be referenced before defrencing
                    // the group so to avoid delete its children.
                    Node* node = group->getChild(0);
                    node->ref();
                    group->unref();
                    return node;
                }                // group->getNumChildren()==0
                else
                {
                    return ReadResult("No data loaded from "+fileName);
                }

            }
            else
            {
                return 0L;
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
            Output fout;
            fout.open(fileName.c_str());
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
