#include "osg/Object"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
// bool Object_readLocalData(Object& obj, Input& fr);
// bool Object_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
// note, Object doesn't currently require any read and write.
RegisterDotOsgWrapperProxy g_ObjectProxy
(
    /*new osg::Object*/NULL,
    "Object",
    "Object",
    NULL,
    NULL
);
