#include <osg/ClearNode>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool ClearNode_readLocalData(Object& obj, Input& fr);
bool ClearNode_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.

REGISTER_DOTOSGWRAPPER(EarthSky)
(
    new osg::ClearNode,
    "EarthSky",
    "Object Node EarthSky Group",
    &ClearNode_readLocalData,
    &ClearNode_writeLocalData
);

REGISTER_DOTOSGWRAPPER(ClearNode)
(
    new osg::ClearNode,
    "ClearNode",
    "Object Node ClearNode Group",
    &ClearNode_readLocalData,
    &ClearNode_writeLocalData
);

bool ClearNode_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    ClearNode& es = static_cast<ClearNode&>(obj);

    if (fr.matchSequence("requiresClear"))
    {
        if (fr[1].matchWord("TRUE"))
        {
            es.setRequiresClear(true);
            iteratorAdvanced = true;
            fr+=2;
        }
        else if (fr[1].matchWord("FALSE"))
        {
            es.setRequiresClear(false);
            iteratorAdvanced = true;
            fr+=2;
        }
    }

    osg::Vec4 vec4(0.0f,0.0f,0.0f,1.0f);

    if (fr[0].matchWord("clearColor") &&
        fr[1].getFloat(vec4[0]) &&
        fr[2].getFloat(vec4[1]) &&
        fr[3].getFloat(vec4[2]) &&
        fr[4].getFloat(vec4[3]))
    {
        es.setClearColor(vec4);
        fr+=5;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("clearMask"))
    {
        if (fr[1].isUInt())
        {
            unsigned int value=0;
            fr[1].getUInt(value);
            es.setClearMask(static_cast<GLbitfield>(value));
            iteratorAdvanced = true;
            fr+=2;
        }
    }

    return iteratorAdvanced;
}


bool ClearNode_writeLocalData(const Object& obj, Output& fw)
{
    const ClearNode& es = static_cast<const ClearNode&>(obj);

    fw.indent() << "requiresClear ";
    if (es.getRequiresClear())
    {
        fw<<"TRUE"<< std::endl;
    }
    else
    {
        fw<<"FALSE"<< std::endl;
    }

    fw.indent() << "clearColor "<<es.getClearColor()<< std::endl;
    fw.indent() << "clearMask "<<static_cast<unsigned int>(es.getClearMask())<< std::endl;

    return true;
}
