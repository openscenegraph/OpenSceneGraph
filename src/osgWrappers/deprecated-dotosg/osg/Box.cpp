#include <osg/Shape>
#include <osg/Notify>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/ParameterOutput>

using namespace osg;
using namespace osgDB;


//////////////////////////////////////////////////////////////////////////////
// forward declare functions to use later.
bool Box_readLocalData(Object& obj, Input& fr);
bool Box_writeLocalData(const Object& obj, Output& fw);

//register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Box)
(
    new osg::Box,
    "Box",
    "Object Box",
    &Box_readLocalData,
    &Box_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool Box_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Box& box = static_cast<Box&>(obj);

    if (fr.matchSequence("Center %f %f %f"))
    {
        osg::Vec3 center;
        fr[1].getFloat(center.x());
        fr[2].getFloat(center.y());
        fr[3].getFloat(center.z());
        box.setCenter(center);
        fr+=4;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("HalfLengths %f %f %f"))
    {
        osg::Vec3 lenghts;
        fr[1].getFloat(lenghts.x());
        fr[2].getFloat(lenghts.y());
        fr[3].getFloat(lenghts.z());
        box.setHalfLengths(lenghts);
        fr+=4;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Rotation %f %f %f %f"))
    {
        osg::Quat rotation;
        fr[1].getFloat(rotation.x());
        fr[2].getFloat(rotation.y());
        fr[3].getFloat(rotation.z());
        fr[4].getFloat(rotation.w());
        box.setRotation(rotation);
        fr+=5;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool Box_writeLocalData(const Object& obj, Output& fw)
{
    const Box& box = static_cast<const Box&>(obj);

    fw.indent()<<"Center "<<box.getCenter()<<std::endl;
    fw.indent()<<"HalfLengths "<<box.getHalfLengths()<<std::endl;
    fw.indent()<<"Rotation "<<box.getRotation()<<std::endl;

    return true;
}
