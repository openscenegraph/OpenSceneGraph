#include <osg/TextureBase>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool TextureBase_readLocalData(Object& obj, Input& fr);
bool TextureBase_writeLocalData(const Object& obj, Output& fw);

bool TextureBase_matchWrapStr(const char* str,TextureBase::WrapMode& wrap);
const char* TextureBase_getWrapStr(TextureBase::WrapMode wrap);
bool TextureBase_matchFilterStr(const char* str,TextureBase::FilterMode& filter);
const char* TextureBase_getFilterStr(TextureBase::FilterMode filter);
bool TextureBase_matchInternalFormatModeStr(const char* str,TextureBase::InternalFormatMode& mode);
const char* TextureBase_getInternalFormatModeStr(TextureBase::InternalFormatMode mode);
bool TextureBase_matchInternalFormatStr(const char* str,int& value);
const char* TextureBase_getInternalFormatStr(int value);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_TextureBaseProxy
(
    0,
    "TextureBase",
    "Object StateAttribute TextureBase",
    &TextureBase_readLocalData,
    &TextureBase_writeLocalData
);


bool TextureBase_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    TextureBase& texture = static_cast<TextureBase&>(obj);

    TextureBase::WrapMode wrap;
    if (fr[0].matchWord("wrap_s") && TextureBase_matchWrapStr(fr[1].getStr(),wrap))
    {
        texture.setWrap(TextureBase::WRAP_S,wrap);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("wrap_t") && TextureBase_matchWrapStr(fr[1].getStr(),wrap))
    {
        texture.setWrap(TextureBase::WRAP_T,wrap);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("wrap_r") && TextureBase_matchWrapStr(fr[1].getStr(),wrap))
    {
        texture.setWrap(TextureBase::WRAP_R,wrap);
        fr+=2;
        iteratorAdvanced = true;
    }

    TextureBase::FilterMode filter;
    if (fr[0].matchWord("min_filter") && TextureBase_matchFilterStr(fr[1].getStr(),filter))
    {
        texture.setFilter(TextureBase::MIN_FILTER,filter);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("mag_filter") && TextureBase_matchFilterStr(fr[1].getStr(),filter))
    {
        texture.setFilter(TextureBase::MAG_FILTER,filter);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("maxAnisotropy %f"))
    {
        float anis=1.0f;
        fr[1].getFloat(anis);
        texture.setMaxAnisotropy(anis);
        fr +=2 ;
        iteratorAdvanced = true;
    }

    TextureBase::InternalFormatMode mode;
    if (fr[0].matchWord("internalFormatMode") && TextureBase_matchInternalFormatModeStr(fr[1].getStr(),mode))
    {
        texture.setInternalFormatMode(mode);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("internalFormatValue"))
    {
        int value;
        if (TextureBase_matchInternalFormatStr(fr[1].getStr(),value) || fr[1].getInt(value))
        {
            texture.setInternalFormat(value);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    return iteratorAdvanced;
}


bool TextureBase_writeLocalData(const Object& obj, Output& fw)
{
    const TextureBase& texture = static_cast<const TextureBase&>(obj);

    fw.indent() << "wrap_s " << TextureBase_getWrapStr(texture.getWrap(TextureBase::WRAP_S)) << std::endl;
    fw.indent() << "wrap_t " << TextureBase_getWrapStr(texture.getWrap(TextureBase::WRAP_T)) << std::endl;
    fw.indent() << "wrap_r " << TextureBase_getWrapStr(texture.getWrap(TextureBase::WRAP_R)) << std::endl;

    fw.indent() << "min_filter " << TextureBase_getFilterStr(texture.getFilter(TextureBase::MIN_FILTER)) << std::endl;
    fw.indent() << "mag_filter " << TextureBase_getFilterStr(texture.getFilter(TextureBase::MAG_FILTER)) << std::endl;
    fw.indent() << "maxAnisotropy " << texture.getMaxAnisotropy() << std::endl;

    fw.indent() << "internalFormatMode " << TextureBase_getInternalFormatModeStr(texture.getInternalFormatMode()) << std::endl;

    if (texture.getInternalFormatMode()==TextureBase::USE_USER_DEFINED_FORMAT)
    {
        const char* str = TextureBase_getInternalFormatStr(texture.getInternalFormat());
        if (str) fw.indent() << "internalFormat " << str << std::endl;
        else fw.indent() << "internalFormat " << texture.getInternalFormat() << std::endl;
    }

    return true;
}


bool TextureBase_matchWrapStr(const char* str,TextureBase::WrapMode& wrap)
{
    if (strcmp(str,"CLAMP")==0) wrap = TextureBase::CLAMP;
    else if (strcmp(str,"CLAMP_TO_EDGE")==0) wrap = TextureBase::CLAMP_TO_EDGE;
    else if (strcmp(str,"CLAMP_TO_BORDER")==0) wrap = TextureBase::CLAMP_TO_BORDER;
    else if (strcmp(str,"REPEAT")==0) wrap = TextureBase::REPEAT;
    else if (strcmp(str,"MIRROR")==0) wrap = TextureBase::MIRROR;
    else return false;
    return true;
}


const char* TextureBase_getWrapStr(TextureBase::WrapMode wrap)
{
    switch(wrap)
    {
        case(TextureBase::CLAMP): return "CLAMP";
        case(TextureBase::CLAMP_TO_EDGE): return "CLAMP_TO_EDGE";
        case(TextureBase::CLAMP_TO_BORDER): return "CLAMP_TO_BORDER";
        case(TextureBase::REPEAT): return "REPEAT";
        case(TextureBase::MIRROR): return "MIRROR";
    }
    return "";
}


bool TextureBase_matchFilterStr(const char* str,TextureBase::FilterMode& filter)
{
    if (strcmp(str,"NEAREST")==0) filter = TextureBase::NEAREST;
    else if (strcmp(str,"LINEAR")==0) filter = TextureBase::LINEAR;
    else if (strcmp(str,"NEAREST_MIPMAP_NEAREST")==0) filter = TextureBase::NEAREST_MIPMAP_NEAREST;
    else if (strcmp(str,"LINEAR_MIPMAP_NEAREST")==0) filter = TextureBase::LINEAR_MIPMAP_NEAREST;
    else if (strcmp(str,"NEAREST_MIPMAP_LINEAR")==0) filter = TextureBase::NEAREST_MIPMAP_LINEAR;
    else if (strcmp(str,"LINEAR_MIPMAP_LINEAR")==0) filter = TextureBase::LINEAR_MIPMAP_LINEAR;
    else if (strcmp(str,"ANISOTROPIC")==0) filter = TextureBase::LINEAR;
    else return false;
    return true;
}


const char* TextureBase_getFilterStr(TextureBase::FilterMode filter)
{
    switch(filter)
    {
        case(TextureBase::NEAREST): return "NEAREST";
        case(TextureBase::LINEAR): return "LINEAR";
        case(TextureBase::NEAREST_MIPMAP_NEAREST): return "NEAREST_MIPMAP_NEAREST";
        case(TextureBase::LINEAR_MIPMAP_NEAREST): return "LINEAR_MIPMAP_NEAREST";
        case(TextureBase::NEAREST_MIPMAP_LINEAR): return "NEAREST_MIPMAP_LINEAR";
        case(TextureBase::LINEAR_MIPMAP_LINEAR): return "LINEAR_MIPMAP_LINEAR";
    }
    return "";
}

bool TextureBase_matchInternalFormatModeStr(const char* str,TextureBase::InternalFormatMode& mode)
{
    if (strcmp(str,"USE_IMAGE_DATA_FORMAT")==0)          mode = TextureBase::USE_IMAGE_DATA_FORMAT;
    else if (strcmp(str,"USE_USER_DEFINED_FORMAT")==0)   mode = TextureBase::USE_USER_DEFINED_FORMAT;
    else if (strcmp(str,"USE_ARB_COMPRESSION")==0)       mode = TextureBase::USE_ARB_COMPRESSION;
    else if (strcmp(str,"USE_S3TC_DXT1_COMPRESSION")==0) mode = TextureBase::USE_S3TC_DXT1_COMPRESSION;
    else if (strcmp(str,"USE_S3TC_DXT3_COMPRESSION")==0) mode = TextureBase::USE_S3TC_DXT3_COMPRESSION;
    else if (strcmp(str,"USE_S3TC_DXT5_COMPRESSION")==0) mode = TextureBase::USE_S3TC_DXT5_COMPRESSION;
    else return false;
    return true;
}


const char* TextureBase_getInternalFormatModeStr(TextureBase::InternalFormatMode mode)
{
    switch(mode)
    {
        case(TextureBase::USE_IMAGE_DATA_FORMAT):        return "USE_IMAGE_DATA_FORMAT";
        case(TextureBase::USE_USER_DEFINED_FORMAT):      return "USE_USER_DEFINED_FORMAT";
        case(TextureBase::USE_ARB_COMPRESSION):          return "USE_ARB_COMPRESSION";
        case(TextureBase::USE_S3TC_DXT1_COMPRESSION):    return "USE_S3TC_DXT1_COMPRESSION";
        case(TextureBase::USE_S3TC_DXT3_COMPRESSION):    return "USE_S3TC_DXT3_COMPRESSION";
        case(TextureBase::USE_S3TC_DXT5_COMPRESSION):    return "USE_S3TC_DXT5_COMPRESSION";
    }
    return "";
}


bool TextureBase_matchInternalFormatStr(const char* str,int& value)
{

    if (     strcmp(str,"GL_INTENSITY")==0) value = GL_INTENSITY;
    else if (strcmp(str,"GL_LUMINANCE")==0) value = GL_LUMINANCE;
    else if (strcmp(str,"GL_ALPHA")==0) value = GL_ALPHA;
    else if (strcmp(str,"GL_LUMINANCE_ALPHA")==0) value = GL_LUMINANCE_ALPHA;
    else if (strcmp(str,"GL_RGB")==0) value = GL_RGB;
    else if (strcmp(str,"GL_RGBA")==0) value = GL_RGBA;
    else if (strcmp(str,"GL_COMPRESSED_ALPHA_ARB")==0) value = GL_COMPRESSED_ALPHA_ARB;
    else if (strcmp(str,"GL_COMPRESSED_LUMINANCE_ARB")==0) value = GL_COMPRESSED_LUMINANCE_ARB;
    else if (strcmp(str,"GL_COMPRESSED_INTENSITY_ARB")==0) value = GL_COMPRESSED_INTENSITY_ARB;
    else if (strcmp(str,"GL_COMPRESSED_LUMINANCE_ALPHA_ARB")==0) value = GL_COMPRESSED_LUMINANCE_ALPHA_ARB;
    else if (strcmp(str,"GL_COMPRESSED_RGB_ARB")==0) value = GL_COMPRESSED_RGB_ARB;
    else if (strcmp(str,"GL_COMPRESSED_RGBA_ARB")==0) value = GL_COMPRESSED_RGBA_ARB;
    else if (strcmp(str,"GL_COMPRESSED_RGB_S3TC_DXT1_EXT")==0) value = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    else if (strcmp(str,"GL_COMPRESSED_RGBA_S3TC_DXT1_EXT")==0) value = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    else if (strcmp(str,"GL_COMPRESSED_RGBA_S3TC_DXT3_EXT")==0) value = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    else if (strcmp(str,"GL_COMPRESSED_RGBA_S3TC_DXT5_EXT")==0) value = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    else return false;
    return true;
}


const char* TextureBase_getInternalFormatStr(int value)
{
    switch(value)
    {
        case(GL_INTENSITY): return "GL_INTENSITY";
        case(GL_LUMINANCE): return "GL_LUMINANCE";
        case(GL_ALPHA): return "GL_ALPHA";
        case(GL_LUMINANCE_ALPHA): return "GL_LUMINANCE_ALPHA";
        case(GL_RGB): return "GL_RGB";
        case(GL_RGBA): return "GL_RGBA";
        case(GL_COMPRESSED_ALPHA_ARB): return "GL_COMPRESSED_ALPHA_ARB";
        case(GL_COMPRESSED_LUMINANCE_ARB): return "GL_COMPRESSED_LUMINANCE_ARB";
        case(GL_COMPRESSED_INTENSITY_ARB): return "GL_COMPRESSED_INTENSITY_ARB";
        case(GL_COMPRESSED_LUMINANCE_ALPHA_ARB): return "GL_COMPRESSED_LUMINANCE_ALPHA_ARB";
        case(GL_COMPRESSED_RGB_ARB): return "GL_COMPRESSED_RGB_ARB";
        case(GL_COMPRESSED_RGBA_ARB): return "GL_COMPRESSED_RGBA_ARB";
        case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT): return "GL_COMPRESSED_RGB_S3TC_DXT1_EXT";
        case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT): return "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT";
        case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT): return "GL_COMPRESSED_RGBA_S3TC_DXT3_EXT";
        case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT): return "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT";
    }

    return NULL;
}
