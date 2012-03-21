#include "osg/LOD"
#include <osg/io_utils>

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool LOD_readLocalData(Object& obj, Input& fr);
bool LOD_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(LOD)
(
    new osg::LOD,
    "LOD",
    "Object Node LOD Group",
    &LOD_readLocalData,
    &LOD_writeLocalData
);

bool LOD_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    LOD& lod = static_cast<LOD&>(obj);

    if (fr.matchSequence("Center %f %f %f"))
    {
        Vec3 center;
        fr[1].getFloat(center[0]);
        fr[2].getFloat(center[1]);
        fr[3].getFloat(center[2]);
        lod.setCenter(center);

        iteratorAdvanced = true;
        fr+=4;
    }

    float radius;
    if (fr[0].matchWord("Radius") && fr[1].getFloat(radius))
    {
        lod.setRadius(radius);
        fr+=2;
        iteratorAdvanced = true;
    }


    if(fr[0].matchWord("RangeMode")){
      if(fr[1].matchWord("DISTANCE_FROM_EYE_POINT")){
        lod.setRangeMode(osg::LOD::DISTANCE_FROM_EYE_POINT);
      }
      else {
        lod.setRangeMode(osg::LOD::PIXEL_SIZE_ON_SCREEN);
      }
      fr+=2;
      iteratorAdvanced = true;
    }



    // For backwards compatibility with old style LOD's (pre October 2002).
    bool matchFirst = false;
    if ((matchFirst=fr.matchSequence("Ranges {")) || fr.matchSequence("Ranges %i {"))
    {

        // set up coordinates.
        int entry = fr[0].getNoNestedBrackets();
        if (matchFirst)
        {
            fr += 2;
        }
        else
        {
            fr += 3;
        }

        float minRange=0.0;
        float maxRange=0.0;
        unsigned int i=0;
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            if (fr[0].getFloat(maxRange))
            {
                if (i>0) lod.setRange(i-1,minRange,maxRange);
                ++fr;
                ++i;
                minRange = maxRange;
            }
            else
            {
                ++fr;
            }
        }

        iteratorAdvanced = true;
        ++fr;

    }

    if ((matchFirst=fr.matchSequence("RangeList {")) || fr.matchSequence("RangeList %i {"))
    {

        // set up coordinates.
        int entry = fr[0].getNoNestedBrackets();
        if (matchFirst)
        {
            fr += 2;
        }
        else
        {
            fr += 3;
        }

        float minRange=0.0;
        float maxRange=0.0;
        unsigned int i=0;
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            if (fr[0].getFloat(minRange) && fr[1].getFloat(maxRange) )
            {
                lod.setRange(i,minRange,maxRange);
                fr+=2;
                ++i;
            }
            else
            {
                ++fr;
            }
        }

        iteratorAdvanced = true;
        ++fr;

    }
    return iteratorAdvanced;
}


bool LOD_writeLocalData(const Object& obj, Output& fw)
{
    const LOD& lod = static_cast<const LOD&>(obj);

    if (lod.getCenterMode()==osg::LOD::USER_DEFINED_CENTER) fw.indent() << "Center "<< lod.getCenter() << std::endl;

    fw.indent() << "Radius "<<lod.getRadius()<<std::endl;

    if(lod.getRangeMode()==osg::LOD::DISTANCE_FROM_EYE_POINT){
      fw.indent() << "RangeMode DISTANCE_FROM_EYE_POINT"<<std::endl;
    }
    else {
      fw.indent() << "RangeMode PIXEL_SIZE_ON_SCREEN"<<std::endl;
    }

    fw.indent() << "RangeList "<<lod.getNumRanges()<<" {"<< std::endl;
    fw.moveIn();

    for(unsigned int i=0; i<lod.getNumRanges();++i)
    {
        fw.indent() << lod.getMinRange(i) << " "<<lod.getMaxRange(i)<<std::endl;
    }
    fw.moveOut();
    fw.indent() << "}"<< std::endl;

    return true;
}
