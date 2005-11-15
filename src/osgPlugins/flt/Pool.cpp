// Pool.cpp

// Modify TexturePool to store a flt::AttrData object instead of a osg::StateSet
// Julian Ortiz, June 18th 2003.

#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <osg/Vec4>
#include <osg/Texture2D>
#include <osg/TexEnv>

#include <osgDB/ReadFile>
#include <osgDB/Registry>

#include "MaterialPaletteRecord.h"
#include "OldMaterialPaletteRecord.h"
#include "Pool.h"
#include "Registry.h"
#include "AttrData.h"

#include <stdio.h>


using namespace flt;


osg::Vec4 ColorPool::getColor(int nColorIntensity)
{
    // nColorIntensity:
    // bit 0-6:  intensity
    // bit 7-15  color index

    osg::Vec4 col(1,1,1,1);

    if (nColorIntensity >= 0)
    {
        ColorName* cn = getColorName(nColorIntensity >> 7);
        if (cn)
            col = cn->getColor();

        float intensity = (float)(nColorIntensity & 0x7f)/127.f;
        col[0] *= intensity;
        col[1] *= intensity;
        col[2] *= intensity;
    }

    return col;
}


// getColor for version 11, 12 & 13.
osg::Vec4 ColorPool::getOldColor(int nColorIntensity)
{
    // nColorIntensity:
    // bit 0-6:  intensity
    // bit 7-11  color index
    // bit 12    fixed intensity bit

    osg::Vec4 col(1,1,1,1);

    if (nColorIntensity >= 0)
    {
        int nIndex;
        bool bFixedIntensity = (nColorIntensity & 0x1000) ? true : false;

        if (bFixedIntensity)
            nIndex = (nColorIntensity & 0x0fff)+(4096>>7);
        else
            nIndex = nColorIntensity >> 7;

        ColorName* cn = getColorName(nIndex);
        if (cn)
            col = cn->getColor();

        // intensity
        if (!bFixedIntensity)
        {
            float intensity = (float)(nColorIntensity & 0x7f)/127.f;
            col[0] *= intensity;
            col[1] *= intensity;
            col[2] *= intensity;
        }
    }

    return col;
}


void ColorPool::addColor(int nIndex, const osg::Vec4& color)
{
    if (nIndex >= 0)
    {
        ColorName* colorname = new ColorName;
        colorname->setColor(color);

        _colorNameMap[nIndex] = colorname;
    }
}


ColorPool::ColorName* ColorPool::getColorName(int nIndex)
{
    ColorNameMap::iterator itr = _colorNameMap.find(nIndex);
    if (itr != _colorNameMap.end())
        return (*itr).second.get();

    return NULL;
}


////////////////////////////////////////////////////////////////////


flt::AttrData* TexturePool::getTexture(int nIndex, osgDB::ReaderWriter::Options* options)
{
    TexturePaletteMap::iterator fitr = _textureMap.find(nIndex);
    if (fitr != _textureMap.end())
    {
        return (*fitr).second.get();
    }
    else
    {
        // no existing texture state set set up so lets look
        // for a file name for this nIndex..
        TextureNameMap::iterator nitr = _textureNameMap.find(nIndex);
        if (nitr != _textureNameMap.end())
        {
            const std::string& textureName = (*nitr).second;
            flt::AttrData* textureAttrData = 0;

            if(options->getObjectCacheHint() & osgDB::ReaderWriter::Options::CACHE_IMAGES)
            {

                // Valid index, find the texture
                // Get AttrData containing texture from registry pool.
                textureAttrData = Registry::instance()->getTexture(textureName);

                if (textureAttrData)
                {
                    // Add texture to local pool to be ab121le to get by index.
                    addTexture(nIndex, textureAttrData);
                    return textureAttrData;
                }
            }

            CERR<<"setTexture attempting to load ("<<textureName<<")"<<std::endl;

            unsigned int unit = 0;

            // Read texture and attribute file
            osg::ref_ptr<osg::Image> image = osgDB::readImageFile(textureName, options ? options : osgDB::Registry::instance()->getOptions());
            if (image.valid())
            {
                std::string attrName(textureName);
                attrName += ".attr";

                textureAttrData = dynamic_cast<flt::AttrData*>(osgDB::readObjectFile(attrName, options ? options : osgDB::Registry::instance()->getOptions() ));

                // if not found create default StateSet for the AttrData
                if (textureAttrData == NULL)
                {
                    textureAttrData = new flt::AttrData;
                    textureAttrData->stateset = new osg::StateSet;

                    osg::Texture2D* osgTexture = new osg::Texture2D;
                    osgTexture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::REPEAT);
                    osgTexture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::REPEAT);
                    textureAttrData->stateset->setTextureAttributeAndModes( unit, osgTexture,osg::StateAttribute::ON);

                    osg::TexEnv* osgTexEnv = new osg::TexEnv;
                    osgTexEnv->setMode(osg::TexEnv::MODULATE);
                    textureAttrData->stateset->setTextureAttribute( unit, osgTexEnv );
                }

                osg::Texture2D *osgTexture = dynamic_cast<osg::Texture2D*>(textureAttrData->stateset->getTextureAttribute( unit, osg::StateAttribute::TEXTURE));
                if (osgTexture == NULL)
                {
                    osgTexture = new osg::Texture2D;
                    textureAttrData->stateset->setTextureAttributeAndModes( unit, osgTexture,osg::StateAttribute::ON);
                }

                osgTexture->setImage(image.get());

            }
            else
            {
                // invalid image file, register an empty state set AttrData
                textureAttrData = new flt::AttrData;
                textureAttrData->stateset = new osg::StateSet;
            }

            // Add new texture to registry pool
            // ( umm... should this have reference to the texture unit? RO. July2002)
            if(options->getObjectCacheHint() & osgDB::ReaderWriter::Options::CACHE_IMAGES)
            {
                Registry::instance()->addTexture(textureName, textureAttrData);
            }

            // Also add to local pool to be able to get texture by index.
            // ( umm... should this have reference to the texture unit? RO. July2002)
            addTexture(nIndex, textureAttrData);

            CERR<<"Registry::instance()->addTexture("<<textureName<<", "<<textureAttrData<<")"<<std::endl;
            CERR<<"pTexturePool->addTexture("<<nIndex<<", "<<textureAttrData<<")"<<std::endl;
            
            return textureAttrData;
        }
    }
    return NULL;
}

std::string* TexturePool::getTextureName(int nIndex)
{
    TextureNameMap::iterator fitr = _textureNameMap.find(nIndex);
    if (fitr != _textureNameMap.end())
        return &(*fitr).second;
    else
        return NULL;
}


void TexturePool::addTexture(int nIndex, flt::AttrData* attrdata)
{
    _textureMap[nIndex] = attrdata;
}

void TexturePool::addTextureName(int nIndex, const std::string& name)
{
    _textureNameMap[nIndex] = name;
}

////////////////////////////////////////////////////////////////////


osg::Light* LightPool::getLight(int nIndex)
{
    if (nIndex < 0) return NULL;
    LightPaletteMap::iterator fitr = _lightMap.find(nIndex);
    if (fitr != _lightMap.end())
        return (*fitr).second.get();

    return NULL;
}


void LightPool::addLight(int nIndex, osg::Light* light)
{
    _lightMap[nIndex] = light;
}

MaterialPool::PoolMaterial* MaterialPool::getMaterial(int nIndex)
{
    if (nIndex < 0) return NULL;
    MaterialMap::iterator fitr = _MaterialMap.find(nIndex);
    if (fitr != _MaterialMap.end())
        return (*fitr).second.get();

    return NULL;
}


void MaterialPool::addMaterial(int nIndex, PoolMaterial* material)
{
    _MaterialMap[nIndex] = material;
}

osg::Group* InstancePool::getInstance(int nIndex)
{
    InstanceMap::iterator fitr = _instanceMap.find(nIndex);
    if (fitr != _instanceMap.end())
        return (*fitr).second.get();
    else
        return NULL;
}


void InstancePool::addInstance(int nIndex, osg::Group* instance)
{
    _instanceMap[nIndex] = instance;
}


LtPtAppearancePool::PoolLtPtAppearance* LtPtAppearancePool::get(int nIndex)
{
    if (nIndex < 0)
        return NULL;

    AppearanceMap::iterator fitr = _appearanceMap.find(nIndex);
    if (fitr != _appearanceMap.end())
        return (*fitr).second.get();

    return NULL;
}


void LtPtAppearancePool::add(int nIndex, PoolLtPtAppearance* appearance)
{
    _appearanceMap[nIndex] = appearance;
}


LtPtAnimationPool::PoolLtPtAnimation*
LtPtAnimationPool::get( int nIndex )
{
    if (nIndex < 0)
        return NULL;

    AnimationMap::iterator fitr = _animationMap.find(nIndex);
    if (fitr != _animationMap.end())
        return (*fitr).second.get();

    return NULL;
}


void
LtPtAnimationPool::add(int nIndex, PoolLtPtAnimation* anim)
{
    _animationMap[nIndex] = anim;
}
