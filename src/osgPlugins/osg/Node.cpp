#include "osg/Node"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;
using namespace std;

// forward declare functions to use later.
bool Node_readLocalData(Object& obj, Input& fr);
bool Node_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_NodeProxy
(
    new osg::Node,
    "Node",
    "Object Node",
    &Node_readLocalData,
    &Node_writeLocalData
);

bool Node_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Node& node = static_cast<Node&>(obj);

    if (fr.matchSequence("name %s"))
    {
        node.setName(fr[1].getStr());
        fr+=2;
        iteratorAdvanced = true;
    }

    unsigned int mask = node.getNodeMask();
    if (fr[0].matchWord("nodeMask") && fr[1].getUInt(mask))
    {
        node.setNodeMask(mask);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("cullingActive"))
    {
        if (fr[1].matchWord("FALSE"))
        {
            node.setCullingActive(false);
            iteratorAdvanced = true;
            fr+=2;
        }
        else if (fr[1].matchWord("TRUE"))
        {
            node.setCullingActive(true);
            iteratorAdvanced = true;
            fr+=2;
        }
    }

    //     if (fr.matchSequence("user_data {"))
    //     {
    //         notify(DEBUG) << "Matched user_data {"<< std::endl;
    //         int entry = fr[0].getNoNestedBrackets();
    //         fr += 2;
    //
    //         while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
    //         {
    //             Object* object = fr.readObject();
    //             if (object) setUserData(object);
    //             notify(DEBUG) << "read "<<object<< std::endl;
    //             ++fr;
    //         }
    //         iteratorAdvanced = true;
    //     }

    while (fr.matchSequence("description {"))
    {
        int entry = fr[0].getNoNestedBrackets();
        fr += 2;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            if (fr[0].getStr()) node.addDescription(std::string(fr[0].getStr()));
            ++fr;
        }
        iteratorAdvanced = true;

    }

    while (fr.matchSequence("description %s"))
    {
        node.addDescription(fr[1].getStr());
        fr+=2;
        iteratorAdvanced = true;
    }

    static ref_ptr<StateSet> s_drawstate = new osg::StateSet;
    if (StateSet* readState = static_cast<StateSet*>(fr.readObjectOfType(*s_drawstate)))
    {
        node.setStateSet(readState);
        iteratorAdvanced = true;
    }


    static ref_ptr<NodeCallback> s_nodecallback = new osg::NodeCallback;
    while (fr.matchSequence("UpdateCallback {"))
    {
        int entry = fr[0].getNoNestedBrackets();
        fr += 2;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            NodeCallback* nodecallback = dynamic_cast<NodeCallback*>(fr.readObjectOfType(*s_nodecallback));
            if (nodecallback) {
                if (node.getUpdateCallback() == NULL) {
                    node.setUpdateCallback(nodecallback);
                } else {
                    node.getUpdateCallback()->addNestedCallback(nodecallback);
                }
            }
            else ++fr;
        }
        iteratorAdvanced = true;

    }

    while (fr.matchSequence("CullCallbacks {"))
    {
        int entry = fr[0].getNoNestedBrackets();
        fr += 2;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            NodeCallback* nodecallback = dynamic_cast<NodeCallback*>(fr.readObjectOfType(*s_nodecallback));
            if (nodecallback) {
                if (node.getCullCallback() == NULL) {
                    node.setCullCallback(nodecallback);
                } else {
                    node.getCullCallback()->addNestedCallback(nodecallback);
                }
            }
            else ++fr;
        }
        iteratorAdvanced = true;

    }

    return iteratorAdvanced;
}


bool Node_writeLocalData(const Object& obj, Output& fw)
{
    const Node& node = static_cast<const Node&>(obj);

    if (!node.getName().empty()) fw.indent() << "name "<<fw.wrapString(node.getName())<< std::endl;

    fw.indent() << "nodeMask 0x" << hex << node.getNodeMask() << dec << std::endl;

    fw.indent() << "cullingActive ";
    if (node.getCullingActive()) fw << "TRUE"<< std::endl;
    else fw << "FALSE"<< std::endl;

    //     if (_userData)
    //     {
    //         Object* object = dynamic_cast<Object*>(_userData);
    //         if (object)
    //         {
    //             fw.indent() << "user_data {"<< std::endl;
    //             fw.moveIn();
    //             object->write(fw);
    //             fw.moveOut();
    //             fw.indent() << "}"<< std::endl;
    //         }
    //     }

    if (!node.getDescriptions().empty())
    {
        if (node.getDescriptions().size()==1)
        {
            fw.indent() << "description "<<fw.wrapString(node.getDescriptions().front())<< std::endl;
        }
        else
        {
            fw.indent() << "description {"<< std::endl;
            fw.moveIn();
            for(Node::DescriptionList::const_iterator ditr=node.getDescriptions().begin();
                ditr!=node.getDescriptions().end();
                ++ditr)
            {
                fw.indent() << fw.wrapString(*ditr)<< std::endl;
            }
            fw.moveOut();
            fw.indent() << "}"<< std::endl;
        }
    }

    if (node.getStateSet())
    {
        fw.writeObject(*node.getStateSet());
    }
    
    if (node.getUpdateCallback())
    {
        fw.indent() << "UpdateCallbacks {" << std::endl;
        fw.moveIn();
        fw.writeObject(*node.getUpdateCallback());
        fw.moveOut();
        fw.indent() << "}" << std::endl;
    }

    if (node.getCullCallback())
    {
        fw.indent() << "CullCallbacks {" << std::endl;
        fw.moveIn();
        fw.writeObject(*node.getCullCallback());
        fw.moveOut();
        fw.indent() << "}" << std::endl;
    }

    return true;
}
