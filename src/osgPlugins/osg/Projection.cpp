#include <osg/Projection>
#include <osg/Matrix>

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Projection_readLocalData(Object& obj, Input& fr);
bool Projection_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_ProjectionProxy
(
    new osg::Projection,
    "Projection",
    "Object Node Group Projection",
    &Projection_readLocalData,
    &Projection_writeLocalData
);

bool Projection_readLocalData(Object& obj, Input& fr)
{
    Projection &myobj = static_cast<Projection &>(obj);
    bool iteratorAdvanced = false;    

    static Matrix s_matrix;

    if (Matrix* tmpMatrix = static_cast<Matrix*>(fr.readObjectOfType(s_matrix)))
    {
        myobj.setMatrix(*tmpMatrix);        
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Projection_writeLocalData(const Object& obj, Output& fw)
{
    const Projection& myobj = static_cast<const Projection&>(obj);

    fw.writeObject(myobj.getMatrix());

    return true;
}
