#include <osg/TexMat>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool TexMat_readLocalData(Object& obj, Input& fr);
bool TexMat_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_TexMatProxy
(
    new osg::TexMat,
    "TexMat",
    "Object StateAttribute TexMat",
    &TexMat_readLocalData,
    &TexMat_writeLocalData
);


bool TexMat_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    TexMat& texmat = static_cast<TexMat&>(obj);

    bool matched = true;
    for(int k=0;k<16 && matched;++k)
    {
        matched = fr[k].isFloat();
    }
    if (matched)
    {
    
        Matrix& matrix = texmat.getMatrix();
    
        int k=0;
        double v;
        for(int i=0;i<4;++i)
        {
            for(int j=0;j<4;++j)
            {
                fr[k].getFloat(v);
                matrix(i,j)=v;
                k++;
            }
        }
        fr += 16;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("scaleByTextureRectangleSize"))
    {
        if (fr[1].matchWord("TRUE")) 
        {
            texmat.setScaleByTextureRectangleSize(true);
            fr +=2 ;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("FALSE")) 
        {
            texmat.setScaleByTextureRectangleSize(false);
            fr +=2 ;
            iteratorAdvanced = true;
        }
    }

    return iteratorAdvanced;
}


bool TexMat_writeLocalData(const Object& obj, Output& fw)
{
    const TexMat& texmat = static_cast<const TexMat&>(obj);
    const Matrix& matrix = texmat.getMatrix();
    fw.indent() << matrix(0,0) << " " << matrix(0,1) << " " << matrix(0,2) << " " << matrix(0,3) << std::endl;
    fw.indent() << matrix(1,0) << " " << matrix(1,1) << " " << matrix(1,2) << " " << matrix(1,3) << std::endl;
    fw.indent() << matrix(2,0) << " " << matrix(2,1) << " " << matrix(2,2) << " " << matrix(2,3) << std::endl;
    fw.indent() << matrix(3,0) << " " << matrix(3,1) << " " << matrix(3,2) << " " << matrix(3,3) << std::endl;
    
    if (texmat.getScaleByTextureRectangleSize())
    {
        fw.indent() << "scaleByTextureRectangleSize TRUE"<<std::endl;
    } 
    
    return true;
}
