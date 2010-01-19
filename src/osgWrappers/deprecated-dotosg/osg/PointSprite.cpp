#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include "osg/PointSprite"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool PointSprite_readLocalData(Object& obj, Input& fr);
bool PointSprite_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(PointSprite)
(
    new osg::PointSprite,
    "PointSprite",
    "Object StateAttribute PointSprite",
    &PointSprite_readLocalData,
    &PointSprite_writeLocalData
);


bool PointSprite_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    PointSprite& ps = static_cast<PointSprite&>(obj);

    if (fr[0].matchWord("coordOriginMode"))
    {
        if (fr[1].matchWord("UPPER_LEFT"))
        {
            ps.setCoordOriginMode(PointSprite::UPPER_LEFT);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("LOWER_LEFT"))
        {
            ps.setCoordOriginMode(PointSprite::LOWER_LEFT);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    return iteratorAdvanced;
}


bool PointSprite_writeLocalData(const Object& obj, Output& fw)
{
    const PointSprite& ps = static_cast<const PointSprite&>(obj);

    switch(ps.getCoordOriginMode())
    {
        case(PointSprite::UPPER_LEFT): fw.indent() << "coordOriginMode UPPER_LEFT" << std::endl; break;
        case(PointSprite::LOWER_LEFT): fw.indent() << "coordOriginMode LOWER_LEFT" << std::endl; break;
    }
    return true;
}
