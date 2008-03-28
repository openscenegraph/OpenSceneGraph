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

#include "VertexPaletteManager.h"
#include "DataOutputStream.h"
#include "Opcodes.h"
#include "Utils.h"
#include <osg/Notify>
#include <osg/Geometry>
#include <map>


namespace flt
{


VertexPaletteManager::VertexPaletteManager( const ExportOptions& fltOpt )
  : _fltOpt( fltOpt ),
    _currentSizeBytes( 8 ),
    _current( NULL ),
    _vertices( NULL )
{
}

VertexPaletteManager::~VertexPaletteManager()
{
    if (!_verticesTempName.empty())
    {
        // Delete our temp file.
        if (_verticesStr.is_open())
        {
            osg::notify( osg::WARN ) << "fltexp: VertexPaletteManager destructor has an open temp file." << std::endl;
            // This should not happen. FltExportVisitor::complete should close
            // this file before we get to this destructor.
            return;
        }
        osg::notify( osg::INFO ) << "fltexp: Deleting temp file " << _verticesTempName << std::endl;    
        FLTEXP_DELETEFILE( _verticesTempName.c_str() );
    }
}

void
VertexPaletteManager::add( const osg::Geometry& geom )
{
    const osg::Array* v = geom.getVertexArray();
    const osg::Array* c = geom.getColorArray();
    const osg::Array* n = geom.getNormalArray();
    const osg::Array* t = geom.getTexCoordArray( 0 );
    // TBD eventually need to be able to support other array types.
    const osg::Vec3Array* v3 = dynamic_cast<const osg::Vec3Array*>( v );
    const osg::Vec4Array* c4 = dynamic_cast<const osg::Vec4Array*>( c );
    const osg::Vec3Array* n3 = dynamic_cast<const osg::Vec3Array*>( n );
    const osg::Vec2Array* t2 = dynamic_cast<const osg::Vec2Array*>( t );
    if (v && !v3)
    {
        std::string warning( "fltexp: VertexPalette: VertexArray is not Vec3Array." );
        osg::notify( osg::WARN ) << warning << std::endl;
        _fltOpt.getWriteResult().warn( warning );
        return;
    }
    if (c && !c4)
    {
        std::string warning( "fltexp: VertexPalette: ColorArray is not Vec4Array." );
        osg::notify( osg::WARN ) << warning << std::endl;
        _fltOpt.getWriteResult().warn( warning );
        return;
    }
    if (n && !n3)
    {
        std::string warning( "fltexp: VertexPalette: NormalArray is not Vec3Array." );
        osg::notify( osg::WARN ) << warning << std::endl;
        _fltOpt.getWriteResult().warn( warning );
        return;
    }
    if (t && !t2)
    {
        std::string warning( "fltexp: VertexPalette: TexCoordArray is not Vec2Array." );
        osg::notify( osg::WARN ) << warning << std::endl;
        _fltOpt.getWriteResult().warn( warning );
        return;
    }

    const bool cpv =( geom.getColorBinding() == osg::Geometry::BIND_PER_VERTEX );
    const bool npv =( geom.getNormalBinding() == osg::Geometry::BIND_PER_VERTEX );
    add( v3, c4, n3, t2, cpv, npv );
}
void
VertexPaletteManager::add( const osg::Vec3Array* v, const osg::Vec4Array* c,
    const osg::Vec3Array* n, const osg::Vec2Array* t,
    bool colorPerVertex, bool normalPerVertex, bool allowSharing )
{
    bool needsInit( true );

    if (allowSharing)
    {
        ArrayMap::iterator it = _arrayMap.find( v );
        if (it != _arrayMap.end())
            needsInit = false;
        _current = &( _arrayMap[ v ] );
    }
    else
        _current = &( _nonShared );

    if (needsInit)
    {
        _current->_byteStart = _currentSizeBytes;

        _current->_idxCount = v->size();

        _current->_idxSizeBytes = recordSize( recordType( v, c, n, t ) );
        _currentSizeBytes += ( _current->_idxSizeBytes * _current->_idxCount );

        // Next we'll write the vertex palette record data. But,
        //   if we don't have a DataOutputStream yet, open the temp file.
        if (!_vertices)
        {
            _verticesTempName = _fltOpt.getTempDir() + "/ofw_temp_vertices";
            _verticesStr.open( _verticesTempName.c_str(), std::ios::out | std::ios::binary );
            _vertices = new DataOutputStream( _verticesStr.rdbuf(), _fltOpt.getValidateOnly() );
        }
        writeRecords( v, c, n, t, colorPerVertex, normalPerVertex );
    }
}

unsigned int
VertexPaletteManager::byteOffset( unsigned int idx ) const
{
    if (!_current)
    {
        osg::notify( osg::WARN ) << "fltexp: No current vertex array in VertexPaletteManager." << std::endl;
        return 4;
    }
    if (idx >= _current->_idxCount)
    {
        osg::notify( osg::WARN ) << "fltexp: Index out of range in VertexPaletteManager." << std::endl;
        return 4;
    }

    return( _current->_byteStart + (_current->_idxSizeBytes * idx) );
}

void
VertexPaletteManager::write( DataOutputStream& dos ) const
{
    if (_currentSizeBytes == 8)
        // Empty palette. Don't write anything.
        return;

    dos.writeInt16( (int16) VERTEX_PALETTE_OP );
    dos.writeUInt16( 8 );
    dos.writeInt32( _currentSizeBytes );

    // Close the temp file. We're done writing new data to it.
    _verticesStr.close();

    // Open that temp file again, this time for reading.
    //   Then copy to dos.
    char buf;
    std::ifstream vertIn;
    vertIn.open( _verticesTempName.c_str(), std::ios::in | std::ios::binary );
    while (!vertIn.eof() )
    {
        vertIn.read( &buf, 1 );
        if (vertIn.good())
            dos << buf;
    }
    vertIn.close();
}



VertexPaletteManager::PaletteRecordType
VertexPaletteManager::recordType( const osg::Array* v, const osg::Array* c,
    const osg::Array* n, const osg::Array* t )
{
    if (t)
    {
        // Texture coordinstes
        if (n)
            return VERTEX_CNT;
        else
            return VERTEX_CT;
    }
    else
    {
        // No textue coordinates
        if (n)
            return VERTEX_CN;
        else
            return VERTEX_C;
    }
}

unsigned int
VertexPaletteManager::recordSize( PaletteRecordType recType )
{
    switch (recType)
    {
    case VERTEX_C:
        return 40;
        break;
    case VERTEX_CN:
        return (_fltOpt.getFlightFileVersionNumber() > ExportOptions::VERSION_15_7) ? 56 : 52;
        break;
    case VERTEX_CT:
        return 48;
        break;
    case VERTEX_CNT:
        return 64;
        break;
    default:
        return 0;
    }
}

void
VertexPaletteManager::writeRecords( const osg::Vec3Array* v, const osg::Vec4Array* c,
    const osg::Vec3Array* n, const osg::Vec2Array* t,
    bool colorPerVertex, bool normalPerVertex )
{
    const PaletteRecordType recType = recordType( v, c, n, t );
    const int16 sizeBytes = recordSize( recType );

    int16 opcode;
    switch( recType )
    {
    case VERTEX_C:
        opcode = VERTEX_C_OP;
        break;
    case VERTEX_CN:
        opcode = VERTEX_CN_OP;
        if (!n)
            osg::notify( osg::WARN ) << "fltexp: VPM::writeRecords: no normal array." << std::endl;
        break;
    case VERTEX_CNT:
        opcode = VERTEX_CNT_OP;
        if (!n)
            osg::notify( osg::WARN ) << "fltexp: VPM::writeRecords: no normal array." << std::endl;
        if (!t)
            osg::notify( osg::WARN ) << "fltexp: VPM::writeRecords: no tex coord array." << std::endl;
        break;
    case VERTEX_CT:
        opcode = VERTEX_CT_OP;
        if (!t)
            osg::notify( osg::WARN ) << "fltexp: VPM::writeRecords: no tex coord array." << std::endl;
        break;
    }

    enum FlagBits
    {
        START_HARD_EDGE = (0x8000 >> 0),
        NORMAL_FROZEN   = (0x8000 >> 1),
        NO_COLOR        = (0x8000 >> 2),
        PACKED_COLOR    = (0x8000 >> 3)
    };
    uint32 flags( NO_COLOR );
    if (colorPerVertex)
        flags = PACKED_COLOR;


    int cIdx( 0 );
    int nIdx( 0 );
    size_t idx;
    for( idx=0; idx<v->size(); idx++)
    {
        uint32  packedColor( 0 );
        if (c)
        {
            osg::Vec4 color = (*c)[ cIdx ];
            packedColor = (int)(color[3]*255) << 24 |
                (int)(color[2]*255) << 16 | (int)(color[1]*255) << 8 |
                (int)(color[0]*255);
        }
        else
            osg::notify( osg::DEBUG_INFO ) << "fltexp: VPM::writeRecords: no color array." << std::endl;

        // Write fields common to all record types.
        _vertices->writeInt16( opcode );
        _vertices->writeUInt16( sizeBytes );
        _vertices->writeUInt16( 0 ); // Color name
        _vertices->writeInt16( flags ); // Flags
        _vertices->writeVec3d( osg::Vec3d( (*v)[ idx ] ) ); // Vertex

        // Now write record-specific field.
        switch( recType )
        {
        case VERTEX_C:
            _vertices->writeInt32( packedColor ); // Packed color
            _vertices->writeUInt32( 0 ); // Vertex color index
            break;
        case VERTEX_CN:
            _vertices->writeVec3f( (*n)[ nIdx ] ); // Normal
            _vertices->writeInt32( packedColor ); // Packed color
            _vertices->writeUInt32( 0 ); // Vertex color index
            if (_fltOpt.getFlightFileVersionNumber() > ExportOptions::VERSION_15_7)
                _vertices->writeUInt32( 0 ); // Reserved
            break;
        case VERTEX_CNT:
            _vertices->writeVec3f( (*n)[ nIdx ] ); // Normal
            _vertices->writeVec2f( (*t)[ idx ] ); // Tex coord
            _vertices->writeInt32( packedColor ); // Packed color
            _vertices->writeUInt32( 0 ); // Vertex color index
            _vertices->writeUInt32( 0 ); // Reserved
            break;
        case VERTEX_CT:
            _vertices->writeVec2f( (*t)[ idx ] ); // Tex coord
            _vertices->writeInt32( packedColor ); // Packed color
            _vertices->writeUInt32( 0 ); // Vertex color index
            break;
        }

        if (colorPerVertex)
            cIdx++;
        if (normalPerVertex)
            nIdx++;
    }
}



VertexPaletteManager::ArrayInfo::ArrayInfo()
  : _byteStart( 0 ),
    _idxSizeBytes( 0 ),
    _idxCount( 0 )
{
}


}
