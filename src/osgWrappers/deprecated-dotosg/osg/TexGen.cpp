#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include "osg/TexGen"
#include "osg/io_utils"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

#include <string.h>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool TexGen_readLocalData(Object& obj, Input& fr);
bool TexGen_writeLocalData(const Object& obj, Output& fw);
bool TexGen_matchModeStr(const char* str,TexGen::Mode& mode);
const char* TexGen_getModeStr(TexGen::Mode mode);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(TexGen)
(
    new osg::TexGen,
    "TexGen",
    "Object StateAttribute TexGen",
    &TexGen_readLocalData,
    &TexGen_writeLocalData
);


bool TexGen_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    TexGen& texgen = static_cast<TexGen&>(obj);

    TexGen::Mode mode;

    if (fr[0].matchWord("mode") && TexGen_matchModeStr(fr[1].getStr(),mode))
    {
        texgen.setMode(mode);
        fr+=2;
        iteratorAdvanced = true;
    }

    osg::Plane plane;
    if (fr[0].matchWord("plane_s"))
    {
        if (fr[1].getFloat(plane[0]) && fr[2].getFloat(plane[1]) &&
            fr[3].getFloat(plane[2]) && fr[4].getFloat(plane[3]))
        {
            texgen.setPlane(TexGen::S,plane);
            fr+=5;
            iteratorAdvanced = true;
        }
    }
    if (fr[0].matchWord("plane_t"))
    {
        if (fr[1].getFloat(plane[0]) && fr[2].getFloat(plane[1]) &&
            fr[3].getFloat(plane[2]) && fr[4].getFloat(plane[3]))
        {
            texgen.setPlane(TexGen::T,plane);
            fr+=5;
            iteratorAdvanced = true;
        }
    }
    if (fr[0].matchWord("plane_r"))
    {
        if (fr[1].getFloat(plane[0]) && fr[2].getFloat(plane[1]) &&
            fr[3].getFloat(plane[2]) && fr[4].getFloat(plane[3]))
        {
            texgen.setPlane(TexGen::R,plane);
            fr+=5;
            iteratorAdvanced = true;
        }
    }
    if (fr[0].matchWord("plane_q"))
    {
        if (fr[1].getFloat(plane[0]) && fr[2].getFloat(plane[1]) &&
            fr[3].getFloat(plane[2]) && fr[4].getFloat(plane[3]))
        {
            texgen.setPlane(TexGen::Q,plane);
            fr+=5;
            iteratorAdvanced = true;
        }
    }

    return iteratorAdvanced;
}


bool TexGen_matchModeStr(const char* str,TexGen::Mode& mode)
{
    if (strcmp(str,"EYE_LINEAR")==0) mode = TexGen::EYE_LINEAR;
    else if (strcmp(str,"OBJECT_LINEAR")==0) mode = TexGen::OBJECT_LINEAR;
    else if (strcmp(str,"SPHERE_MAP")==0) mode = TexGen::SPHERE_MAP;
    else if (strcmp(str,"NORMAL_MAP")==0) mode = TexGen::NORMAL_MAP;
    else if (strcmp(str,"REFLECTION_MAP")==0) mode = TexGen::REFLECTION_MAP;
    else return false;
    return true;
}


const char* TexGen_getModeStr(TexGen::Mode mode)
{
    switch(mode)
    {
        case(TexGen::EYE_LINEAR): return "EYE_LINEAR";
        case(TexGen::OBJECT_LINEAR): return "OBJECT_LINEAR";
        case(TexGen::SPHERE_MAP): return "SPHERE_MAP";
        case(TexGen::NORMAL_MAP): return "NORMAL_MAP";
        case(TexGen::REFLECTION_MAP): return "REFLECTION_MAP";
    }
    return "";
}


bool TexGen_writeLocalData(const Object& obj, Output& fw)
{
    const TexGen& texgen = static_cast<const TexGen&>(obj);

    fw.indent() << "mode " << TexGen_getModeStr(texgen.getMode()) << std::endl;
    if (texgen.getMode() == TexGen::OBJECT_LINEAR || texgen.getMode() == TexGen::EYE_LINEAR)
    {
        fw.indent() << "plane_s " << texgen.getPlane(TexGen::S) << std::endl;
        fw.indent() << "plane_t " << texgen.getPlane(TexGen::T) << std::endl;
        fw.indent() << "plane_r " << texgen.getPlane(TexGen::R) << std::endl;
        fw.indent() << "plane_q " << texgen.getPlane(TexGen::Q) << std::endl;
    }

    return true;
}
