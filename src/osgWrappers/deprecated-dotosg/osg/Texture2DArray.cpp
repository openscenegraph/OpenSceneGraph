#include "osg/Texture2DArray"
#include "osg/ImageSequence"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"
#include "osgDB/WriteFile"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Texture2DArray_readLocalData(Object& obj, Input& fr);
bool Texture2DArray_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Texture2DArray)
    (
    new osg::Texture2DArray,
    "Texture2DArray",
    "Object StateAttribute Texture2DArray TextureBase",
    &Texture2DArray_readLocalData,
    &Texture2DArray_writeLocalData
    );

bool Texture2DArray_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;
    int index = 0;
    GLint textureW = 0;
    GLint textureH = 0;
    bool matched=true;

    Texture2DArray& texture = static_cast<Texture2DArray&>(obj);

    while (matched && (fr[0].matchWord("file") || fr[0].matchWord("ImageSequence") || fr[0].matchWord("Image")))
    {
        matched = false;
        osg::ref_ptr<Image> image;
        if (fr[0].matchWord("file") && fr[1].isString())
        {
            std::string filename = fr[1].getStr();
            image = fr.readImage(filename.c_str());
            fr += 2;
            iteratorAdvanced = true;
            matched = true;
        }
        else if (fr[0].matchWord("ImageSequence") || fr[0].matchWord("Image"))
        {
            image = fr.readImage();
            fr += 1;
            iteratorAdvanced = true;
            matched = true;
        }

        if(image)
        {
            if(textureW == 0) //if first image,save size
            {
                textureW = image->s();
                textureH = image->t();
            }
            else if(textureW != image->s() ||
                textureH != image->t())
            {
                //scale to match size of first image.
                image->scaleImage(textureW,textureH,1);
            }
            texture.setImage(index,image);
        }
        index++;
    }
    return iteratorAdvanced;
}

bool Texture2DArray_writeLocalData(const Object& obj, Output& fw)
{
    const Texture2DArray& ta = static_cast<const Texture2DArray&>(obj);

    for(size_t i = 0; i < ta.getNumImages();i++)
    {
        const Image* image = ta.getImage(i);
        if(image)
        {
            const osg::ImageSequence* is = dynamic_cast<const osg::ImageSequence*>(image);
            if (is)
            {
                fw.writeObject(*is);
            }
            else
            {
                std::string fileName = image->getFileName();
                if (fw.getOutputTextureFiles())
                {
                    if (fileName.empty())
                    {
                        fileName = fw.getTextureFileNameForOutput();
                    }
                    osgDB::writeImageFile(*image, fileName);
                }
                if (!fileName.empty())
                {
                    fw.indent() << "file "<<fw.wrapString(fw.getFileNameForOutput(fileName))<< std::endl;
                }
            }
        }
    }
    return true;
}
