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
    KFbxMaterialMap::const_iterator it = _kFbxMaterialMap.find(pFbxMat);
    if (it != _kFbxMaterialMap.end())
        return it->second;
    static int nbMat = 0;
    osg::ref_ptr<osg::Material> pOsgMat = new osg::Material;
    osg::ref_ptr<osg::Texture2D> pOsgTex = NULL;
    pOsgMat->setName(pFbxMat->GetName());

    const KFbxSurfaceLambert* pFbxLambert = dynamic_cast<const KFbxSurfaceLambert*>(pFbxMat);

    const KFbxProperty lProperty = pFbxMat->FindProperty(KFbxSurfaceMaterial::sDiffuse);
    if (lProperty.IsValid())
    {
        int lNbTex = lProperty.GetSrcObjectCount(KFbxTexture::ClassId);
        for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
        {
            KFbxTexture* lTexture = KFbxCast<KFbxTexture>(lProperty.GetSrcObject(KFbxTexture::ClassId, lTextureIndex));
            if (lTexture)
            {
                pOsgTex = fbxTextureToOsgTexture(lTexture);
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
        }
    }
    StateSetContent result(pOsgMat.release(), pOsgTex.release());
    _kFbxMaterialMap.insert(KFbxMaterialMap::value_type(pFbxMat, result));
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
        return pOsgTex.release();
    }
    else
    {
        return NULL;
    }
}

void FbxMaterialToOsgStateSet::checkInvertTransparency()
{
    int zeroAlpha = 0, oneAlpha = 0;
    for (KFbxMaterialMap::const_iterator it = _kFbxMaterialMap.begin(); it != _kFbxMaterialMap.end(); ++it)
    {
        const osg::Material* pMaterial = it->second.first;
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

        for (KFbxMaterialMap::const_iterator it = _kFbxMaterialMap.begin(); it != _kFbxMaterialMap.end(); ++it)
        {
            osg::Material* pMaterial = it->second.first;
            osg::Vec4 diffuse = pMaterial->getDiffuse(osg::Material::FRONT);
            diffuse.a() = 1.0f - diffuse.a();
            pMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, diffuse);
        }
    }
}