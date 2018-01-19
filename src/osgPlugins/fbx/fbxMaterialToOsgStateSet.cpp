#include "fbxMaterialToOsgStateSet.h"
#include <sstream>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>


static osg::Texture::WrapMode convertWrap(FbxFileTexture::EWrapMode wrap)
{
    return wrap == FbxFileTexture::eRepeat ?
        osg::Texture2D::REPEAT : osg::Texture2D::CLAMP_TO_EDGE;
}

StateSetContent
FbxMaterialToOsgStateSet::convert(const FbxSurfaceMaterial* pFbxMat)
{
    FbxMaterialMap::const_iterator it = _fbxMaterialMap.find(pFbxMat);
    if (it != _fbxMaterialMap.end())
        return it->second;

    osg::ref_ptr<osg::Material> pOsgMat = new osg::Material;
    pOsgMat->setName(pFbxMat->GetName());

    StateSetContent result;

    result.material = pOsgMat;

    FbxString shadingModel = pFbxMat->ShadingModel.Get();

    const FbxSurfaceLambert* pFbxLambert = FbxCast<FbxSurfaceLambert>(pFbxMat);

    // diffuse map...
    const FbxProperty lProperty = pFbxMat->FindProperty(FbxSurfaceMaterial::sDiffuse);
    if (lProperty.IsValid())
    {
        // check if layered textures are used...
        int layeredTextureCount = lProperty.GetSrcObjectCount<FbxLayeredTexture>();
        if(layeredTextureCount)
        {
            FbxLayeredTexture* layered_texture = FbxCast<FbxLayeredTexture>(lProperty.GetSrcObject<FbxLayeredTexture>(0));
            int lNbTex = layered_texture->GetSrcObjectCount<FbxFileTexture>();

            if(lNbTex)
            {
                // at least 1 texture, use 1st...
                for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
                {
                    FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(layered_texture->GetSrcObject<FbxFileTexture>(lTextureIndex));
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
            else
                OSG_WARN << "FBX: Missing Textures in FbxLayeredTexture." << std::endl;
        }
        else
        {
            int lNbTex = lProperty.GetSrcObjectCount<FbxFileTexture>();
            for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
            {
                FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(lProperty.GetSrcObject<FbxFileTexture>(lTextureIndex));
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
    }

    double transparencyColorFactor = 1.0;
    bool useTransparencyColorFactor = false;

    // opacity map...
    const FbxProperty lOpacityProperty = pFbxMat->FindProperty(FbxSurfaceMaterial::sTransparentColor);
    if (lOpacityProperty.IsValid())
    {
        FbxDouble3 transparentColor = lOpacityProperty.Get<FbxDouble3>();
        // If transparent color is defined set the transparentFactor to gray scale value of transparentColor
        if (transparentColor[0] < 1.0 || transparentColor[1] < 1.0 || transparentColor[2] < 1.0) {
            transparencyColorFactor = transparentColor[0]*0.30 + transparentColor[1]*0.59 + transparentColor[2]*0.11;
            useTransparencyColorFactor = true;
        }

        // check if layered textures are used...
        int layeredTextureCount = lOpacityProperty.GetSrcObjectCount<FbxLayeredTexture>();
        if(layeredTextureCount)
        {
            FbxLayeredTexture* layered_texture = FbxCast<FbxLayeredTexture>(lOpacityProperty.GetSrcObject<FbxLayeredTexture>(0));
            int lNbTex = layered_texture->GetSrcObjectCount<FbxFileTexture>();

            if(lNbTex)
            {
                // at least 1 texture, use 1st...
                for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
                {
                    FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(layered_texture->GetSrcObject<FbxFileTexture>(lTextureIndex));
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
            else
                OSG_WARN << "FBX: Missing Textures in FbxLayeredTexture." << std::endl;
        }
        else
        {
            int lNbTex = lOpacityProperty.GetSrcObjectCount<FbxFileTexture>();
            for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
            {
                FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(lOpacityProperty.GetSrcObject<FbxFileTexture>(lTextureIndex));
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
    }

    // reflection map...
    const FbxProperty lReflectionProperty = pFbxMat->FindProperty(FbxSurfaceMaterial::sReflection);
    if (lReflectionProperty.IsValid())
    {
        // check if layered textures are used...
        int layeredTextureCount = lReflectionProperty.GetSrcObjectCount<FbxLayeredTexture>();
        if(layeredTextureCount)
        {
            FbxLayeredTexture* layered_texture = FbxCast<FbxLayeredTexture>(lReflectionProperty.GetSrcObject<FbxLayeredTexture>(0));
            int lNbTex = layered_texture->GetSrcObjectCount<FbxFileTexture>();

            if(lNbTex)
            {
                // at least 1 texture, use 1st...
                for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
                {
                    FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(layered_texture->GetSrcObject<FbxFileTexture>(lTextureIndex));
                    if (lTexture)
                    {
                        // support only spherical reflection maps...
                        if (FbxFileTexture::eUMT_ENVIRONMENT == lTexture->CurrentMappingType.Get())
                        {
                            result.reflectionTexture = fbxTextureToOsgTexture(lTexture);
                            result.reflectionChannel = lTexture->UVSet.Get();
                        }
                    }

                    //For now only allow 1 texture
                    break;
                }
            }
            else
                OSG_WARN << "FBX: Missing Textures in FbxLayeredTexture." << std::endl;
        }
        else
        {
            int lNbTex = lReflectionProperty.GetSrcObjectCount<FbxFileTexture>();
            for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
            {
                FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(lReflectionProperty.GetSrcObject<FbxFileTexture>(lTextureIndex));
                if (lTexture)
                {
                    // support only spherical reflection maps...
                    if (FbxFileTexture::eUMT_ENVIRONMENT == lTexture->CurrentMappingType.Get())
                    {
                        result.reflectionTexture = fbxTextureToOsgTexture(lTexture);
                        result.reflectionChannel = lTexture->UVSet.Get();
                    }
                }

                //For now only allow 1 texture
                break;
            }
        }
    }

    // emissive map...
    const FbxProperty lEmissiveProperty = pFbxMat->FindProperty(FbxSurfaceMaterial::sEmissive);
    if (lEmissiveProperty.IsValid())
    {
        // check if layered textures are used...
        int layeredTextureCount = lEmissiveProperty.GetSrcObjectCount<FbxLayeredTexture>();
        if(layeredTextureCount)
        {
            FbxLayeredTexture* layered_texture = FbxCast<FbxLayeredTexture>(lEmissiveProperty.GetSrcObject<FbxLayeredTexture>(0));
            int lNbTex = layered_texture->GetSrcObjectCount<FbxFileTexture>();

            if(lNbTex)
            {
                // at least 1 texture, use 1st...
                for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
                {
                    FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(layered_texture->GetSrcObject<FbxFileTexture>(lTextureIndex));
                    if (lTexture)
                    {
                        // support only spherical reflection maps...
                        result.emissiveTexture = fbxTextureToOsgTexture(lTexture);
                        result.emissiveChannel = lTexture->UVSet.Get();
                        result.emissiveScaleU = lTexture->GetScaleU();
                        result.emissiveScaleV = lTexture->GetScaleV();
                    }

                    //For now only allow 1 texture
                    break;
                }
            }
            else
                OSG_WARN << "FBX: Missing Textures in FbxLayeredTexture." << std::endl;
        }
        else
        {
            int lNbTex = lEmissiveProperty.GetSrcObjectCount<FbxFileTexture>();
            for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
            {
                FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(lEmissiveProperty.GetSrcObject<FbxFileTexture>(lTextureIndex));
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
    }

    // ambient map...
    const FbxProperty lAmbientProperty = pFbxMat->FindProperty(FbxSurfaceMaterial::sAmbient);
    if (lAmbientProperty.IsValid())
    {
        // check if layered textures are used...
        int layeredTextureCount = lAmbientProperty.GetSrcObjectCount<FbxLayeredTexture>();
        if(layeredTextureCount)
        {
            FbxLayeredTexture* layered_texture = FbxCast<FbxLayeredTexture>(lAmbientProperty.GetSrcObject<FbxLayeredTexture>(0));
            int lNbTex = layered_texture->GetSrcObjectCount<FbxFileTexture>();

            if(lNbTex)
            {
                // at least 1 texture, use 1st...
                for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
                {
                    FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(layered_texture->GetSrcObject<FbxFileTexture>(lTextureIndex));
                    if (lTexture)
                    {
                        result.ambientTexture = fbxTextureToOsgTexture(lTexture);
                        result.ambientChannel = lTexture->UVSet.Get();
                        result.ambientScaleU = lTexture->GetScaleU();
                        result.ambientScaleV = lTexture->GetScaleV();
                    }

                    //For now only allow 1 texture
                    break;
                }
            }
            else
                OSG_WARN << "FBX: Missing Textures in FbxLayeredTexture." << std::endl;
        }
        else
        {
            int lNbTex = lAmbientProperty.GetSrcObjectCount<FbxFileTexture>();
            for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
            {
                FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(lAmbientProperty.GetSrcObject<FbxFileTexture>(lTextureIndex));
                if (lTexture)
                {
                    result.ambientTexture = fbxTextureToOsgTexture(lTexture);
                    result.ambientChannel = lTexture->UVSet.Get();
                    result.ambientScaleU = lTexture->GetScaleU();
                    result.ambientScaleV = lTexture->GetScaleV();
                }

                //For now only allow 1 texture
                break;
            }
        }
    }

    // normal map...
    const FbxProperty lNormalProperty = pFbxMat->FindProperty(FbxSurfaceMaterial::sNormalMap);
    if (lNormalProperty.IsValid())
    {
        // check if layered textures are used...
        int layeredTextureCount = lNormalProperty.GetSrcObjectCount<FbxLayeredTexture>();
        if(layeredTextureCount)
        {
            FbxLayeredTexture* layered_texture = FbxCast<FbxLayeredTexture>(lNormalProperty.GetSrcObject<FbxLayeredTexture>(0));
            int lNbTex = layered_texture->GetSrcObjectCount<FbxFileTexture>();

            if(lNbTex)
            {
                // at least 1 texture, use 1st...
                for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
                {
                    FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(layered_texture->GetSrcObject<FbxFileTexture>(lTextureIndex));
                    if (lTexture)
                    {
                        result.normalTexture = fbxTextureToOsgTexture(lTexture);
                        result.normalChannel = lTexture->UVSet.Get();
                        result.normalScaleU = lTexture->GetScaleU();
                        result.normalScaleV = lTexture->GetScaleV();
                    }

                    //For now only allow 1 texture
                    break;
                }
            }
            else
                OSG_WARN << "FBX: Missing Textures in FbxLayeredTexture." << std::endl;
        }
        else
        {
            int lNbTex = lNormalProperty.GetSrcObjectCount<FbxFileTexture>();
            for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
            {
                FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(lNormalProperty.GetSrcObject<FbxFileTexture>(lTextureIndex));
                if (lTexture)
                {
                    result.normalTexture = fbxTextureToOsgTexture(lTexture);
                    result.normalChannel = lTexture->UVSet.Get();
                    result.normalScaleU = lTexture->GetScaleU();
                    result.normalScaleV = lTexture->GetScaleV();
                }

                //For now only allow 1 texture
                break;
            }
        }
    }

    // specular map...
    const FbxProperty lSpecularProperty = pFbxMat->FindProperty(FbxSurfaceMaterial::sSpecular);
    if (lSpecularProperty.IsValid())
    {
        // check if layered textures are used...
        int layeredTextureCount = lSpecularProperty.GetSrcObjectCount<FbxLayeredTexture>();
        if(layeredTextureCount)
        {
            FbxLayeredTexture* layered_texture = FbxCast<FbxLayeredTexture>(lSpecularProperty.GetSrcObject<FbxLayeredTexture>(0));
            int lNbTex = layered_texture->GetSrcObjectCount<FbxFileTexture>();

            if(lNbTex)
            {
                // at least 1 texture, use 1st...
                for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
                {
                    FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(layered_texture->GetSrcObject<FbxFileTexture>(lTextureIndex));
                    if (lTexture)
                    {
                        result.specularTexture = fbxTextureToOsgTexture(lTexture);
                        result.specularChannel = lTexture->UVSet.Get();
                        result.specularScaleU = lTexture->GetScaleU();
                        result.specularScaleV = lTexture->GetScaleV();
                    }

                    //For now only allow 1 texture
                    break;
                }
            }
            else
                OSG_WARN << "FBX: Missing Textures in FbxLayeredTexture." << std::endl;
        }
        else
        {
            int lNbTex = lSpecularProperty.GetSrcObjectCount<FbxFileTexture>();
            for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
            {
                FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(lSpecularProperty.GetSrcObject<FbxFileTexture>(lTextureIndex));
                if (lTexture)
                {
                    result.specularTexture = fbxTextureToOsgTexture(lTexture);
                    result.specularChannel = lTexture->UVSet.Get();
                    result.specularScaleU = lTexture->GetScaleU();
                    result.specularScaleV = lTexture->GetScaleV();
                }

                //For now only allow 1 texture
                break;
            }
        }
    }

    // shininess map...
    const FbxProperty lShininessProperty = pFbxMat->FindProperty(FbxSurfaceMaterial::sShininess);
    if (lShininessProperty.IsValid())
    {
        // check if layered textures are used...
        int layeredTextureCount = lShininessProperty.GetSrcObjectCount<FbxLayeredTexture>();
        if(layeredTextureCount)
        {
            FbxLayeredTexture* layered_texture = FbxCast<FbxLayeredTexture>(lShininessProperty.GetSrcObject<FbxLayeredTexture>(0));
            int lNbTex = layered_texture->GetSrcObjectCount<FbxFileTexture>();

            if(lNbTex)
            {
                // at least 1 texture, use 1st...
                for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
                {
                    FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(layered_texture->GetSrcObject<FbxFileTexture>(lTextureIndex));
                    if (lTexture)
                    {
                        result.shininessTexture = fbxTextureToOsgTexture(lTexture);
                        result.shininessChannel = lTexture->UVSet.Get();
                        result.shininessScaleU = lTexture->GetScaleU();
                        result.shininessScaleV = lTexture->GetScaleV();
                    }

                    //For now only allow 1 texture
                    break;
                }
            }
            else
                OSG_WARN << "FBX: Missing Textures in FbxLayeredTexture." << std::endl;
        }
        else
        {
            int lNbTex = lShininessProperty.GetSrcObjectCount<FbxFileTexture>();
            for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
            {
                FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(lShininessProperty.GetSrcObject<FbxFileTexture>(lTextureIndex));
                if (lTexture)
                {
                    result.shininessTexture = fbxTextureToOsgTexture(lTexture);
                    result.shininessChannel = lTexture->UVSet.Get();
                    result.shininessScaleU = lTexture->GetScaleU();
                    result.shininessScaleV = lTexture->GetScaleV();
                }

                //For now only allow 1 texture
                break;
            }
        }
    }

    if (pFbxLambert)
    {
        FbxDouble3 color = pFbxLambert->Diffuse.Get();
        double factor = pFbxLambert->DiffuseFactor.Get();
        double transparencyFactor = useTransparencyColorFactor ? transparencyColorFactor : pFbxLambert->TransparencyFactor.Get();
        pOsgMat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(
            static_cast<float>(color[0] * factor),
            static_cast<float>(color[1] * factor),
            static_cast<float>(color[2] * factor),
            static_cast<float>(1.0 - transparencyFactor)));

        color = pFbxLambert->Ambient.Get();
        factor = pFbxLambert->AmbientFactor.Get();
        pOsgMat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(
            static_cast<float>(color[0] * factor),
            static_cast<float>(color[1] * factor),
            static_cast<float>(color[2] * factor),
            1.0f));

        color = pFbxLambert->Emissive.Get();
        factor = pFbxLambert->EmissiveFactor.Get();
        pOsgMat->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(
            static_cast<float>(color[0] * factor),
            static_cast<float>(color[1] * factor),
            static_cast<float>(color[2] * factor),
            1.0f));

        // get maps factors...
        result.diffuseFactor = pFbxLambert->DiffuseFactor.Get();
        result.emissiveFactor = pFbxLambert->EmissiveFactor.Get();
        result.ambientFactor = pFbxLambert->AmbientFactor.Get();
        result.normalFactor = pFbxLambert->BumpFactor.Get();

        if (const FbxSurfacePhong* pFbxPhong = FbxCast<FbxSurfacePhong>(pFbxLambert))
        {
            color = pFbxPhong->Specular.Get();
            factor = pFbxPhong->SpecularFactor.Get();
            pOsgMat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(
                static_cast<float>(color[0] * factor),
                static_cast<float>(color[1] * factor),
                static_cast<float>(color[2] * factor),
                1.0f));
            // Since Maya and 3D studio Max stores their glossiness values in exponential format (2^(log2(x))
            // We need to linearize to values between 0-100 and then scale to values between 0-128.
            // Glossiness values above 100 will result in shininess larger than 128.0 and will be clamped
            double shininess = (64.0 * log (pFbxPhong->Shininess.Get())) / (5.0 * log(2.0));
            pOsgMat->setShininess(osg::Material::FRONT_AND_BACK,
                static_cast<float>(shininess));

            // get maps factors...
            result.reflectionFactor = pFbxPhong->ReflectionFactor.Get();
            result.specularFactor = pFbxPhong->SpecularFactor.Get();
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
FbxMaterialToOsgStateSet::fbxTextureToOsgTexture(const FbxFileTexture* fbx)
{
    ImageMap::iterator it = _imageMap.find(fbx->GetFileName());
    if (it != _imageMap.end())
        return it->second;
    osg::ref_ptr<osg::Image> pImage = NULL;

    // Warning: fbx->GetRelativeFileName() is relative TO EXECUTION DIR
    //          fbx->GetFileName() is as stored initially in the FBX
    if ((pImage = osgDB::readRefImageFile(osgDB::concatPaths(_dir, fbx->GetFileName()), _options)) ||                // First try "export dir/name"
        (pImage = osgDB::readRefImageFile(fbx->GetFileName(), _options)) ||                                        // Then try  "name" (if absolute)
        (pImage = osgDB::readRefImageFile(osgDB::concatPaths(_dir, fbx->GetRelativeFileName()), _options)))        // Else try  "current dir/name"
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
