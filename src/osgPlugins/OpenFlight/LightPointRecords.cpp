//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include <osgSim/LightPointNode>
#include <osg/Texture2D>
#include "Registry.h"
#include "Document.h"
#include "RecordInputStream.h"

using namespace flt;

/** LightPoint
*/
class LightPoint : public PrimaryRecord
{
    enum Directionality
    {
        OMNIDIRECTIONAL = 0,
        UNIDIRECTIONAL = 1,
        BIDIRECTIONAL = 2
    };

    // flags
    static const unsigned int NO_BACK_COLOR_BIT = 0x80000000u >> 1;

    int16   _material;
    int16   _feature;
    osg::Vec4f _backColor;
    int32   _displayMode;
    float32 _intensityFront;
    float32 _intensityBack;
    float32 _minDefocus;
    float32 _maxDefocus;
    int32   _fadeMode;
    int32   _fogPunchMode;
    int32   _directionalMode;
    int32   _rangeMode;
    float32 _minPixelSize;
    float32 _maxPixelSize;
    float32 _actualPixelSize;
    float32 _transparentFalloff;
    float32 _transparentFalloffExponent;
    float32 _transparentFalloffScalar;
    float32 _transparentFalloffClamp;
    float32 _fog;
    float32 _sizeDifferenceThreshold;
    int32   _directionality;
    float32 _lobeHorizontal;
    float32 _lobeVertical;
    float32 _lobeRoll;
    float32 _falloff;
    float32 _ambientIntensity;
    float32 _animationPeriod;
    float32 _animationPhaseDelay;
    float32 _animationPeriodEnable;
    float32 _significance;
    int32   _drawOrder;
    uint32  _flags;
    osg::Vec3f _animationAxis;

    osg::ref_ptr<osgSim::LightPointNode> _lpn;

public:

    LightPoint() {}

    META_Record(LightPoint)

    META_setID(_lpn)
    META_setComment(_lpn)
    META_setMatrix(_lpn)

    // Add lightpoint, add two if bidirectional.
    virtual void addVertex(Vertex& vertex)
    {
        osgSim::LightPoint lp;
        lp._position = vertex._coord;
        lp._radius = 0.5f * _actualPixelSize;
        lp._intensity = _intensityFront;

        // color
        lp._color = (vertex.validColor()) ? vertex._color : osg::Vec4(1,1,1,1);

        // sector
        bool directional = (_directionality==UNIDIRECTIONAL) || (_directionality==BIDIRECTIONAL);
        if (directional && vertex.validNormal())
        {
            lp._sector = new osgSim::DirectionalSector(
                vertex._normal, 
                osg::DegreesToRadians(_lobeHorizontal),
                osg::DegreesToRadians(_lobeVertical),
                osg::DegreesToRadians(_lobeRoll));
        }

        _lpn->addLightPoint(lp);

        // Create a new lightpoint if bi-directional.
        if ((_directionality==BIDIRECTIONAL) && vertex.validNormal())
        {
            // back intensity
            lp._intensity = _intensityBack;

            // back color
            if (!(_flags & NO_BACK_COLOR_BIT))
                lp._color = _backColor;

            // back sector
            lp._sector = new osgSim::DirectionalSector(
                -vertex._normal, 
                osg::DegreesToRadians(_lobeHorizontal),
                osg::DegreesToRadians(_lobeVertical),
                osg::DegreesToRadians(_lobeRoll));

            _lpn->addLightPoint(lp);
        }
    }

protected:

    virtual ~LightPoint() {}

    virtual void readRecord(RecordInputStream& in, Document& document)
    {
        std::string id = in.readString(8);
        _material = in.readInt16();
        _feature = in.readInt16();

        int32 backColorIndex = in.readInt32();
        
        _backColor = document.getColorPool() ? 
                            document.getColorPool()->getColor(backColorIndex) : 
                            osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
                            
        _displayMode = in.readInt32();
        _intensityFront = in.readFloat32();
        _intensityBack = in.readFloat32();
        _minDefocus = in.readFloat32();
        _maxDefocus = in.readFloat32();
        _fadeMode = in.readInt32();
        _fogPunchMode = in.readInt32();
        _directionalMode = in.readInt32();
        _rangeMode = in.readInt32();
        _minPixelSize = in.readFloat32(); // * document.unitScale();
        _maxPixelSize = in.readFloat32(); // * document.unitScale();
        _actualPixelSize = in.readFloat32(); // * document.unitScale();
        _transparentFalloff = in.readFloat32();
        _transparentFalloffExponent = in.readFloat32();
        _transparentFalloffScalar = in.readFloat32();
        _transparentFalloffClamp = in.readFloat32();
        _fog = in.readFloat32();
        in.forward(4);
        _sizeDifferenceThreshold = in.readFloat32();
        _directionality = in.readInt32();
        _lobeHorizontal = in.readFloat32();
        _lobeVertical = in.readFloat32();
        _lobeRoll = in.readFloat32();
        _falloff = in.readFloat32();
        _ambientIntensity = in.readFloat32();
        _animationPeriod = in.readFloat32();
        _animationPhaseDelay = in.readFloat32();
        _animationPeriodEnable = in.readFloat32();
        _significance = in.readFloat32();
        _drawOrder = in.readInt32();
        _flags = in.readUInt32(0);
        _animationAxis = in.readVec3f();

        _lpn = new osgSim::LightPointNode;
        _lpn->setName(id);
        _lpn->setMinPixelSize(_minPixelSize);
        _lpn->setMaxPixelSize(_maxPixelSize);

        // Add to parent
        if (_parent.valid())
            _parent->addChild(*_lpn);
    }
};

RegisterRecordProxy<LightPoint> g_LightPoint(LIGHT_POINT_OP);


/** IndexedLightPoint
*/
class IndexedLightPoint : public PrimaryRecord
{
    enum Directionality
    {
        OMNIDIRECTIONAL = 0,
        UNIDIRECTIONAL = 1,
        BIDIRECTIONAL = 2
    };

    // flags
    static const unsigned int NO_BACK_COLOR_BIT = 0x80000000u >> 1;

    osg::ref_ptr<osgSim::LightPointNode> _lpn;
    osg::ref_ptr<LPAppearance> _appearance;

public:

    IndexedLightPoint() {}

    META_Record(IndexedLightPoint)

    META_setID(_lpn)
    META_setComment(_lpn)
    META_setMatrix(_lpn)

    // Add lightpoint, add two if bidirectional.
    virtual void addVertex(Vertex& vertex)
    {
        if (_appearance.valid())
        {
            osgSim::LightPoint lp;
            lp._position = vertex._coord;
            lp._radius = 0.5f * _appearance->actualPixelSize;
            lp._intensity = _appearance->intensityFront;

            // color
            lp._color = (vertex.validColor()) ? vertex._color : osg::Vec4(1,1,1,1);

            // sector
            bool directional = (_appearance->directionality==UNIDIRECTIONAL) || (_appearance->directionality==BIDIRECTIONAL);
            if (directional && vertex.validNormal())
            {
                lp._sector = new osgSim::DirectionalSector(
                    vertex._normal, 
                    osg::DegreesToRadians(_appearance->horizontalLobeAngle),
                    osg::DegreesToRadians(_appearance->verticalLobeAngle),
                    osg::DegreesToRadians(_appearance->lobeRollAngle));
            }

            _lpn->addLightPoint(lp);

            // Create a new lightpoint if bi-directional.
            if ((_appearance->directionality==BIDIRECTIONAL) && vertex.validNormal())
            {
                // back intensity
                lp._intensity = _appearance->intensityBack;

                // back color
                if (!(_appearance->flags & NO_BACK_COLOR_BIT))
                    lp._color = _appearance->backColor;

                // back sector
                lp._sector = new osgSim::DirectionalSector(
                    -vertex._normal, 
                    osg::DegreesToRadians(_appearance->horizontalLobeAngle),
                    osg::DegreesToRadians(_appearance->verticalLobeAngle),
                    osg::DegreesToRadians(_appearance->lobeRollAngle));

                _lpn->addLightPoint(lp);
            }
        }
    }


protected:

    virtual ~IndexedLightPoint() {}

    virtual void readRecord(RecordInputStream& in, Document& document)
    {
        std::string id = in.readString(8);
        int32 appearanceIndex = in.readInt32();
        /*int32 animationIndex =*/ in.readInt32();
        /*int32 drawOrder =*/ in.readInt32();           // for calligraphic lights

        LightPointAppearancePool* lpaPool = document.getOrCreateLightPointAppearancePool();
        _appearance = lpaPool->get(appearanceIndex);

        _lpn = new osgSim::LightPointNode;
        _lpn->setName(id);

        if (_appearance.valid())
        {
            _lpn->setMinPixelSize(_appearance->minPixelSize);
            _lpn->setMaxPixelSize(_appearance->maxPixelSize);

            if (_appearance->texturePatternIndex != -1)
            {
                // Use point sprites for light points.
                _lpn->setPointSprite();

                TexturePool* tp = document.getOrCreateTexturePool();
                osg::StateSet* textureStateSet = tp->get(_appearance->texturePatternIndex);
                if (textureStateSet)
                {
                    // Merge face stateset with texture stateset
                    osg::StateSet* stateset = _lpn->getOrCreateStateSet();
                    stateset->merge(*textureStateSet);
                }
            }
        }

        // Add to parent
        if (_parent.valid())
            _parent->addChild(*_lpn);

    }
};

RegisterRecordProxy<IndexedLightPoint> g_IndexedLightPoint(INDEXED_LIGHT_POINT_OP);


/** LightPointSystem
*/
class LightPointSystem : public PrimaryRecord
{
public:

    LightPointSystem() {}

    META_Record(LightPointSystem)

protected:

    virtual ~LightPointSystem() {}

    virtual void readRecord(RecordInputStream& in, Document& /*document*/)
    {
        std::string id = in.readString(8);
        osg::notify(osg::INFO) << "ID: " << id << std::endl;

        osg::Group* group = new osg::Group;
        group->setName(id);

        if (_parent.valid())
            _parent->addChild(*group);
    }
};

RegisterRecordProxy<LightPointSystem> g_LightPointSystem(LIGHT_POINT_SYSTEM_OP);

