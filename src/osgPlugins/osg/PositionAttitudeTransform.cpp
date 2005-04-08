#include "osg/PositionAttitudeTransform"
#include <osg/io_utils>

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
    new osg::PositionAttitudeTransform,
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

    if (fr.matchSequence("position %f %f %f"))
    {
        osg::Vec3d pos;
        fr[1].getFloat(pos[0]);
        fr[2].getFloat(pos[1]);
        fr[3].getFloat(pos[2]);
        
        transform.setPosition(pos);

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
        
        transform.setAttitude(att);
        
        fr += 5;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("scale %f %f %f"))
    {
        osg::Vec3d scale;
        fr[1].getFloat(scale[0]);
        fr[2].getFloat(scale[1]);
        fr[3].getFloat(scale[2]);
        
        transform.setScale(scale);

        fr += 4;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("pivotPoint %f %f %f"))
    {
        osg::Vec3d pivot;
        fr[1].getFloat(pivot[0]);
        fr[2].getFloat(pivot[1]);
        fr[3].getFloat(pivot[2]);
        
        transform.setPivotPoint(pivot);
        
        fr += 4;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool PositionAttitudeTransform_writeLocalData(const Object& obj, Output& fw)
{
    const PositionAttitudeTransform& transform = static_cast<const PositionAttitudeTransform&>(obj);
    
    fw.indent()<<"position "<<transform.getPosition()<<std::endl;
    fw.indent()<<"attitude "<<transform.getAttitude()<<std::endl;
    fw.indent()<<"scale "<<transform.getScale()<<std::endl;
    fw.indent()<<"pivotPoint "<<transform.getPivotPoint()<<std::endl;


    return true;
}
