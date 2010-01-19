#include <osgSim/OverlayNode>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;
using namespace osgSim;

// forward declare functions to use later.
bool OverlayNode_readLocalData(Object& obj, Input& fr);
bool OverlayNode_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(g_OverlayNodeProxy)
(
    new OverlayNode,
    "OverlayNode",
    "Object Node OverlayNode Group",
    &OverlayNode_readLocalData,
    &OverlayNode_writeLocalData
);

bool OverlayNode_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    OverlayNode& es = static_cast<OverlayNode&>(obj);

    if (fr.matchSequence("technique"))
    {
        if (fr[1].matchWord("OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY"))
        {
            es.setOverlayTechnique(OverlayNode::OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY);
            iteratorAdvanced = true;
            fr+=2;
        }
        else if (fr[1].matchWord("VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY"))
        {
            es.setOverlayTechnique(OverlayNode::VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY);
            iteratorAdvanced = true;
            fr+=2;
        }
        else if (fr[1].matchWord("VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY"))
        {
            es.setOverlayTechnique(OverlayNode::VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY);
            iteratorAdvanced = true;
            fr+=2;
        }
    }

    osg::Vec4 vec4(0.0f,0.0f,0.0f,1.0f);

    if (fr[0].matchWord("clear_color") &&
        fr[1].getFloat(vec4[0]) &&
        fr[2].getFloat(vec4[1]) &&
        fr[3].getFloat(vec4[2]) &&
        fr[4].getFloat(vec4[3]))
    {
        es.setOverlayClearColor(vec4);
        fr+=5;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("texture_size_hint"))
    {
        if (fr[1].isUInt())
        {
            unsigned int value=0;
            fr[1].getUInt(value);
            es.setOverlayTextureSizeHint(value);
            iteratorAdvanced = true;
            fr+=2;
        }
    }

    if (fr[0].matchWord("texture_unit"))
    {
        if (fr[1].isUInt())
        {
            unsigned int value=0;
            fr[1].getUInt(value);
            es.setOverlayTextureUnit(value);
            iteratorAdvanced = true;
            fr+=2;
        }
    }

    if (fr[0].matchWord("subgraph"))
    {
        fr+=1;
        es.setOverlaySubgraph(fr.readNode());
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool OverlayNode_writeLocalData(const Object& obj, Output& fw)
{
    const OverlayNode& es = static_cast<const OverlayNode&>(obj);

    fw.indent() << "technique ";
    if (es.getOverlayTechnique() == OverlayNode::OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY)
    {
        fw<<"OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY"<< std::endl;
    }
    else if (es.getOverlayTechnique() == OverlayNode::VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY)
    {
        fw<<"VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY"<< std::endl;
    }
    else if (es.getOverlayTechnique() == OverlayNode::VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY)
    {
        fw<<"VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY"<< std::endl;
    }
    else
    {
        fw<<"UNKNOWN"<< std::endl;
    }

    fw.indent() << "clear_color "<<es.getOverlayClearColor()<< std::endl;
    fw.indent() << "texture_size_hint " << es.getOverlayTextureSizeHint() << std::endl;
    fw.indent() << "texture_unit " << es.getOverlayTextureUnit() << std::endl;
    fw.indent() << "subgraph " ;
    fw.writeObject(*es.getOverlaySubgraph());

    return true;
}
