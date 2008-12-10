#include "osg/PolygonMode"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool PolygonMode_readLocalData(Object& obj, Input& fr);
bool PolygonMode_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(PolygonMode)
(
    new osg::PolygonMode,
    "PolygonMode",
    "Object StateAttribute PolygonMode",
    &PolygonMode_readLocalData,
    &PolygonMode_writeLocalData
);


bool PolygonMode_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    PolygonMode& polygonmode = static_cast<PolygonMode&>(obj);

    if (fr[0].matchWord("mode"))
    {
        PolygonMode::Face face = PolygonMode::FRONT_AND_BACK;
        bool readFace = true;
        if (fr[1].matchWord("FRONT")) face = PolygonMode::FRONT;
        else if (fr[1].matchWord("BACK")) face = PolygonMode::BACK;
        else if (fr[1].matchWord("FRONT_AND_BACK")) face = PolygonMode::FRONT_AND_BACK;
        else readFace = false;
        if (readFace)
        {
            PolygonMode::Mode mode = PolygonMode::FILL;
            bool readMode = true;
            if (fr[2].matchWord("POINT")) mode = PolygonMode::POINT;
            else if (fr[2].matchWord("LINE")) mode = PolygonMode::LINE;
            else if (fr[2].matchWord("FILL")) mode = PolygonMode::FILL;
            else readMode = false;
            if (readMode)
            {
                polygonmode.setMode(face,mode);
                fr+=3;
                iteratorAdvanced = true;
            }
        }
    }

    return iteratorAdvanced;
}


bool PolygonMode_writeLocalData(const Object& obj, Output& fw)
{
    const PolygonMode& polygonmode = static_cast<const PolygonMode&>(obj);

    if (polygonmode.getFrontAndBack())
    {
        switch(polygonmode.getMode(PolygonMode::FRONT))
        {
            case(PolygonMode::POINT): fw.indent() << "mode FRONT_AND_BACK POINT" << std::endl; break;
            case(PolygonMode::LINE):  fw.indent() << "mode FRONT_AND_BACK LINE" << std::endl; break;
            case(PolygonMode::FILL):  fw.indent() << "mode FRONT_AND_BACK FILL" << std::endl; break;
        }
    }
    else
    {
        switch(polygonmode.getMode(PolygonMode::FRONT))
        {
            case(PolygonMode::POINT): fw.indent() << "mode FRONT POINT" << std::endl; break;
            case(PolygonMode::LINE):  fw.indent() << "mode FRONT LINE" << std::endl; break;
            case(PolygonMode::FILL):  fw.indent() << "mode FRONT FILL" << std::endl; break;
        }
        switch(polygonmode.getMode(PolygonMode::BACK))
        {
            case(PolygonMode::POINT): fw.indent() << "mode BACK POINT" << std::endl; break;
            case(PolygonMode::LINE):  fw.indent() << "mode BACK LINE" << std::endl; break;
            case(PolygonMode::FILL):  fw.indent() << "mode BACK FILL" << std::endl; break;
       }
    }

    return true;
}
