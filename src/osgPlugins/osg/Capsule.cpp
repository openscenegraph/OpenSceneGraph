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
bool Capsule_readLocalData(Object& obj, Input& fr);
bool Capsule_writeLocalData(const Object& obj, Output& fw);

//register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Capsule)
(
    new osg::Capsule,
    "Capsule",
    "Object Capsule",
    &Capsule_readLocalData,
    &Capsule_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool Capsule_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Capsule& capsule = static_cast<Capsule&>(obj);

    if (fr.matchSequence("Center %f %f %f"))
    {
        osg::Vec3 center;
        fr[1].getFloat(center.x());
        fr[2].getFloat(center.y());
        fr[3].getFloat(center.z());
        capsule.setCenter(center);
        fr+=4;
        iteratorAdvanced = true;
    }
    
    if (fr.matchSequence("Radius %f"))
    {
        float radius;
        fr[1].getFloat(radius);
        capsule.setRadius(radius);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Height %f"))
    {
        float height;
        fr[1].getFloat(height);
        capsule.setHeight(height);
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
        capsule.setRotation(rotation);
        fr+=5;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool Capsule_writeLocalData(const Object& obj, Output& fw)
{
    const Capsule& capsule = static_cast<const Capsule&>(obj);

    fw.indent()<<"Center "<<capsule.getCenter()<<std::endl;
    fw.indent()<<"Radius "<<capsule.getRadius()<<std::endl;
    fw.indent()<<"Height "<<capsule.getHeight()<<std::endl;
    fw.indent()<<"Rotation "<<capsule.getRotation()<<std::endl;

    return true;
}


