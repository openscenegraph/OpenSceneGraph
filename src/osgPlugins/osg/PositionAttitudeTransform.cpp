#include "osg/PositionAttitudeTransform"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool PositionAttitudeTransform_readLocalData(Object& obj, Input& fr);
bool PositionAttitudeTransform_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_PositionAttitudeTransformProxy
(
    osgNew osg::PositionAttitudeTransform,
    "PositionAttitudeTransform",
    "Object Node Transform PositionAttitudeTransform Group",
    &PositionAttitudeTransform_readLocalData,
    &PositionAttitudeTransform_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool PositionAttitudeTransform_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    PositionAttitudeTransform& transform = static_cast<PositionAttitudeTransform&>(obj);

    return iteratorAdvanced;
}


bool PositionAttitudeTransform_writeLocalData(const Object& obj, Output& fw)
{
    const PositionAttitudeTransform& transform = static_cast<const PositionAttitudeTransform&>(obj);

    return true;
}
