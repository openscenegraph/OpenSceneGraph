#include "osg/TextureCubeMap"
#include "osg/ImageSequence"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"
#include "osgDB/WriteFile"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool TextureCubeMap_readLocalData(Object& obj, Input& fr);
bool TextureCubeMap_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(TextureCubeMap)
(
    new osg::TextureCubeMap,
    "TextureCubeMap",
    "Object StateAttribute TextureCubeMap TextureBase",
    &TextureCubeMap_readLocalData,
    &TextureCubeMap_writeLocalData
);


#define READ_IMAGE(FACE)\
        matched = false;\
        if (fr[1].matchWord(#FACE)) \
        {\
            if (fr[2].matchWord("ImageSequence") || fr[2].matchWord("Image")) \
            { \
                fr += 2;\
                osg::Image* image = fr.readImage(); \
                if (image) texture.setImage(osg::TextureCubeMap::FACE,image); \
                iteratorAdvanced = true; \
                matched = true;\
            } \
            else if (fr[2].isString())\
            { \
                Image* image = fr.readImage(fr[2].getStr());\
                if (image) texture.setImage(osg::TextureCubeMap::FACE,image);\
                fr += 3;\
                iteratorAdvanced = true; \
                matched = true;\
            }\
        }\

bool TextureCubeMap_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    TextureCubeMap& texture = static_cast<TextureCubeMap&>(obj);

    bool matched=true;
    while (fr[0].matchWord("image") && matched)
    {
        READ_IMAGE(POSITIVE_X)
        else READ_IMAGE(NEGATIVE_X)
        else READ_IMAGE(POSITIVE_Y)
        else READ_IMAGE(NEGATIVE_Y)
        else READ_IMAGE(POSITIVE_Z)
        else READ_IMAGE(NEGATIVE_Z)
    }

    return iteratorAdvanced;
}


#define WRITE_IMAGE(FACE) \
    {\
        const osg::Image* image = texture.getImage(osg::TextureCubeMap::FACE);\
        if (image)\
        {\
            const osg::ImageSequence* is = dynamic_cast<const osg::ImageSequence*>(image); \
            if (is) \
            { \
                fw.indent() << "image "<<#FACE<<std::endl; \
                fw.writeObject(*is); \
            } \
            else \
            { \
                std::string fileName = image->getFileName();\
                if (fw.getOutputTextureFiles())\
                {\
                    if (fileName.empty())\
                    {\
                        fileName = fw.getTextureFileNameForOutput();\
                    }\
                    osgDB::writeImageFile(*image, fileName);\
                }\
                if (!fileName.empty())\
                {\
                    fw.indent() << "image "<<#FACE<<" "<<fw.wrapString(fw.getFileNameForOutput(fileName))<< std::endl;\
                }\
            }\
        } \
    }

bool TextureCubeMap_writeLocalData(const Object& obj, Output& fw)
{
    const TextureCubeMap& texture = static_cast<const TextureCubeMap&>(obj);

    WRITE_IMAGE(POSITIVE_X)
    WRITE_IMAGE(NEGATIVE_X)
    WRITE_IMAGE(POSITIVE_Y)
    WRITE_IMAGE(NEGATIVE_Y)
    WRITE_IMAGE(POSITIVE_Z)
    WRITE_IMAGE(NEGATIVE_Z)

    return true;
}


