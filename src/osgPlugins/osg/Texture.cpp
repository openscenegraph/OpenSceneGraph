#include <osg/Texture>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Texture_readLocalData(Object& obj, Input& fr);
bool Texture_writeLocalData(const Object& obj, Output& fw);

bool Texture_matchWrapStr(const char* str,Texture::WrapMode& wrap);
const char* Texture_getWrapStr(Texture::WrapMode wrap);
bool Texture_matchFilterStr(const char* str,Texture::FilterMode& filter);
const char* Texture_getFilterStr(Texture::FilterMode filter);
bool Texture_matchInternalFormatModeStr(const char* str,Texture::InternalFormatMode& mode);
const char* Texture_getInternalFormatModeStr(Texture::InternalFormatMode mode);
bool Texture_matchInternalFormatStr(const char* str,int& value);
const char* Texture_getInternalFormatStr(int value);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_TextureProxy
(
    0,
    "TextureBase",
    "Object StateAttribute TextureBase",
    &Texture_readLocalData,
    &Texture_writeLocalData
);


bool Texture_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Texture& texture = static_cast<Texture&>(obj);

    Texture::WrapMode wrap;
    if (fr[0].matchWord("wrap_s") && Texture_matchWrapStr(fr[1].getStr(),wrap))
    {
        texture.setWrap(Texture::WRAP_S,wrap);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("wrap_t") && Texture_matchWrapStr(fr[1].getStr(),wrap))
    {
        texture.setWrap(Texture::WRAP_T,wrap);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("wrap_r") && Texture_matchWrapStr(fr[1].getStr(),wrap))
    {
        texture.setWrap(Texture::WRAP_R,wrap);
        fr+=2;
        iteratorAdvanced = true;
    }

    Texture::FilterMode filter;
    if (fr[0].matchWord("min_filter") && Texture_matchFilterStr(fr[1].getStr(),filter))
    {
        texture.setFilter(Texture::MIN_FILTER,filter);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("mag_filter") && Texture_matchFilterStr(fr[1].getStr(),filter))
    {
        texture.setFilter(Texture::MAG_FILTER,filter);
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

    if (fr.matchSequence("borderWidth %f %f %f %f"))
    {
        Vec4 color;
        fr[1].getFloat(color[0]);
        fr[2].getFloat(color[1]);
        fr[3].getFloat(color[2]);
        fr[4].getFloat(color[3]);
        texture.setBorderColor(color);
        fr +=5 ;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("borderWidth %i"))
    {
        int width=0;
        fr[1].getInt(width);
        texture.setBorderWidth(width);
        fr +=2 ;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("useHardwareMipMapGeneration"))
    {
        if (fr[1].matchWord("TRUE")) 
        {
            texture.setUseHardwareMipMapGeneration(true);
            fr +=2 ;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("FALSE")) 
        {
            texture.setUseHardwareMipMapGeneration(false);
            fr +=2 ;
            iteratorAdvanced = true;
        }
    }

    if (fr[0].matchWord("unRefImageDataAfterApply"))
    {
        if (fr[1].matchWord("TRUE")) 
        {
            texture.setUnRefImageDataAfterApply(true);
            fr +=2 ;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("FALSE")) 
        {
            texture.setUnRefImageDataAfterApply(false);
            fr +=2 ;
            iteratorAdvanced = true;
        }
    }


    Texture::InternalFormatMode mode;
    if (fr[0].matchWord("internalFormatMode") && Texture_matchInternalFormatModeStr(fr[1].getStr(),mode))
    {
        texture.setInternalFormatMode(mode);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("internalFormatValue"))
    {
        int value;
        if (Texture_matchInternalFormatStr(fr[1].getStr(),value) || fr[1].getInt(value))
        {
            texture.setInternalFormat(value);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    return iteratorAdvanced;
}


bool Texture_writeLocalData(const Object& obj, Output& fw)
{
    const Texture& texture = static_cast<const Texture&>(obj);

    fw.indent() << "wrap_s " << Texture_getWrapStr(texture.getWrap(Texture::WRAP_S)) << std::endl;
    fw.indent() << "wrap_t " << Texture_getWrapStr(texture.getWrap(Texture::WRAP_T)) << std::endl;
    fw.indent() << "wrap_r " << Texture_getWrapStr(texture.getWrap(Texture::WRAP_R)) << std::endl;

    fw.indent() << "min_filter " << Texture_getFilterStr(texture.getFilter(Texture::MIN_FILTER)) << std::endl;
    fw.indent() << "mag_filter " << Texture_getFilterStr(texture.getFilter(Texture::MAG_FILTER)) << std::endl;
    fw.indent() << "maxAnisotropy " << texture.getMaxAnisotropy() << std::endl;
    
    fw.indent() << "borderColor " << texture.getBorderColor() << std::endl;
    fw.indent() << "borderWidth " << texture.getBorderWidth() << std::endl;

    fw.indent() << "useHardwareMipMapGeneration "<< (texture.getUseHardwareMipMapGeneration()?"TRUE":"FALSE") << std::endl;
    fw.indent() << "unRefImageDataAfterApply "<< (texture.getUnRefImageDataAfterApply()?"TRUE":"FALSE") << std::endl;
            
    fw.indent() << "internalFormatMode " << Texture_getInternalFormatModeStr(texture.getInternalFormatMode()) << std::endl;

    if (texture.getInternalFormatMode()==Texture::USE_USER_DEFINED_FORMAT)
    {
        const char* str = Texture_getInternalFormatStr(texture.getInternalFormat());
        if (str) fw.indent() << "internalFormat " << str << std::endl;
        else fw.indent() << "internalFormat " << texture.getInternalFormat() << std::endl;
    }

    return true;
}


bool Texture_matchWrapStr(const char* str,Texture::WrapMode& wrap)
{
    if (strcmp(str,"CLAMP")==0) wrap = Texture::CLAMP;
    else if (strcmp(str,"CLAMP_TO_EDGE")==0) wrap = Texture::CLAMP_TO_EDGE;
    else if (strcmp(str,"CLAMP_TO_BORDER")==0) wrap = Texture::CLAMP_TO_BORDER;
    else if (strcmp(str,"REPEAT")==0) wrap = Texture::REPEAT;
    else if (strcmp(str,"MIRROR")==0) wrap = Texture::MIRROR;
    else return false;
    return true;
}


const char* Texture_getWrapStr(Texture::WrapMode wrap)
{
    switch(wrap)
    {
        case(Texture::CLAMP): return "CLAMP";
        case(Texture::CLAMP_TO_EDGE): return "CLAMP_TO_EDGE";
        case(Texture::CLAMP_TO_BORDER): return "CLAMP_TO_BORDER";
        case(Texture::REPEAT): return "REPEAT";
        case(Texture::MIRROR): return "MIRROR";
    }
    return "";
}


bool Texture_matchFilterStr(const char* str,Texture::FilterMode& filter)
{
    if (strcmp(str,"NEAREST")==0) filter = Texture::NEAREST;
    else if (strcmp(str,"LINEAR")==0) filter = Texture::LINEAR;
    else if (strcmp(str,"NEAREST_MIPMAP_NEAREST")==0) filter = Texture::NEAREST_MIPMAP_NEAREST;
    else if (strcmp(str,"LINEAR_MIPMAP_NEAREST")==0) filter = Texture::LINEAR_MIPMAP_NEAREST;
    else if (strcmp(str,"NEAREST_MIPMAP_LINEAR")==0) filter = Texture::NEAREST_MIPMAP_LINEAR;
    else if (strcmp(str,"LINEAR_MIPMAP_LINEAR")==0) filter = Texture::LINEAR_MIPMAP_LINEAR;
    else if (strcmp(str,"ANISOTROPIC")==0) filter = Texture::LINEAR;
    else return false;
    return true;
}


const char* Texture_getFilterStr(Texture::FilterMode filter)
{
    switch(filter)
    {
        case(Texture::NEAREST): return "NEAREST";
        case(Texture::LINEAR): return "LINEAR";
        case(Texture::NEAREST_MIPMAP_NEAREST): return "NEAREST_MIPMAP_NEAREST";
        case(Texture::LINEAR_MIPMAP_NEAREST): return "LINEAR_MIPMAP_NEAREST";
        case(Texture::NEAREST_MIPMAP_LINEAR): return "NEAREST_MIPMAP_LINEAR";
        case(Texture::LINEAR_MIPMAP_LINEAR): return "LINEAR_MIPMAP_LINEAR";
    }
    return "";
}

bool Texture_matchInternalFormatModeStr(const char* str,Texture::InternalFormatMode& mode)
{
    if (strcmp(str,"USE_IMAGE_DATA_FORMAT")==0)          mode = Texture::USE_IMAGE_DATA_FORMAT;
    else if (strcmp(str,"USE_USER_DEFINED_FORMAT")==0)   mode = Texture::USE_USER_DEFINED_FORMAT;
    else if (strcmp(str,"USE_ARB_COMPRESSION")==0)       mode = Texture::USE_ARB_COMPRESSION;
    else if (strcmp(str,"USE_S3TC_DXT1_COMPRESSION")==0) mode = Texture::USE_S3TC_DXT1_COMPRESSION;
    else if (strcmp(str,"USE_S3TC_DXT3_COMPRESSION")==0) mode = Texture::USE_S3TC_DXT3_COMPRESSION;
    else if (strcmp(str,"USE_S3TC_DXT5_COMPRESSION")==0) mode = Texture::USE_S3TC_DXT5_COMPRESSION;
    else return false;
    return true;
}


const char* Texture_getInternalFormatModeStr(Texture::InternalFormatMode mode)
{
    switch(mode)
    {
        case(Texture::USE_IMAGE_DATA_FORMAT):        return "USE_IMAGE_DATA_FORMAT";
        case(Texture::USE_USER_DEFINED_FORMAT):      return "USE_USER_DEFINED_FORMAT";
        case(Texture::USE_ARB_COMPRESSION):          return "USE_ARB_COMPRESSION";
        case(Texture::USE_S3TC_DXT1_COMPRESSION):    return "USE_S3TC_DXT1_COMPRESSION";
        case(Texture::USE_S3TC_DXT3_COMPRESSION):    return "USE_S3TC_DXT3_COMPRESSION";
        case(Texture::USE_S3TC_DXT5_COMPRESSION):    return "USE_S3TC_DXT5_COMPRESSION";
    }
    return "";
}


bool Texture_matchInternalFormatStr(const char* str,int& value)
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


const char* Texture_getInternalFormatStr(int value)
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
