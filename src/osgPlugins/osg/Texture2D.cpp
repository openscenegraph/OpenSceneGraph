#include "osg/Texture2D"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

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

RegisterDotOsgWrapperProxy g_OldTextureProxy
(
    osgNew osg::Texture2D,
    "Texture",
    "Object StateAttribute Texture2D TextureBase",
    0,
    0
);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_Texture2DProxy
(
    osgNew osg::Texture2D,
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

    return iteratorAdvanced;
}


bool Texture2D_writeLocalData(const Object& obj, Output& fw)
{
    const Texture2D& texture = static_cast<const Texture2D&>(obj);

    if (texture.getImage() && !(texture.getImage()->getFileName().empty()))
    {
        fw.indent() << "file \""<<fw.getFileNameForOutput(texture.getImage()->getFileName())<<"\""<< std::endl;
    }

    return true;
}
