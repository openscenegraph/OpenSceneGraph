#include <osg/ShapeDrawable>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool ShapeDrawable_readLocalData(Object& obj, Input& fr);
bool ShapeDrawable_writeLocalData(const Object& obj, Output& fw);

//register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_ShapeDrawableFuncProxy
(
    new osg::ShapeDrawable,
    "ShapeDrawable",
    "Object Drawable ShapeDrawable",
    0,
    0,
    DotOsgWrapper::READ_AND_WRITE
);

// RegisterDotOsgWrapperProxy g_ShapeDrawableFuncProxy
// (
//     new osg::ShapeDrawable,
//     "ShapeDrawable",
//     "Object Drawable ShapeDrawable",
//     &ShapeDrawable_readLocalData,
//     &ShapeDrawable_writeLocalData,
//     DotOsgWrapper::READ_AND_WRITE
// );
// 
// bool ShapeDrawable_readLocalData(Object& obj, Input& fr)
// {
//     bool iteratorAdvanced = false;
// 
//     ShapeDrawable& geom = static_cast<ShapeDrawable&>(obj);
// 
//     bool matchedFirst = false;
// 
//     return iteratorAdvanced;
// }
// 
// bool ShapeDrawable_writeLocalData(const Object& obj, Output& fw)
// {
//     const ShapeDrawable& geom = static_cast<const ShapeDrawable&>(obj);
// 
//     return true;
// }
