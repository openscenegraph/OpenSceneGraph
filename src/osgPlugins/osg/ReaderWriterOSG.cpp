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

        virtual Object* readObject(const std::string& fileName, const osgDB::ReaderWriter::Options* opt) { return readNode(fileName,opt); }

        virtual Node* readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {
            std::string ext = getFileExtension(fileName);
            if (!acceptsExtension(ext)) return NULL;

            ifstream fin(fileName.c_str());
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
                    return 0L;
                }

            }
            else
            {
                return 0L;
            }
        }

        virtual bool writeObject(const Object& obj,const std::string& fileName)
        {
            Output fout;
            fout.open(fileName.c_str());
            if (fout)
            {
                fout.writeObject(obj);
                fout.close();
                return true;
            }
            return false;
        }

        virtual bool writeNode(const Node& node,const std::string& fileName)
        {
            Output fout;
            fout.open(fileName.c_str());
            if (fout)
            {
                fout.writeObject(node);
                fout.close();
                return true;
            }
            return false;
        }

};

// now register with Registry to instantiate the above
// reader/writer.
RegisterReaderWriterProxy<OSGReaderWriter> g_OSGReaderWriterProxy;
