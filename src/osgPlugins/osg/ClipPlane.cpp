#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <osg/ClipPlane>
#include <osg/io_utils>

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool ClipPlane_readLocalData(Object& obj, Input& fr);
bool ClipPlane_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_ClipPlaneProxy
(
    new osg::ClipPlane,
    "ClipPlane",
    "Object StateAttribute ClipPlane",
    &ClipPlane_readLocalData,
    &ClipPlane_writeLocalData
);


bool ClipPlane_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    ClipPlane& clipplane = static_cast<ClipPlane&>(obj);

    if (fr.matchSequence("clipPlaneNum %i"))
    {
        unsigned int num;
        fr[1].getUInt(num);
        clipplane.setClipPlaneNum(num);

        fr+=2;
        iteratorAdvanced = true;
    }


    if (fr.matchSequence("plane %f %f %f %f"))
    {
        double plane[4];
        fr[1].getFloat(plane[0]);
        fr[2].getFloat(plane[1]);
        fr[3].getFloat(plane[2]);
        fr[4].getFloat(plane[3]);
        clipplane.setClipPlane(plane[0],plane[1],plane[2],plane[3]);

        fr+=5;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool ClipPlane_writeLocalData(const Object& obj,Output& fw)
{
    const ClipPlane& clipplane = static_cast<const ClipPlane&>(obj);

    fw.indent() << "clipPlaneNum " << clipplane.getClipPlaneNum() <<std::endl;

    fw.indent() << "plane " << clipplane.getClipPlane()<< std::endl;
    return true;
}

