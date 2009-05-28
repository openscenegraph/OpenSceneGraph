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

#include <osgSim/MultiSwitch>
#include <osgSim/LightPointSystem>
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
    enum Flags
    {
                                                // bit 0 = reserved
        NO_BACK_COLOR    = 0x80000000u >> 1,        // bit 1 = no back color
                                                // bit 2 = reserved
        CALLIGRAPHIC    = 0x80000000u >> 3,        // bit 3 = calligraphic proximity occulting
        REFLECTIVE        = 0x80000000u >> 4,        // bit 4 = reflective, non-emissive point
                                                // bit 5-7 = randomize intensity
                                                //   0 = never
                                                //     1 = low
                                                //   2 = medium
                                                //   3 = high
        PERSPECTIVE        = 0x80000000u >> 8,        // bit 8 = perspective mode
        FLASHING        = 0x80000000u >> 9,        // bit 9 = flashing
        ROTATING        = 0x80000000u >> 10,    // bit 10 = rotating
        ROTATE_CC        = 0x80000000u >> 11,    // bit 11 = rotate counter clockwise
                                                // bit 12 = reserved
                                                // bit 13-14 = quality
                                                //   0 = low
                                                //     1 = medium
                                                //   2 = high
                                                //   3 = undefined
        VISIBLE_DAY        = 0x80000000u >> 15,    // bit 15 = visible during day
        VISIBLE_DUSK    = 0x80000000u >> 16,    // bit 16 = visible during dusk
        VISIBLE_NIGHT    = 0x80000000u >> 17        // bit 17 = visible during night
                                                // bit 18-31 = spare
    };


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
    META_dispose(_lpn)

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

        // if the flashing or rotating bit is set in the flags, add a blink sequence
        if ((_flags & FLASHING) || (_flags & ROTATING))
        {
            lp._blinkSequence = new osgSim::BlinkSequence();
            if (lp._blinkSequence.valid())
            {
                lp._blinkSequence->setDataVariance(osg::Object::DYNAMIC);
                lp._blinkSequence->setPhaseShift(_animationPhaseDelay);
                lp._blinkSequence->addPulse(_animationPeriod - _animationPeriodEnable, 
                    osg::Vec4f(0.0f, 0.0f, 0.0f, 0.0f));
                lp._blinkSequence->addPulse(_animationPeriodEnable, lp._color);
            }
        }

        _lpn->addLightPoint(lp);

        // Create a new lightpoint if bi-directional.
        if ((_directionality==BIDIRECTIONAL) && vertex.validNormal())
        {
            // back intensity
            lp._intensity = _intensityBack;

            // back color
            if (!(_flags & NO_BACK_COLOR))
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

REGISTER_FLTRECORD(LightPoint, LIGHT_POINT_OP)



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
    osg::ref_ptr<LPAnimation> _animation;

public:

    IndexedLightPoint() {}

    META_Record(IndexedLightPoint)

    META_setID(_lpn)
    META_setComment(_lpn)
    META_dispose(_lpn)

    // Add lightpoint, add two if bidirectional.
    virtual void addVertex(Vertex& vertex)
    {
        osgSim::LightPoint lp;

        if (_appearance.valid())
        {
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

            // Blink sequence
            if (_animation.valid())
            {
                osgSim::BlinkSequence* blinkSequence = new osgSim::BlinkSequence;
                blinkSequence->setName(_animation->name);

                switch (_animation->animationType)
                {
                case LPAnimation::ROTATING:
                case LPAnimation::STROBE:
                    blinkSequence->setPhaseShift(_animation->animationPhaseDelay);
                    blinkSequence->addPulse(_animation->animationPeriod-_animation->animationEnabledPeriod, osg::Vec4(0,0,0,0));
                    blinkSequence->addPulse(_animation->animationEnabledPeriod, lp._color);
                    break;

                case LPAnimation::MORSE_CODE:
                    // todo
                    //blinkSequence->addPulse(double length,lp._color);
                    break;

                case LPAnimation::FLASHING_SEQUENCE:
                    {
                        blinkSequence->setPhaseShift(_animation->animationPhaseDelay);

                        for (LPAnimation::PulseArray::iterator itr=_animation->sequence.begin();
                            itr!=_animation->sequence.end();
                            ++itr)
                        {
                            double duration = itr->duration;

                            osg::Vec4 color;
                            switch (itr->state)
                            {
                            case LPAnimation::ON:
                                color = lp._color;
                                break;
                            case LPAnimation::OFF:
                                color = osg::Vec4(0,0,0,0);
                                break;
                            case LPAnimation::COLOR_CHANGE:
                                color = itr->color;
                                break;
                            }

                            blinkSequence->addPulse(duration, color);
                        }
                    }
                    break;
                }

                lp._blinkSequence = blinkSequence;
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
        int32 animationIndex = in.readInt32();
        /*int32 drawOrder =*/ in.readInt32();           // for calligraphic lights

        LightPointAppearancePool* lpAppearancePool = document.getOrCreateLightPointAppearancePool();
        _appearance = lpAppearancePool->get(appearanceIndex);

        LightPointAnimationPool* lpAnimationPool = document.getOrCreateLightPointAnimationPool();
        _animation = lpAnimationPool->get(animationIndex);

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

REGISTER_FLTRECORD(IndexedLightPoint, INDEXED_LIGHT_POINT_OP)



/** LightPointSystem
*/
class LightPointSystem : public PrimaryRecord
{
    float32 _intensity;
    int32 _animationState;
    int32  _flags;

    osg::ref_ptr<osgSim::MultiSwitch> _switch;
    osg::ref_ptr<osgSim::LightPointSystem> _lps;

public:

    LightPointSystem():
        _intensity(1.0f),
        _animationState(0),
        _flags(0)
    {}

    META_Record(LightPointSystem)
    META_addChild(_switch)

protected:

    virtual ~LightPointSystem() {}

    virtual void readRecord(RecordInputStream& in, Document& document)
    {
        std::string id = in.readString(8);

        _intensity = in.readFloat32();
        _animationState = in.readInt32(0);
        _flags = in.readInt32(0);

        _switch = new osgSim::MultiSwitch;
        _lps = new osgSim::LightPointSystem;

        _switch->setName(id);
        _lps->setName(id);
        _lps->setIntensity(_intensity);

        switch (_animationState)
        {
            // Note that OpenFlight 15.8 spec says 0 means on and 1 means off.
            // However, if animation is set on in Creator, it stores a 1, and
            // a zero is stored for off! So, for now, we ignore the spec...
            case 0:
                _lps->setAnimationState( osgSim::LightPointSystem::ANIMATION_OFF );
                break;
            default:
            case 1:
                _lps->setAnimationState( osgSim::LightPointSystem::ANIMATION_ON );
                break;
            case 2:
                _lps->setAnimationState( osgSim::LightPointSystem::ANIMATION_RANDOM );
                break;
        }

        if (_parent.valid())
            _parent->addChild(*((osg::Group*)_switch.get()));
    }

    virtual void dispose(Document& document)
    {
        if (!_switch.valid()) return;

        // Insert transform(s)
        if (_matrix.valid())
        {
            insertMatrixTransform(*_switch,*_matrix,_numberOfReplications);
        }

        // Set default sets: 0 for all off, 1 for all on
        _switch->setAllChildrenOff( 0 );
        _switch->setAllChildrenOn( 1 );

        // set initial on/off state
        unsigned int initialSet = ( (_flags & 0x80000000) != 0 ) ? 1 : 0;
        _switch->setActiveSwitchSet( initialSet );

        for (unsigned int i = 0; i < _switch->getNumChildren(); i++)
        {
            osg::Node* child = _switch->getChild(i);
            if (osgSim::LightPointNode* lpn = dynamic_cast<osgSim::LightPointNode*>(child))
                lpn->setLightPointSystem(_lps.get());
        }
    }
};

REGISTER_FLTRECORD(LightPointSystem, LIGHT_POINT_SYSTEM_OP)


