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
RegisterDotOsgWrapperProxy g_PointSpriteProxy
(
    new osg::PointSprite,
    "PointSprite",
    "Object StateAttribute PointSprite",
    &PointSprite_readLocalData,
    &PointSprite_writeLocalData
);


bool PointSprite_readLocalData(Object&, Input&)
{
    return false;
}


bool PointSprite_writeLocalData(const Object&,Output&)
{
    return true;
}
