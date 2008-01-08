#include <osgTerrain/Layer>

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

bool HeightFieldLayer_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool HeightFieldLayer_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy HeightFieldLayer_Proxy
(
    new osgTerrain::HeightFieldLayer,
    "HeightFieldLayer",
    "Object HeightFieldLayer Layer",
    HeightFieldLayer_readLocalData,
    HeightFieldLayer_writeLocalData
);

bool HeightFieldLayer_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgTerrain::HeightFieldLayer& layer = static_cast<osgTerrain::HeightFieldLayer&>(obj);

    bool itrAdvanced = false;
    
    if (fr.matchSequence("file %w") || fr.matchSequence("file %s"))
    {
        osg::ref_ptr<osg::HeightField> hf = osgDB::readHeightFieldFile(fr[1].getStr());
        if (hf.valid())
        {
            layer.setHeightField(hf.get());
        }

        fr += 2;
        itrAdvanced = true;
    }
   

    return itrAdvanced;
}

bool HeightFieldLayer_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgTerrain::HeightFieldLayer& layer = static_cast<const osgTerrain::HeightFieldLayer&>(obj);
    
    if (!layer.getFileName().empty())
    {
        fw.indent()<<"file "<<layer.getFileName()<<std::endl;
    }

    return true;
}
