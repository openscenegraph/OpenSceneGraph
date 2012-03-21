#include "osg/Billboard"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Billboard_readLocalData(Object& obj, Input& fr);
bool Billboard_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Billboard)
(
    new osg::Billboard,
    "Billboard",
    "Object Node Geode Billboard",
    &Billboard_readLocalData,
    &Billboard_writeLocalData
);

bool Billboard_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Billboard& billboard = static_cast<Billboard&>(obj);

    if (fr[0].matchWord("Mode"))
    {
        if (fr[1].matchWord("AXIAL_ROT"))
        {
            billboard.setMode(Billboard::AXIAL_ROT);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if  (fr[1].matchWord("POINT_ROT_EYE"))
        {
            billboard.setMode(Billboard::POINT_ROT_EYE);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if  (fr[1].matchWord("POINT_ROT_WORLD"))
        {
            billboard.setMode(Billboard::POINT_ROT_WORLD);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    if (fr[0].matchWord("Axis"))
    {
        float x,y,z;
        if (fr[1].getFloat(x) && fr[2].getFloat(y) && fr[3].getFloat(z))
        {
            billboard.setAxis(Vec3(x,y,z));
            fr+=4;
            iteratorAdvanced = true;
        }
    }

    if (fr[0].matchWord("Normal"))
    {
        float x,y,z;
        if (fr[1].getFloat(x) && fr[2].getFloat(y) && fr[3].getFloat(z))
        {
            billboard.setNormal(Vec3(x,y,z));
            fr+=4;
            iteratorAdvanced = true;
        }
    }


    // read the position data.
    bool matchFirst = false;
    if ((matchFirst=fr.matchSequence("Positions {")) || fr.matchSequence("Positions %i {"))
    {

        // set up coordinates.
        int entry = fr[0].getNoNestedBrackets();

        Billboard::PositionList& positionList = billboard.getPositionList();
        positionList.clear();

        if (matchFirst)
        {
            fr += 2;
        }
        else
        {
            //positionList.(capacity);
            fr += 3;
        }

        Vec3 pos;
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            if (fr[0].getFloat(pos[0]) && fr[1].getFloat(pos[1]) && fr[2].getFloat(pos[2]))
            {
                fr += 3;
                positionList.push_back(pos);
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


bool Billboard_writeLocalData(const Object& obj, Output& fw)
{

    const Billboard& billboard = static_cast<const Billboard&>(obj);

    switch(billboard.getMode())
    {
        case(Billboard::AXIAL_ROT): fw.indent() << "Mode AXIAL_ROT"<<std::endl; break;
        case(Billboard::POINT_ROT_EYE): fw.indent() << "Mode POINT_ROT_EYE"<<std::endl; break;
        case(Billboard::POINT_ROT_WORLD): fw.indent() << "Mode POINT_ROT_WORLD"<<std::endl; break;
    }

    const Vec3& axis = billboard.getAxis();
    fw.indent() << "Axis " << axis[0] << " "<<axis[1]<<" "<<axis[2]<<std::endl;

    const Vec3& normal = billboard.getNormal();
    fw.indent() << "Normal " << normal[0] << " "<<normal[1]<<" "<<normal[2]<<std::endl;

    fw.indent() << "Positions {"<<std::endl;
    fw.moveIn();


    Billboard::PositionList positionList = billboard.getPositionList();
    for(Billboard::PositionList::iterator piter = positionList.begin();
        piter != positionList.end();
        ++piter)
    {
        fw.indent() << (*piter)[0] << " "<<(*piter)[1]<<" "<<(*piter)[2]<<std::endl;
    }
    fw.moveOut();
    fw.indent() << "}"<<std::endl;

    return true;
}
