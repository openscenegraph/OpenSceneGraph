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

#ifndef __FLTEXP_DATA_OUTPUT_STREAM_H__
#define __FLTEXP_DATA_OUTPUT_STREAM_H__ 1


#include <ostream>
#include <string>
#include <osg/Vec3f>
#include <osg/Vec3d>
#include <osg/Vec4f>
#include "Types.h"
//#include "Export.h"


namespace flt {


class Record;

class DataOutputStream : public std::ostream
{
public:
    explicit DataOutputStream( std::streambuf* sb, bool validate=false );

    void writeInt8( const int8 val );
    void writeUInt8( const uint8 val );
    void writeInt16( const int16 val );
    void writeUInt16( const uint16 val );
    void writeInt32( const int32 val );
    void writeUInt32( const uint32 val );
    void writeFloat32( const float32 val );
    void writeFloat64( const float64 val );

    // Write the entire string. If nullTerminate is true, write an additional NULL.
    // Always writes either 'val.size()' bytes or 'val.size()+1' bytes.
    void writeString( const std::string& val, bool nullTerminate=true );

    // Never write more than 'size-1' bytes from 'val', and write 'fill' so that 'size' bytes total are written.
    // Always writes 'size' bytes..
    void writeString( const std::string& val, int size, char fill='\0' );

    void writeID( const std::string& val );
    void writeVec2f( const osg::Vec2f& val );
    void writeVec3f( const osg::Vec3f& val );
    void writeVec4f( const osg::Vec4f& val );
    void writeVec3d( const osg::Vec3d& val );

    void writeFill( int sizeBytes, const char val='\0' );

protected:
    virtual std::ostream& vwrite( char_type* str, std::streamsize count );

    bool _byteswap;
    bool _validate;

    static char _null;
};

}

#endif

