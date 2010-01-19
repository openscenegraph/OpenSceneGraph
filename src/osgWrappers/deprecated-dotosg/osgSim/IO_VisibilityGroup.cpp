#include "osgSim/VisibilityGroup"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgSim;
using namespace osgDB;

// forward declare functions to use later.
bool VisibilityGroup_readLocalData(Object& obj, Input& fr);
bool VisibilityGroup_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_VisibilityGroupProxy
(
    new VisibilityGroup,
    "VisibilityGroup",
    "Object Node VisibilityGroup Group",
    &VisibilityGroup_readLocalData,
    &VisibilityGroup_writeLocalData
);

bool VisibilityGroup_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    VisibilityGroup& vg = static_cast<VisibilityGroup&>(obj);

    unsigned int mask = vg.getVolumeIntersectionMask();
    if (fr[0].matchWord("volumeIntersectionMask") && fr[1].getUInt(mask))
    {
        vg.setNodeMask(mask);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("segmentLength"))
    {
        if (fr[1].isFloat())
        {
            float value;
            fr[1].getFloat(value);
            vg.setSegmentLength(value);
            iteratorAdvanced = true;
            fr += 2;
        }
    }

    if (fr.matchSequence("visibilityVolume"))
    {
//        int entry = fr[0].getNoNestedBrackets();
        ++fr;
        Node* node = NULL;
        if((node=fr.readNode())!=NULL)
        {
            vg.setVisibilityVolume(node);
            iteratorAdvanced = true;
        }
    }

    return iteratorAdvanced;
}


bool VisibilityGroup_writeLocalData(const Object& obj, Output& fw)
{
    const VisibilityGroup& vg = static_cast<const VisibilityGroup&>(obj);

    fw.indent()<<"volumeIntersectionMask 0x"<<std::hex<<vg.getVolumeIntersectionMask()<<std::dec<<std::endl;
    fw.indent()<<"segmentLength "<<vg.getSegmentLength()<<std::endl;
    fw.indent()<<"visibilityVolume" <<std::endl;
    fw.moveIn();
    fw.writeObject(*vg.getVisibilityVolume());
    fw.moveOut();

    return true;
}
