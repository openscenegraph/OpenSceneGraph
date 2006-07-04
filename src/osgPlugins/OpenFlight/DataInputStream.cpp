//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include "DataInputStream.h"

#include <osg/Endian>
#include <osg/Notify>

using namespace flt;


DataInputStream::DataInputStream(std::streambuf* sb):
    std::istream(sb)
{
    _byteswap = osg::getCpuByteOrder() == osg::LittleEndian;
}

int8 DataInputStream::readInt8(int8 def)
{
    int8 d=def;
    vread((char*)&d, sizeof(int8));
    return d;
}


uint8 DataInputStream::readUInt8(uint8 def)
{
    uint8 d=def;
    vread((char*)&d, sizeof(uint8));
    return d;
}


int16 DataInputStream::readInt16(int16 def)
{
    int16 d=def;
    vread((char*)&d, sizeof(int16));
    if (_byteswap && good())
        osg::swapBytes2((char *)&d);
    return d;
}


uint16 DataInputStream::readUInt16(uint16 def)
{
    uint16 d=def;
    vread((char*)&d, sizeof(uint16));
    if (_byteswap && good())
        osg::swapBytes2((char *)&d);
    return d;
}


int32 DataInputStream::readInt32(int32 def)
{
    int32 d=def;
    vread((char*)&d, sizeof(int32));
    if (_byteswap && good())
        osg::swapBytes4((char *)&d);
    return d;
}


uint32 DataInputStream::readUInt32(uint32 def)
{
    uint32 d=def;
    vread((char*)&d, sizeof(uint32));
    if (_byteswap && good())
        osg::swapBytes4((char *)&d);
    return d;
}


float32 DataInputStream::readFloat32(float32 def)
{
    float32 d=def;
    char buf[sizeof(float32)];
    vread(buf, sizeof(float32));
    if (_byteswap && good())
    {
        osg::swapBytes4(buf);
        memcpy(&d,buf,sizeof(float32));
    }
    return d;
}


float64 DataInputStream::readFloat64(float64 def)
{
    float64 d=def;
    char buf[sizeof(float64)];
    vread(buf, sizeof(float64));
    if (_byteswap && good())
    {
        osg::swapBytes8(buf);
        memcpy(&d,buf,sizeof(float64));
    }
    return d;
}


void DataInputStream::readCharArray(char* data, int size)
{
    vread(data,size);
}


std::string DataInputStream::readString(int size)
{
    char* buf = new char[size+1];
    vread(buf,size);
    buf[size] = '\0';
    std::string str = buf;
    delete [] buf;
    return str;
}


osg::Vec4f DataInputStream::readColor32()
{
    uint8 alpha = readUInt8();
    uint8 blue  = readUInt8();
    uint8 green = readUInt8();
    uint8 red   = readUInt8();

    osg::Vec4f color((float)red/255,(float)green/255,(float)blue/255,(float)alpha/255);

    return color;
}


osg::Vec2f DataInputStream::readVec2f()
{
    float32 x = readFloat32();
    float32 y = readFloat32();

    osg::Vec2f vec(x,y);

    return vec;
}


osg::Vec3f DataInputStream::readVec3f()
{
    float32 x = readFloat32();
    float32 y = readFloat32();
    float32 z = readFloat32();

    osg::Vec3f vec(x,y,z);

    return vec;
}


osg::Vec3d DataInputStream::readVec3d()
{
    float64 x = readFloat64();
    float64 y = readFloat64();
    float64 z = readFloat64();

    osg::Vec3d vec(x,y,z);

    return vec;
}


int16 DataInputStream::peekInt16()
{
    // Get current read position in stream.
    std::istream::pos_type pos = tellg();

    int16 value = readInt16();

    // Restore position
    seekg(pos, std::ios_base::beg);

    return value;
}


std::istream& DataInputStream::forward(std::istream::off_type off)
{
    return vforward(off);
}


std::istream& DataInputStream::vread(char_type *str, std::streamsize count)
{
    return read(str,count);
}


std::istream& DataInputStream::vforward(std::istream::off_type off)
{
    return seekg(off, std::ios_base::cur);
}

