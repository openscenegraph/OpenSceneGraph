// Pool.cpp

#ifndef __FLT_POOL_H
#define __FLT_POOL_H

#include "flt.h"

#include <osg/Vec4>
#include <osg/Texture>

#include <string>
#include <algorithm>
#include <map>
//#include <vector>


namespace flt {


class ColorPool
{

public:
    ColorPool();
    virtual ~ColorPool();

    osg::Vec4 getColor(int nColorIndex);
    void addColor(int nIndex, const osg::Vec4& color);

private:

    class ColorName
    {
        public:
            void setName( const std::string& name ) { _name = name; }
            const std::string& getName( void )      { return _name; }
            void setColor(const osg::Vec4& color )  { _color = color; }
            osg::Vec4& getColor()                   { return _color; }

        private:
            osg::Vec4   _color;
            std::string _name;
    };

    void eraseAll();
    ColorName* getColorName(int nIndex);

    typedef std::map<int,ColorName*>ColorNameMap;
    ColorNameMap _colorNameMap;
};


class TexturePool
{
public:

    TexturePool();
    virtual ~TexturePool();

    osg::Texture* getTexture(int nIndex);
    void addTexture(int nIndex, osg::Texture* osgTexture);

private:

    void eraseAll();

    typedef std::map<int,osg::Texture*> TexturePaletteMap;
    TexturePaletteMap _textureMap;
};


struct PoolMaterial
{
    float32x3    Ambient;         // Ambient  component of material
    float32x3    Diffuse;        // Diffuse  component of material
    float32x3    Specular;         // Specular component of material
    float32x3    Emissive;        // Emissive component of material
    float32        sfShininess;    // Shininess. [0.0-128.0]
    float32        sfAlpha;        // Alpha. [0.0-1.0], where 1.0 is opaque
};

struct SMaterial;
struct SOldMaterial;

class MaterialPool
{
public:

    MaterialPool();
    virtual ~MaterialPool();

    PoolMaterial* getMaterial(int nIndex);
    void addMaterial(int nIndex, PoolMaterial* material);

private:

    void eraseAll();

    typedef std::map<int, PoolMaterial*> MaterialMap;
    MaterialMap _MaterialMap;
};


}; // end namespace flt

#endif

