#include "osg/CameraView"
#include <osg/io_utils>

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool CameraView_readLocalData(Object& obj, Input& fr);
bool CameraView_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_CameraViewProxy
(
    new osg::CameraView,
    "CameraView",
    "Object Node Transform CameraView Group",
    &CameraView_readLocalData,
    &CameraView_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool CameraView_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    CameraView& cameraview = static_cast<CameraView&>(obj);

    if (fr.matchSequence("position %f %f %f"))
    {
        osg::Vec3d pos;
        fr[1].getFloat(pos[0]);
        fr[2].getFloat(pos[1]);
        fr[3].getFloat(pos[2]);
        
        cameraview.setPosition(pos);

        fr += 4;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("attitude %f %f %f %f"))
    {
        osg::Quat att;
        fr[1].getFloat(att[0]);
        fr[2].getFloat(att[1]);
        fr[3].getFloat(att[2]);
        fr[4].getFloat(att[3]);
        
        cameraview.setAttitude(att);
        
        fr += 5;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool CameraView_writeLocalData(const Object& obj, Output& fw)
{
    const CameraView& cameraview = static_cast<const CameraView&>(obj);
    
    fw.indent()<<"position "<<cameraview.getPosition()<<std::endl;
    fw.indent()<<"attitude "<<cameraview.getAttitude()<<std::endl;

    return true;
}
