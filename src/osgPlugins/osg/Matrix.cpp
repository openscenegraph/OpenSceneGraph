#include "osg/Matrix"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Matrix_readLocalData(Object& obj, Input& fr);
bool Matrix_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_MatrixFuncProxy
(
    osgNew osg::Matrix,
    "Matrix",
    "Object Matrix",
    &Matrix_readLocalData,
    &Matrix_writeLocalData
);

bool Matrix_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Matrix& matrix = static_cast<Matrix&>(obj);

    bool matched = true;
    for(int k=0;k<16 && matched;++k)
    {
        matched = fr[k].isFloat();
    }
    if (matched)
    {
        int k=0;
        for(int i=0;i<4;++i)
        {
            for(int j=0;j<4;++j)
            {
                fr[k].getFloat(matrix(i,j));
                k++;
            }
        }
        fr += 16;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Matrix_writeLocalData(const Object& obj, Output& fw)
{
    const Matrix& matrix = static_cast<const Matrix&>(obj);

    fw.indent() << matrix(0,0) << " " << matrix(0,1) << " " << matrix(0,2) << " " << matrix(0,3) << std::endl;
    fw.indent() << matrix(1,0) << " " << matrix(1,1) << " " << matrix(1,2) << " " << matrix(1,3) << std::endl;
    fw.indent() << matrix(2,0) << " " << matrix(2,1) << " " << matrix(2,2) << " " << matrix(2,3) << std::endl;
    fw.indent() << matrix(3,0) << " " << matrix(3,1) << " " << matrix(3,2) << " " << matrix(3,3) << std::endl;
    return true;
}

