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
    "Object Layer HeightFieldLayer",
    HeightFieldLayer_readLocalData,
    HeightFieldLayer_writeLocalData
);

bool HeightFieldLayer_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgTerrain::HeightFieldLayer& layer = static_cast<osgTerrain::HeightFieldLayer&>(obj);

    bool itrAdvanced = false;
    
    if (fr.matchSequence("file %w") || fr.matchSequence("file %s"))
    {
        std::string setname;
        std::string filename;
        osgTerrain::extractSetNameAndFileName(fr[1].getStr(),setname, filename);
        if (!filename.empty())
        {
            osg::ref_ptr<osg::HeightField> hf = osgDB::readHeightFieldFile(filename);
            if (hf.valid())
            {
                layer.setName(setname);
                layer.setFileName(filename);
                layer.setHeightField(hf.get());                
            }
        }
        fr += 2;
        itrAdvanced = true;
    }

    osg::ref_ptr<osg::Object> readObject = fr.readObjectOfType(osgDB::type_wrapper<osg::HeightField>());
    if (readObject.valid()) itrAdvanced = true;

    osg::HeightField* hf = dynamic_cast<osg::HeightField*>(readObject.get());
    if (hf)
    {
        layer.setHeightField(hf);
    }
    
    return itrAdvanced;
}

bool HeightFieldLayer_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgTerrain::HeightFieldLayer& layer = static_cast<const osgTerrain::HeightFieldLayer&>(obj);
    
    if (!layer.getFileName().empty())
    {
        std::string str = osgTerrain::createCompondSetNameAndFileName(layer.getName(), layer.getFileName());
        fw.indent()<<"file "<< str << std::endl;
    }
    else
    {
        if (layer.getHeightField())
        {
            fw.writeObject(*layer.getHeightField());
        }
    }

    return true;
}
