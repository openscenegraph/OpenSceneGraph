#include <osgVolume/Layer>

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
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <osgVolume/VolumeTile>

bool ImageLayer_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool ImageLayer_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy ImageLayer_Proxy
(
    new osgVolume::ImageLayer,
    "ImageLayer",
    "Object Layer ImageLayer",
    ImageLayer_readLocalData,
    ImageLayer_writeLocalData
);

bool ImageLayer_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgVolume::ImageLayer& layer = static_cast<osgVolume::ImageLayer&>(obj);

    bool itrAdvanced = false;
    
    if (fr.matchSequence("file %w") || fr.matchSequence("file %s"))
    {
        std::string filename = fr[1].getStr();
        if (!filename.empty())
        {
            bool deferExternalLayerLoading = false;

            layer.setFileName(filename);

            if (!deferExternalLayerLoading)
            {

                osgDB::FileType fileType = osgDB::fileType(filename);
                if (fileType == osgDB::FILE_NOT_FOUND)
                {
                    filename = osgDB::findDataFile(filename);
                    fileType = osgDB::fileType(filename);
                }

                osg::ref_ptr<osg::Image> image;
                if (fileType == osgDB::DIRECTORY)
                {
                    image = osgDB::readRefImageFile(filename+".dicom");
                    
                }
                else if (fileType == osgDB::REGULAR_FILE)
                {
                    image = osgDB::readRefImageFile( filename );
                }

                osg::notify(osg::INFO)<<"osgVolume::ImageLyaer image read: "<<filename<<" pixelFormat "<<std::hex<<image->getPixelFormat()<<" textureFormat "<<image->getInternalTextureFormat()<<" dataType "<<image->getDataType()<<std::endl;

                if (image.valid())
                {                
                    layer.setImage(image.get());
                    layer.rescaleToZeroToOneRange();
                }
            }
        }
        
        fr += 2;
        itrAdvanced = true;
    }
   

    return itrAdvanced;
}

bool ImageLayer_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgVolume::ImageLayer& layer = static_cast<const osgVolume::ImageLayer&>(obj);
    
    if (!layer.getFileName().empty())
    {
        fw.indent()<<"file "<< layer.getFileName() << std::endl;
    }

    return true;
}
