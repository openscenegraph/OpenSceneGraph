#include "osg/Registry"

#include "osg/Object"
#include "osg/Image"
#include "osg/Node"
#include "osg/Group"

#include "osg/Input"
#include "osg/Output"

#ifdef __sgi
using std::string;
#endif

using namespace osg;

class DefaultReaderWriter : public ReaderWriter
{
    public:
        virtual const char* className() { return "Default OSG Reader/Writer"; }
        virtual bool acceptsExtension(const string& extension) { return extension=="osg" || extension=="OSG"; }

        virtual Object* readObject(const string& fileName) { return readNode(fileName); }
        virtual Node* readNode(const string& fileName)
        {
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
                }                                 // group->getNumChildren()==0
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

        virtual bool writeObject(Object& obj,const string& fileName)
        {
            Output fout;
            fout.open(fileName.c_str());
            if (fout)
            {
                obj.write(fout);
                fout.close();
                return true;
            }
            return false;
        }

        virtual bool writeNode(Node& node,const string& fileName)
        {
            Output fout;
            fout.open(fileName.c_str());
            if (fout)
            {

                node.write(fout);
                fout.close();
                return true;
            }
            return false;
        }

};

// now register with Registry to instantiate the above
// reader/writer.
RegisterReaderWriterProxy<DefaultReaderWriter> g_defaultReaderWriterProxy;
