#include <osg/CameraNode>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include "Matrix.h"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool CameraNode_readLocalData(Object& obj, Input& fr);
bool CameraNode_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_CameraNodeProxy
(
    new osg::CameraNode,
    "CameraNode",
    "Object Node Transform CameraNode Group",
    &CameraNode_readLocalData,
    &CameraNode_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool CameraNode_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    CameraNode& camera = static_cast<CameraNode&>(obj);

    if (fr[0].matchWord("Type"))
    {
        if (fr[1].matchWord("DYNAMIC"))
        {
            camera.setDataVariance(osg::Object::DYNAMIC);
            fr +=2 ;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("STATIC"))
        {
            camera.setDataVariance(osg::Object::STATIC);
            fr +=2 ;
            iteratorAdvanced = true;
        }
        
    }
    
    Matrix matrix; 
    if (readMatrix(matrix,fr,"ProjectionMatrix"))
    {
        camera.setProjectionMatrix(matrix);
        iteratorAdvanced = true;
    }

    if (readMatrix(matrix,fr,"ViewMatrix"))
    {
        camera.setViewMatrix(matrix);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool CameraNode_writeLocalData(const Object& obj, Output& fw)
{
    const CameraNode& camera = static_cast<const CameraNode&>(obj);

    writeMatrix(camera.getProjectionMatrix(),fw,"ProjectionMatrix");
    writeMatrix(camera.getViewMatrix(),fw,"ViewMatrix");

    return true;
}
