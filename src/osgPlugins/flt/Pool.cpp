// Pool.cpp

#include <osg/Vec4>
#include <osg/Texture>

#include "Pool.h"

using namespace flt;

ColorPool::ColorPool()
{
}


// virtual
ColorPool::~ColorPool()
{
    eraseAll();
}


osg::Vec4 ColorPool::getColor(int nColorIndex)
{
    osg::Vec4 col(1,1,1,1);

    if (nColorIndex >= 0)
    {
        int index = nColorIndex / 128;
        float intensity = (nColorIndex % 128) / 128.0f;

        ColorName* cn = getColorName(index);
        if (cn)
            col = cn->getColor();

        col[0] *= intensity;
        col[1] *= intensity;
        col[2] *= intensity;
    }

    return col;
}


void ColorPool::regisiterColor(int nIndex, const osg::Vec4& color)
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
        return (*itr).second;

    return NULL;
}


void ColorPool::eraseAll()
{
    for(ColorNameMap::iterator itr=_colorNameMap.begin();
        itr!=_colorNameMap.end();
        ++itr)
    {
        ColorName* colorname = (*itr).second;
        if (colorname)
            delete colorname;
    }
    _colorNameMap.erase(_colorNameMap.begin(), _colorNameMap.end());
}


////////////////////////////////////////////////////////////////////


TexturePool::TexturePool()
{
}


// virtual
TexturePool::~TexturePool()
{
    eraseAll();
}


osg::Texture* TexturePool::getTexture(int nIndex)
{
    TexturePaletteMap::iterator fitr = _textureMap.find(nIndex);
    if (fitr != _textureMap.end())
        return (*fitr).second;
    else
        return NULL;
}


void TexturePool::regisiterTexture(int nIndex, osg::Texture* osgTexture)
{
    _textureMap[nIndex] = osgTexture;
}


void TexturePool::eraseAll()
{
    _textureMap.erase(_textureMap.begin(), _textureMap.end());
}


////////////////////////////////////////////////////////////////////

MaterialPool::MaterialPool()
{
}


// virtual
MaterialPool::~MaterialPool()
{
    eraseAll();
}


SMaterial* MaterialPool::getMaterial(int nIndex)
{
    MaterialMap::iterator fitr = _MaterialMap.find(nIndex);
    if (fitr != _MaterialMap.end())
        return (*fitr).second;

    return NULL;
}


void MaterialPool::regisiterMaterial(int nIndex, SMaterial* material)
{
    _MaterialMap[nIndex] = material;
}


void MaterialPool::eraseAll()
{
    _MaterialMap.erase(_MaterialMap.begin(), _MaterialMap.end());
}
