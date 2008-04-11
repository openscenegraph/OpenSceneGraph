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

#include "DataOutputStream.h"

#include <osg/Endian>
#include <osg/Notify>


using namespace flt;


char DataOutputStream::_null( 0 );


DataOutputStream::DataOutputStream( std::streambuf* sb, bool validate )
  : std::ostream( sb ),
    _validate( validate )
{
    _byteswap = osg::getCpuByteOrder() == osg::LittleEndian;
}


void
DataOutputStream::writeInt8( const int8 val )
{
    vwrite( (char*)&val, sizeof( int8 ) );
}

void
DataOutputStream::writeUInt8( const uint8 val )
{
    vwrite( (char*)&val, sizeof( uint8 ) );
}


void
DataOutputStream::writeInt16( const int16 val )
{
    int16 data=val;
    if (_byteswap && good())
        osg::swapBytes2( (char*)&data );
    vwrite( (char*)&data, sizeof( int16 ) );
}

void
DataOutputStream::writeUInt16( const uint16 val )
{
    uint16 data=val;
    if (_byteswap && good())
        osg::swapBytes2( (char*)&data );
    vwrite( (char*)&data, sizeof( uint16 ) );
}


void
DataOutputStream::writeInt32( const int32 val )
{
    int32 data=val;
    if (_byteswap && good())
        osg::swapBytes4( (char*)&data );
    vwrite( (char*)&data, sizeof( int32 ) );
}

void
DataOutputStream::writeUInt32( const uint32 val )
{
    uint32 data=val;
    if (_byteswap && good())
        osg::swapBytes4( (char*)&data );
    vwrite( (char*)&data, sizeof( uint32 ) );
}


void
DataOutputStream::writeFloat32( const float32 val )
{
    float32 data=val;
    if (_byteswap && good())
        osg::swapBytes4( (char*)&data );
    vwrite( (char*)&data, sizeof( float32 ) );
}

void
DataOutputStream::writeFloat64( const float64 val )
{
    float64 data=val;
    if (_byteswap && good())
        osg::swapBytes8( (char*)&data );
    vwrite( (char*)&data, sizeof( float64 ) );
}


void
DataOutputStream::writeString( const std::string& val, bool nullTerminate )
{
    vwrite( const_cast<char*>( val.c_str() ), val.size() );
    if (nullTerminate)
        vwrite( &_null, 1 );
}

void
DataOutputStream::writeString( const std::string& val, int size, char fill )
{
    if (val.size() > ((unsigned int)size)-1)
    {
        vwrite( const_cast<char*>( val.c_str() ), size-1 );
        vwrite( &fill, 1 );
    }
    else
    {
        vwrite( const_cast<char*>( val.c_str() ), val.size() );
        writeFill( size - val.size(), fill );
    }
}

void
DataOutputStream::writeID( const std::string& val )
{
    unsigned int len = val.size();

    vwrite( const_cast<char*>( val.c_str() ), len );

    while (len++ < 8)
        vwrite( &_null, 1 );
}


void
DataOutputStream::writeVec2f( const osg::Vec2f& val )
{
    writeFloat32( val.x() );
    writeFloat32( val.y() );
}

void
DataOutputStream::writeVec3f( const osg::Vec3f& val )
{
    writeFloat32( val.x() );
    writeFloat32( val.y() );
    writeFloat32( val.z() );
}

void
DataOutputStream::writeVec4f( const osg::Vec4f& val )
{
    writeFloat32( val.x() );
    writeFloat32( val.y() );
    writeFloat32( val.z() );
    writeFloat32( val.w() );
}

void
DataOutputStream::writeVec3d( const osg::Vec3d& val )
{
    writeFloat64( val.x() );
    writeFloat64( val.y() );
    writeFloat64( val.z() );
}


void
DataOutputStream::writeFill( int sizeBytes, const char val )
{
    for (int i = 0; i < sizeBytes; ++i)
    {
        put(val);
    }
}


std::ostream&
DataOutputStream::vwrite( char_type* str, std::streamsize count )
{
    if (_validate)
        return *this;
    else
        return write( str, count );
}

