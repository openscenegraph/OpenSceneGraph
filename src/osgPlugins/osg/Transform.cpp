#include "osg/Transform"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Transform_readLocalData(Object& obj, Input& fr);
bool Transform_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_TransformProxy
(
    new osg::Transform,
    "Transform",
    "Object Node Transform Group",
    &Transform_readLocalData,
    &Transform_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

// register old style 'DCS' read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_DCSProxy
(
    new osg::Transform,
    "DCS",
    "Object Node Group DCS",
    &Transform_readLocalData,
    NULL,
    DotOsgWrapper::READ_ONLY
);

bool Transform_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Transform& transform = static_cast<Transform&>(obj);

    static Matrix s_matrix;
    
    if (Matrix* tmpMatrix = static_cast<Matrix*>(fr.readObjectOfType(s_matrix)))
    {

        transform.setMatrix(*tmpMatrix);

        delete tmpMatrix;

        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Transform_writeLocalData(const Object& obj, Output& fw)
{
    const Transform& transform = static_cast<const Transform&>(obj);

    fw.writeObject(transform.getMatrix());

    return true;
}
