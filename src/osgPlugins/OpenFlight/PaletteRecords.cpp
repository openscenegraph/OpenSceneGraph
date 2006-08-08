//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include <assert.h>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/BlendFunc>
#include <osgSim/LightPointNode>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include "Registry.h"
#include "Document.h"
#include "AttrData.h"
#include "RecordInputStream.h"

namespace flt {

class VertexPalette : public Record
{
public:

    VertexPalette() {}

    META_Record(VertexPalette)

protected:

    virtual ~VertexPalette() {}

    virtual void readRecord(RecordInputStream& in, Document& document)
    {
        uint32 paletteSize = in.readUInt32();
        in.moveToStartOfRecord();

        std::string buffer(paletteSize,'\0');
        in.read(&(*buffer.begin()),paletteSize);
        document.setVertexPool(new VertexPool(buffer));
    }
};

RegisterRecordProxy<VertexPalette> g_VertexPalette(VERTEX_PALETTE_OP);


class ColorPalette : public Record
{
public:

    ColorPalette() {}

    META_Record(ColorPalette)

protected:

    virtual ~ColorPalette() {}

    virtual void readRecord(RecordInputStream& in, Document& document)
    {
        if (document.getColorPoolParent())
            // Using parent's color pool -- ignore this record.
            return;

        if (document.version() > VERSION_13)
        {
            bool oldVersion = false;
            bool colorNameSection = in.getRecordSize() > 4228;
            int maxColors = (document.version()>=VERSION_15_1) ? 1024 : 512;

            // It might be less.
            if (!colorNameSection)
            {
                // Max colors calculated by record size.
                int maxColorsByRecordSize = (in.getRecordBodySize()-128) / 4;
                if (maxColorsByRecordSize < maxColors)
                    maxColors = maxColorsByRecordSize;
            }

            ColorPool* cp = new ColorPool(oldVersion,maxColors);
            document.setColorPool(cp);

            in.forward(128);
            for (int i=0; i<maxColors; i++)
            {
                uint8 alpha = in.readUInt8(1);
                uint8 blue  = in.readUInt8(1);
                uint8 green = in.readUInt8(1);
                uint8 red   = in.readUInt8(1);

                (*cp)[i] = osg::Vec4((float)red/255,(float)green/255,(float)blue/255,(float)alpha/255);
            }
        }
        else // version <= 13
        {
            bool oldVersion = true;
            int maxColors = 32+56;

            ColorPool* cp = new ColorPool(oldVersion,maxColors);
            document.setColorPool(cp);

            // variable intensity
            for (int i=0; i < 32; i++)
            {
                uint16 red   = in.readUInt16(1);
                uint16 green = in.readUInt16(1);
                uint16 blue  = in.readUInt16(1);
                (*cp)[i] = osg::Vec4((float)red/255,(float)green/255,(float)blue/255,1);
            }

            // fixed intensity
            for (int i=0; i < 56; i++)
            {
                uint16 red   = in.readUInt16(1);
                uint16 green = in.readUInt16(1);
                uint16 blue  = in.readUInt16(1);
                (*cp)[i+32] = osg::Vec4((float)red/255,(float)green/255,(float)blue/255,1);
            }
        }
    }
};


RegisterRecordProxy<ColorPalette> g_ColorPalette(COLOR_PALETTE_OP);


class NameTable : public Record
{
public:

    NameTable() {}

    META_Record(NameTable)

protected:

    virtual ~NameTable() {}

    virtual void readRecord(RecordInputStream& /*in*/, Document& /*document*/)
    {
    }
};

RegisterRecordProxy<NameTable> g_NameTable(NAME_TABLE_OP);


class MaterialPalette : public Record
{
public:

    MaterialPalette() {}

    META_Record(MaterialPalette)

protected:

    virtual ~MaterialPalette() {}

    virtual void readRecord(RecordInputStream& in, Document& document)
    {
        if (document.getMaterialPoolParent())
            // Using parent's material pool -- ignore this record.
            return;

        int32 index = in.readInt32();
        std::string name = in.readString(12);
        /*uint32 flags =*/ in.readUInt32();
        osg::Vec3f ambient = in.readVec3f();
        osg::Vec3f diffuse = in.readVec3f();
        osg::Vec3f specular = in.readVec3f();
        osg::Vec3f emissive = in.readVec3f();
        float32 shininess = in.readFloat32();
        float32 alpha = in.readFloat32();

        osg::Material* material = new osg::Material;
        material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(ambient,alpha));
        material->setDiffuse (osg::Material::FRONT_AND_BACK,osg::Vec4(diffuse,alpha));
        material->setSpecular(osg::Material::FRONT_AND_BACK,osg::Vec4(specular,alpha));
        material->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(emissive,alpha));
        material->setShininess(osg::Material::FRONT_AND_BACK,shininess);
        
        MaterialPool* mp = document.getOrCreateMaterialPool();
        (*mp)[index] = material;
    }
};

RegisterRecordProxy<MaterialPalette> g_MaterialPalette(MATERIAL_PALETTE_OP);


class OldMaterialPalette : public Record
{
public:

    OldMaterialPalette() {}

    META_Record(OldMaterialPalette)

protected:

    virtual ~OldMaterialPalette() {}

    virtual void readRecord(RecordInputStream& in, Document& document)
    {
        if (document.getMaterialPoolParent())
            // Using parent's material pool -- ignore this record.
            return;

        for (int i=0; i < 64; i++)
        {
            osg::Vec3f ambient = in.readVec3f();
            osg::Vec3f diffuse = in.readVec3f();
            osg::Vec3f specular = in.readVec3f();
            osg::Vec3f emissive = in.readVec3f();
            float32 shininess = in.readFloat32();
            float32 alpha = in.readFloat32();
            /*uint32 flags =*/ in.readUInt32();
            std::string name = in.readString(12);
            in.forward(4*28);
            
            osg::Material* material = new osg::Material;
            material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(ambient,alpha));
            material->setDiffuse (osg::Material::FRONT_AND_BACK,osg::Vec4(diffuse,alpha));
            material->setSpecular(osg::Material::FRONT_AND_BACK,osg::Vec4(specular,alpha));
            material->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(emissive,alpha));
            material->setShininess(osg::Material::FRONT_AND_BACK,shininess);
            
            MaterialPool* mp = document.getOrCreateMaterialPool();
            (*mp)[i] = material;
        }
    }

};

RegisterRecordProxy<OldMaterialPalette> g_OldMaterialPalette(OLD_MATERIAL_PALETTE_OP);


class TexturePalette : public Record
{
public:

    TexturePalette() {}

    META_Record(TexturePalette)
    
protected:

    virtual ~TexturePalette() {}

    osg::Texture2D::WrapMode convertWrapMode( int32 wrap )
    {
        switch( wrap )
        {
        case AttrData::WRAP_CLAMP:
            return osg::Texture2D::CLAMP;
            break;
        case AttrData::WRAP_MIRRORED_REPEAT:
            return osg::Texture2D::MIRROR;
            break;
        default:
        case AttrData::WRAP_REPEAT:
            return osg::Texture2D::REPEAT;
            break;
        }
        return osg::Texture2D::REPEAT;
    }

    virtual void readRecord(RecordInputStream& in, Document& document)
    {
        if (document.getTexturePoolParent())
            // Using parent's texture pool -- ignore this record.
            return;

        int maxLength = (document.version() < VERSION_14) ? 80 : 200;
        std::string filename = in.readString(maxLength);
        int32 index = in.readInt32(-1);
        /*int32 x =*/ in.readInt32();
        /*int32 y =*/ in.readInt32();

        osg::ref_ptr<osg::Image> image = osgDB::readImageFile(filename,document.getOptions());
        if (!image.valid())
        {
            osg::notify(osg::WARN) << "Can't find texture (" << index << ") " << filename << std::endl;
            return;
        }

        // Create stateset
        osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;

        osg::Texture2D* texture = new osg::Texture2D;
        texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::REPEAT);
        texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::REPEAT);
        texture->setResizeNonPowerOfTwoHint(true);
        texture->setImage(image.get());
        stateset->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

        // Read attribute file
        std::string attrname = filename + ".attr";
        osg::ref_ptr<AttrData> attr = dynamic_cast<AttrData*>(osgDB::readObjectFile(attrname,document.getOptions()));
        if (attr.valid())
        {
            // Wrap mode
            osg::Texture2D::WrapMode wrap_s = convertWrapMode( attr->wrapMode_u );
            texture->setWrap(osg::Texture2D::WRAP_S,wrap_s);

            osg::Texture2D::WrapMode wrap_t = convertWrapMode( attr->wrapMode_v );
            texture->setWrap(osg::Texture2D::WRAP_T,wrap_t);

            // Min filter
            switch (attr->minFilterMode)
            {
            case AttrData::MIN_FILTER_POINT:
                texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
                break;
            case AttrData::MIN_FILTER_BILINEAR:
                texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
                break;
            case AttrData::MIN_FILTER_MIPMAP_POINT:
                texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST_MIPMAP_NEAREST);
                break;
            case AttrData::MIN_FILTER_MIPMAP_LINEAR:
                texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST_MIPMAP_LINEAR);
                break;
            case AttrData::MIN_FILTER_MIPMAP_BILINEAR:
                texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR_MIPMAP_NEAREST);
                break;
            case AttrData::MIN_FILTER_MIPMAP_TRILINEAR:
                texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR_MIPMAP_LINEAR);
                break;
            case AttrData::MIN_FILTER_BICUBIC:
            case AttrData::MIN_FILTER_BILINEAR_GEQUAL:
            case AttrData::MIN_FILTER_BILINEAR_LEQUAL:
            case AttrData::MIN_FILTER_BICUBIC_GEQUAL:
            case AttrData::MIN_FILTER_BICUBIC_LEQUAL:
                texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR_MIPMAP_NEAREST);
                break;
            default:
                texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR_MIPMAP_LINEAR);
                break;
            }

            // Mag filter
            switch (attr->magFilterMode)
            {
            case AttrData::MAG_FILTER_POINT:
                texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
                break;
            case AttrData::MAG_FILTER_BILINEAR:
            case AttrData::MAG_FILTER_BILINEAR_GEQUAL:
            case AttrData::MAG_FILTER_BILINEAR_LEQUAL:
            case AttrData::MAG_FILTER_SHARPEN:
            case AttrData::MAG_FILTER_BICUBIC:
            case AttrData::MAG_FILTER_BICUBIC_GEQUAL:
            case AttrData::MAG_FILTER_BICUBIC_LEQUAL:
            case AttrData::MAG_FILTER_ADD_DETAIL:
            case AttrData::MAG_FILTER_MODULATE_DETAIL:
                texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
                break;
            }

            osg::TexEnv* texenv = new osg::TexEnv;
            switch (attr->texEnvMode)
            {
            case AttrData::TEXENV_MODULATE:
                texenv->setMode(osg::TexEnv::MODULATE);
                break;
            case AttrData::TEXENV_BLEND:
                texenv->setMode(osg::TexEnv::BLEND);
                break;
            case AttrData::TEXENV_DECAL:
                texenv->setMode(osg::TexEnv::DECAL);
                break;
            case AttrData::TEXENV_COLOR:
                texenv->setMode(osg::TexEnv::REPLACE);
                break;
            case AttrData::TEXENV_ADD:
                texenv->setMode(osg::TexEnv::ADD);
                break;
            }
            stateset->setTextureAttribute(0, texenv);
        }

        TexturePool* tp = document.getOrCreateTexturePool();
        (*tp)[index] = stateset.get();
    }
};

RegisterRecordProxy<TexturePalette> g_TexturePalette(TEXTURE_PALETTE_OP);


class EyepointAndTrackplanePalette : public Record
{
public:

    EyepointAndTrackplanePalette() {}

    META_Record(EyepointAndTrackplanePalette)

protected:

    virtual ~EyepointAndTrackplanePalette() {}

    virtual void readRecord(RecordInputStream& /*in*/, Document& /*document*/) {}
};

RegisterRecordProxy<EyepointAndTrackplanePalette> g_EyepointAndTrackplanePalette(EYEPOINT_AND_TRACKPLANE_PALETTE_OP);


class LinkagePalette : public Record
{
public:

    LinkagePalette() {}

    META_Record(LinkagePalette)

protected:

    virtual ~LinkagePalette() {}

    virtual void readRecord(RecordInputStream& /*in*/, Document& /*document*/) {}
};

RegisterRecordProxy<LinkagePalette> g_LinkagePalette(LINKAGE_PALETTE_OP);


class SoundPalette : public Record
{
public:

    SoundPalette() {}

    META_Record(SoundPalette)

protected:

    virtual ~SoundPalette() {}

    virtual void readRecord(RecordInputStream& /*in*/, Document& /*document*/) {}
};

RegisterRecordProxy<SoundPalette> g_SoundPalette(SOUND_PALETTE_OP);


class LightSourcePalette : public Record
{
public:

    LightSourcePalette() {}

    META_Record(LightSourcePalette)

protected:

    virtual ~LightSourcePalette() {}

    virtual void readRecord(RecordInputStream& /*in*/, Document& /*document*/) {}
};

RegisterRecordProxy<LightSourcePalette> g_LightSourcePalette(LIGHT_SOURCE_PALETTE_OP);


class LightPointAppearancePalette : public Record
{
public:

    LightPointAppearancePalette() {}

    META_Record(LightPointAppearancePalette)

protected:

    virtual ~LightPointAppearancePalette() {}

    virtual void readRecord(RecordInputStream& in, Document& document)
    {
        if (document.getLightPointAppearancePoolParent())
            // Using parent's light point appearance pool -- ignore this record.
            return;

        osg::ref_ptr<LPAppearance> appearance = new LPAppearance;

        in.forward(4);
        appearance->name = in.readString(256);
        appearance->index = in.readInt32(-1);
        appearance->materialCode = in.readInt16();
        appearance->featureID = in.readInt16();

        int32 backColorIndex = in.readInt32();        
        appearance->backColor = document.getColorPool() ? 
                            document.getColorPool()->getColor(backColorIndex) : 
                            osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f);

        appearance->displayMode = in.readInt32();
        appearance->intensityFront = in.readFloat32();
        appearance->intensityBack = in.readFloat32();
        appearance->minDefocus = in.readFloat32();
        appearance->maxDefocus = in.readFloat32();
        appearance->fadingMode = in.readInt32();
        appearance->fogPunchMode = in.readInt32();
        appearance->directionalMode = in.readInt32();
        appearance->rangeMode = in.readInt32();
        appearance->minPixelSize = in.readFloat32();
        appearance->maxPixelSize = in.readFloat32();
        appearance->actualPixelSize = in.readFloat32();
        appearance->transparentFalloffPixelSize = in.readFloat32();
        appearance->transparentFalloffExponent = in.readFloat32();
        appearance->transparentFalloffScalar = in.readFloat32();
        appearance->transparentFalloffClamp = in.readFloat32();
        appearance->fogScalar = in.readFloat32();
        appearance->fogIntensity = in.readFloat32();
        appearance->sizeDifferenceThreshold = in.readFloat32();
        appearance->directionality = in.readInt32();
        appearance->horizontalLobeAngle = in.readFloat32();
        appearance->verticalLobeAngle = in.readFloat32();
        appearance->lobeRollAngle = in.readFloat32();
        appearance->directionalFalloffExponent = in.readFloat32();
        appearance->directionalAmbientIntensity = in.readFloat32();
        appearance->significance = in.readFloat32();
        appearance->flags = in.readUInt32();
        appearance->visibilityRange = in.readFloat32();
        appearance->fadeRangeRatio = in.readFloat32();
        appearance->fadeInDuration = in.readFloat32();
        appearance->fadeOutDuration = in.readFloat32();
        appearance->LODRangeRatio = in.readFloat32();
        appearance->LODScale = in.readFloat32();
        appearance->texturePatternIndex = in.readInt16(-1);
        // The final short is reserved; don't bother reading it.
 
        // Add to pool
        LightPointAppearancePool* lpaPool = document.getOrCreateLightPointAppearancePool();
        (*lpaPool)[appearance->index] = appearance.get();
    }

};

RegisterRecordProxy<LightPointAppearancePalette> g_LightPointAppearancePalette(LIGHT_POINT_APPEARANCE_PALETTE_OP);


class LightPointAnimationPalette : public Record
{
public:

    LightPointAnimationPalette() {}

    META_Record(LightPointAnimationPalette)

protected:

    virtual ~LightPointAnimationPalette() {}

    virtual void readRecord(RecordInputStream& in, Document& /*document*/)
    {
        in.forward(4);
        std::string name = in.readString(256);
        /*int32 index =*/ in.readInt32(-1);
        /*float32 animationPeriod =*/ in.readFloat32();
        /*float32 animationPhaseDelay =*/ in.readFloat32();
        /*float32 animationEnabledPeriod =*/ in.readFloat32();
        /*osg::Vec3f axisOfRotation =*/ in.readVec3f();
        /*uint32 flags =*/ in.readUInt32();
        /*int32 animationType =*/ in.readInt32();
        /*int32 morseCodeTiming =*/ in.readInt32();
        /*int32 wordRate =*/ in.readInt32();
        /*int32 characterRate =*/ in.readInt32();
        std::string morseCodeString = in.readString(1024);
        int32 numberOfSequences = in.readInt32();
        for (int n=0; n<numberOfSequences; ++n)
        {
            /*uint32 sequenceState =*/ in.readUInt32();
            /*float32 sequenceDuration =*/ in.readFloat32();
            /*osg::Vec4f sequenceColor =*/ in.readColor32();
        }
    }
};

RegisterRecordProxy<LightPointAnimationPalette> g_LightPointAnimationPalette(LIGHT_POINT_ANIMATION_PALETTE_OP);


class LineStylePalette : public Record
{
public:

    LineStylePalette() {}

    META_Record(LineStylePalette)

protected:

    virtual ~LineStylePalette() {}

    virtual void readRecord(RecordInputStream& /*in*/, Document& /*document*/)
    {
    }
};

RegisterRecordProxy<LineStylePalette> g_LineStylePalette(LINE_STYLE_PALETTE_OP);


class TextureMappingPalette : public Record
{
public:

    TextureMappingPalette() {}

    META_Record(TextureMappingPalette)

protected:

    virtual ~TextureMappingPalette() {}

    virtual void readRecord(RecordInputStream& /*in*/, Document& /*document*/)
    {
    }
};

RegisterRecordProxy<TextureMappingPalette> g_TextureMappingPalette(TEXTURE_MAPPING_PALETTE_OP);


class ShaderPalette : public Record
{
public:

    ShaderPalette() {}

    META_Record(ShaderPalette)

    enum ShaderType
    {
        CG=0,
        CGFX=1,
        GLSL=2
    };

protected:

    virtual ~ShaderPalette() {}

    virtual void readRecord(RecordInputStream& in, Document& document)
    {
        if (document.getShaderPoolParent())
            // Using parent's shader pool -- ignore this record.
            return;

        int32 index = in.readInt32(-1);
        int32 type = in.readInt32(-1);
        std::string name = in.readString(1024);

        if (type == CG)
        {
            // CG support is currently not implemented. Just parse.
            std::string vertexProgramFilename = in.readString(1024);
            std::string fragmentProgramFilename = in.readString(1024);
            /*int32 vertexProgramProfile =*/ in.readInt32();
            /*int32 fragmentProgramProfile =*/ in.readInt32();
            std::string vertexProgramEntry = in.readString(256);
            std::string fragmentProgramEntry = in.readString(256);
        }
        else if (type == GLSL)
        {
            int32 vertexProgramFileCount(1);
            int32 fragmentProgramFileCount(1);

            if (document.version() >= VERSION_16_1)
            {
                // In 16.1, possibly multiple filenames for each vertex and fragment program.
                vertexProgramFileCount = in.readInt32();
                fragmentProgramFileCount = in.readInt32();
            }
            // else 16.0
            //   Technically, 16.0 didn't support GLSL, but this plugin
            //   supports it with a single vertex shader filename and a
            //   single fragment shader filename.

            osg::Program* program = new osg::Program;
            program->setName(name);

            int idx;
            for( idx=0; idx<vertexProgramFileCount; idx++)
            {
                std::string vertexProgramFilename = in.readString(1024);

                std::string vertexProgramFilePath = osgDB::findDataFile(vertexProgramFilename);
                if (!vertexProgramFilePath.empty())
                {
                    osg::Shader* vertexShader = osg::Shader::readShaderFile(osg::Shader::VERTEX, vertexProgramFilePath);
                    if (vertexShader)
                        program->addShader( vertexShader );
                }
            }
            for( idx=0; idx<fragmentProgramFileCount; idx++)
            {
                std::string fragmentProgramFilename = in.readString(1024);

                std::string fragmentProgramFilePath = osgDB::findDataFile(fragmentProgramFilename);
                if (!fragmentProgramFilePath.empty())
                {
                    osg::Shader* fragmentShader = osg::Shader::readShaderFile(osg::Shader::FRAGMENT, fragmentProgramFilePath);
                    if (fragmentShader)
                        program->addShader( fragmentShader );
                }
            }

            ShaderPool* shaderPool = document.getOrCreateShaderPool();
            (*shaderPool)[index] = program;
        }
    }
};

RegisterRecordProxy<ShaderPalette> g_ShaderPalette(SHADER_PALETTE_OP);

} // end namespace


