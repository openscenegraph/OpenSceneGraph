#include <osg/ShapeDrawable>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool ShapeDrawable_readLocalData(Object& obj, Input& fr);
bool ShapeDrawable_writeLocalData(const Object& obj, Output& fw);

// //register the read and write functions with the osgDB::Registry.
// RegisterDotOsgWrapperProxy g_ShapeDrawableFuncProxy
// (
//     new osg::ShapeDrawable,
//     "ShapeDrawable",
//     "Object Drawable ShapeDrawable",
//     0,
//     0,
//     DotOsgWrapper::READ_AND_WRITE
// );

RegisterDotOsgWrapperProxy g_ShapeDrawableFuncProxy
(
    new osg::ShapeDrawable,
    "ShapeDrawable",
    "Object Drawable ShapeDrawable",
    &ShapeDrawable_readLocalData,
    &ShapeDrawable_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool ShapeDrawable_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    ShapeDrawable& geom = static_cast<ShapeDrawable&>(obj);

    if (fr.matchSequence("color %f %f %f %f"))
    {
        osg::Vec4 color;
        fr[1].getFloat(color[0]);
        fr[2].getFloat(color[1]);
        fr[3].getFloat(color[2]);
        fr[4].getFloat(color[3]);
    
        geom.setColor(color);
        
        fr+=5;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool ShapeDrawable_writeLocalData(const Object& obj, Output& fw)
{
    const ShapeDrawable& geom = static_cast<const ShapeDrawable&>(obj);

    fw.indent() << "color " << geom.getColor() << std::endl;

    return true;
}
