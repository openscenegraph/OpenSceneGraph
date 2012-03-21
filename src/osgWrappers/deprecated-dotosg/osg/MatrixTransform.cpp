#include <osg/MatrixTransform>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include "Matrix.h"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool MatrixTransform_readLocalData(Object& obj, Input& fr);
bool MatrixTransform_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(MatrixTransform)
(
    new osg::MatrixTransform,
    "MatrixTransform",
    "Object Node Transform MatrixTransform Group",
    &MatrixTransform_readLocalData,
    &MatrixTransform_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

// register old style 'DCS' read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(DCS)
(
    new osg::MatrixTransform,
    "DCS",
    "Object Node Group DCS",
    &MatrixTransform_readLocalData,
    NULL,
    DotOsgWrapper::READ_ONLY
);

bool MatrixTransform_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    MatrixTransform& transform = static_cast<MatrixTransform&>(obj);

    if (fr[0].matchWord("Type"))
    {
        if (fr[1].matchWord("DYNAMIC"))
        {
            transform.setDataVariance(osg::Object::DYNAMIC);
            fr +=2 ;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("STATIC"))
        {
            transform.setDataVariance(osg::Object::STATIC);
            fr +=2 ;
            iteratorAdvanced = true;
        }

    }

    Matrix matrix;
    if (readMatrix(matrix,fr))
    {
        transform.setMatrix(matrix);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool MatrixTransform_writeLocalData(const Object& obj, Output& fw)
{
    const MatrixTransform& transform = static_cast<const MatrixTransform&>(obj);

    writeMatrix(transform.getMatrix(),fw);

    return true;
}
