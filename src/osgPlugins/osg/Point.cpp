#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include "osg/Point"
#include <osg/io_utils>

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Point_readLocalData(Object& obj, Input& fr);
bool Point_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Point)
(
    new osg::Point,
    "Point",
    "Object StateAttribute Point",
    &Point_readLocalData,
    &Point_writeLocalData
);


bool Point_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Point& point = static_cast<Point&>(obj);

    float data;
    if (fr[0].matchWord("size") && fr[1].getFloat(data))
    {

        point.setSize(data);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("fade_threshold_size") && fr[1].getFloat(data))
    {

        point.setFadeThresholdSize(data);
        fr+=2;
        iteratorAdvanced = true;
    }

    Vec3 distAtten;
    if (fr[0].matchWord("distance_attenuation") &&
        fr[1].getFloat(distAtten[0]) && fr[2].getFloat(distAtten[1]) && fr[3].getFloat(distAtten[2]))
    {

        point.setDistanceAttenuation(distAtten);
        fr+=4;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Point_writeLocalData(const Object& obj, Output& fw)
{
    const Point& point = static_cast<const Point&>(obj);

    fw.indent() << "size " << point.getSize() << std::endl;
    fw.indent() << "fade_threshold_size  " << point.getFadeThresholdSize() << std::endl;
    fw.indent() << "distance_attenuation  " << point.getDistanceAttenuation() << std::endl;
    return true;
}
