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

#ifndef FLT_DATAINPUTSTREAM
#define FLT_DATAINPUTSTREAM 1

#include <istream>
#include <string>
#include <osg/Vec3f>
#include <osg/Vec3d>
#include <osg/Vec4f>
#include "Types.h"

namespace flt {

class Record;

class DataInputStream : public std::istream
{
    public:

        explicit DataInputStream(std::streambuf* sb);

        int8 readInt8(int8 def=0);
        uint8 readUInt8(uint8 def=0);
        int16 readInt16(int16 def=0);
        uint16 readUInt16(uint16 def=0);
        int32 readInt32(int32 def=0);
        uint32 readUInt32(uint32 def=0);
        float32 readFloat32(float32 def=0);
        float64 readFloat64(float64 def=0);
        void readCharArray(char* data, int size);
        std::string readString(int size);
        osg::Vec4f readColor32();
        osg::Vec2f readVec2f();
        osg::Vec3f readVec3f();
        osg::Vec4f readVec4f();
        osg::Vec3d readVec3d();

        std::istream& forward(std::istream::off_type off);

        int16 peekInt16();

    protected:

        bool                _byteswap;
};

} // end namespace

#endif

