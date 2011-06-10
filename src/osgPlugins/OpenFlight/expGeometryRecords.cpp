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
#include "DataOutputStream.h"
#include "Opcodes.h"
#include "MaterialPaletteManager.h"
#include "TexturePaletteManager.h"
#include "VertexPaletteManager.h"
#include <osg/CullFace>
#include <osg/BlendFunc>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/io_utils>
#include <osg/Material>
#include <osg/Texture2D>

#include <sstream>


namespace flt
{


// Bit flags for multitexturing
static unsigned int LAYER_1( 0x80000000 >> 0 );
#if 0
// unused so if'deffing out
static unsigned int LAYER_2( 0x80000000 >> 1 );
static unsigned int LAYER_3( 0x80000000 >> 2 );
static unsigned int LAYER_4( 0x80000000 >> 3 );
static unsigned int LAYER_5( 0x80000000 >> 4 );
static unsigned int LAYER_6( 0x80000000 >> 5 );
static unsigned int LAYER_7( 0x80000000 >> 6 );
#endif


bool
FltExportVisitor::isLit( const osg::Geometry& geom ) const
{
    const osg::StateSet* ss = getCurrentStateSet();
    if ( ss->getMode( GL_LIGHTING ) & osg::StateAttribute::ON )
        return true;
    else
        return false; //( geom.getNormalBinding() == osg::Geometry::BIND_PER_VERTEX );
}

bool
FltExportVisitor::isTextured( int unit, const osg::Geometry& geom ) const
{
    const osg::StateSet* ss = getCurrentStateSet();
    bool texOn( ss->getTextureMode( unit, GL_TEXTURE_2D ) & osg::StateAttribute::ON );
    bool hasCoords( geom.getTexCoordArray( unit ) != NULL );

    return( texOn && hasCoords );
}

bool
FltExportVisitor::isMesh( const GLenum mode ) const
{
    return( (mode == GL_TRIANGLE_STRIP) ||
        (mode == GL_TRIANGLE_FAN) ||
        (mode == GL_QUAD_STRIP) );
}

bool
FltExportVisitor::atLeastOneFace( const osg::Geometry& geom ) const
{
    // Return true if at least one PrimitiveSet mode will use a Face record.
    unsigned int jdx;
    for (jdx=0; jdx < geom.getNumPrimitiveSets(); jdx++)
    {
        const osg::PrimitiveSet* prim = geom.getPrimitiveSet( jdx );
        if( !isMesh( prim->getMode() ) )
            return true;
    }
    // All PrimitiveSet modes will use Mesh records.
    return false;
}
bool
FltExportVisitor::atLeastOneMesh( const osg::Geometry& geom ) const
{
    // Return true if at least one PrimitiveSet mode will use a Mesh record.
    unsigned int jdx;
    for (jdx=0; jdx < geom.getNumPrimitiveSets(); jdx++)
    {
        const osg::PrimitiveSet* prim = geom.getPrimitiveSet( jdx );
        if( isMesh( prim->getMode() ) )
            return true;
    }
    // All PrimitiveSet modes will use Face records.
    return false;
}

void
FltExportVisitor::writeFace( const osg::Geode& geode, const osg::Geometry& geom, GLenum mode )
{
    enum DrawMode
    {
        SOLID_BACKFACE = 0,
        SOLID_NO_BACKFACE = 1,
        WIREFRAME_CLOSED = 2,
        WIREFRAME_NOT_CLOSED = 3,
        SURROUND_ALTERNATE_COLOR = 4,
        OMNIDIRECTIONAL_LIGHT = 8,
        UNIDIRECTIONAL_LIGHT = 9,
        BIDIRECTIONAL_LIGHT = 10
    };
    enum TemplateMode
    {
        FIXED_NO_ALPHA_BLENDING = 0,
        FIXED_ALPHA_BLENDING = 1,
        AXIAL_ROTATE_WITH_ALPHA_BLENDING = 2,
        POINT_ROTATE_WITH_ALPHA_BLENDING = 4
    };

    // const unsigned int TERRAIN_BIT      = 0x80000000u >> 0;
    // const unsigned int NO_COLOR_BIT     = 0x80000000u >> 1;
    // const unsigned int NO_ALT_COLOR_BIT = 0x80000000u >> 2;
    const unsigned int PACKED_COLOR_BIT = 0x80000000u >> 3;
    // const unsigned int FOOTPRINT_BIT    = 0x80000000u >> 4;    // Terrain culture cutout
    const unsigned int HIDDEN_BIT       = 0x80000000u >> 5;
    // const unsigned int ROOFLINE_BIT     = 0x80000000u >> 6;

    uint32 flags( PACKED_COLOR_BIT );
    if (geode.getNodeMask() == 0)
        flags |= HIDDEN_BIT;

    osg::StateSet const* ss = getCurrentStateSet();
    enum LightMode
    {
        FACE_COLOR = 0,
        VERTEX_COLOR = 1,
        FACE_COLOR_LIGHTING = 2,
        VERTEX_COLOR_LIGHTING = 3
    };
    int8 lightMode;
    osg::Vec4 packedColorRaw( 1., 1., 1., 1. );
    uint16 transparency( 0 );
    if (geom.getColorBinding() == osg::Geometry::BIND_PER_VERTEX)
    {
        if( isLit( geom ) )
            lightMode = VERTEX_COLOR_LIGHTING;
        else
            lightMode = VERTEX_COLOR;
    }
    else
    {
        const osg::Vec4Array* c = dynamic_cast<const osg::Vec4Array*>( geom.getColorArray() );
        if (c && (c->size() > 0))
        {
            packedColorRaw = (*c)[0];
            transparency = flt::uint16((1. - packedColorRaw[3]) * (double)0xffff);
        }

        if ( isLit( geom ) )
            lightMode = FACE_COLOR_LIGHTING;
        else
            lightMode = FACE_COLOR;
    }
    uint32 packedColor;
    packedColor = (int)(packedColorRaw[3]*255) << 24 |
        (int)(packedColorRaw[2]*255) << 16 | (int)(packedColorRaw[1]*255) << 8 |
        (int)(packedColorRaw[0]*255);


    int8 drawType = SOLID_NO_BACKFACE;

    switch( mode )
    {
    case GL_POINTS:
    {
        std::string warning( "fltexp: GL_POINTS not supported in FLT export." );
        OSG_WARN << warning << std::endl;
        _fltOpt->getWriteResult().warn( warning );
        return;
        break;
    }
    case GL_TRIANGLE_STRIP:
    case GL_TRIANGLE_FAN:
    case GL_QUAD_STRIP:
    {
        std::string warning( "fltexp: Wrong mode in Face record." );
        OSG_WARN << warning << std::endl;
        _fltOpt->getWriteResult().warn( warning );
        return;
        break;
    }
    case GL_LINES:
    case GL_LINE_STRIP:
        drawType = WIREFRAME_NOT_CLOSED;
        break;
    case GL_LINE_LOOP:
        drawType = WIREFRAME_CLOSED;
        break;
    case GL_TRIANGLES:
    case GL_QUADS:
    case GL_POLYGON:
    {
        // Default to no facet culling
        drawType = SOLID_NO_BACKFACE;

        // If facet-culling isn't *dis*abled, check whether the CullFace mode is BACK
        if (ss->getMode(GL_CULL_FACE) & osg::StateAttribute::ON)
        {
            osg::CullFace const* cullFace = static_cast<osg::CullFace const*>(
                ss->getAttribute(osg::StateAttribute::CULLFACE) );
            if( cullFace->getMode() == osg::CullFace::BACK )
                drawType = SOLID_BACKFACE;

            // Note: OpenFlt can't handle FRONT or FRONT_AND_BACK settings, so ignore these(??)
        }
        break;
    }
    }

    // Determine the material properties for the face
    int16 materialIndex( -1 );
    if (isLit( geom ))
    {
        osg::Material const* currMaterial = static_cast<osg::Material const*>(
            ss->getAttribute(osg::StateAttribute::MATERIAL) );
        materialIndex = _materialPalette->add(currMaterial);
    }

    // Get base texture
    int16 textureIndex( -1 );
    if (isTextured( 0, geom ))
    {
        const osg::Texture2D* texture = static_cast<const osg::Texture2D*>(
            ss->getTextureAttribute( 0, osg::StateAttribute::TEXTURE )  );
        if (texture != NULL)
            textureIndex = _texturePalette->add( 0, texture );
        else
        {
            std::string warning( "fltexp: Face is textured, but Texture2D StateAttribute is NULL." );
            OSG_WARN << warning << std::endl;
            _fltOpt->getWriteResult().warn( warning );
        }
    }

    // Set the appropriate template mode based
    // on blending or Billboarding.
    TemplateMode templateMode( FIXED_NO_ALPHA_BLENDING );
    const osg::Billboard* bb = dynamic_cast< const osg::Billboard* >( &geode );
    if (bb != NULL)
    {
        if( bb->getMode() == osg::Billboard::AXIAL_ROT )
            templateMode = AXIAL_ROTATE_WITH_ALPHA_BLENDING;
        else
            templateMode = POINT_ROTATE_WITH_ALPHA_BLENDING;
    }
    else if ( ss->getMode( GL_BLEND ) & osg::StateAttribute::ON )
    {
        const osg::BlendFunc* bf = static_cast<const osg::BlendFunc*>(
            ss->getAttribute(osg::StateAttribute::BLENDFUNC) );
        if( (bf->getSource() == osg::BlendFunc::SRC_ALPHA) &&
            (bf->getDestination() == osg::BlendFunc::ONE_MINUS_SRC_ALPHA) )
            templateMode = FIXED_ALPHA_BLENDING;
    }


    uint16 length( 80 );
    IdHelper id( *this, geode.getName() );

    _records->writeInt16( (int16) FACE_OP );
    _records->writeUInt16( length );
    _records->writeID( id );
    _records->writeInt32( 0 ); // IR color code
    _records->writeInt16( 0 ); // Relative priority
    _records->writeInt8( drawType ); // Draw type
    _records->writeInt8( 0 ); // Texture white
    _records->writeInt16( -1 ); // Color name index
    _records->writeInt16( -1 ); // Alternate color name index
    _records->writeInt8( 0 ); // Reserved
    _records->writeInt8( templateMode ); // Template (billboard)
    _records->writeInt16( -1 ); // Detail texture pattern index
    _records->writeInt16( textureIndex ); // Texture pattern index
    _records->writeInt16( materialIndex ); // Material index
    _records->writeInt16( 0 ); // Surface material code
    _records->writeInt16( 0 ); // Feature ID
    _records->writeInt32( 0 ); // IR material code
    _records->writeUInt16( transparency ); // Transparency
    _records->writeInt8( 0 ); // LOD generation control
    _records->writeInt8( 0 ); // Line style index
    _records->writeUInt32( flags ); // Flags
    _records->writeInt8( lightMode ); // Light mode
    _records->writeFill( 7 ); // Reserved
    _records->writeUInt32( packedColor ); // Packed color, primary
    _records->writeUInt32( 0x00ffffff ); // Packed color, alternate
    _records->writeInt16( -1 ); // Texture mapping index
    _records->writeInt16( 0 ); // Reserved
    _records->writeInt32( -1 ); // Primary color index
    _records->writeInt32( -1 ); // Alternate color index
    // Next four bytes:
    //   15.8: two 2-byte "reserved" fields
    //   15.9: one 4-byte "reserved" field;
    _records->writeInt16( 0 ); // Reserved
    _records->writeInt16( -1 ); // Shader index
}


void
FltExportVisitor::writeMesh( const osg::Geode& geode, const osg::Geometry& geom )
{
    enum DrawMode
    {
        SOLID_BACKFACE = 0,
        SOLID_NO_BACKFACE = 1,
        WIREFRAME_CLOSED = 2,
        WIREFRAME_NOT_CLOSED = 3,
        SURROUND_ALTERNATE_COLOR = 4,
        OMNIDIRECTIONAL_LIGHT = 8,
        UNIDIRECTIONAL_LIGHT = 9,
        BIDIRECTIONAL_LIGHT = 10
    };
    enum TemplateMode
    {
        FIXED_NO_ALPHA_BLENDING = 0,
        FIXED_ALPHA_BLENDING = 1,
        AXIAL_ROTATE_WITH_ALPHA_BLENDING = 2,
        POINT_ROTATE_WITH_ALPHA_BLENDING = 4
    };

    // const unsigned int TERRAIN_BIT      = 0x80000000u >> 0;
    //const unsigned int NO_COLOR_BIT     = 0x80000000u >> 1;
    //const unsigned int NO_ALT_COLOR_BIT = 0x80000000u >> 2;
    const unsigned int PACKED_COLOR_BIT = 0x80000000u >> 3;
    //const unsigned int FOOTPRINT_BIT    = 0x80000000u >> 4;    // Terrain culture cutout
    const unsigned int HIDDEN_BIT       = 0x80000000u >> 5;
    //const unsigned int ROOFLINE_BIT     = 0x80000000u >> 6;
    uint32 flags( PACKED_COLOR_BIT );
    if (geode.getNodeMask() == 0)
        flags |= HIDDEN_BIT;

    enum LightMode
    {
        FACE_COLOR = 0,
        VERTEX_COLOR = 1,
        FACE_COLOR_LIGHTING = 2,
        VERTEX_COLOR_LIGHTING = 3
    };
    int8 lightMode;
    osg::Vec4 packedColorRaw( 1., 1., 1., 1. );
    uint16 transparency( 0 );
    if (geom.getColorBinding() == osg::Geometry::BIND_PER_VERTEX)
    {
        if (isLit( geom ))
            lightMode = VERTEX_COLOR_LIGHTING;
        else
            lightMode = VERTEX_COLOR;
    }
    else
    {
        const osg::Vec4Array* c = dynamic_cast<const osg::Vec4Array*>( geom.getColorArray() );
        if (c && (c->size() > 0))
        {
            packedColorRaw = (*c)[0];
            transparency = flt::uint16((1. - packedColorRaw[3]) * (double)0xffff);
        }

        if (isLit( geom ))
            lightMode = FACE_COLOR_LIGHTING;
        else
            lightMode = FACE_COLOR;
    }
    uint32 packedColor;
    packedColor = (int)(packedColorRaw[3]*255) << 24 |
        (int)(packedColorRaw[2]*255) << 16 | (int)(packedColorRaw[1]*255) << 8 |
        (int)(packedColorRaw[0]*255);


    int8 drawType;
    osg::StateSet const* ss = getCurrentStateSet();

    {
        // Default to no facet culling
        drawType = SOLID_NO_BACKFACE;

        // If facet-culling isn't *dis*abled, check whether the CullFace mode is BACK
        if (ss->getMode(GL_CULL_FACE) & osg::StateAttribute::ON)
        {
            osg::CullFace const* cullFace = static_cast<osg::CullFace const*>(
                ss->getAttribute(osg::StateAttribute::CULLFACE) );
            if( cullFace->getMode() == osg::CullFace::BACK )
                drawType = SOLID_BACKFACE;

            // Note: OpenFlt can't handle FRONT or FRONT_AND_BACK settings, so ignore these(??)
        }
    }

    // Determine the material properties for the face
    int16 materialIndex( -1 );
    if (isLit( geom ))
    {
        osg::Material const* currMaterial = static_cast<osg::Material const*>(
            ss->getAttribute(osg::StateAttribute::MATERIAL) );
        materialIndex = _materialPalette->add(currMaterial);
    }

    // Get base texture
    int16 textureIndex( -1 );
    if (isTextured( 0, geom ))
    {
        const osg::Texture2D* texture = static_cast<const osg::Texture2D*>(
            ss->getTextureAttribute( 0, osg::StateAttribute::TEXTURE )  );
        if (texture != NULL)
            textureIndex = _texturePalette->add( 0, texture );
        else
        {
            std::string warning( "fltexp: Mesh is textured, but Texture2D StateAttribute is NULL." );
            OSG_WARN << warning << std::endl;
            _fltOpt->getWriteResult().warn( warning );
        }
    }

    // Set the appropriate template mode based
    // on blending or Billboarding.
    TemplateMode templateMode( FIXED_NO_ALPHA_BLENDING );
    const osg::Billboard* bb = dynamic_cast< const osg::Billboard* >( &geode );
    if (bb != NULL)
    {
        if( bb->getMode() == osg::Billboard::AXIAL_ROT )
            templateMode = AXIAL_ROTATE_WITH_ALPHA_BLENDING;
        else
            templateMode = POINT_ROTATE_WITH_ALPHA_BLENDING;
    }
    else if ( ss->getMode( GL_BLEND ) & osg::StateAttribute::ON )
    {
        const osg::BlendFunc* bf = static_cast<const osg::BlendFunc*>(
            ss->getAttribute(osg::StateAttribute::BLENDFUNC) );
        if( (bf->getSource() == osg::BlendFunc::SRC_ALPHA) &&
            (bf->getDestination() == osg::BlendFunc::ONE_MINUS_SRC_ALPHA) )
            templateMode = FIXED_ALPHA_BLENDING;
    }


    uint16 length( 84 );
    IdHelper id( *this, geode.getName() );

    _records->writeInt16( (int16) MESH_OP );
    _records->writeUInt16( length );
    _records->writeID( id );
    _records->writeInt32( 0 ); // Reserved
    _records->writeInt32( 0 ); // IR color code
    _records->writeInt16( 0 ); // Relative priority
    _records->writeInt8( drawType ); // Draw type
    _records->writeInt8( 0 ); // Texture white
    _records->writeInt16( -1 ); // Color name index
    _records->writeInt16( -1 ); // Alternate color name index
    _records->writeInt8( 0 ); // Reserved
    _records->writeInt8( templateMode ); // Template (billboard)
    _records->writeInt16( -1 ); // Detail texture pattern index
    _records->writeInt16( textureIndex ); // Texture pattern index
    _records->writeInt16( materialIndex ); // Material index
    _records->writeInt16( 0 ); // Surface material code
    _records->writeInt16( 0 ); // Feature ID
    _records->writeInt32( 0 ); // IR material code
    _records->writeUInt16( transparency ); // Transparency
    _records->writeInt8( 0 ); // LOD generation control
    _records->writeInt8( 0 ); // Line style index
    _records->writeUInt32( flags ); // Flags
    _records->writeInt8( lightMode ); // Light mode
    _records->writeFill( 7 ); // Reserved
    _records->writeUInt32( packedColor ); // Packed color, primary
    _records->writeUInt32( 0x00ffffff ); // Packed color, alternate
    _records->writeInt16( -1 ); // Texture mapping index
    _records->writeInt16( 0 ); // Reserved
    _records->writeInt32( -1 ); // Primary color index
    _records->writeInt32( -1 ); // Alternate color index
    // Next four bytes:
    //   15.8: two 2-byte "reserved" fields
    //   15.9: one 4-byte "reserved" field
    _records->writeInt16( 0 ); // Reserved
    _records->writeInt16( -1 ); // Shader index
}

int
FltExportVisitor::writeVertexList( int first, unsigned int count )
{
    _records->writeInt16( (int16) VERTEX_LIST_OP );
    _records->writeUInt16( 4 + (count*4) );

    unsigned int idx;
    for( idx=0; idx<count; idx++)
        // I'm imagining that 'first' will be a 0-based index into the 
        //   'current' set of vertices held by the vertex palette manager.
        _records->writeInt32( _vertexPalette->byteOffset( first+idx ) );

    return count;
}

int
FltExportVisitor::writeVertexList( const std::vector<unsigned int>& indices, unsigned int count )
{
    _records->writeInt16( (int16) VERTEX_LIST_OP );
    _records->writeUInt16( 4 + (count*4) );

    unsigned int idx;
    for( idx=0; idx<count; idx++)
        // I'm imagining that 'first' will be a 0-based index into the 
        //   'current' set of vertices held by the vertex palette manager.
        _records->writeInt32( _vertexPalette->byteOffset( indices[ idx ] ) );

    return count;
}

void
FltExportVisitor::writeMeshPrimitive( const std::vector<unsigned int>& indices, GLenum mode )
{
    int16 primType;
    switch( mode )
    {
    case GL_TRIANGLE_STRIP:
        primType = 1;
        break;
    case GL_TRIANGLE_FAN:
        primType = 2;
        break;
    case GL_QUAD_STRIP:
        primType = 3;
        break;
    default:
        // Warning should already be recorded. Do nothing.
        return;
        break;
    }

    uint16 length( 12 + (4 * indices.size()) );

    _records->writeInt16( (int16) MESH_PRIMITIVE_OP );
    _records->writeUInt16( length );
    _records->writeInt16( primType ); // primitive type
    _records->writeInt16( 4 ); // index size, 4 bytes
    _records->writeInt32( indices.size() ); // vertex count

    std::vector<unsigned int>::const_iterator it = indices.begin();
    while (it != indices.end())
    {
        _records->writeUInt32( (*it) );
        it++;
    }
}

void
FltExportVisitor::writeLocalVertexPool( const osg::Geometry& geom )
{
    // Attribute Mask
    static const unsigned int HAS_POSITION      = 0x80000000u >> 0;
    // static const unsigned int HAS_COLOR_INDEX   = 0x80000000u >> 1;
    static const unsigned int HAS_RGBA_COLOR    = 0x80000000u >> 2;
    static const unsigned int HAS_NORMAL        = 0x80000000u >> 3;
    static const unsigned int HAS_BASE_UV       = 0x80000000u >> 4;
    static const unsigned int HAS_UV_LAYER1     = 0x80000000u >> 5;
    static const unsigned int HAS_UV_LAYER2     = 0x80000000u >> 6;
    static const unsigned int HAS_UV_LAYER3     = 0x80000000u >> 7;
    static const unsigned int HAS_UV_LAYER4     = 0x80000000u >> 8;
    static const unsigned int HAS_UV_LAYER5     = 0x80000000u >> 9;
    static const unsigned int HAS_UV_LAYER6     = 0x80000000u >> 10;
    static const unsigned int HAS_UV_LAYER7     = 0x80000000u >> 11;

    const osg::Array* v = geom.getVertexArray();
    uint32 numVerts( v->getNumElements() );
    osg::ref_ptr< const osg::Vec3dArray > v3 = VertexPaletteManager::asVec3dArray( v, numVerts );
    if (!v3)
    {
        std::string warning( "fltexp: writeLocalVertexPool: VertexArray is not Vec3Array." );
        OSG_WARN << warning << std::endl;
        _fltOpt->getWriteResult().warn( warning );
        return;
    }

    // Compute attribute bits and vertex size.
    const osg::Array* c = geom.getColorArray();
    const osg::Array* n = geom.getNormalArray();
    const osg::Array* t = geom.getTexCoordArray( 0 );

    osg::ref_ptr< const osg::Vec4Array > c4 = VertexPaletteManager::asVec4Array( c, numVerts );
    osg::ref_ptr< const osg::Vec3Array > n3 = VertexPaletteManager::asVec3Array( n, numVerts );
    osg::ref_ptr< const osg::Vec2Array > t2 = VertexPaletteManager::asVec2Array( t, numVerts );
    if (c && !c4)
        return;
    if (n && !n3)
        return;
    if (t && !t2)
        return;

    std::vector< osg::ref_ptr< const osg::Vec2Array > > mtc;
    mtc.resize( 8 );
    int unit=1;
    for( ;unit<8; unit++)
        mtc[ unit ] = VertexPaletteManager::asVec2Array( geom.getTexCoordArray( unit ), numVerts );

    uint32 attr( HAS_POSITION );
    unsigned int vertSize( sizeof( float64 ) * 3 );

    if ( ( c4 != NULL ) && ( geom.getColorBinding() == osg::Geometry::BIND_PER_VERTEX) )
    {
        attr |= HAS_RGBA_COLOR;
        vertSize += sizeof( unsigned int );
    }
    if ( ( n3 != NULL ) && ( geom.getNormalBinding() == osg::Geometry::BIND_PER_VERTEX) )
    {
        attr |= HAS_NORMAL;
        vertSize += ( sizeof( float32 ) * 3 );
    }
    if ( t2 != NULL )
    {
        attr |= HAS_BASE_UV;
        vertSize += ( sizeof( float32 ) * 2 );
    }
    // Add multitex
    if (isTextured( 1, geom ))
    {
        attr |= HAS_UV_LAYER1;
        vertSize += ( sizeof( float32 ) * 2 );
    }
    if (isTextured( 2, geom ))
    {
        attr |= HAS_UV_LAYER2;
        vertSize += ( sizeof( float32 ) * 2 );
    }
    if (isTextured( 3, geom ))
    {
        attr |= HAS_UV_LAYER3;
        vertSize += ( sizeof( float32 ) * 2 );
    }
    if (isTextured( 4, geom ))
    {
        attr |= HAS_UV_LAYER4;
        vertSize += ( sizeof( float32 ) * 2 );
    }
    if (isTextured( 5, geom ))
    {
        attr |= HAS_UV_LAYER5;
        vertSize += ( sizeof( float32 ) * 2 );
    }
    if (isTextured( 6, geom ))
    {
        attr |= HAS_UV_LAYER6;
        vertSize += ( sizeof( float32 ) * 2 );
    }
    if (isTextured( 7, geom ))
    {
        attr |= HAS_UV_LAYER7;
        vertSize += ( sizeof( float32 ) * 2 );
    }

    unsigned int maxVerts = (0xffff - 12) / vertSize;
    unsigned int thisVertCount = (maxVerts > numVerts) ? numVerts : maxVerts;
    unsigned int currentIndexLimit = maxVerts;
    uint16 length( 12 + (vertSize * thisVertCount) );


    _records->writeInt16( (int16) LOCAL_VERTEX_POOL_OP );
    _records->writeUInt16( length );
    _records->writeUInt32( numVerts ); // number of vertices
    _records->writeUInt32( attr ); // attribute bits

    unsigned int idx;
    for( idx=0; idx<numVerts; idx++ )
    {
        _records->writeVec3d( (*v3)[ idx ] );

        if (attr & HAS_RGBA_COLOR)
        {
            osg::Vec4 color = (*c4)[ idx ];
            unsigned int packedColor = (int)(color[3]*255) << 24 |
                (int)(color[2]*255) << 16 | (int)(color[1]*255) << 8 |
                (int)(color[0]*255);
            _records->writeUInt32( packedColor );
        }

        if (attr & HAS_NORMAL)
            _records->writeVec3f( (*n3)[ idx ] );

        if (attr & HAS_BASE_UV)
            _records->writeVec2f( (*t2)[ idx ] );

        if (attr & HAS_UV_LAYER1)
            _records->writeVec2f( (*mtc[1])[ idx ] );
        if (attr & HAS_UV_LAYER2)
            _records->writeVec2f( (*mtc[2])[ idx ] );
        if (attr & HAS_UV_LAYER3)
            _records->writeVec2f( (*mtc[3])[ idx ] );
        if (attr & HAS_UV_LAYER4)
            _records->writeVec2f( (*mtc[4])[ idx ] );
        if (attr & HAS_UV_LAYER5)
            _records->writeVec2f( (*mtc[5])[ idx ] );
        if (attr & HAS_UV_LAYER6)
            _records->writeVec2f( (*mtc[6])[ idx ] );
        if (attr & HAS_UV_LAYER7)
            _records->writeVec2f( (*mtc[7])[ idx ] );


        // Handle continuation record if necessary.
        if ( (idx+1 == currentIndexLimit) && (idx+1 < numVerts) )
        {
            currentIndexLimit += maxVerts;
            unsigned int remaining( numVerts - (idx+1) );
            thisVertCount = (maxVerts > remaining) ? remaining : maxVerts;
            writeContinuationRecord( (vertSize * thisVertCount) );
        }
    }
}

void
FltExportVisitor::writeMultitexture( const osg::Geometry& geom )
{
    unsigned int numLayers( 0 );
    uint32 flags( 0 );
    unsigned int idx;
    for( idx=1; idx<8; idx++)
    {
        if( isTextured( idx, geom ) )
        {
            flags |= LAYER_1 >> (idx-1);
            numLayers++;
        }
    }
    if( numLayers == 0 )
        return;

    uint16 length( 8 + (8*numLayers) );

    _records->writeInt16( (int16) MULTITEXTURE_OP );
    _records->writeUInt16( length );
    _records->writeInt32( flags );

    const osg::StateSet* ss = getCurrentStateSet();
    for( idx=1; idx<8; idx++)
    {
        if( isTextured( idx, geom ) )
        {
            int16 textureIndex( -1 );
            const osg::Texture2D* texture = static_cast<const osg::Texture2D*>(
                ss->getTextureAttribute( idx, osg::StateAttribute::TEXTURE )  );
            if (texture != NULL)
                textureIndex = _texturePalette->add( idx, texture );
            else
            {
                std::ostringstream warning;
                warning << "fltexp: No Texture2D for unit " << idx;
                OSG_WARN << warning.str() << std::endl;
                _fltOpt->getWriteResult().warn( warning.str() );
            }

            // texture index (this value is an unsigned int, but has a -1 default per oflt spec: ugh)
            _records->writeUInt16( static_cast< uint16 >( textureIndex ) );
            _records->writeUInt16( 0 ); // TBD effect
             // mapping index (this value is an unsigned int, but has a -1 default per oflt spec: ugh)
            _records->writeUInt16( static_cast< uint16 >( -1 ) );
            _records->writeUInt16( 0 ); // data
        }
    }
}

void
FltExportVisitor::writeUVList( int numVerts, const osg::Geometry& geom, const std::vector<unsigned int>& indices )
{
    unsigned int numLayers( 0 );
    uint32 flags( 0 );
    unsigned int idx;
    for( idx=1; idx<8; idx++)
    {
        if( isTextured( idx, geom ) )
        {
            flags |= LAYER_1 >> (idx-1);
            numLayers++;
        }
    }
    if( numLayers == 0 )
        return;

    uint16 length( 8 + (8*numLayers*numVerts) );

    _records->writeInt16( (int16) UV_LIST_OP );
    _records->writeUInt16( length );
    _records->writeInt32( flags );

    osg::Vec2 defaultCoord( 0., 0. );
    // const osg::StateSet* ss = getCurrentStateSet();
    for( int vertexIdx=0; vertexIdx<numVerts; vertexIdx++)
    {
        for( idx=1; idx<8; idx++)
        {
            if( isTextured( idx, geom ) )
            {
                osg::Array* t = const_cast<osg::Array*>( geom.getTexCoordArray( idx ) );
                osg::ref_ptr<osg::Vec2Array> t2 = dynamic_cast<osg::Vec2Array*>( t );
                if (!t2.valid())
                {
                    std::ostringstream warning;
                    warning << "fltexp: No Texture2D for unit " << idx;
                    OSG_WARN << warning.str() << std::endl;
                    _fltOpt->getWriteResult().warn( warning.str() );
                    t2 = new osg::Vec2Array;
                }

                const int size = t2->getNumElements();
                int vIdx = indices[ vertexIdx ];
                osg::Vec2& tc( defaultCoord );
                if (vIdx < size)
                    tc = ( *t2 )[ vIdx ];
                _records->writeFloat32( tc[ 0 ] );
                _records->writeFloat32( tc[ 1 ] );
            }
        }
    }
}


void
FltExportVisitor::writeUVList( int numVerts, const osg::Geometry& geom, unsigned int first )
{
    unsigned int numLayers( 0 );
    uint32 flags( 0 );
    unsigned int idx;
    for( idx=1; idx<8; idx++)
    {
        if( isTextured( idx, geom ) )
        {
            flags |= LAYER_1 >> (idx-1);
            numLayers++;
        }
    }
    if( numLayers == 0 )
        return;

    uint16 length( 8 + (8*numLayers*numVerts) );

    _records->writeInt16( (int16) UV_LIST_OP );
    _records->writeUInt16( length );
    _records->writeInt32( flags );

    osg::Vec2 defaultCoord( 0., 0. );
    for( int vertexIdx=0; vertexIdx<numVerts; vertexIdx++)
    {
        for( idx=1; idx<8; idx++)
        {
            if( isTextured( idx, geom ) )
            {
                osg::Array* t = const_cast<osg::Array*>( geom.getTexCoordArray( idx ) );
                osg::ref_ptr<osg::Vec2Array> t2 = dynamic_cast<osg::Vec2Array*>( t );
                if (!t2.valid())
                {
                    std::ostringstream warning;
                    warning << "fltexp: No Texture2D for unit " << idx;
                    osg::notify( osg::WARN ) << warning.str() << std::endl;
                    _fltOpt->getWriteResult().warn( warning.str() );
                    t2 = new osg::Vec2Array;
                }
                else if (t2->getNumElements() < first + numVerts)
                {
                    std::ostringstream warning;
                    warning << "fltexp: Invalid number of texture coordinates for unit " << idx;
                    OSG_WARN << warning.str() << std::endl;
                    _fltOpt->getWriteResult().warn( warning.str() );
                }

                const int size = t2->getNumElements();
                int vIdx = first + vertexIdx;
                osg::Vec2& tc( defaultCoord );
                if (vIdx < size)
                    tc = (*t2)[ vIdx ];
                _records->writeFloat32( tc[0] );
                _records->writeFloat32( tc[1] );
            }
        }
    }
}



void
FltExportVisitor::handleDrawArrays( const osg::DrawArrays* da, const osg::Geometry& geom, const osg::Geode& geode )
{
    GLint first = da->getFirst();
    GLsizei count = da->getCount();
    GLenum mode = da->getMode();

    int n( 0 );
    bool useMesh( false );
    switch( mode )
    {
    case GL_TRIANGLE_STRIP:
    case GL_TRIANGLE_FAN:
    case GL_QUAD_STRIP:
        useMesh = true;
        break;
    case GL_POINTS:
        n = 1;
        break;
    case GL_LINES:
        n = 2;
        break;
    case GL_TRIANGLES:
        n = 3;
        break;
    case GL_QUADS:
        n = 4;
        break;
    case GL_LINE_STRIP:
    case GL_LINE_LOOP:
    case GL_POLYGON:
    default:
        n = count;
        break;
    }

    if (useMesh)
    {
        std::vector< unsigned int > indices;
        int jdx;
        for (jdx=0; jdx<count; jdx++)
            indices.push_back( first+jdx );
        writeMeshPrimitive( indices, mode );
    }
    else
    {
        const unsigned int max( first+count );
        while ((unsigned int)( first+n ) <= max)
        {
            writeFace( geode, geom, mode );

            writeMatrix( geode.getUserData() );
            writeComment( geode );
            writeMultitexture( geom );
            writePush();

            // Write vertex list records.
            int numVerts = writeVertexList( first, n );
            first += n;

            writeUVList( numVerts, geom );

            writePop();
        }
    }
}

void
FltExportVisitor::handleDrawArrayLengths( const osg::DrawArrayLengths* dal, const osg::Geometry& geom, const osg::Geode& geode )
{
    GLint first = dal->getFirst();
    GLenum mode = dal->getMode();

    int n( 0 );
    bool useMesh( false );
    switch( mode )
    {
    case GL_TRIANGLE_STRIP:
    case GL_TRIANGLE_FAN:
    case GL_QUAD_STRIP:
        useMesh = true;
        break;
    case GL_POINTS:
        n = 1;
        break;
    case GL_LINES:
        n = 2;
        break;
    case GL_TRIANGLES:
        n = 3;
        break;
    case GL_QUADS:
        n = 4;
        break;
    case GL_LINE_STRIP:
    case GL_LINE_LOOP:
    case GL_POLYGON:
    default:
        break;
    }

    // Push and pop subfaces if polygon offset is on.
    SubfaceHelper subface( *this, getCurrentStateSet() );

    if (useMesh)
    {
        int idx( 0 );
        for( osg::DrawArrayLengths::const_iterator itr=dal->begin();
             itr!=dal->end(); itr++ )
        {
            std::vector< unsigned int > indices;
            int jdx;
            for (jdx=0; jdx<(*itr); idx++, jdx++)
                indices.push_back( idx );
            writeMeshPrimitive( indices, mode );
        }
    }
    else
    {
        // Hm. You wouldn't usually use DrawArrayLengths for non-strip/fan prims...
        for( osg::DrawArrayLengths::const_iterator itr=dal->begin();
             itr!=dal->end(); itr++ )
        {
            while (first+n <= *itr)
            {
                writeFace( geode, geom, mode );

                writeMatrix( geode.getUserData() );
                writeComment( geode );
                writeMultitexture( geom );
                writePush();

                // Write vertex list records.
                int numVerts;
                if (n == 0)
                {
                    numVerts = writeVertexList( first, *itr );
                    first += *itr;
                }
                else
                {
                    numVerts = writeVertexList( first, n );
                    first += n;
                }

                writeUVList( numVerts, geom );

                writePop();
            }

            first += *itr;
        }
    }
}

void
FltExportVisitor::handleDrawElements( const osg::DrawElements* de, const osg::Geometry& geom, const osg::Geode& geode )
{
    GLenum mode = de->getMode();

    int n( 0 );
    bool useMesh( false );
    switch( mode )
    {
    case GL_TRIANGLE_STRIP:
    case GL_TRIANGLE_FAN:
    case GL_QUAD_STRIP:
        n = de->getNumIndices();
        useMesh = true;
        break;
    case GL_POINTS:
        n = 1;
        break;
    case GL_LINES:
        n = 2;
        break;
    case GL_TRIANGLES:
        n = 3;
        break;
    case GL_QUADS:
        n = 4;
        break;
    case GL_LINE_STRIP:
    case GL_LINE_LOOP:
    case GL_POLYGON:
    default:
        n = de->getNumIndices();
        break;
    }

    // Push and pop subfaces if polygon offset is on.
    SubfaceHelper subface( *this, getCurrentStateSet() );

    if (useMesh)
    {
        std::vector< unsigned int > indices;
        int idx;
        for (idx=0; idx<n; idx++)
            indices.push_back( de->index( idx ) );
        writeMeshPrimitive( indices, mode );
    }
    else
    {
        unsigned int first( 0 );
        while (first+n <= de->getNumIndices())
        {
            // Need:
            // * Geode for record name (but also need to handle
            //   multi Geometry objects and multi PrimitiveSet objects;
            //   all Face records can't have the same name).
            // * Mode
            writeFace( geode, geom, mode );

            writeMatrix( geode.getUserData() );
            writeComment( geode );
            writeMultitexture( geom );
            writePush();

            // Write vertex list records.
            std::vector<unsigned int> indices;
            int idx;
            for(idx=0; idx<n; idx++)
                indices.push_back( de->index( first+idx ) );
            int numVerts = writeVertexList( indices, n );
            first += n;

            writeUVList( numVerts, geom, indices );

            writePop();
        }
    }
}


}
