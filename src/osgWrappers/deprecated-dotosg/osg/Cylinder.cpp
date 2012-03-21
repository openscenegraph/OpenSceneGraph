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
bool Cylinder_readLocalData(Object& obj, Input& fr);
bool Cylinder_writeLocalData(const Object& obj, Output& fw);

//register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Cylinder)
(
    new osg::Cylinder,
    "Cylinder",
    "Object Cylinder",
    &Cylinder_readLocalData,
    &Cylinder_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool Cylinder_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Cylinder& cylinder = static_cast<Cylinder&>(obj);

    if (fr.matchSequence("Center %f %f %f"))
    {
        osg::Vec3 center;
        fr[1].getFloat(center.x());
        fr[2].getFloat(center.y());
        fr[3].getFloat(center.z());
        cylinder.setCenter(center);
        fr+=4;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Radius %f"))
    {
        float radius;
        fr[1].getFloat(radius);
        cylinder.setRadius(radius);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Height %f"))
    {
        float height;
        fr[1].getFloat(height);
        cylinder.setHeight(height);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Rotation %f %f %f %f"))
    {
        osg::Quat rotation;
        fr[1].getFloat(rotation.x());
        fr[2].getFloat(rotation.y());
        fr[3].getFloat(rotation.z());
        fr[4].getFloat(rotation.w());
        cylinder.setRotation(rotation);
        fr+=5;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool Cylinder_writeLocalData(const Object& obj, Output& fw)
{
    const Cylinder& cylinder = static_cast<const Cylinder&>(obj);

    fw.indent()<<"Center "<<cylinder.getCenter()<<std::endl;
    fw.indent()<<"Radius "<<cylinder.getRadius()<<std::endl;
    fw.indent()<<"Height "<<cylinder.getHeight()<<std::endl;
    fw.indent()<<"Rotation "<<cylinder.getRotation()<<std::endl;

    return true;
}
