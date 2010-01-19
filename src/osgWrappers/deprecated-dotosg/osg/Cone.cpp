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
bool Cone_readLocalData(Object& obj, Input& fr);
bool Cone_writeLocalData(const Object& obj, Output& fw);

//register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Cone)
(
    new osg::Cone,
    "Cone",
    "Object Cone",
    &Cone_readLocalData,
    &Cone_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool Cone_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Cone& cone = static_cast<Cone&>(obj);

    if (fr.matchSequence("Center %f %f %f"))
    {
        osg::Vec3 center;
        fr[1].getFloat(center.x());
        fr[2].getFloat(center.y());
        fr[3].getFloat(center.z());
        cone.setCenter(center);
        fr+=4;
        iteratorAdvanced = true;
    }
    
    if (fr.matchSequence("Radius %f"))
    {
        float radius;
        fr[1].getFloat(radius);
        cone.setRadius(radius);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Height %f"))
    {
        float height;
        fr[1].getFloat(height);
        cone.setHeight(height);
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
        cone.setRotation(rotation);
        fr+=5;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool Cone_writeLocalData(const Object& obj, Output& fw)
{
    const Cone& cone = static_cast<const Cone&>(obj);

    fw.indent()<<"Center "<<cone.getCenter()<<std::endl;
    fw.indent()<<"Radius "<<cone.getRadius()<<std::endl;
    fw.indent()<<"Height "<<cone.getHeight()<<std::endl;
    fw.indent()<<"Rotation "<<cone.getRotation()<<std::endl;

    return true;
}
