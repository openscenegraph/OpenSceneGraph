#include "osg/TexGenNode"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool TexGenNode_readLocalData(Object& obj, Input& fr);
bool TexGenNode_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_TexGenNodeProxy
(
    new osg::TexGenNode,
    "TexGenNode",
    "Object Node TexGenNode Group",
    &TexGenNode_readLocalData,
    &TexGenNode_writeLocalData
);

bool TexGenNode_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    TexGenNode& texGenNode = static_cast<TexGenNode&>(obj);

    unsigned int textureUnit = 0;
    if (fr[0].matchWord("TextureUnit") && fr[1].getUInt(textureUnit))
    {

        texGenNode.setTextureUnit(textureUnit);
        
        fr+=2;
        iteratorAdvanced = true;
    }


    osg::ref_ptr<StateAttribute> sa=0;
    while((sa=fr.readStateAttribute())!=0)
    {
        TexGen* texgen = dynamic_cast<TexGen*>(sa.get());
        if (texgen) texGenNode.setTexGen(texgen);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool TexGenNode_writeLocalData(const Object& obj, Output& fw)
{
    const TexGenNode& texGenNode = static_cast<const TexGenNode&>(obj);

    fw.indent()<<"TextureUnit "<<texGenNode.getTextureUnit()<<std::endl;

    if (texGenNode.getTexGen())
    {
        fw.writeObject(*texGenNode.getTexGen());
    }

    return true;
}
