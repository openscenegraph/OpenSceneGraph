// Pool.cpp

#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <osg/Vec4>

#include "MaterialPaletteRecord.h"
#include "OldMaterialPaletteRecord.h"
#include "Pool.h"

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


osg::StateSet* TexturePool::getTexture(int nIndex)
{
    TexturePaletteMap::iterator fitr = _textureMap.find(nIndex);
    if (fitr != _textureMap.end())
        return (*fitr).second.get();
    else
        return NULL;
}


void TexturePool::addTexture(int nIndex, osg::StateSet* stateset)
{
    _textureMap[nIndex] = stateset;
}


////////////////////////////////////////////////////////////////////


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

