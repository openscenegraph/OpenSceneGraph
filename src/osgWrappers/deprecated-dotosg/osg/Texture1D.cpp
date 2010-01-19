#include "osg/Texture1D"
#include "osg/ImageSequence"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"
#include "osgDB/WriteFile"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Texture1D_readLocalData(Object& obj, Input& fr);
bool Texture1D_writeLocalData(const Object& obj, Output& fw);

bool Texture1D_matchWrapStr(const char* str,Texture1D::WrapMode& wrap);
const char* Texture1D_getWrapStr(Texture1D::WrapMode wrap);
bool Texture1D_matchFilterStr(const char* str,Texture1D::FilterMode& filter);
const char* Texture1D_getFilterStr(Texture1D::FilterMode filter);
bool Texture1D_matchInternalFormatModeStr(const char* str,Texture1D::InternalFormatMode& mode);
const char* Texture1D_getInternalFormatModeStr(Texture1D::InternalFormatMode mode);
bool Texture1D_matchInternalFormatStr(const char* str,int& value);
const char* Texture1D_getInternalFormatStr(int value);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Texture1D)
(
    new osg::Texture1D,
    "Texture1D",
    "Object StateAttribute Texture1D TextureBase",
    &Texture1D_readLocalData,
    &Texture1D_writeLocalData
);

bool Texture1D_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Texture1D& texture = static_cast<Texture1D&>(obj);

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


bool Texture1D_writeLocalData(const Object& obj, Output& fw)
{
    const Texture1D& texture = static_cast<const Texture1D&>(obj);

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
