#include "osg/Texture2D"
#include "osg/ImageSequence"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"
#include "osgDB/WriteFile"
#include "osgDB/FileNameUtils"
#include "osgDB/FileUtils"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Texture2D_readLocalData(Object& obj, Input& fr);
bool Texture2D_writeLocalData(const Object& obj, Output& fw);

bool Texture2D_matchWrapStr(const char* str,Texture2D::WrapMode& wrap);
const char* Texture2D_getWrapStr(Texture2D::WrapMode wrap);
bool Texture2D_matchFilterStr(const char* str,Texture2D::FilterMode& filter);
const char* Texture2D_getFilterStr(Texture2D::FilterMode filter);
bool Texture2D_matchInternalFormatModeStr(const char* str,Texture2D::InternalFormatMode& mode);
const char* Texture2D_getInternalFormatModeStr(Texture2D::InternalFormatMode mode);
bool Texture2D_matchInternalFormatStr(const char* str,int& value);
const char* Texture2D_getInternalFormatStr(int value);

REGISTER_DOTOSGWRAPPER(OldTexture)
(
    new osg::Texture2D,
    "Texture",
    "Object StateAttribute Texture2D TextureBase",
    0,
    0
);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Texture2D)
(
    new osg::Texture2D,
    "Texture2D",
    "Object StateAttribute Texture2D TextureBase",
    &Texture2D_readLocalData,
    &Texture2D_writeLocalData
);

bool Texture2D_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Texture2D& texture = static_cast<Texture2D&>(obj);

    if (fr[0].matchWord("file") && fr[1].isString())
    {
        std::string filename = fr[1].getStr();
        Image* image = fr.readImage(filename.c_str());
        if (image)
        {
            // name will have already been set by the image plugin,
            // but it will have absolute path, so will override it 
            // here to keep the original name intact.
            //image->setFileName(filename);
            texture.setImage(image);
        }
        
        fr += 2;
        iteratorAdvanced = true;
    }
    
    if (fr[0].matchWord("ImageSequence") || fr[0].matchWord("Image"))
    {
        osg::Image* image = fr.readImage();
        if (image) texture.setImage(image);
    }

    return iteratorAdvanced;
}

bool Texture2D_writeLocalData(const Object& obj, Output& fw)
{
    const Texture2D& texture = static_cast<const Texture2D&>(obj);

    if (texture.getImage())
    {
        const osg::ImageSequence* is = dynamic_cast<const osg::ImageSequence*>(texture.getImage());
        if (is)
        {
            fw.writeObject(*is);
        }
        else
        {
            std::string fileName = texture.getImage()->getFileName();
            if (fw.getOutputRelativeTextures())
            {
				std::string generatedFileName = osgDB::getSimpleFileName(osgDB::getNameLessExtension(fw.getTextureFileNameForOutput()));
				std::string textureDir = osgDB::getNameLessExtension(fw.getFileName()) + "_textures";
				std::string relativeDir = osgDB::getSimpleFileName(textureDir);
				std::string texFnWithOriginalFormat = generatedFileName + osgDB::getFileExtensionIncludingDot(fileName);
				std::string fullFileName =  osgDB::concatPaths(textureDir, texFnWithOriginalFormat);
				fileName = relativeDir + "/" + texFnWithOriginalFormat;
				osgDB::makeDirectory(textureDir);
                osgDB::writeImageFile(*texture.getImage(), fullFileName);
            }
            else if (fw.getOutputTextureFiles())
            {
                if (fileName.empty())
                {
                    fileName = fw.getTextureFileNameForOutput();
                }
                osgDB::writeImageFile(*texture.getImage(), fileName);
            }

            if (!fileName.empty())
            {    
                fw.indent() << "file "<<fw.wrapString(fw.getFileNameForOutput(fileName))<< std::endl;
            }
        }
    }

    return true;
}
