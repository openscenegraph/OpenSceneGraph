/*
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or (at
 * your option) any later version. The full license is in the LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * OpenSceneGraph Public License for more details.
*/

//
// Copyright(c) 2008 Skew Matrix Software LLC.
//

#include "FltExportVisitor.h"
#include "ExportOptions.h"
#include "VertexPaletteManager.h"
#include "LightSourcePaletteManager.h"
#include "DataOutputStream.h"
#include "Opcodes.h"
#include <osg/Group>
#include <osg/Sequence>
#include <osg/LightSource>
#include <osg/LOD>
#include <osg/ProxyNode>
#include <osg/Switch>
#include <osgSim/MultiSwitch>
#include <osgSim/DOFTransform>
#include <osgSim/LightPointNode>
#include <osgSim/ObjectRecordData>

#include <stdlib.h>

// FIXME: This header was copied verbatim from the importer, with the only change
// being that the symbols it defines are placed in namespace 'fltexp' instead
// of 'flt'.  Thus, this one-off copy has to be kept in sync with the
// importer until the reader/writer are unified...
#include "Pools.h"


namespace flt
{


void
FltExportVisitor::writeHeader( const std::string& headerName )
{
    int16 length;
    int32 version;
    const int ver = _fltOpt->getFlightFileVersionNumber();
    if (ver == ExportOptions::VERSION_15_7)
    {
        length = 304;
        version = 1570;
    }
    else if (ver == ExportOptions::VERSION_15_8)
    {
        length = 324;
        version = 1580;
    }
    else // ExportOptions::VERSION_16_1:
    {
        length = 324;
        version = 1610;
    }

    int8 units;
    switch( _fltOpt->getFlightUnits() )
    {
    case ExportOptions::KILOMETERS:
        units = 1;
        break;
    case ExportOptions::FEET:
        units = 4;
        break;
    case ExportOptions::INCHES:
        units = 5;
        break;
    case ExportOptions::NAUTICAL_MILES:
        units = 8;
        break;
    default:
    case ExportOptions::METERS:
        units = 0;
        break;
    }

    static const unsigned int SAVE_VERTEX_NORMALS_BIT = 0x80000000u >> 0;
    //static const unsigned int PACKED_COLOR_MODE_BIT   = 0x80000000u >> 1;
    //static const unsigned int CAD_VIEW_MODE_BIT       = 0x80000000u >> 2;
    uint32 flags( SAVE_VERTEX_NORMALS_BIT );

    IdHelper id(*this, headerName);
    id.dos_ = &_dos;

    _dos.writeInt16( (int16) HEADER_OP );
    _dos.writeInt16( length );
    _dos.writeID( id );
    _dos.writeInt32( version );
    _dos.writeInt32( 0 ); // edit revision
    // TBD need platform-independent method to generate date/time string
    _dos.writeString( std::string(" "), 32 ); // date and time string for last rev
    _dos.writeInt16( 0 ); // next group id
    _dos.writeInt16( 0 ); // next LOD id
    _dos.writeInt16( 0 ); // next object id
    _dos.writeInt16( 0 ); // next face id
    _dos.writeInt16( 1 ); // unit multiplier
    _dos.writeInt8( units ); // coordinate units
    _dos.writeInt8( 0 ); // if TRUE, texwhite on new faces
    _dos.writeUInt32( flags ); // flags
    _dos.writeFill( sizeof( int32 ) * 6 ); // reserved
    _dos.writeInt32( 0 ); // projection
    _dos.writeFill( sizeof( int32 ) * 7 ); // reserved
    _dos.writeInt16( 0 ); // next DOF id
    _dos.writeInt16( 1 ); // vertex storage type, should always be 1
    _dos.writeInt32( 100 ); // DB origin, 100=OpenFlight
    _dos.writeFloat64( 0. ); // SW corner X
    _dos.writeFloat64( 0. ); // SW corner Y
    _dos.writeFloat64( 0. ); // delta X
    _dos.writeFloat64( 0. ); // delta Y
    _dos.writeInt16( 0 ); // next sound id
    _dos.writeInt16( 0 ); // next path id
    _dos.writeFill( sizeof( int32 ) * 2 ); // reserved
    _dos.writeInt16( 0 ); // next clip id
    _dos.writeInt16( 0 ); // next text id
    _dos.writeInt16( 0 ); // next BSP id
    _dos.writeInt16( 0 ); // next switch id
    _dos.writeInt32( 0 ); // reserved
    _dos.writeFloat64( 0. ); // SW corner lat
    _dos.writeFloat64( 0. ); // SW corner lon
    _dos.writeFloat64( 0. ); // NE corner lat
    _dos.writeFloat64( 0. ); // NE corner lon
    _dos.writeFloat64( 0. ); // origin lat
    _dos.writeFloat64( 0. ); // origin lon
    _dos.writeFloat64( 0. ); // lambert upper lat
    _dos.writeFloat64( 0. ); // lambert upper lon
    _dos.writeInt16( 0 ); // next light source id
    _dos.writeInt16( 0 ); // next light point id
    _dos.writeInt16( 0 ); // next road id
    _dos.writeInt16( 0 ); // next CAT id
    _dos.writeFill( sizeof( int16 ) * 4 ); // reserved
    _dos.writeInt32( 0 ); // ellipsoid model, 0=WGS84
    _dos.writeInt16( 0 ); // next adaptive id
    _dos.writeInt16( 0 ); // next curve id
    _dos.writeInt16( 0 ); // utm zone
    _dos.writeFill( 6 ); // reserved
    _dos.writeFloat64( 0. ); // delta z
    _dos.writeFloat64( 0. ); // radius
    _dos.writeInt16( 0 ); // next mesh id
    _dos.writeInt16( 0 ); // next light system id

    if (version >= 1580)
    {
        _dos.writeInt32( 0 ); // reserved
        _dos.writeFloat64( 0. ); // earth major axis for user defined ellipsoid
        _dos.writeFloat64( 0. ); // earth minor axis for user defined ellipsoid
    }
}


// Group flags
static const unsigned int FORWARD_ANIM     = 0x80000000u >> 1;
static const unsigned int SWING_ANIM       = 0x80000000u >> 2;
static const unsigned int BOUND_BOX_FOLLOW = 0x80000000u >> 3;
static const unsigned int FREEZE_BOUND_BOX = 0x80000000u >> 4;
static const unsigned int DEFAULT_PARENT   = 0x80000000u >> 5;
static const unsigned int BACKWARD_ANIM    = 0x80000000u >> 6;


//
//  Convenience routine for writing Group nodes that aren't animated
//
void
FltExportVisitor::writeGroup( const osg::Group& group )
{
    int32 flags = 0, loopCount = 0;
    float32 loopDuration = 0.0f, lastFrameDuration = 0.0f;

    writeGroup(group, flags, loopCount, loopDuration, lastFrameDuration);
}



void
FltExportVisitor::writeGroup( const osg::Group& group,
                            int32 flags,
                            int32 loopCount,
                            float32 loopDuration,
                            float32 lastFrameDuration)  // <-- placeholder: ignored
{
    int16 length( 44 );
    IdHelper id(*this, group.getName() );

    _records->writeInt16( (int16) GROUP_OP );
    _records->writeInt16( length );
    _records->writeID( id );
    _records->writeInt16( 0 );        // Relative priority
    _records->writeInt16( 0 );        // Reserved
    _records->writeUInt32( flags );
    _records->writeInt16( 0 );        // Special effect ID1
    _records->writeInt16( 0 );        // Special effect ID2
    _records->writeInt16( 0 );        // Significance
    _records->writeInt8( 0 );         // Layer code
    _records->writeInt8( 0 );         // Reserved
    _records->writeInt32( 0 );        // Reserved
    _records->writeInt32( loopCount );
    _records->writeFloat32( loopDuration );
    _records->writeFloat32( lastFrameDuration );
}


//
//  Since OpenFlight doesn't have 'Sequence' records---just Group records that
//  may, optionally, be animated---this routine sets the animation-related
//  parameters for a Group record and simply forwards to writeGroup()
//
void
FltExportVisitor::writeSequence( const osg::Sequence& sequence )
{

    int32 flags = 0, loopCount = 0;
    float32 loopDuration = 0.0f, lastFrameDuration = 0.0f;

    osg::Sequence::LoopMode mode;
    int firstChildDisplayed, lastChildDisplayed;
    sequence.getInterval(mode, firstChildDisplayed, lastChildDisplayed);

    if (firstChildDisplayed == 0)
    {
        flags |= FORWARD_ANIM;
    }

    else
    {
        flags &= ~FORWARD_ANIM;
    }

    if (mode == osg::Sequence::SWING)
    {
        flags |= SWING_ANIM;
    }

    else
    {
        flags &= ~SWING_ANIM;
    }

    // Do we loop infinitely, or only a certain number of times?
    float speedUp;
    int numReps;
    sequence.getDuration(speedUp, numReps);

    if (numReps != -1)
    {
        loopCount = numReps;
    }

    else
    {
        loopCount = 0;  // == loop continuously
    }

    // Sum individual frame durations to get the total loopDuration
    for (unsigned int i = 0; i < sequence.getNumChildren(); ++i)
    {
        loopDuration += sequence.getTime(i);
    }

    lastFrameDuration = sequence.getLastFrameTime();

    writeGroup(sequence, flags, loopCount, loopDuration, lastFrameDuration);
}


void
FltExportVisitor::writeObject( const osg::Group& group, osgSim::ObjectRecordData* ord )
{
    uint16 length( 28 );
    IdHelper id(*this, group.getName() );

    if (!ord)
    {
        std::string warning( "fltexp: writeObject has invalid ObjectRecordData." );
        OSG_WARN << warning << std::endl;
        _fltOpt->getWriteResult().warn( warning );
        return;
    }

    _records->writeInt16( (int16) OBJECT_OP );
    _records->writeInt16( length );
    _records->writeID( id );
    _records->writeInt32( ord->_flags );
    _records->writeInt16( ord->_relativePriority );
    _records->writeUInt16( ord->_transparency );
    _records->writeUInt16( ord->_effectID1 );
    _records->writeUInt16( ord->_effectID2 );
    _records->writeUInt16( ord->_significance );
    _records->writeUInt16( 0 ); // reserved
}

void
FltExportVisitor::writeDegreeOfFreedom( const osgSim::DOFTransform* dof )
{
    const osg::Matrix& invPut = dof->getInversePutMatrix();

    // Origin of DOF coord sys
    osg::Vec3d origin( invPut.getTrans() );

    osg::Vec3 xAxis( invPut(0,0), invPut(0,1), invPut(0,2) );
    osg::Vec3 yAxis( invPut(1,0), invPut(1,1), invPut(1,2) );
    // Reference point along DOF coord sys's X axis
    osg::Vec3d pointOnXAxis = origin + xAxis;
    // Reference point in DOF coord sys's X-Y plane
    osg::Vec3d pointInXYPlane = origin + yAxis;

    // Translations
    osg::Vec3d minTranslate( dof->getMinTranslate() );
    osg::Vec3d maxTranslate( dof->getMaxTranslate() );
    osg::Vec3d currTranslate( dof->getCurrentTranslate() );
    osg::Vec3d incrTranslate( dof->getIncrementTranslate() );

    // Rotations
    osg::Vec3d minHPR( dof->getMinHPR() );
    osg::Vec3d maxHPR( dof->getMaxHPR() );
    osg::Vec3d currHPR( dof->getCurrentHPR() );
    osg::Vec3d incrHPR( dof->getIncrementHPR() );

    // Scaling
    osg::Vec3d minScale( dof->getMinScale() );
    osg::Vec3d maxScale( dof->getMaxScale() );
    osg::Vec3d currScale( dof->getCurrentScale() );
    osg::Vec3d incrScale( dof->getIncrementScale() );


    uint16 length( 384 );
    IdHelper id(*this, dof->getName() );

    _records->writeInt16( (int16) DOF_OP );
    _records->writeInt16( length );
    _records->writeID( id );
    _records->writeInt32( 0 );  // 'Reserved' (unused)
    _records->writeVec3d( origin );
    _records->writeVec3d( pointOnXAxis );
    _records->writeVec3d( pointInXYPlane );

    // Translations
    _records->writeFloat64( minTranslate.z() );
    _records->writeFloat64( maxTranslate.z() );
    _records->writeFloat64( currTranslate.z() );
    _records->writeFloat64( incrTranslate.z() );

    _records->writeFloat64( minTranslate.y() );
    _records->writeFloat64( maxTranslate.y() );
    _records->writeFloat64( currTranslate.y() );
    _records->writeFloat64( incrTranslate.y() );

    _records->writeFloat64( minTranslate.x() );
    _records->writeFloat64( maxTranslate.x() );
    _records->writeFloat64( currTranslate.x() );
    _records->writeFloat64( incrTranslate.x() );

    // Rotations: 0 = Yaw, 1 = Pitch, 2 = Roll
    _records->writeFloat64( osg::RadiansToDegrees(minHPR[1]) );
    _records->writeFloat64( osg::RadiansToDegrees(maxHPR[1]) );
    _records->writeFloat64( osg::RadiansToDegrees(currHPR[1]) );
    _records->writeFloat64( osg::RadiansToDegrees(incrHPR[1]) );

    _records->writeFloat64( osg::RadiansToDegrees(minHPR[2]) );
    _records->writeFloat64( osg::RadiansToDegrees(maxHPR[2]) );
    _records->writeFloat64( osg::RadiansToDegrees(currHPR[2]) );
    _records->writeFloat64( osg::RadiansToDegrees(incrHPR[2]) );

    _records->writeFloat64( osg::RadiansToDegrees(minHPR[0]) );
    _records->writeFloat64( osg::RadiansToDegrees(maxHPR[0]) );
    _records->writeFloat64( osg::RadiansToDegrees(currHPR[0]) );
    _records->writeFloat64( osg::RadiansToDegrees(incrHPR[0]) );

    // Scaling
    _records->writeFloat64( minScale.z() );
    _records->writeFloat64( maxScale.z() );
    _records->writeFloat64( currScale.z() );
    _records->writeFloat64( incrScale.z() );

    _records->writeFloat64( minScale.y() );
    _records->writeFloat64( maxScale.y() );
    _records->writeFloat64( currScale.y() );
    _records->writeFloat64( incrScale.y() );

    _records->writeFloat64( minScale.x() );
    _records->writeFloat64( maxScale.x() );
    _records->writeFloat64( currScale.x() );
    _records->writeFloat64( incrScale.y() );

    _records->writeInt32( dof->getLimitationFlags() );  //  Constraint flags
    _records->writeInt32( 0 );                          // 'Reserved' (unused)

}

// Parent pool override flags
static const unsigned long COLOR_PALETTE_OVERRIDE        = 0x80000000u >> 0;
static const unsigned long MATERIAL_PALETTE_OVERRIDE     = 0x80000000u >> 1;
static const unsigned long TEXTURE_PALETTE_OVERRIDE      = 0x80000000u >> 2;
static const unsigned long LINE_STYLE_PALETTE_OVERRIDE   = 0x80000000u >> 3;
static const unsigned long SOUND_PALETTE_OVERRIDE        = 0x80000000u >> 4;
static const unsigned long LIGHT_SOURCE_PALETTE_OVERRIDE = 0x80000000u >> 5;
static const unsigned long LIGHT_POINT_PALETTE_OVERRIDE  = 0x80000000u >> 6;
static const unsigned long SHADER_PALETTE_OVERRIDE       = 0x80000000u >> 7;

void
FltExportVisitor::writeExternalReference( const osg::ProxyNode& proxy )
{
    uint16 length( 216 );

    // Set sane defaults for the override flags
    unsigned long flags = COLOR_PALETTE_OVERRIDE       |
                          MATERIAL_PALETTE_OVERRIDE    |
                          TEXTURE_PALETTE_OVERRIDE     |
                          LIGHT_POINT_PALETTE_OVERRIDE |
                          SHADER_PALETTE_OVERRIDE ;

    // Selectively turn off overrides for resources we don't need
    const ParentPools* pp = dynamic_cast<const ParentPools*>(proxy.getUserData() );

    if (pp && pp->getColorPool() )
      flags &= ~COLOR_PALETTE_OVERRIDE;

    if (pp && pp->getMaterialPool() )
      flags &= ~MATERIAL_PALETTE_OVERRIDE;

    if (pp && pp->getTexturePool() )
      flags &= ~TEXTURE_PALETTE_OVERRIDE;

    if (pp && pp->getLightSourcePool() )
      flags &= ~LIGHT_SOURCE_PALETTE_OVERRIDE;

    if (pp && pp->getLPAppearancePool() )
      flags &= ~LIGHT_POINT_PALETTE_OVERRIDE;

    if (pp && pp->getShaderPool() )
      flags &= ~SHADER_PALETTE_OVERRIDE;

    _records->writeInt16( (int16) EXTERNAL_REFERENCE_OP );
    _records->writeInt16( length );
    _records->writeString(proxy.getFileName(0), 200);
    _records->writeInt32(0);   // Reserved
    _records->writeInt32(flags);
    _records->writeInt16(0);   // ViewAsBoundingBox flag
    _records->writeInt16(0);   // Reserved
}

void
FltExportVisitor::writeLevelOfDetail( const osg::LOD& lod,
                                    osg::Vec3d const& center,
                                    double switchInDist,
                                    double switchOutDist)
{
    uint16 length( 80 );
    IdHelper id(*this, lod.getName() );

    _records->writeInt16( (int16) LOD_OP );
    _records->writeInt16( length );
    _records->writeID( id );
    _records->writeInt32( 0 );                 // 'Reserved' field
    _records->writeFloat64( switchInDist );
    _records->writeFloat64( switchOutDist );   // Switch-out distance
    _records->writeInt16( 0 );                 // Special Effect ID1
    _records->writeInt16( 0 );                 // Special Effect ID2
    _records->writeInt32( 0 );                 // Flags
    _records->writeFloat64( center.x() );
    _records->writeFloat64( center.y() );
    _records->writeFloat64( center.z() );
    _records->writeFloat64( 0 );               // Transition range
    _records->writeFloat64( 0 );               // Significant size

}

void
FltExportVisitor::writeLightSource( const osg::LightSource& node )
{

    static const unsigned int ENABLED = 0x80000000u >> 0;
    static const unsigned int GLOBAL  = 0x80000000u >> 1;

    osg::Light const* light = node.getLight();
    int index = _lightSourcePalette->add(light);

    osg::Vec4d const& lightPos = light->getPosition();
    osg::Vec3f const& lightDir = light->getDirection();

    uint32 flags = 0;
    osg::StateSet const* ss = getCurrentStateSet();
    if (ss->getMode(GL_LIGHT0 + light->getLightNum() ) & osg::StateAttribute::ON)
    {
        flags |= ENABLED;
    }

    // If this light is enabled for the node at the top of our StateSet stack,
    // assume it is 'global' for OpenFlight's purposes
    ss = _stateSetStack.front().get();
    if (ss->getMode(GL_LIGHT0 + light->getLightNum() ) & osg::StateAttribute::ON)
    {
        flags |= GLOBAL;
    }

    uint16 length( 64 );
    IdHelper id(*this, node.getName() );

    _records->writeInt16( (int16) LIGHT_SOURCE_OP );
    _records->writeInt16( length );
    _records->writeID( id );
    _records->writeInt32( 0 );              // Reserved
    _records->writeInt32( index );          // Index into light source palette
    _records->writeInt32( 0 );              // Reserved
    _records->writeUInt32( flags );         // Flags
    _records->writeInt32( 0 );              // Reserved
    _records->writeVec3d( osg::Vec3d(
                              lightPos.x() , lightPos.y() , lightPos.z() ) );

    // TODO: Verify that indices 0 and 1 correspond to yaw and pitch
    _records->writeFloat32( lightDir[0] );  // Yaw
    _records->writeFloat32( lightDir[1] );  // Pitch
}



void
FltExportVisitor::writeSwitch( const osgSim::MultiSwitch* ms )
{
    int32 currMask = ms->getActiveSwitchSet();
    int32 numMasks = ms->getSwitchSetList().size();
    int32 numWordsPerMask = ms->getNumChildren() / 32;
    if (ms->getNumChildren() % 32 != 0) ++numWordsPerMask;

    uint16 length( 28 + numMasks * numWordsPerMask * sizeof(int32) );
    IdHelper id(*this, ms->getName() );

    _records->writeInt16( (int16) SWITCH_OP );
    _records->writeInt16( length );
    _records->writeID( id );
    _records->writeInt32( 0 );  // <-- 'Reserved' (unused)
    _records->writeInt32( currMask );
    _records->writeInt32( numMasks );
    _records->writeInt32( numWordsPerMask );

    // For each mask...
    for (int i = 0; i < numMasks; ++i)
    {
        // ... write out the set of 32-bit words comprising the mask
        uint32 maskWord = 0;
        const osgSim::MultiSwitch::ValueList& maskBits = ms->getValueList(i);

        for (size_t j = 0; j < maskBits.size(); ++j)
        {
            // If this bit is set, set the corresponding mask word
            if (maskBits[j]) maskWord |= 1 << (j % 32);

            // If we just set the 31st (last) bit of the current word, need
            // to write it out and reset prior to continuing the loop
            if ( (j + 1) % 32 == 0 )
            {
                _records->writeUInt32(maskWord);
                maskWord = 0;
            }
        }

        // If the mask size wasn't a multiple of 32, need to write out
        // the final word containing the 'remainder' bits
        if (maskBits.size() % 32 != 0)
        {
            _records->writeUInt32(maskWord);
        }
    }

}


void
FltExportVisitor::writeSwitch( const osg::Switch* sw )
{
    // An osg::Switch is just a special case of an osgSim::MultiSwitch
    // that only has a single mask
    int32 currMask = 0;
    int32 numMasks = 1;
    int32 numWordsPerMask = sw->getNumChildren() / 32;
    if (sw->getNumChildren() % 32 != 0) ++numWordsPerMask;

    uint16 length( 28 + numMasks * numWordsPerMask * sizeof(int32) );
    IdHelper id(*this, sw->getName() );

    _records->writeInt16( (int16) SWITCH_OP );
    _records->writeInt16( length );
    _records->writeID( id );
    _records->writeInt32( 0 );  // <-- 'Reserved' (unused)
    _records->writeInt32( currMask );
    _records->writeInt32( numMasks );
    _records->writeInt32( numWordsPerMask );

    // Bust the mask up into as many 32-bit words as are necessary to hold it
    uint32 maskWord = 0;
    const osg::Switch::ValueList& maskBits = sw->getValueList();

    for (size_t i = 0; i < maskBits.size(); ++i)
    {
        // If this bit is set, set the corresponding mask word
        if (maskBits[i]) maskWord |= 1 << (i % 32);

        // If we just set the 31st (last) bit of the current word, need
        // to write it out and reset prior to continuing the loop
        if ( (i + 1) % 32 == 0 )
        {
            _records->writeUInt32(maskWord);
            maskWord = 0;
        }
    }

    // If the mask size wasn't a multiple of 32, need to write out
    // the final word containing the 'remainder' bits
    if (maskBits.size() % 32 != 0)
    {
        _records->writeUInt32(maskWord);
    }

}

void
FltExportVisitor::writeLightPoint( const osgSim::LightPointNode* lpn )
{
    enum Directionality
    {
        OMNIDIRECTIONAL = 0,
        UNIDIRECTIONAL = 1,
        BIDIRECTIONAL = 2
    };
    enum DisplayMode
    {
        RASTER = 0,
        CALLIG = 1,
        EITHER = 2
    };
    enum Modes
    {
        ENABLE = 0,
        DISABLE = 1
    };
    enum Flags
    {
        NO_BACK_COLOR    = 0x80000000u >> 1,
        CALLIGRAPHIC    = 0x80000000u >> 3,
        REFLECTIVE        = 0x80000000u >> 4,
        PERSPECTIVE        = 0x80000000u >> 8,
        FLASHING        = 0x80000000u >> 9,
        ROTATING        = 0x80000000u >> 10,
        ROTATE_CC        = 0x80000000u >> 11,
        VISIBLE_DAY        = 0x80000000u >> 15,
        VISIBLE_DUSK    = 0x80000000u >> 16,
        VISIBLE_NIGHT    = 0x80000000u >> 17
    };
    int32 flags( NO_BACK_COLOR );

    if (lpn->getNumLightPoints() == 0)
        return;

    // In OSG, each LightPoint within a LightPointNode  can have different appearance
    // parameters, but in OpenFlight, a Light Point Record contains a list of homogeneous
    // vertices. To be correct, we'd have to look at all LightPoints in the LightPointNode
    // and spew out multiple FLT records for each group that shared common appearance
    // parameters. Instead, we cheat: We take the first LightPoint and use its appearance
    // parameters for all LightPoints in the LightPointNode.
    const osgSim::LightPoint& lp0 = lpn->getLightPoint( 0 );

    // No really good mapping between OSG and FLT light point animations.
    float32 animPeriod( 0.f );
    float32 animEnabled( 0.f );
    float32 animPhaseDelay( 0.f );
    if (lp0._blinkSequence != NULL)
    {
        flags |= FLASHING;
        animPeriod = 4.f;
        animEnabled = 2.f;
        animPhaseDelay = lp0._blinkSequence->getPhaseShift();
    }

    // Note that true bidirectional light points are currently unsupported (they are unavailable
    // in OSG, so we never write them out to FLT as BIDIRECTIONAL.
    int32 directionality( OMNIDIRECTIONAL );
    float32 horizLobe( 360.f );
    float32 vertLobe( 360.f );
    float32 lobeRoll( 0.f );
    const osgSim::DirectionalSector* ds = dynamic_cast< osgSim::DirectionalSector* >( lp0._sector.get() );
    if (ds)
    {
        directionality = UNIDIRECTIONAL;
        horizLobe = osg::RadiansToDegrees( ds->getHorizLobeAngle() );
        vertLobe = osg::RadiansToDegrees( ds->getVertLobeAngle() );
        lobeRoll = osg::RadiansToDegrees( ds->getLobeRollAngle() );
    }

    {
        // Braces req'd to invoke idHelper destructor (and potentially
        // write LongID record) before Push Record.

        const uint16 length( 156 );
        IdHelper id( *this, lpn->getName() );

        _records->writeInt16( (int16) LIGHT_POINT_OP );
        _records->writeInt16( length );
        _records->writeID( id );
        _records->writeInt16( 0 ); // Surface material code
        _records->writeInt16( 0 ); // Feature ID
        _records->writeUInt32( ~0u ); // OpenFlight erronously say -1, so will assume ~0u is OK.  Back color for bidirectional
        _records->writeInt32( EITHER ); // Display mode
        _records->writeFloat32( lp0._intensity ); // Intensity
        _records->writeFloat32( 0.f ); // Back intensity TBD
        _records->writeFloat32( 0.f ); // min defocus
        _records->writeFloat32( 0.f ); // max defocus
        _records->writeInt32( DISABLE ); // Fading mode
        _records->writeInt32( DISABLE ); // Fog punch mode
        _records->writeInt32( DISABLE ); // Directional mode
        _records->writeInt32( 0 ); // Range mode
        _records->writeFloat32( lpn->getMinPixelSize() ); // min pixel size
        _records->writeFloat32( lpn->getMaxPixelSize() ); // max pixel size
        _records->writeFloat32( lp0._radius * 2.f ); // Actual size
        _records->writeFloat32( 1.f ); // transparent falloff pixel size
        _records->writeFloat32( 1.f ); // Transparent falloff exponent
        _records->writeFloat32( 1.f ); // Transparent falloff scalar
        _records->writeFloat32( 0.f ); // Transparent falloff clamp
        _records->writeFloat32( 1.f ); // Fog scalar
        _records->writeFloat32( 0.f ); // Reserved
        _records->writeFloat32( 0.f ); // Size difference threshold
        _records->writeInt32( directionality ); // Directionality
        _records->writeFloat32( horizLobe ); // Horizontal lobe angle
        _records->writeFloat32( vertLobe ); // Vertical lobe angle
        _records->writeFloat32( lobeRoll ); // Lobe roll angle
        _records->writeFloat32( 0.f ); // Directional falloff exponent
        _records->writeFloat32( 0.f ); // Directional ambient intensity
        _records->writeFloat32( animPeriod ); // Animation period in seconds
        _records->writeFloat32( animPhaseDelay ); // Animation phase delay in seconds
        _records->writeFloat32( animEnabled ); // Animation enabled period in seconds
        _records->writeFloat32( 1.f ); // Significance
        _records->writeInt32( 0 ); // Calligraphic draw order
        _records->writeInt32( flags ); // Flags
        _records->writeVec3f( osg::Vec3f( 0.f, 0.f, 0.f ) ); // Axis of rotation
    }

    {
        osg::ref_ptr< osg::Vec3dArray > v = new osg::Vec3dArray( lpn->getNumLightPoints() );
        osg::ref_ptr< osg::Vec4Array > c = new osg::Vec4Array( lpn->getNumLightPoints() );
        osg::ref_ptr< osg::Vec3Array > n = new osg::Vec3Array( lpn->getNumLightPoints() );
        osg::Vec3f normal( 0.f, 0.f, 1.f );

        unsigned int idx;
        for( idx=0; idx<lpn->getNumLightPoints(); idx++)
        {
            const osgSim::LightPoint& lp = lpn->getLightPoint( idx );
            (*v)[ idx ] = lp._position;
            (*c)[ idx ] = lp._color;

            const osgSim::DirectionalSector* ds = dynamic_cast< osgSim::DirectionalSector* >( lp._sector.get() );
            if (ds)
                normal = ds->getDirection();
            (*n)[ idx ] = normal;
        }
        _vertexPalette->add( (const osg::Array*)NULL, v.get(), c.get(), n.get(), NULL, true, true, false );
    }

    writeMatrix( lpn->getUserData() );
    writeComment( *lpn );
    writePush();
    writeVertexList( 0, lpn->getNumLightPoints() );
    writePop();
}


void
FltExportVisitor::writeColorPalette()
{
    // FLT exporter doesn't use a color palette but writes
    // a bogus one to satisfy loaders that require it.
    uint16 length( 4228 );

    _dos.writeInt16( (int16) COLOR_PALETTE_OP );
    _dos.writeInt16( length );
    _dos.writeFill( 128 ); // Reserved
    int idx;
    for( idx=0; idx<1024; idx++)
        _dos.writeUInt32( 0xffffffff ); // Color n
}




}
