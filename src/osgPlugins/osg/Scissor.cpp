#include "osg/Scissor"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Scissor_readLocalData(Object& obj, Input& fr);
bool Scissor_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_ScissorProxy
(
    new osg::Scissor,
    "Scissor",
    "Object StateAttribute Scissor",
    &Scissor_readLocalData,
    &Scissor_writeLocalData
);


bool Scissor_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;
    int x = 0, y = 0, width = 0, height = 0;


    if (fr[0].matchWord("x") && fr[1].getInt(x))
    {
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("y") && fr[1].getInt(y))
    {
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("width") && fr[1].getInt(width))
    {
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("height") && fr[1].getInt(height))
    {
        fr+=2;
        iteratorAdvanced = true;
    }

    Scissor& scissor = static_cast<Scissor&>(obj);
    scissor.setScissor( x, y, width, height );
    return iteratorAdvanced;
}


bool Scissor_writeLocalData(const Object& obj, Output& fw)
{
    const Scissor& scissor = static_cast<const Scissor&>(obj);

    fw.indent() << "x " << scissor.x() << std::endl;
    fw.indent() << "y " << scissor.y() << std::endl;
    fw.indent() << "width " << scissor.width() << std::endl;
    fw.indent() << "height " << scissor.height() << std::endl;

    return true;
}
