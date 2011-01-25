#include "fbxMaterialToOsgStateSet.h"
#include <sstream>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>


static osg::Texture::WrapMode convertWrap(KFbxTexture::EWrapMode wrap)
{
    return wrap == KFbxTexture::eREPEAT ?
        osg::Texture2D::REPEAT : osg::Texture2D::CLAMP_TO_EDGE;
}

StateSetContent
FbxMaterialToOsgStateSet::convert(const KFbxSurfaceMaterial* pFbxMat)
{
    FbxMaterialMap::const_iterator it = _fbxMaterialMap.find(pFbxMat);
    if (it != _fbxMaterialMap.end())
        return it->second;

    osg::ref_ptr<osg::Material> pOsgMat = new osg::Material;
    pOsgMat->setName(pFbxMat->GetName());
    
    StateSetContent result;

    result.material = pOsgMat;

    fbxString shadingModel = pFbxMat->GetShadingModel().Get();

    const KFbxSurfaceLambert* pFbxLambert = dynamic_cast<const KFbxSurfaceLambert*>(pFbxMat);

    // diffuse map...
    const KFbxProperty lProperty = pFbxMat->FindProperty(KFbxSurfaceMaterial::sDiffuse);
    if (lProperty.IsValid())
    {
        int lNbTex = lProperty.GetSrcObjectCount(KFbxTexture::ClassId);
        for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
        {
            KFbxTexture* lTexture = KFbxCast<KFbxTexture>(lProperty.GetSrcObject(KFbxTexture::ClassId, lTextureIndex));
            if (lTexture)
            {
                result.diffuseTexture = fbxTextureToOsgTexture(lTexture);
                result.diffuseChannel = lTexture->UVSet.Get();
                result.diffuseScaleU = lTexture->GetScaleU();
                result.diffuseScaleV = lTexture->GetScaleV();
            }

            //For now only allow 1 texture
            break;
        }
    }

    // opacity map...
    const KFbxProperty lOpacityProperty = pFbxMat->FindProperty(KFbxSurfaceMaterial::sTransparentColor);
    if (lOpacityProperty.IsValid())
    {
        int lNbTex = lOpacityProperty.GetSrcObjectCount(KFbxTexture::ClassId);
        for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
        {
            KFbxTexture* lTexture = KFbxCast<KFbxTexture>(lOpacityProperty.GetSrcObject(KFbxTexture::ClassId, lTextureIndex));
            if (lTexture)
            {
                // TODO: if texture image does NOT have an alpha channel, should it be added?

                result.opacityTexture = fbxTextureToOsgTexture(lTexture);
                result.opacityChannel = lTexture->UVSet.Get();
                result.opacityScaleU = lTexture->GetScaleU();
                result.opacityScaleV = lTexture->GetScaleV();
            }

            //For now only allow 1 texture
            break;
        }
    }

    // reflection map...
    const KFbxProperty lReflectionProperty = pFbxMat->FindProperty(KFbxSurfaceMaterial::sReflection);
    if (lReflectionProperty.IsValid())
    {
        int lNbTex = lReflectionProperty.GetSrcObjectCount(KFbxTexture::ClassId);
        for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
        {
            KFbxTexture* lTexture = KFbxCast<KFbxTexture>(lReflectionProperty.GetSrcObject(KFbxTexture::ClassId, lTextureIndex));
            if (lTexture)
            {
                // support only spherical reflection maps...
                if (KFbxTexture::eUMT_ENVIRONMENT == lTexture->GetMappingType())
                {
                    result.reflectionTexture = fbxTextureToOsgTexture(lTexture);
                    result.reflectionChannel = lTexture->UVSet.Get();
                }
            }

            //For now only allow 1 texture
            break;
        }
    }

    // emissive map...
    const KFbxProperty lEmissiveProperty = pFbxMat->FindProperty(KFbxSurfaceMaterial::sEmissive);
    if (lEmissiveProperty.IsValid())
    {
        int lNbTex = lEmissiveProperty.GetSrcObjectCount(KFbxTexture::ClassId);
        for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
        {
            KFbxTexture* lTexture = KFbxCast<KFbxTexture>(lEmissiveProperty.GetSrcObject(KFbxTexture::ClassId, lTextureIndex));
            if (lTexture)
            {
                result.emissiveTexture = fbxTextureToOsgTexture(lTexture);
                result.emissiveChannel = lTexture->UVSet.Get();
                result.emissiveScaleU = lTexture->GetScaleU();
                result.emissiveScaleV = lTexture->GetScaleV();
            }

            //For now only allow 1 texture
            break;
        }
    }

    if (pFbxLambert)
    {
        fbxDouble3 color = pFbxLambert->GetDiffuseColor().Get();
        double factor = pFbxLambert->GetDiffuseFactor().Get();
        pOsgMat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(
            static_cast<float>(color[0] * factor),
            static_cast<float>(color[1] * factor),
            static_cast<float>(color[2] * factor),
            static_cast<float>(1.0 - pFbxLambert->GetTransparencyFactor().Get())));

        color = pFbxLambert->GetAmbientColor().Get();
        factor = pFbxLambert->GetAmbientFactor().Get();
        pOsgMat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(
            static_cast<float>(color[0] * factor),
            static_cast<float>(color[1] * factor),
            static_cast<float>(color[2] * factor),
            1.0f));

        color = pFbxLambert->GetEmissiveColor().Get();
        factor = pFbxLambert->GetEmissiveFactor().Get();
        pOsgMat->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(
            static_cast<float>(color[0] * factor),
            static_cast<float>(color[1] * factor),
            static_cast<float>(color[2] * factor),
            1.0f));

        if (const KFbxSurfacePhong* pFbxPhong = dynamic_cast<const KFbxSurfacePhong*>(pFbxLambert))
        {
            color = pFbxPhong->GetSpecularColor().Get();
            factor = pFbxPhong->GetSpecularFactor().Get();
            pOsgMat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(
                static_cast<float>(color[0] * factor),
                static_cast<float>(color[1] * factor),
                static_cast<float>(color[2] * factor),
                1.0f));

            pOsgMat->setShininess(osg::Material::FRONT_AND_BACK,
                static_cast<float>(pFbxPhong->GetShininess().Get()));

            // get maps factors...
            result.diffuseFactor = pFbxPhong->GetDiffuseFactor().Get();
            result.reflectionFactor = pFbxPhong->GetReflectionFactor().Get();
            result.opacityFactor = pFbxPhong->GetTransparencyFactor().Get();
            // get more factors here...
        }
    }

    if (_lightmapTextures)
    {
        // if using an emission map then adjust material properties accordingly...
        if (result.emissiveTexture)
        {
            osg::Vec4 diffuse = pOsgMat->getDiffuse(osg::Material::FRONT_AND_BACK);
            pOsgMat->setEmission(osg::Material::FRONT_AND_BACK, diffuse);
            pOsgMat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,diffuse.a()));
            pOsgMat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,diffuse.a()));
        }
    }

    _fbxMaterialMap.insert(FbxMaterialMap::value_type(pFbxMat, result));
    return result;
}

osg::ref_ptr<osg::Texture2D>
FbxMaterialToOsgStateSet::fbxTextureToOsgTexture(const KFbxTexture* fbx)
{
    ImageMap::iterator it = _imageMap.find(fbx->GetFileName());
    if (it != _imageMap.end())
        return it->second;
    osg::ref_ptr<osg::Image> pImage = NULL;

    // Warning: fbx->GetRelativeFileName() is relative TO EXECUTION DIR
    //          fbx->GetFileName() is as stored initially in the FBX
    if ((pImage = osgDB::readImageFile(osgDB::concatPaths(_dir, fbx->GetFileName()), _options)) ||                // First try "export dir/name"
        (pImage = osgDB::readImageFile(fbx->GetFileName(), _options)) ||                                        // Then try  "name" (if absolute)
        (pImage = osgDB::readImageFile(osgDB::concatPaths(_dir, fbx->GetRelativeFileName()), _options)))        // Else try  "current dir/name"
    {
        osg::ref_ptr<osg::Texture2D> pOsgTex = new osg::Texture2D;
        pOsgTex->setImage(pImage.get());
        pOsgTex->setWrap(osg::Texture2D::WRAP_S, convertWrap(fbx->GetWrapModeU()));
        pOsgTex->setWrap(osg::Texture2D::WRAP_T, convertWrap(fbx->GetWrapModeV()));
        _imageMap.insert(std::make_pair(fbx->GetFileName(), pOsgTex.get()));
        return pOsgTex;
    }
    else
    {
        return NULL;
    }
}

void FbxMaterialToOsgStateSet::checkInvertTransparency()
{
    int zeroAlpha = 0, oneAlpha = 0;
    for (FbxMaterialMap::const_iterator it = _fbxMaterialMap.begin(); it != _fbxMaterialMap.end(); ++it)
    {
        const osg::Material* pMaterial = it->second.material.get();
        float alpha = pMaterial->getDiffuse(osg::Material::FRONT).a();
        if (alpha > 0.999f)
        {
            ++oneAlpha;
        }
        else if (alpha < 0.001f)
        {
            ++zeroAlpha;
        }
    }

    if (zeroAlpha > oneAlpha)
    {
        //Transparency values seem to be back to front so invert them.

        for (FbxMaterialMap::const_iterator it = _fbxMaterialMap.begin(); it != _fbxMaterialMap.end(); ++it)
        {
            osg::Material* pMaterial = it->second.material.get();
            osg::Vec4 diffuse = pMaterial->getDiffuse(osg::Material::FRONT);
            diffuse.a() = 1.0f - diffuse.a();
            pMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, diffuse);
        }
    }
}