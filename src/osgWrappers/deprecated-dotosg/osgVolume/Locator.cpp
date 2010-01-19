#include <osgVolume/VolumeTile>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/io_utils>

#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool Locator_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool Locator_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(Locator_Proxy)
(
    new osgVolume::Locator,
    "Locator",
    "Object Locator",
    Locator_readLocalData,
    Locator_writeLocalData
);


bool Locator_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgVolume::Locator& locator = static_cast<osgVolume::Locator&>(obj);

    bool itrAdvanced = false;

    if (fr.matchSequence("Transform {"))
    {
        int tansform_entry = fr[0].getNoNestedBrackets();

        fr += 2;

        int row=0;
        int col=0;
        double v;
        osg::Matrixd matrix;
        while (!fr.eof() && fr[0].getNoNestedBrackets()>tansform_entry)
        {
            if (fr[0].getFloat(v))
            {
                matrix(row,col)=v;
                ++col;
                if (col>=4)
                {
                    col = 0;
                    ++row;
                }
                ++fr;
            }
            else fr.advanceOverCurrentFieldOrBlock();
        }

        locator.setTransform(matrix);

        ++fr;
        itrAdvanced = true;
    }

    return itrAdvanced;
}

bool Locator_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgVolume::Locator& locator = static_cast<const osgVolume::Locator&>(obj);

    const osg::Matrixd& matrix = locator.getTransform();
    fw.indent() << "Transform {" << std::endl;
    fw.moveIn();
    fw.indent() << matrix(0,0) << " " << matrix(0,1) << " " << matrix(0,2) << " " << matrix(0,3) << std::endl;
    fw.indent() << matrix(1,0) << " " << matrix(1,1) << " " << matrix(1,2) << " " << matrix(1,3) << std::endl;
    fw.indent() << matrix(2,0) << " " << matrix(2,1) << " " << matrix(2,2) << " " << matrix(2,3) << std::endl;
    fw.indent() << matrix(3,0) << " " << matrix(3,1) << " " << matrix(3,2) << " " << matrix(3,3) << std::endl;
    fw.moveOut();
    fw.indent() << "}"<< std::endl;

    return true;
}
