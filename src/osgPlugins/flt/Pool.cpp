// Pool.cpp

#if defined(WIN32) && !defined(__CYGWIN__)
    #pragma warning( disable : 4786 )
#endif

#include <osg/Vec4>
#include <osg/Texture>

#include "MaterialPaletteRecord.h"
#include "OldMaterialPaletteRecord.h"
#include "Pool.h"

using namespace flt;


osg::Vec4 ColorPool::getColor(int nColorIndex)
{
    osg::Vec4 col(1,1,1,1);

    if (nColorIndex >= 0)
    {
        int index = nColorIndex / 128;                          // = nColorIndex >> 7
        float intensity = (nColorIndex % 128) / 128.0f;         // = nColorIndex & 0x7f

        ColorName* cn = getColorName(index);
        if (cn)
            col = cn->getColor();

        col[0] *= intensity;
        col[1] *= intensity;
        col[2] *= intensity;
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


