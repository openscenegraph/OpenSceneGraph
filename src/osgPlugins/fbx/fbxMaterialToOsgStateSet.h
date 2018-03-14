#ifndef FBXMATERIALTOOSGSTATESET_H
#define FBXMATERIALTOOSGSTATESET_H

#include <map>
#include <memory>
#include <osg/Material>
#include <osg/StateSet>
#include <osgDB/Options>
#include <osg/Texture2D>

#if defined(_MSC_VER)
#pragma warning( disable : 4505 )
#pragma warning( default : 4996 )
#endif
#include <fbxsdk.h>

//The only things we need to create a new StateSet are texture and materials. So we store that in a pair.
//We Don't store directly in stateSet because getOrCreateStateSet function set some parameters.

struct TextureDetails : public osg::Referenced
{
    TextureDetails();

    bool valid() const { return texture.valid(); }

    bool transparent() const;

    void assignTextureIfRequired(osg::StateSet* stateSet, unsigned int unit);
    void assignTexMatIfRequired(osg::StateSet* stateSet, unsigned int unit);

    std::string                     channel;
    osg::ref_ptr<osg::Texture2D>    texture;
    double                          factor;
    osg::Vec2d                      scale;
};

struct StateSetContent
{
    StateSetContent()
    {
    }

    // texture units (eventually used for each texture map)...
    enum TextureUnit
    {
        DIFFUSE_TEXTURE_UNIT = 0,
        OPACITY_TEXTURE_UNIT,
        REFLECTION_TEXTURE_UNIT,
        EMISSIVE_TEXTURE_UNIT,
        AMBIENT_TEXTURE_UNIT,
        NORMAL_TEXTURE_UNIT,
        SPECULAR_TEXTURE_UNIT,
        SHININESS_TEXTURE_UNIT
        // more texture units here...
    };



    osg::ref_ptr<osg::Material> material;

    osg::ref_ptr<TextureDetails> diffuse;
    osg::ref_ptr<TextureDetails> opacity;
    osg::ref_ptr<TextureDetails> reflection;
    osg::ref_ptr<TextureDetails> emissive;
    osg::ref_ptr<TextureDetails> ambient;
    osg::ref_ptr<TextureDetails> normalMap;
    osg::ref_ptr<TextureDetails> specular;
    osg::ref_ptr<TextureDetails> shininess;
};

//We use the pointers set by the importer to not duplicate materials and textures.
typedef std::map<const FbxSurfaceMaterial *, StateSetContent> FbxMaterialMap;

//This map is used to not load the same image more than 1 time.
typedef std::map<std::string, osg::Texture2D *> ImageMap;

class FbxMaterialToOsgStateSet
{
public:
    //Convert a FbxSurfaceMaterial to a osgMaterial and an osgTexture.
    StateSetContent convert(const FbxSurfaceMaterial* pFbxMat);

    //dir is the directory where fbx is stored (for relative path).
    FbxMaterialToOsgStateSet(const std::string& dir, const osgDB::Options* options, bool lightmapTextures) :
        _options(options),
        _dir(dir),
        _lightmapTextures(lightmapTextures){}

    void checkInvertTransparency();

private:
    //Convert a texture fbx to an osg texture.
    osg::ref_ptr<osg::Texture2D> fbxTextureToOsgTexture(const FbxFileTexture* pOsgTex);

    FbxFileTexture* selectFbxFileTexture(const FbxProperty& lProperty);

    TextureDetails* selectTextureDetails(const FbxProperty& lProperty);

    FbxMaterialMap       _fbxMaterialMap;
    ImageMap              _imageMap;
    const osgDB::Options* _options;
    const std::string     _dir;
    bool                  _lightmapTextures;
};


#endif //FBXMATERIALTOOSGSTATESET_H
