#include "fbxMaterialToOsgStateSet.h"
#include <sstream>
#include <osg/TexMat>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

TextureDetails::TextureDetails():
    factor(1.0),
    scale(1.0, 1.0)
{
}

bool TextureDetails::transparent() const
{
    const osg::Image* image = texture.valid() ? texture->getImage() : 0;
    return image!=0 ? image->isImageTranslucent() : false;
}

void TextureDetails::assignTextureIfRequired(osg::StateSet* stateSet, unsigned int unit)
{
    if (!texture) return;

    stateSet->setTextureAttributeAndModes(unit, texture.get());
}

void TextureDetails::assignTexMatIfRequired(osg::StateSet* stateSet, unsigned int unit)
{
    if (scale.x() != 1.0 || scale.y() != 1.0)
    {
        // set UV scaling...
        stateSet->setTextureAttributeAndModes(unit, new osg::TexMat(osg::Matrix::scale(scale.x(), scale.y(), 1.0)), osg::StateAttribute::ON);
    }
}

static osg::Texture::WrapMode convertWrap(FbxFileTexture::EWrapMode wrap)
{
    return wrap == FbxFileTexture::eRepeat ?
        osg::Texture2D::REPEAT : osg::Texture2D::CLAMP_TO_EDGE;
}

StateSetContent FbxMaterialToOsgStateSet::convert(const FbxSurfaceMaterial* pFbxMat)
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
    result.diffuse = selectTextureDetails(pFbxMat->FindProperty(FbxSurfaceMaterial::sDiffuse));

    // opacity map...
    double transparencyColorFactor = 1.0;
    bool useTransparencyColorFactor = false;

    const FbxProperty lOpacityProperty = pFbxMat->FindProperty(FbxSurfaceMaterial::sTransparentColor);
    if (lOpacityProperty.IsValid())
    {
        FbxDouble3 transparentColor = lOpacityProperty.Get<FbxDouble3>();
        // If transparent color is defined set the transparentFactor to gray scale value of transparentColor
        if (transparentColor[0] < 1.0 || transparentColor[1] < 1.0 || transparentColor[2] < 1.0) {
            transparencyColorFactor = transparentColor[0]*0.30 + transparentColor[1]*0.59 + transparentColor[2]*0.11;
            useTransparencyColorFactor = true;
        }

        result.opacity = selectTextureDetails(lOpacityProperty);
    }

    // reflection map...
    result.reflection = selectTextureDetails(pFbxMat->FindProperty(FbxSurfaceMaterial::sReflection));

    // emissive map...
    result.emissive = selectTextureDetails(pFbxMat->FindProperty(FbxSurfaceMaterial::sEmissive));

    // ambient map...
    result.ambient = selectTextureDetails(pFbxMat->FindProperty(FbxSurfaceMaterial::sAmbient));

    // normal map...
    result.normalMap = selectTextureDetails(pFbxMat->FindProperty(FbxSurfaceMaterial::sNormalMap));

    // specular map...
    result.specular = selectTextureDetails(pFbxMat->FindProperty(FbxSurfaceMaterial::sSpecular));

    // shininess map...
    result.shininess = selectTextureDetails(pFbxMat->FindProperty(FbxSurfaceMaterial::sShininess));


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
        if (result.diffuse.valid()) result.diffuse->factor = pFbxLambert->DiffuseFactor.Get();
        if (result.emissive.valid()) result.emissive->factor = pFbxLambert->EmissiveFactor.Get();
        if (result.ambient.valid()) result.ambient->factor = pFbxLambert->AmbientFactor.Get();
        if (result.normalMap.valid()) result.normalMap->factor = pFbxLambert->BumpFactor.Get();

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
            if (result.reflection.valid()) result.reflection->factor = pFbxPhong->ReflectionFactor.Get();
            if (result.specular.valid()) result.specular->factor = pFbxPhong->SpecularFactor.Get();
            // get more factors here...
        }
    }

    if (_lightmapTextures)
    {
        // if using an emission map then adjust material properties accordingly...
        if (result.emissive.valid())
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

osg::ref_ptr<osg::Texture2D> FbxMaterialToOsgStateSet::fbxTextureToOsgTexture(const FbxFileTexture* fbx)
{
    // Try to find image in cache
	ImageMap::iterator it = _imageMap.find(fbx->GetFileName());
    if (it != _imageMap.end())
        return it->second;
    

	// Try to locate valid filename
	std::string filename = "";

	// Warning: fbx->GetRelativeFileName() is relative TO EXECUTION DIR
	//          fbx->GetFileName() is as stored initially in the FBX
	if (osgDB::fileExists(osgDB::concatPaths(_dir, fbx->GetFileName()))) // First try "export dir/name"
	{
		filename = osgDB::concatPaths(_dir, fbx->GetFileName());
	} 
	else if (osgDB::fileExists(fbx->GetFileName())) // Then try "name" (if absolute)
	{
		filename = fbx->GetFileName();
	} 
	else if (osgDB::fileExists(osgDB::concatPaths(_dir, fbx->GetRelativeFileName()))) // Else try  "current dir + relative filename"
	{
		filename = osgDB::concatPaths(_dir, fbx->GetRelativeFileName());
	} 
	else if (osgDB::fileExists(osgDB::concatPaths(_dir, osgDB::getSimpleFileName(fbx->GetFileName())))) // Else try "current dir + simple filename"
	{
		filename = osgDB::concatPaths(_dir, osgDB::getSimpleFileName(fbx->GetFileName()));
	}
	else 
	{
		OSG_WARN << "Could not find valid file for " << fbx->GetFileName() << std::endl;
		return NULL;
	}
	
	osg::ref_ptr<osg::Image> pImage = osgDB::readRefImageFile(filename, _options);
	if (pImage.valid())        
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

FbxFileTexture* FbxMaterialToOsgStateSet::selectFbxFileTexture(const FbxProperty& lProperty)
{
    if (lProperty.IsValid())
    {
        // check if layered textures are used...
        int layeredTextureCount = lProperty.GetSrcObjectCount<FbxLayeredTexture>();
        if (layeredTextureCount)
        {
            // find the first valud FileTexture
            for (int layeredTextureIndex = 0; layeredTextureIndex<layeredTextureCount; ++layeredTextureIndex)
            {
                FbxLayeredTexture* layered_texture = FbxCast<FbxLayeredTexture>(lProperty.GetSrcObject<FbxLayeredTexture>(layeredTextureIndex));
                int lNbTex = layered_texture->GetSrcObjectCount<FbxFileTexture>();
                for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
                {
                    FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(layered_texture->GetSrcObject<FbxFileTexture>(lTextureIndex));
                    if (lTexture) return lTexture;
                }
            }
        }
        else
        {
            // find the first valud FileTexture
            int lNbTex = lProperty.GetSrcObjectCount<FbxFileTexture>();
            for(int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
            {
                FbxFileTexture* lTexture = FbxCast<FbxFileTexture>(lProperty.GetSrcObject<FbxFileTexture>(lTextureIndex));
                if (lTexture) return lTexture;
            }
        }
    }
    return 0;
}

TextureDetails* FbxMaterialToOsgStateSet::selectTextureDetails(const FbxProperty& lProperty)
{
    if (lProperty.IsValid())
    {
        FbxFileTexture* fileTexture = selectFbxFileTexture(lProperty);
        if (fileTexture)
        {
            TextureDetails* textureDetails = new TextureDetails();
            textureDetails->texture = fbxTextureToOsgTexture(fileTexture);
            textureDetails->channel = fileTexture->UVSet.Get();
            textureDetails->scale.set(fileTexture->GetScaleU(), fileTexture->GetScaleV());
            return textureDetails;
        }
    }
    return 0;
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
