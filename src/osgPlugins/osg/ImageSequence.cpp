#include "osg/ImageSequence"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool ImageSequence_readLocalData(Object& obj, Input& fr);
bool ImageSequence_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_ImageSequenceProxy
(
    new osg::ImageSequence,
    "ImageSequence",
    "Object ImageSequence",
    &ImageSequence_readLocalData,
    &ImageSequence_writeLocalData
);

bool ImageSequence_readLocalData(Object& /*obj*/, Input& /*fr*/)
{
    bool iteratorAdvanced = false;

    // Image& image = static_cast<ImageSequence&>(obj);

    // no current image reading code 
    // as it is all handled by osg::Registry::readImage() via plugins.

    return iteratorAdvanced;
}


bool ImageSequence_writeLocalData(const Object& /*obj*/, Output& /*fw*/)
{
    // const ImageSequence& image = static_cast<const ImageSequence&>(obj);

    // no current image writing code here 
    // as it is all handled by osg::Registry::writeImage() via plugins.

    return true;
}
