#ifndef __FLT_POOL_H
#define __FLT_POOL_H

// Modify TexturePool to store a flt::AttrData object instead of a osg::StateSet
// Julian Ortiz, June 18th 2003.

#include "flt.h"

#include <osg/ref_ptr>
#include <osg/Referenced>
#include <osg/Vec4>
#include <osg/Material>
#include <osg/StateSet>
#include <osg/Light>
#include <osg/Group>
#include <osgSim/BlinkSequence>

#include <osgDB/ReaderWriter>

#include "AttrData.h"

#include <string>
#include <algorithm>
#include <map>

namespace flt {


class ColorPool : public osg::Referenced
{
    public :

        ColorPool() {}

        osg::Vec4 getColor(int nColorIntensity);
        osg::Vec4 getOldColor(int nColorIntensity);
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

        flt::AttrData* getTexture(int nIndex, osgDB::ReaderWriter::Options* options);
        std::string* getTextureName(int nIndex);
        void addTexture(int nIndex, flt::AttrData* attrdata);
        void addTextureName(int nIndex, const std::string& name);

    protected :

        virtual ~TexturePool() {}

    private :

        typedef std::map<int,osg::ref_ptr<flt::AttrData> > TexturePaletteMap;
        TexturePaletteMap _textureMap;
        typedef std::map<int,std::string > TextureNameMap;
        TextureNameMap _textureNameMap;
};



class LightPool : public osg::Referenced
{
    public :

        LightPool() {}

        osg::Light* getLight(int nIndex );
        void addLight(int nIndex, osg::Light* light);

    protected :

        virtual ~LightPool() {}

    private :

        typedef std::map<int,osg::ref_ptr<osg::Light> > LightPaletteMap;
        LightPaletteMap _lightMap;
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


class LtPtAppearancePool : public osg::Referenced
{
public:
    struct PoolLtPtAppearance : public osg::Referenced
    {
        unsigned int _iBackColorIdx;
        float _bIntensity;
        float _sfMinPixelSize;
        float _sfMaxPixelSize;
        float _sfActualSize;
        int _iDirectionality;
        float _sfHLobeAngle;
        float _sfVLobeAngle;
        float _sfLobeRollAngle;
    };

    LtPtAppearancePool()
    {}

    PoolLtPtAppearance* get(int nIndex);
    void add(int nIndex, PoolLtPtAppearance* appearance);

protected:
    ~LtPtAppearancePool() {}

private:
    typedef std::map<int, osg::ref_ptr<PoolLtPtAppearance> > AppearanceMap;
    AppearanceMap _appearanceMap;
};

class LtPtAnimationPool : public osg::Referenced
{
public:
    struct PoolLtPtAnimation : public osg::Referenced
    {
		std::string _name;
		osg::ref_ptr<osgSim::BlinkSequence> _blink;
    };

    LtPtAnimationPool()
    {}

    PoolLtPtAnimation* get( int nIndex );
    void add( int nIndex, PoolLtPtAnimation* anim );

protected:
    ~LtPtAnimationPool() {}

private:
    typedef std::map<int, osg::ref_ptr<PoolLtPtAnimation> > AnimationMap;
    AnimationMap _animationMap;
};


}; // end namespace flt

#endif

