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

osgDB::RegisterDotOsgWrapperProxy TransferFunction1D_Proxy
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
    return itrAdvanced;
}

bool TransferFunction1D_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osg::TransferFunction1D& tf = static_cast<const osg::TransferFunction1D&>(obj);

    fw.indent()<<"Minimum "<<tf.getMinimum()<<std::endl;
    fw.indent()<<"Maximum "<<tf.getMaximum()<<std::endl;
    fw.indent()<<"Colours "<<tf.getNumberCellsX()<<" {"<<std::endl;

    fw.moveIn();
    for(unsigned int i = 0; i<tf.getNumberCellsX(); ++i)
    {
        const osg::Vec4& c = tf.getValue(i);
        fw.indent()<<c.r()<<" "<<c.g()<<" "<<c.b()<<" "<<c.a()<<std::endl;
    }        
    fw.moveOut();
    fw.indent()<<"}"<<std::endl;

    return true;
}
