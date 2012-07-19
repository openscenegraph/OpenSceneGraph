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

REGISTER_DOTOSGWRAPPER(ImageLayer_Proxy)
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
                    filename = osgDB::findDataFile(filename, fr.getOptions());
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


                if (image.valid())
                {
                    osg::notify(osg::INFO)<<"osgVolume::ImageLayer image read: "<<filename<<" pixelFormat "<<std::hex<<image->getPixelFormat()<<" textureFormat "<<image->getInternalTextureFormat()<<" dataType "<<image->getDataType()<<std::dec<<std::endl;

                    osg::ref_ptr<osgVolume::ImageDetails> details = dynamic_cast<osgVolume::ImageDetails*>(image->getUserData());
                    osg::ref_ptr<osg::RefMatrix> matrix = details ? details->getMatrix() : dynamic_cast<osg::RefMatrix*>(image->getUserData());

                    layer.setImage(image.get());

                    if (details)
                    {
                        layer.setTexelOffset(details->getTexelOffset());
                        layer.setTexelScale(details->getTexelScale());
                    }
                    if (matrix)
                    {
                        layer.setLocator(new osgVolume::Locator(*matrix));
                    }

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
