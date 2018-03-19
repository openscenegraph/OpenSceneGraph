#include <osg/Texture>
#include <osg/Notify>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <stdlib.h>
#include <string.h>

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
bool Texture_matchSourceTypeStr(const char* str,int& value);
const char* Texture_getSourceTypeStr(int value);
bool Texture_matchShadowCompareFuncStr(const char* str,Texture::ShadowCompareFunc& value);
const char* Texture_getShadowCompareFuncStr(Texture::ShadowCompareFunc value);
bool Texture_matchShadowTextureModeStr(const char* str,Texture::ShadowTextureMode& value);
const char* Texture_getShadowTextureModeStr(Texture::ShadowTextureMode value);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Texture)
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

    if (fr.matchSequence("borderColor %f %f %f %f"))
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

    if (fr[0].matchWord("internalFormat"))
    {
        int value;
        if (Texture_matchInternalFormatStr(fr[1].getStr(),value) || fr[1].getInt(value))
        {
            texture.setInternalFormat(value);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    if (fr[0].matchWord("sourceFormat"))
    {
        int value;
        if (Texture_matchInternalFormatStr(fr[1].getStr(),value) || fr[1].getInt(value))
        {
            texture.setSourceFormat(value);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    if (fr[0].matchWord("sourceType"))
    {
        int value;
        if (Texture_matchInternalFormatStr(fr[1].getStr(),value) || fr[1].getInt(value))
        {
            texture.setSourceType(value);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    if (fr[0].matchWord("resizeNonPowerOfTwo"))
    {
        if (fr[1].matchWord("TRUE"))
        {
            texture.setResizeNonPowerOfTwoHint(true);
            fr +=2 ;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("FALSE"))
        {
            texture.setResizeNonPowerOfTwoHint(false);
            fr +=2 ;
            iteratorAdvanced = true;
        }
    }

    if (fr[0].matchWord("shadowComparison"))
    {
        if (fr[1].matchWord("TRUE"))
        {
            texture.setShadowComparison(true);
            fr +=2 ;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("FALSE"))
        {
            texture.setShadowComparison(false);
            fr +=2 ;
            iteratorAdvanced = true;
        }
    }

    if (fr[0].matchWord("shadowCompareFunc"))
    {
        Texture::ShadowCompareFunc value;
        if (Texture_matchShadowCompareFuncStr(fr[1].getStr(),value))
        {
            texture.setShadowCompareFunc(value);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    if (fr[0].matchWord("shadowTextureMode"))
    {
        Texture::ShadowTextureMode value;
        if (Texture_matchShadowTextureModeStr(fr[1].getStr(),value))
        {
            texture.setShadowTextureMode(value);
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

    if (texture.getSourceFormat())
    {
        const char* str = Texture_getInternalFormatStr(texture.getSourceFormat());

        if (str) fw.indent() << "sourceFormat " << str << std::endl;
        else fw.indent() << "sourceFormat " << texture.getSourceFormat() << std::endl;

    }

    if (texture.getSourceType())
    {
        const char* str = Texture_getSourceTypeStr(texture.getSourceType());

        if (str) fw.indent() << "sourceType " << str << std::endl;
        else fw.indent() << "sourceType " << texture.getSourceType() << std::endl;

    }

    fw.indent() << "resizeNonPowerOfTwo "<< (texture.getResizeNonPowerOfTwoHint()?"TRUE":"FALSE") << std::endl;

    fw.indent() << "shadowComparison "<< (texture.getShadowComparison()?"TRUE":"FALSE") << std::endl;

    fw.indent() << "shadowCompareFunc " << Texture_getShadowCompareFuncStr(texture.getShadowCompareFunc()) << std::endl;

    fw.indent() << "shadowTextureMode " << Texture_getShadowTextureModeStr(texture.getShadowTextureMode()) << std::endl;

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
    else if (strcmp(str,"USE_PVRTC_2BPP_COMPRESSION")==0) mode = Texture::USE_PVRTC_2BPP_COMPRESSION;
    else if (strcmp(str,"USE_PVRTC_4BPP_COMPRESSION")==0) mode = Texture::USE_PVRTC_4BPP_COMPRESSION;
    else if (strcmp(str,"USE_ETC_COMPRESSION")==0)        mode = Texture::USE_ETC_COMPRESSION;
    else if (strcmp(str,"USE_RGTC1_COMPRESSION")==0)      mode = Texture::USE_RGTC1_COMPRESSION;
    else if (strcmp(str,"USE_RGTC2_COMPRESSION")==0)      mode = Texture::USE_RGTC2_COMPRESSION;
    else if (strcmp(str,"USE_S3TC_DXT1c_COMPRESSION")==0) mode = Texture::USE_S3TC_DXT1c_COMPRESSION;
    else if (strcmp(str,"USE_S3TC_DXT1a_COMPRESSION")==0) mode = Texture::USE_S3TC_DXT1a_COMPRESSION;
    else if (strcmp(str,"USE_ETC2_COMPRESSION")==0)       mode = Texture::USE_ETC2_COMPRESSION;
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
        case(Texture::USE_PVRTC_2BPP_COMPRESSION):   return "USE_PVRTC_2BPP_COMPRESSION";
        case(Texture::USE_PVRTC_4BPP_COMPRESSION):   return "USE_PVRTC_4BPP_COMPRESSION";
        case(Texture::USE_ETC_COMPRESSION):          return "USE_ETC_COMPRESSION";
        case(Texture::USE_RGTC1_COMPRESSION):        return "USE_RGTC1_COMPRESSION";
        case(Texture::USE_RGTC2_COMPRESSION):        return "USE_RGTC2_COMPRESSION";
        case(Texture::USE_S3TC_DXT1c_COMPRESSION):   return "USE_S3TC_DXT1c_COMPRESSION";
        case(Texture::USE_S3TC_DXT1a_COMPRESSION):   return "USE_S3TC_DXT1a_COMPRESSION";
        case(Texture::USE_ETC2_COMPRESSION):         return "USE_ETC2_COMPRESSION";
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
    else
    {
        osgDB::Field::FieldType type = osgDB::Field::calculateFieldType(str);
        if (type==osgDB::Field::INTEGER)
        {
            value = atoi(str);
            return true;
        }
        else
        {
            return false;
        }
    }

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

bool Texture_matchSourceTypeStr(const char* str,int& value)
{
    if (     strcmp(str,"GL_BYTE")==0) value = GL_BYTE;
    else if (strcmp(str,"GL_SHORT")==0) value = GL_SHORT;
    else if (strcmp(str,"GL_INT")==0) value = GL_INT;
    else if (strcmp(str,"GL_UNSIGNED_BYTE")==0) value = GL_UNSIGNED_BYTE;
    else if (strcmp(str,"GL_UNSIGNED_SHORT")==0) value = GL_UNSIGNED_SHORT;
    else if (strcmp(str,"GL_UNSIGNED_INT")==0) value = GL_UNSIGNED_INT;
    else if (strcmp(str,"GL_FLOAT")==0) value = GL_FLOAT;
    else
    {
        osgDB::Field::FieldType type = osgDB::Field::calculateFieldType(str);
        if (type==osgDB::Field::INTEGER)
        {
            value = atoi(str);
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

const char* Texture_getSourceTypeStr(int value)
{
    switch(value)
    {
        case(GL_BYTE): return "GL_BYTE";
        case(GL_SHORT): return "GL_SHORT";
        case(GL_INT): return "GL_INT";
        case(GL_FLOAT): return "GL_FLOAT";
        case(GL_UNSIGNED_BYTE): return "GL_UNSIGNED_BYTE";
        case(GL_UNSIGNED_SHORT): return "GL_UNSIGNED_SHORT";
        case(GL_UNSIGNED_INT): return "GL_UNSIGNED_INT";
    }
    return NULL;
}

bool Texture_matchShadowCompareFuncStr(const char* str, Texture::ShadowCompareFunc& value)
{
    if (     strcmp(str,"GL_NEVER")==0) value = Texture::NEVER;
    else if (strcmp(str,"GL_LESS")==0) value = Texture::LESS;
    else if (strcmp(str,"GL_EQUAL")==0) value = Texture::EQUAL;
    else if (strcmp(str,"GL_LEQUAL")==0) value = Texture::LEQUAL;
    else if (strcmp(str,"GL_GREATER")==0) value = Texture::GREATER;
    else if (strcmp(str,"GL_NOTEQUAL")==0) value = Texture::NOTEQUAL;
    else if (strcmp(str,"GL_GEQUAL")==0) value = Texture::GEQUAL;
    else if (strcmp(str,"GL_ALWAYS")==0) value = Texture::ALWAYS;
    else return false;

    return true;
}

const char* Texture_getShadowCompareFuncStr(Texture::ShadowCompareFunc value)
{
    switch(value)
    {
        case(Texture::NEVER): return "GL_NEVER";
        case(Texture::LESS): return "GL_LESS";
        case(Texture::EQUAL): return "GL_EQUAL";
        case(Texture::LEQUAL): return "GL_LEQUAL";
        case(Texture::GREATER): return "GL_GREATER";
        case(Texture::NOTEQUAL): return "GL_NOTEQUAL";
        case(Texture::GEQUAL): return "GL_GEQUAL";
        case(Texture::ALWAYS): return "GL_ALWAYS";
    }
    return NULL;
}

bool Texture_matchShadowTextureModeStr(const char* str,Texture::ShadowTextureMode& value)
{
    if (     strcmp(str,"GL_LUMINANCE")==0) value = Texture::LUMINANCE;
    else if (strcmp(str,"GL_INTENSITY")==0) value = Texture::INTENSITY;
    else if (strcmp(str,"GL_ALPHA")==0) value = Texture::ALPHA;
    else if (strcmp(str,"GL_NONE")==0) value = Texture::NONE;
    else return false;

    return true;
}

const char* Texture_getShadowTextureModeStr(Texture::ShadowTextureMode value)
{
    switch(value)
    {
        case( Texture::LUMINANCE ): return "GL_LUMINANCE";
        case( Texture::INTENSITY ): return "GL_INTENSITY";
        case( Texture::ALPHA ): return "GL_ALPHA";
        case( Texture::NONE ): return "GL_NONE";
    }
    return NULL;
}
