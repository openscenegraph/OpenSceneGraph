#include "osg/Image"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Image_readLocalData(Object& obj, Input& fr);
bool Image_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_ImageFuncProxy
(
    new osg::Image,
    "Image",
    "Object Image",
    &Image_readLocalData,
    &Image_writeLocalData
);

bool Image_readLocalData(Object& /*obj*/, Input& /*fr*/)
{
    bool iteratorAdvanced = false;

    // Image& image = static_cast<Image&>(obj);

    // no current image reading code 
    // as it is all handled by osg::Registry::readImage() via plugins.

    return iteratorAdvanced;
}


bool Image_writeLocalData(const Object& /*obj*/, Output& /*fw*/)
{
    // const Image& image = static_cast<const Image&>(obj);

    // no current image writing code here 
    // as it is all handled by osg::Registry::writeImage() via plugins.

    return true;
}
