#ifndef __FLT_POOL_H
#define __FLT_POOL_H

#include "flt.h"

#include <osg/ref_ptr>
#include <osg/Referenced>
#include <osg/Vec4>
#include <osg/Texture>
#include <osg/Material>
#include <osg/StateSet>
#include <osg/Group>

#include <string>
#include <algorithm>
#include <map>

namespace flt {


class ColorPool : public osg::Referenced
{
    public :

        ColorPool() {}

        osg::Vec4 getColor(int nColorIndex);
        void addColor(int nIndex, const osg::Vec4& color);

    protected :

        virtual ~ColorPool() {}

    private :

        class ColorName : public osg::Referenced
        {
            public:
                void setName( const std::string& name ) { _name = name; }
                const std::string& getName( void )      { return _name; }
                void setColor(const osg::Vec4& color )  { _color = color; }
                osg::Vec4 getColor()                    { return _color; }

            private:
                osg::Vec4   _color;
                std::string _name;
        };

        ColorName* getColorName(int nIndex);

        typedef std::map<int,osg::ref_ptr<ColorName> > ColorNameMap;
        ColorNameMap _colorNameMap;
};


class TexturePool : public osg::Referenced
{
    public :

        TexturePool() {}

        osg::StateSet* getTexture(int nIndex);
        void addTexture(int nIndex, osg::StateSet* stateset);

    protected :

        virtual ~TexturePool() {}

    private :

        typedef std::map<int,osg::ref_ptr<osg::StateSet> > TexturePaletteMap;
        TexturePaletteMap _textureMap;
};



class MaterialPool : public osg::Referenced
{
    public:

        struct PoolMaterial : public osg::Referenced
        {
            float32x3    Ambient;        // Ambient  component of material
            float32x3    Diffuse;        // Diffuse  component of material
            float32x3    Specular;       // Specular component of material
            float32x3    Emissive;       // Emissive component of material
            float32      sfShininess;    // Shininess. [0.0-128.0]
            float32      sfAlpha;        // Alpha. [0.0-1.0], where 1.0 is opaque
        };
        
        MaterialPool() {}

        PoolMaterial* getMaterial(int nIndex);
        void addMaterial(int nIndex, PoolMaterial* material);

    protected :

        virtual ~MaterialPool() {}

    private:

        typedef std::map<int, osg::ref_ptr<PoolMaterial> > MaterialMap;
        MaterialMap _MaterialMap;
};


class InstancePool : public osg::Referenced
{
    public :

        InstancePool() {}

        osg::Group* getInstance(int nIndex);
        void addInstance(int nIndex, osg::Group* instance);

    protected :

        virtual ~InstancePool() {}

    private :

        typedef std::map<int,osg::ref_ptr<osg::Group> > InstanceMap;
        InstanceMap _instanceMap;
};

}; // end namespace flt

#endif

