#include <osg/TransferFunction>

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

bool TransferFunction1D_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool TransferFunction1D_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(TransferFunction1D_Proxy)
(
    new osg::TransferFunction1D,
    "TransferFunction1D",
    "Object TransferFunction1D",
    TransferFunction1D_readLocalData,
    TransferFunction1D_writeLocalData
);


bool TransferFunction1D_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osg::TransferFunction1D& tf = static_cast<osg::TransferFunction1D&>(obj);

    bool itrAdvanced = false;

    unsigned int numCells;
    if (fr.read("NumberImageCells ",numCells))
    {
        tf.allocate(numCells);
        itrAdvanced = true;
    }

    if (fr.matchSequence("Colours {"))
    {
        int entry = fr[0].getNoNestedBrackets();

        fr += 2;

        float v;
        osg::Vec4 color;
        osg::TransferFunction1D::ColorMap colorMap;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            if (fr.read(v, color.r(), color.g(), color.b(), color.a()))
            {
                colorMap[v] = color;
            }
            else
            {
                ++fr;
            }
        }

        tf.assign(colorMap);

        itrAdvanced = true;
    }


    return itrAdvanced;
}

bool TransferFunction1D_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osg::TransferFunction1D& tf = static_cast<const osg::TransferFunction1D&>(obj);
    const osg::TransferFunction1D::ColorMap& colorMap = tf.getColorMap();

    fw.indent()<<"NumberImageCells "<<tf.getNumberImageCells()<<std::endl;
    fw.indent()<<"Colours {"<<std::endl;

    fw.moveIn();
    for(osg::TransferFunction1D::ColorMap::const_iterator itr = colorMap.begin();
        itr != colorMap.end();
        ++itr)
    {
        const osg::Vec4& c = itr->second;
        fw.indent()<<itr->first<<" "<<c.r()<<" "<<c.g()<<" "<<c.b()<<" "<<c.a()<<std::endl;
    }
    fw.moveOut();
    fw.indent()<<"}"<<std::endl;

    return true;
}
