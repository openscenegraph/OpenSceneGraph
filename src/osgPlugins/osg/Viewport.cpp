#include "osg/Viewport"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Viewport_readLocalData(Object& obj, Input& fr);
bool Viewport_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Viewport)
(
    new osg::Viewport,
    "Viewport",
    "Object StateAttribute Viewport",
    &Viewport_readLocalData,
    &Viewport_writeLocalData
);


bool Viewport_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;
    double x = 0, y = 0, width = 0, height = 0;

    if (fr[0].matchWord("x") && fr[1].getFloat(x))
    {
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("y") && fr[1].getFloat(y))
    {
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("width") && fr[1].getFloat(width))
    {
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("height") && fr[1].getFloat(height))
    {
        fr+=2;
        iteratorAdvanced = true;
    }

    Viewport& viewport = static_cast<Viewport&>(obj);
    viewport.setViewport( x, y, width, height );
    return iteratorAdvanced;
}


bool Viewport_writeLocalData(const Object& obj, Output& fw)
{
    const Viewport& viewport = static_cast<const Viewport&>(obj);

    fw.indent() << "x " << viewport.x() << std::endl;
    fw.indent() << "y " << viewport.y() << std::endl;
    fw.indent() << "width " << viewport.width() << std::endl;
    fw.indent() << "height " << viewport.height() << std::endl;

    return true;
}
