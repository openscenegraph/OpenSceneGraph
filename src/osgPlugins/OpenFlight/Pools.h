/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#ifndef FLT_POOLS_H
#define FLT_POOLS_H 1

#include <vector>
#include <map>
#include <sstream>
#include <osg/Vec4>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Light>
#include <osg/Program>
#include "Types.h"


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


class LightSourcePool : public osg::Referenced , public std::map<int,osg::ref_ptr<osg::Light> >
{
public:

    LightSourcePool() {}

    osg::Light* get(int index)
    {
        iterator itr = find(index);
        if (itr != end())
            return (*itr).second.get();
        return NULL;
    }

protected:

    virtual ~LightSourcePool() {}
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

struct LPAnimation : public osg::Referenced
{
    enum AnimationType 
    {
        FLASHING_SEQUENCE = 0,
        ROTATING = 1,
        STROBE = 2,
        MORSE_CODE = 3
    };

    enum State
    {
        ON = 0,
        OFF = 1,
        COLOR_CHANGE = 2
    };

    struct Pulse
    {
        uint32 state;
        float32 duration;
        osg::Vec4 color;
    };

    typedef std::vector<Pulse>  PulseArray;

    std::string name;                        // animation name
    int32 index;                            // animation index
    float32 animationPeriod;                // animation period, in seconds
    float32 animationPhaseDelay;            // animation phase delay, in seconds from start of period
    float32 animationEnabledPeriod;            // animation enabled period (time on), in seconds
    osg::Vec3f axisOfRotation;                // axis of rotation for rotating animation (i, j, k)
    uint32 flags;                            // flags (bits, from left to right)
                                            //     0 = flashing
                                            //   1 = rotating
                                            //   2 = rotate counter clockwise
                                            //   3-31 = spare
    int32 animationType;                    // animation type
                                            //     0 = flashing sequence
                                            //   1 = rotating
                                            //   2 = strobe
                                            //   3 = morse code
    int32 morseCodeTiming;                    // morse code timing
                                            //     0 = standard timing
                                            //   1 = Farnsworth timing
    int32 wordRate;                            // word rate (for Farnsworth timing)
    int32 characterRate;                    // character rate (for Farnsworth timing)
    std::string morseCodeString;            // morse code string
    PulseArray sequence;
};


class LightPointAnimationPool : public osg::Referenced , public std::map<int,osg::ref_ptr<LPAnimation> >
{
public:

    LightPointAnimationPool() {}

    LPAnimation* get(int index)
    {
        iterator itr = find(index);
        if (itr != end())
            return (*itr).second.get();
        return NULL;
    }

protected:

    virtual ~LightPointAnimationPool() {}

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
// When an external record is parsed, this object is instantiated and populated with
// the parent model's palettes, then stored as UserData on the ProxyNode.
// When the ReadExternalsVisitor hits the ProxyNode, it moves this object
// into the ReaderWriter Options' UserData before calling osgDB::ReadNode,
// enabling  access to the parent palettes during load of the ext ref model.
class ParentPools : public osg::Referenced
{
public:

    ParentPools() {}

    void setColorPool(ColorPool* pool) { _colorPool=pool; }
    ColorPool* getColorPool() const { return _colorPool.get(); }

    void setTexturePool(TexturePool* pool) { _texturePool=pool; }
    TexturePool* getTexturePool() const { return _texturePool.get(); }

    void setMaterialPool(MaterialPool* pool) { _materialPool=pool; }
    MaterialPool* getMaterialPool() const { return _materialPool.get(); }

    void setLightSourcePool(LightSourcePool* pool) { _lightSourcePool=pool; }
    LightSourcePool* getLightSourcePool() const { return _lightSourcePool.get(); }

    void setLPAppearancePool(LightPointAppearancePool* pool) { _lpAppearancePool=pool; }
    LightPointAppearancePool* getLPAppearancePool() const { return _lpAppearancePool.get(); }

    void setLPAnimationPool(LightPointAnimationPool* pool) { _lpAnimationPool=pool; }
    LightPointAnimationPool* getLPAnimationPool() const { return _lpAnimationPool.get(); }

    void setShaderPool(ShaderPool* pool) { _shaderPool=pool; }
    ShaderPool* getShaderPool() const { return _shaderPool.get(); }

protected:

    virtual ~ParentPools() {}

    osg::ref_ptr<ColorPool> _colorPool;
    osg::ref_ptr<MaterialPool> _materialPool;
    osg::ref_ptr<TexturePool> _texturePool;
    osg::ref_ptr<LightSourcePool> _lightSourcePool;
    osg::ref_ptr<LightPointAppearancePool> _lpAppearancePool;
    osg::ref_ptr<LightPointAnimationPool> _lpAnimationPool;
    osg::ref_ptr<ShaderPool> _shaderPool;
};


} // end namespace

#endif
