#include <osg/BlendColor>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool BlendColor_readLocalData(Object& obj, Input& fr);
bool BlendColor_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.

REGISTER_DOTOSGWRAPPER(BlendColor)
(
    new osg::BlendColor,
    "BlendColor",
    "Object StateAttribute BlendColor",
    &BlendColor_readLocalData,
    &BlendColor_writeLocalData
);

bool BlendColor_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    BlendColor& bc = static_cast<BlendColor&>(obj);

    if (fr.matchSequence("constantColor %f %f %f %f"))
    {
        osg::Vec4 color;
        fr[1].getFloat(color[0]);
        fr[2].getFloat(color[1]);
        fr[3].getFloat(color[2]);
        fr[4].getFloat(color[3]);

        bc.setConstantColor(color);
        fr+=5;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool BlendColor_writeLocalData(const Object& obj, Output& fw)
{
    const BlendColor& bc = static_cast<const BlendColor&>(obj);

    fw.indent() << "constantColor " << bc.getConstantColor() << std::endl;

    return true;
}


