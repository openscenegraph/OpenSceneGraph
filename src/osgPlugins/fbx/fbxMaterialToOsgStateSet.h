#ifndef FBXMATERIALTOOSGSTATESET_H
#define FBXMATERIALTOOSGSTATESET_H

#include <map>
#include <memory>
#include <osg/Material>
#include <osg/StateSet>
#include <osgDB/ReaderWriter>
#include <osg/Texture2D>

#if defined(_MSC_VER)
#pragma warning( disable : 4505 )
#endif
#include <fbxsdk.h>

//The only things we need to create a new StateSet are texture and materials. So we store that in a pair.
//We Don't store directly in stateSet because getOrCreateStateSet function set some parameters.
typedef std::pair<osg::Material *, osg::Texture2D *> StateSetContent;

//We use the pointers setted by the importer to not duplicate materials and textures.
typedef std::map<const KFbxSurfaceMaterial *, StateSetContent> KFbxMaterialMap;

//This map is used to not load the same image more than 1 time.
typedef std::map<std::string, osg::Texture2D *> ImageMap;

class FbxMaterialToOsgStateSet
{
public:
    //Convert a KfbxSurfaceMaterial to a osgMaterial and an osgTexture.
    StateSetContent convert(const KFbxSurfaceMaterial* pFbxMat);

    //dir is the directory where fbx is stored (for relative path).
    FbxMaterialToOsgStateSet::FbxMaterialToOsgStateSet(const std::string& dir, const osgDB::ReaderWriter::Options* options) :
        _options(options),
        _dir(dir) {}

    void checkInvertTransparency();
private:
    //Convert a texture fbx to an osg texture.
    osg::ref_ptr<osg::Texture2D>
    fbxTextureToOsgTexture(const KFbxTexture* pOsgTex);
    KFbxMaterialMap       _kFbxMaterialMap;
    ImageMap              _imageMap;
    const osgDB::ReaderWriter::Options* _options;
    const std::string     _dir;
};


#endif //FBXMATERIALTOOSGSTATESET_H