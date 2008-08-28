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

bool ImageLayer_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool ImageLayer_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy ImageLayer_Proxy
(
    new osgTerrain::ImageLayer,
    "ImageLayer",
    "Object Layer ImageLayer",
    ImageLayer_readLocalData,
    ImageLayer_writeLocalData
);

bool ImageLayer_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgTerrain::ImageLayer& layer = static_cast<osgTerrain::ImageLayer&>(obj);

    bool itrAdvanced = false;
    
    if (fr.matchSequence("file %w") || fr.matchSequence("file %s"))
    {
        std::string setname;
        std::string filename;
        osgTerrain::extractSetNameAndFileName(fr[1].getStr(),setname, filename);
        if (!filename.empty())
        {
            osg::ref_ptr<osg::Image> image = osgDB::readImageFile(filename);
            if (image.valid())
            {
                layer.setName(setname);
                layer.setFileName(filename);
                layer.setImage(image.get());
            }
        }
        
        fr += 2;
        itrAdvanced = true;
    }
   

    return itrAdvanced;
}

bool ImageLayer_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgTerrain::ImageLayer& layer = static_cast<const osgTerrain::ImageLayer&>(obj);
    
    if (!layer.getFileName().empty())
    {
        std::string str = osgTerrain::createCompondSetNameAndFileName(layer.getName(), layer.getFileName());
        fw.indent()<<"file "<< str << std::endl;
    }

    return true;
}
