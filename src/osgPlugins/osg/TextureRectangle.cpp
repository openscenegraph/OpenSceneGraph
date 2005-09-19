#include "osg/TextureRectangle"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"
#include "osgDB/WriteFile"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool TextureRectangle_readLocalData(Object& obj, Input& fr);
bool TextureRectangle_writeLocalData(const Object& obj, Output& fw);

bool TextureRectangle_matchWrapStr(const char* str,TextureRectangle::WrapMode& wrap);
const char* TextureRectangle_getWrapStr(TextureRectangle::WrapMode wrap);
bool TextureRectangle_matchFilterStr(const char* str,TextureRectangle::FilterMode& filter);
const char* TextureRectangle_getFilterStr(TextureRectangle::FilterMode filter);
bool TextureRectangle_matchInternalFormatModeStr(const char* str,TextureRectangle::InternalFormatMode& mode);
const char* TextureRectangle_getInternalFormatModeStr(TextureRectangle::InternalFormatMode mode);
bool TextureRectangle_matchInternalFormatStr(const char* str,int& value);
const char* TextureRectangle_getInternalFormatStr(int value);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_TextureRectangleProxy
(
    new osg::TextureRectangle,
    "TextureRectangle",
    "Object StateAttribute TextureRectangle TextureBase",
    &TextureRectangle_readLocalData,
    &TextureRectangle_writeLocalData
);

bool TextureRectangle_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    TextureRectangle& texture = static_cast<TextureRectangle&>(obj);

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


bool TextureRectangle_writeLocalData(const Object& obj, Output& fw)
{
    const TextureRectangle& texture = static_cast<const TextureRectangle&>(obj);

    if (texture.getImage())
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

    return true;
}
