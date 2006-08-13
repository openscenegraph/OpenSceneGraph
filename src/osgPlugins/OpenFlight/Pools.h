//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#ifndef FLT_POOLS_H
#define FLT_POOLS_H 1

#include <vector>
#include <map>
#include <sstream>
#include <osg/Vec4>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Program>
#include "types.h"


namespace flt {

class VertexPool : public osg::Referenced, public std::istringstream
{
public:

    explicit VertexPool( const std::string& str) :
        std::istringstream(str,std::istringstream::in|std::istringstream::binary) {}

protected:

    virtual ~VertexPool() {}
};


class ColorPool : public osg::Referenced , public std::vector<osg::Vec4>
{
public:

    explicit ColorPool(bool old,int size) :
        std::vector<osg::Vec4>(size),
        _old(old) {}

    osg::Vec4 getColor(int indexIntensity) const;

protected:

    virtual ~ColorPool() {}

    bool _old; // true if version <= 13
};


class TexturePool : public osg::Referenced , public std::map<int,osg::ref_ptr<osg::StateSet> >
{
public:

    TexturePool() {}

    osg::StateSet* get(int index)
    {
        iterator itr = find(index);
        if (itr != end())
            return (*itr).second.get();
        return NULL;
    }

protected:

    virtual ~TexturePool() {}

};


class MaterialPool : public osg::Referenced , public std::map<int,osg::ref_ptr<osg::Material> >
{
public:

    MaterialPool();

    // Get material, return default material if not found in palette.
    osg::Material* get(int index);

    // Get or create material based on 
    // index: face material index
    // color: face color with alpha set to 1-face transparency.
    osg::Material* getOrCreateMaterial(int index, const osg::Vec4& faceColor);

protected:

    virtual ~MaterialPool() {}

    osg::ref_ptr<osg::Material> _defaultMaterial;

    struct MaterialParameters
    {
        int index;              // face index to material pool
        osg::Vec4 color;        // face color with alpha set to 1-face transparency.

        MaterialParameters():
            index(-1) {}

        MaterialParameters(int i, const osg::Vec4& c):
            index(i),
            color(c) {}

        bool operator < (const MaterialParameters& rhs) const
        {
            if (index < rhs.index) return true;
            else if (index > rhs.index) return false;
            else return (color < rhs.color);
        }
    };

    // Material from palette combined with face color stored here for reuse.
    typedef std::map<MaterialParameters,osg::ref_ptr<osg::Material> > FinalMaterialMap;
    FinalMaterialMap _finalMaterialMap;
};


struct LPAppearance : public osg::Referenced
{
    std::string name;
    int32 index;
    int16 materialCode;
    int16 featureID;
    osg::Vec4f backColor;
    int32 displayMode;
    float32 intensityFront;
    float32 intensityBack;
    float32 minDefocus;
    float32 maxDefocus;
    int32 fadingMode;
    int32 fogPunchMode;
    int32 directionalMode;
    int32 rangeMode;
    float32 minPixelSize;
    float32 maxPixelSize;
    float32 actualPixelSize;
    float32 transparentFalloffPixelSize;
    float32 transparentFalloffExponent;
    float32 transparentFalloffScalar;
    float32 transparentFalloffClamp;
    float32 fogScalar;
    float32 fogIntensity;
    float32 sizeDifferenceThreshold;
    int32 directionality;
    float32 horizontalLobeAngle;
    float32 verticalLobeAngle;
    float32 lobeRollAngle;
    float32 directionalFalloffExponent;
    float32 directionalAmbientIntensity;
    float32 significance;
    uint32 flags;
    float32 visibilityRange;
    float32 fadeRangeRatio;
    float32 fadeInDuration;
    float32 fadeOutDuration;
    float32 LODRangeRatio;
    float32 LODScale;
    int16 texturePatternIndex;
};


class LightPointAppearancePool : public osg::Referenced , public std::map<int,osg::ref_ptr<LPAppearance> >
{
public:

    LightPointAppearancePool() {}

    LPAppearance* get(int index)
    {
        iterator itr = find(index);
        if (itr != end())
            return (*itr).second.get();
        return NULL;
    }

protected:

    virtual ~LightPointAppearancePool() {}

};


class ShaderPool : public osg::Referenced , public std::map<int,osg::ref_ptr<osg::Program> >
{
public:

    ShaderPool() {}

    osg::Program* get(int index)
    {
        iterator itr = find(index);
        if (itr != end())
            return (*itr).second.get();
        return NULL;
    }

protected:

    virtual ~ShaderPool() {}
};


// This object records parent palettes for external record support.
// When an external record is parsed, this object is instatiated and populated with
// the parent model's paettes, then stored as UserData on the ProxyNode.
// When the ReadExternalsVisitor hits the ProcyNode, it moves this object
// into the ReaderWriter Options' UserData before calling osgDB::ReadNode,
// enabling  access to the parent palattes during load of the ext ref model.
class ParentPools : public osg::Referenced
{
public:

    ParentPools(
        ColorPool* color,
        MaterialPool* material,
        TexturePool* texture,
        LightPointAppearancePool* lpAppearance,
        ShaderPool* shader )
      : osg::Referenced(),
        _colorPool( color ),
        _materialPool( material ),
        _texturePool( texture ),
        _lpAppearancePool( lpAppearance ),
        _shaderPool( shader ) {}

    ColorPool* getColorPool() const { return _colorPool.get(); }
    TexturePool* getTexturePool() const { return _texturePool.get(); }
    MaterialPool* getMaterialPool() const { return _materialPool.get(); }
    LightPointAppearancePool* getLPAppearancePool() const { return _lpAppearancePool.get(); }
    ShaderPool* getShaderPool() const { return _shaderPool.get(); }

protected:

    virtual ~ParentPools() {}

    osg::ref_ptr<ColorPool> _colorPool;
    osg::ref_ptr<MaterialPool> _materialPool;
    osg::ref_ptr<TexturePool> _texturePool;
    osg::ref_ptr<LightPointAppearancePool> _lpAppearancePool;
    osg::ref_ptr<ShaderPool> _shaderPool;
};


} // end namespace

#endif
