// Pool.cpp

#ifndef __FLT_POOL_H
#define __FLT_POOL_H

#include <string>
#include <algorithm>
#include <map>
//#include <vector>

#include <osg/Vec4>
#include <osg/Texture>


namespace flt {

struct SMaterial;

class ColorPool
{

public:
    ColorPool();
    virtual ~ColorPool();

    osg::Vec4 getColor(int nColorIndex);
    void regisiterColor(int nIndex, const osg::Vec4& color);

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
    void regisiterTexture(int nIndex, osg::Texture* osgTexture);

private:

    void eraseAll();

    typedef std::map<int,osg::Texture*> TexturePaletteMap;
    TexturePaletteMap _textureMap;
};


class MaterialPool
{
public:

    MaterialPool();
    virtual ~MaterialPool();

    SMaterial* getMaterial(int nIndex);
    void regisiterMaterial(int nIndex, SMaterial* material);

private:

    void eraseAll();

    typedef std::map<int,SMaterial*> MaterialMap;
    MaterialMap _MaterialMap;
};


}; // end namespace flt

#endif

