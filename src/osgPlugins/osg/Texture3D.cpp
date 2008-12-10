#include "osg/Texture3D"
#include "osg/ImageSequence"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"
#include "osgDB/WriteFile"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Texture3D_readLocalData(Object& obj, Input& fr);
bool Texture3D_writeLocalData(const Object& obj, Output& fw);

bool Texture3D_matchWrapStr(const char* str,Texture3D::WrapMode& wrap);
const char* Texture3D_getWrapStr(Texture3D::WrapMode wrap);
bool Texture3D_matchFilterStr(const char* str,Texture3D::FilterMode& filter);
const char* Texture3D_getFilterStr(Texture3D::FilterMode filter);
bool Texture3D_matchInternalFormatModeStr(const char* str,Texture3D::InternalFormatMode& mode);
const char* Texture3D_getInternalFormatModeStr(Texture3D::InternalFormatMode mode);
bool Texture3D_matchInternalFormatStr(const char* str,int& value);
const char* Texture3D_getInternalFormatStr(int value);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Texture3D)
(
    new osg::Texture3D,
    "Texture3D",
    "Object StateAttribute Texture3D TextureBase",
    &Texture3D_readLocalData,
    &Texture3D_writeLocalData
);

bool Texture3D_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Texture3D& texture = static_cast<Texture3D&>(obj);

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

bool Texture3D_writeLocalData(const Object& obj, Output& fw)
{
    const Texture3D& texture = static_cast<const Texture3D&>(obj);

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
            if (fw.getOutputTextureFiles())
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
