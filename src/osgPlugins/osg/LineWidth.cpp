#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <osg/LineWidth>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool LineWidth_readLocalData(Object& obj, Input& fr);
bool LineWidth_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_LineWidthProxy
(
    new osg::LineWidth,
    "LineWidth",
    "Object StateAttribute LineWidth",
    &LineWidth_readLocalData,
    &LineWidth_writeLocalData
);


bool LineWidth_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    LineWidth& lineWidth = static_cast<LineWidth&>(obj);

    float data;
    if (fr[0].matchWord("width") && fr[1].getFloat(data))
    {

        lineWidth.setWidth(data);
        fr+=2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool LineWidth_writeLocalData(const Object& obj, Output& fw)
{
    const LineWidth& lineWidth = static_cast<const LineWidth&>(obj);

    fw.indent() << "width " << lineWidth.getWidth() << std::endl;
    return true;
}
