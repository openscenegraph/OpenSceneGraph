#include <osg/ColorMatrix>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include "Matrix.h"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool ColorMatrix_readLocalData(Object& obj, Input& fr);
bool ColorMatrix_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(ColorMatrix)
(
    new osg::ColorMatrix,
    "ColorMatrix",
    "Object StateAttribute ColorMatrix",
    &ColorMatrix_readLocalData,
    &ColorMatrix_writeLocalData
);


bool ColorMatrix_readLocalData(Object& obj, Input& fr)
{
    ColorMatrix& colorMatrix = static_cast<ColorMatrix&>(obj);
    return readMatrix(colorMatrix.getMatrix(), fr);
}


bool ColorMatrix_writeLocalData(const Object& obj, Output& fw)
{
    const ColorMatrix& colorMatrix = static_cast<const ColorMatrix&>(obj);
    return writeMatrix(colorMatrix.getMatrix(), fw);
}
