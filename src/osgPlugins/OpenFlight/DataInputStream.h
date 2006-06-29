//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#ifndef FLT_DATAINPUTSTREAM
#define FLT_DATAINPUTSTREAM 1

//#include <iostream>        // for ifstream
#include <istream>
#include <string>
#include <osg/Vec3f>
#include <osg/Vec3d>
#include <osg/Vec4f>
#include "types.h"    

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
        osg::Vec3d readVec3d();

        std::istream& forward(std::istream::off_type off);

        int16 peekInt16();

    protected:

        virtual std::istream& vread(char_type *str, std::streamsize count);
        virtual std::istream& vforward(std::istream::off_type off);

        bool                _byteswap;
};

} // end namespace

#endif

