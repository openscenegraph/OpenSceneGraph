#include "osg/DOFTransform"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool DOFTransform_readLocalData(Object& obj, Input& fr);
bool DOFTransform_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_DOFTransformProxy
(
    osgNew osg::DOFTransform,
    "DOFTransform",
    "Object Node Transform DOFTransform Group",
    &DOFTransform_readLocalData,
    &DOFTransform_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool DOFTransform_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    DOFTransform& transform = static_cast<DOFTransform&>(obj);

 
    return iteratorAdvanced;
}


bool DOFTransform_writeLocalData(const Object& obj, Output& fw)
{
    const DOFTransform& transform = static_cast<const DOFTransform&>(obj);

    return true;
}
