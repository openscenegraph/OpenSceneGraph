#include <osg/Projection>
#include <osg/Matrix>

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

#include "Matrix.h"

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

    Matrix matrix;
    if (readMatrix(matrix,fr))
    {
        myobj.setMatrix(matrix);        
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Projection_writeLocalData(const Object& obj, Output& fw)
{
    const Projection& myobj = static_cast<const Projection&>(obj);

    writeMatrix(myobj.getMatrix(),fw);

    return true;
}
