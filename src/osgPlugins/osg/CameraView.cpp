#include <osg/CameraView>
#include <osg/io_utils>
#include <osg/Notify>

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

    if (fr.matchSequence("fieldOfView %f"))
    {
        double fov;
        fr[1].getFloat(fov);
        cameraview.setFieldOfView(fov);

        fr += 2;
        iteratorAdvanced = true;
    }


    if (fr.matchSequence("fieldOfViewMode %w"))
    {
        if      (fr[1].matchWord("UNCONSTRAINED")) cameraview.setFieldOfViewMode(osg::CameraView::UNCONSTRAINED);
        else if (fr[1].matchWord("HORIZONTAL")) cameraview.setFieldOfViewMode(osg::CameraView::HORIZONTAL);
        else if (fr[1].matchWord("VERTICAL")) cameraview.setFieldOfViewMode(osg::CameraView::VERTICAL);

        fr += 2;
        iteratorAdvanced = true;
    }


    if (fr.matchSequence("focalLength %f"))
    {
        double fl;
        fr[1].getFloat(fl);
        cameraview.setFocalLength(fl);

        fr += 2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool CameraView_writeLocalData(const Object& obj, Output& fw)
{
    const CameraView& cameraview = static_cast<const CameraView&>(obj);
    
    fw.indent()<<"position "<<cameraview.getPosition()<<std::endl;
    fw.indent()<<"attitude "<<cameraview.getAttitude()<<std::endl;
    
    fw.indent()<<"fieldOfView "<<cameraview.getFieldOfView()<<std::endl;
    fw.indent()<<"fieldOfViewMode ";
    switch(cameraview.getFieldOfViewMode())
    {
        case(osg::CameraView::UNCONSTRAINED): fw <<"UNCONSTRAINED"<<std::endl; break;
        case(osg::CameraView::HORIZONTAL): fw <<"HORIZONTAL"<<std::endl; break;
        case(osg::CameraView::VERTICAL): fw <<"VERTICAL"<<std::endl; break;
    }
    
    fw.indent()<<"focalLength "<<cameraview.getFocalLength()<<std::endl;

    return true;
}
