#include "osg/FrontFace"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool FrontFace_readLocalData(Object& obj, Input& fr);
bool FrontFace_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_FrontFaceProxy
(
    new osg::FrontFace,
    "FrontFace",
    "Object StateAttribute FrontFace",
    &FrontFace_readLocalData,
    &FrontFace_writeLocalData
);

bool FrontFace_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    FrontFace& frontface = static_cast<FrontFace&>(obj);

    if (fr[0].matchWord("mode"))
    {
        if (fr[1].matchWord("CLOCKWISE"))
        {
            frontface.setMode(FrontFace::CLOCKWISE);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("COUNTER_CLOCKWISE"))
        {
            frontface.setMode(FrontFace::COUNTER_CLOCKWISE);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    return iteratorAdvanced;
}


bool FrontFace_writeLocalData(const Object& obj, Output& fw)
{
    const FrontFace& frontface = static_cast<const FrontFace&>(obj);

    switch(frontface.getMode())
    {
        case(FrontFace::CLOCKWISE):         fw.indent() << "mode CLOCKWISE" << std::endl; break;
        case(FrontFace::COUNTER_CLOCKWISE): fw.indent() << "mode COUNTER_CLOCKWISE" << std::endl; break;
    }
    return true;
}
