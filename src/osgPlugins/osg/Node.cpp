#include "osg/Node"
#include "osg/io_utils"

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
REGISTER_DOTOSGWRAPPER(Node)
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
        if (fr[1].getStr()) node.addDescription(fr[1].getStr());
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

    while (fr.matchSequence("EventCallback {"))
    {
        int entry = fr[0].getNoNestedBrackets();
        fr += 2;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            NodeCallback* nodecallback = dynamic_cast<NodeCallback*>(fr.readObjectOfType(*s_nodecallback));
            if (nodecallback) {
                if (node.getEventCallback() == NULL) {
                    node.setEventCallback(nodecallback);
                } else {
                    node.getEventCallback()->addNestedCallback(nodecallback);
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

    if (fr.matchSequence("initialBound %f %f %f %f"))
    {
        BoundingSphere bs;
        fr[1].getFloat(bs.center().x());
        fr[2].getFloat(bs.center().y());
        fr[3].getFloat(bs.center().z());
        fr[4].getFloat(bs.radius());
        node.setInitialBound(bs);
        fr += 5;
        iteratorAdvanced = true;
    }

    while (fr.matchSequence("ComputeBoundingSphereCallback {"))
    {
        int entry = fr[0].getNoNestedBrackets();
        fr += 2;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            Node::ComputeBoundingSphereCallback* callback = dynamic_cast<Node::ComputeBoundingSphereCallback*>(fr.readObjectOfType(type_wrapper<Node::ComputeBoundingSphereCallback>()));
            if (callback) {
                node.setComputeBoundingSphereCallback(callback);
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

    fw.indent() << "nodeMask 0x" << hex << node.getNodeMask() << dec << std::endl;

    fw.indent() << "cullingActive ";
    if (node.getCullingActive()) fw << "TRUE"<< std::endl;
    else fw << "FALSE"<< std::endl;


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

    if (node.getEventCallback())
    {
        fw.indent() << "EventCallbacks {" << std::endl;
        fw.moveIn();
        fw.writeObject(*node.getEventCallback());
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

    if (node.getInitialBound().valid())
    {
        const osg::BoundingSphere& bs = node.getInitialBound();
        fw.indent()<<"initialBound "<<bs.center()<<" "<<bs.radius()<<std::endl;
    }
    
    if (node.getComputeBoundingSphereCallback())
    {
        fw.indent() << "ComputeBoundingSphereCallback {" << std::endl;
        fw.moveIn();
        fw.writeObject(*node.getComputeBoundingSphereCallback());
        fw.moveOut();
        fw.indent() << "}" << std::endl;
    }

    return true;
}
