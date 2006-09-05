//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#ifndef FLT_FLIGHTDATA_H
#define FLT_FLIGHTDATA_H 1

#include <vector>
#include <osg/Notify>
#include <osg/Transform>
#include <osg/Geometry>
#include <osgDB/ReaderWriter>

#include "types.h"
#include "Record.h"
#include "Pools.h"


namespace flt {

class Header;
class PushLevel;
class PopLevel;

enum Version
{
    VERSION_11      = 11,
    VERSION_12      = 12,
    VERSION_13      = 13,
    VERSION_14      = 14,
    VERSION_14_1    = 14,
    VERSION_14_2    = 1420,
    VERSION_15_1    = 1510,
    VERSION_15_4    = 1540,
    VERSION_15_5    = 1550,
    VERSION_15_6    = 1560,
    VERSION_15_7    = 1570,
    VERSION_15_8    = 1580,
    VERSION_16_0    = 1600,
    VERSION_16_1    = 1610
};

enum CoordUnits {
    METERS = 0,
    KILOMETERS = 1,
    FEET = 4,
    INCHES = 5,
    NAUTICAL_MILES = 8
};

double unitsToMeters(CoordUnits unit);


enum Projection {
    FLAT_EARTH = 0,
    TRAPEZOIDAL = 1,
    ROUND_EARTH = 2,
    LAMBERT = 3,
    UTM = 4,
    GEODETIC = 5,
    GEOCENTRIC = 6
};

enum Ellipsoid {
    WGS_1984 = 0,
    WGS_1972 = 1,
    BESSEL = 2,
    CLARKE_1866 = 3,
    NAD_1927 = 4
};


class Document
{
    public:

        Document();
        virtual ~Document();

        void setOptions(const osgDB::ReaderWriter::Options* options) { _options = options; }
        const osgDB::ReaderWriter::Options* getOptions() const { return _options.get(); }

        // Current primar record
        void setCurrentPrimaryRecord(PrimaryRecord* record) {_currentPrimaryRecord=record; }
        PrimaryRecord* getCurrentPrimaryRecord() { return _currentPrimaryRecord.get(); }
        const PrimaryRecord* getCurrentPrimaryRecord() const { return _currentPrimaryRecord.get(); }

        // Level stack
        PrimaryRecord* getTopOfLevelStack();
        void pushLevel();
        void popLevel();

        // Subface stack
        void pushSubface();
        void popSubface();

        // Extension stack
        void pushExtension();
        void popExtension();


        void setHeaderNode(osg::Node* node) { _osgHeader = node; }
        osg::Node* getHeaderNode() { return _osgHeader.get(); }

        // Instance definitions
        void setInstanceDefinition(int no, osg::Node* node) { _instanceDefinitionMap[no] = node; }
        osg::Node* getInstanceDefinition(int no);

        uint32 version() const { return _version; }
        bool done() const { return _done; }
        int level() const { return _level; }
        int subfaceLevel() const { return _subfaceLevel; }
        double unitScale() const { return _unitScale; }

        // Pools
        void setVertexPool(VertexPool* vp) { _vertexPool = vp; }
        VertexPool* getVertexPool() { return _vertexPool.get(); }
        const VertexPool* getVertexPool() const { return _vertexPool.get(); }

        void setColorPool(ColorPool* cp, bool parent=false) { _colorPool = cp; _colorPoolParent=parent; }
        ColorPool* getColorPool() { return _colorPool.get(); }
        const ColorPool* getColorPool() const { return _colorPool.get(); }
        bool getColorPoolParent() const { return _colorPoolParent; }

        void setTexturePool(TexturePool* tp, bool parent=false) { _texturePool = tp; _texturePoolParent=parent; }
        TexturePool* getOrCreateTexturePool();
        bool getTexturePoolParent() const { return _texturePoolParent; }

        void setMaterialPool(MaterialPool* mp, bool parent=false) { _materialPool = mp; _materialPoolParent=parent; }
        MaterialPool* getOrCreateMaterialPool();
        bool getMaterialPoolParent() const { return _materialPoolParent; }

        void setLightPointAppearancePool(LightPointAppearancePool* lpap, bool parent=false) { _lightPointAppearancePool = lpap; _lightPointAppearancePoolParent=parent; }
        LightPointAppearancePool* getOrCreateLightPointAppearancePool();
        bool getLightPointAppearancePoolParent() const { return _lightPointAppearancePoolParent; }

        void setShaderPool(ShaderPool* cp, bool parent=false) { _shaderPool = cp; _shaderPoolParent=parent; }
        ShaderPool* getOrCreateShaderPool();
        bool getShaderPoolParent() const { return _shaderPoolParent; }


        // Options
        void setPreserveFace(bool flag) { _preserveFace = flag; }
        bool getPreserveFace() const { return _preserveFace; }
        void setPreserveObject(bool flag) { _preserveObject = flag; }
        bool getPreserveObject() const { return _preserveObject; }
        void setDefaultDOFAnimationState(bool state) { _defaultDOFAnimationState = state; }
        bool getDefaultDOFAnimationState() const { return _defaultDOFAnimationState; }
        void setUseTextureAlphaForTransparancyBinning(bool flag) { _useTextureAlphaForTransparancyBinning=flag; }
        bool getUseTextureAlphaForTransparancyBinning() const { return _useTextureAlphaForTransparancyBinning; }
        void setDoUnitsConversion(bool flag) { _doUnitsConversion=flag; }
        bool getDoUnitsConversion() const { return _doUnitsConversion; }
        void setDesiredUnits(CoordUnits units ) { _desiredUnits=units; }
        CoordUnits getDesiredUnits() const { return _desiredUnits; }
        
        bool getKeepExternalReferences() const { return _keepExternalReferences; }
        void setKeepExternalReferences( bool flag) { _keepExternalReferences=flag; }

    protected:

        // Options
        osg::ref_ptr<const osgDB::ReaderWriter::Options> _options;
        bool                        _preserveFace;
        bool                        _preserveObject;
        bool                        _defaultDOFAnimationState;
        bool                        _useTextureAlphaForTransparancyBinning;
        bool                        _doUnitsConversion;
        CoordUnits                  _desiredUnits;
        
        bool                        _keepExternalReferences;

        friend class Header;
        bool _done;
        int _level;
        int _subfaceLevel;
        double _unitScale;
        uint32 _version;

        // Header data
        osg::ref_ptr<osg::Node> _osgHeader;

        osg::ref_ptr<VertexPool> _vertexPool;
        osg::ref_ptr<ColorPool> _colorPool;
        osg::ref_ptr<TexturePool> _texturePool;
        osg::ref_ptr<MaterialPool> _materialPool;
        osg::ref_ptr<LightPointAppearancePool> _lightPointAppearancePool;
        osg::ref_ptr<ShaderPool> _shaderPool;
        bool _colorPoolParent;
        bool _texturePoolParent;
        bool _materialPoolParent;
        bool _lightPointAppearancePoolParent;
        bool _shaderPoolParent;

        osg::ref_ptr<PrimaryRecord> _currentPrimaryRecord;

        typedef std::vector<osg::ref_ptr<PrimaryRecord> > LevelStack;
        LevelStack _levelStack;
        LevelStack _extensionStack;

        typedef std::map<int,osg::ref_ptr<osg::Node> > InstanceDefinitionMap;
        InstanceDefinitionMap _instanceDefinitionMap;
};


inline TexturePool* Document::getOrCreateTexturePool()
{
    if (!_texturePool.valid())
        _texturePool = new TexturePool;
    return _texturePool.get();
}


inline MaterialPool* Document::getOrCreateMaterialPool()
{
    if (!_materialPool.valid())
        _materialPool = new MaterialPool;
    return _materialPool.get();
}


inline LightPointAppearancePool* Document::getOrCreateLightPointAppearancePool()
{
    if (!_lightPointAppearancePool.valid())
        _lightPointAppearancePool = new LightPointAppearancePool;
    return _lightPointAppearancePool.get();
}


inline ShaderPool* Document::getOrCreateShaderPool()
{
    if (!_shaderPool.valid())
        _shaderPool = new ShaderPool;
    return _shaderPool.get();
}


inline PrimaryRecord* Document::getTopOfLevelStack()
{
    // Anything on the level stack?
    if (_levelStack.empty())
        return NULL;

    return _levelStack.back().get();
}


} // end namespace

#endif
