#include <osg/ColorMatrix>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool ColorMatrix_readLocalData(Object& obj, Input& fr);
bool ColorMatrix_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_ColorMatrixProxy
(
    new osg::ColorMatrix,
    "ColorMatrix",
    "Object StateAttribute ColorMatrix",
    &ColorMatrix_readLocalData,
    &ColorMatrix_writeLocalData
);


bool ColorMatrix_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    ColorMatrix& colorMatrix = static_cast<ColorMatrix&>(obj);

    bool matched = true;
    for(int k=0;k<16 && matched;++k)
    {
        matched = fr[k].isFloat();
    }
    if (matched)
    {
    
        Matrix& matrix = colorMatrix.getMatrix();
    
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


bool ColorMatrix_writeLocalData(const Object& obj, Output& fw)
{
    const ColorMatrix& colorMatrix = static_cast<const ColorMatrix&>(obj);
    const Matrix& matrix = colorMatrix.getMatrix();
    fw.indent() << matrix(0,0) << " " << matrix(0,1) << " " << matrix(0,2) << " " << matrix(0,3) << std::endl;
    fw.indent() << matrix(1,0) << " " << matrix(1,1) << " " << matrix(1,2) << " " << matrix(1,3) << std::endl;
    fw.indent() << matrix(2,0) << " " << matrix(2,1) << " " << matrix(2,2) << " " << matrix(2,3) << std::endl;
    fw.indent() << matrix(3,0) << " " << matrix(3,1) << " " << matrix(3,2) << " " << matrix(3,3) << std::endl;
    return true;
}
