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
    new osg::Matrix,
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
                fr[k].getFloat(matrix._mat[i][j]);
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

    fw.indent() << matrix._mat[0][0] << " " << matrix._mat[0][1] << " " << matrix._mat[0][2] << " " << matrix._mat[0][3] << endl;
    fw.indent() << matrix._mat[1][0] << " " << matrix._mat[1][1] << " " << matrix._mat[1][2] << " " << matrix._mat[1][3] << endl;
    fw.indent() << matrix._mat[2][0] << " " << matrix._mat[2][1] << " " << matrix._mat[2][2] << " " << matrix._mat[2][3] << endl;
    fw.indent() << matrix._mat[3][0] << " " << matrix._mat[3][1] << " " << matrix._mat[3][2] << " " << matrix._mat[3][3] << endl;
    return true;
}

