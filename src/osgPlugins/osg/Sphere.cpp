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
bool Sphere_readLocalData(Object& obj, Input& fr);
bool Sphere_writeLocalData(const Object& obj, Output& fw);

//register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Sphere)
(
    new osg::Sphere,
    "Sphere",
    "Object Sphere",
    &Sphere_readLocalData,
    &Sphere_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool Sphere_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Sphere& sphere = static_cast<Sphere&>(obj);

    if (fr.matchSequence("Center %f %f %f"))
    {
        osg::Vec3 center;
        fr[1].getFloat(center.x());
        fr[2].getFloat(center.y());
        fr[3].getFloat(center.z());
        sphere.setCenter(center);
        fr+=4;
        iteratorAdvanced = true;
    }
    
    if (fr.matchSequence("Radius %f"))
    {
        float radius;
        fr[1].getFloat(radius);
        sphere.setRadius(radius);
        fr+=2;
        iteratorAdvanced = true;
    }
    

    return iteratorAdvanced;
}

bool Sphere_writeLocalData(const Object& obj, Output& fw)
{
    const Sphere& sphere = static_cast<const Sphere&>(obj);
    
    fw.indent()<<"Center "<<sphere.getCenter()<<std::endl;
    fw.indent()<<"Radius "<<sphere.getRadius()<<std::endl;

    return true;
}

