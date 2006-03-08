//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include "DataInputStream.h"

#include <osg/Endian>
#include <osg/Notify>
#include <osgDB/ReadFile>

using namespace flt;


DataInputStream::DataInputStream(std::istream* istream):
    _istream(istream)
{
    _byteswap = osg::getCpuByteOrder() == osg::LittleEndian;

    if (!istream)
        throw std::string("DataInputStream::DataInputStream(): null pointer exception in argument.");
}


DataInputStream::~DataInputStream()
{
}


int8 DataInputStream::readInt8(int8 def) const
{
    int8 d=def;
    read((char*)&d, sizeof(int8));
    return d;
}


uint8 DataInputStream::readUInt8(uint8 def) const
{
    uint8 d=def;
    read((char*)&d, sizeof(uint8));
    return d;
}


int16 DataInputStream::readInt16(int16 def) const
{
    int16 d=def;
    read((char*)&d, sizeof(int16));
    if (_byteswap && !_istream->fail())
        osg::swapBytes2((char *)&d);
    return d;
}


uint16 DataInputStream::readUInt16(uint16 def) const
{
    uint16 d=def;
    read((char*)&d, sizeof(uint16));
    if (_byteswap && !_istream->fail())
        osg::swapBytes2((char *)&d);
    return d;
}


int32 DataInputStream::readInt32(int32 def) const
{
    int32 d=def;
    read((char*)&d, sizeof(int32));
    if (_byteswap && !_istream->fail())
        osg::swapBytes4((char *)&d);
    return d;
}


uint32 DataInputStream::readUInt32(uint32 def) const
{
    uint32 d=def;
    read((char*)&d, sizeof(uint32));
    if (_byteswap && !_istream->fail())
        osg::swapBytes4((char *)&d);
    return d;
}


float32 DataInputStream::readFloat32(float32 def) const
{
    float32 d=def;
    char buf[sizeof(float32)];
    read(buf, sizeof(float32));
    if (_byteswap && !_istream->fail())
    {
        osg::swapBytes4(buf);
        memcpy(&d,buf,sizeof(float32));
    }
    return d;
}


float64 DataInputStream::readFloat64(float64 def) const
{
    float64 d=def;
    char buf[sizeof(float64)];
    read(buf, sizeof(float64));
    if (_byteswap && !_istream->fail())
    {
        osg::swapBytes8(buf);
        memcpy(&d,buf,sizeof(float64));
    }
    return d;
}


void DataInputStream::readCharArray(char* data, int size) const
{
    read(data, size);
}


std::string DataInputStream::readString(int size) const
{
    char* buf = new char[size+1];
    read(buf,size);
    buf[size] = '\0';
    std::string str = buf;
    delete [] buf;
    return str;
}


osg::Vec4f DataInputStream::readColor32() const
{
    uint8 alpha = readUInt8();
    uint8 blue  = readUInt8();
    uint8 green = readUInt8();
    uint8 red   = readUInt8();

    osg::Vec4f color((float)red/255,(float)green/255,(float)blue/255,1);

    return color;
}


osg::Vec2f DataInputStream::readVec2f() const
{
    float32 x = readFloat32();
    float32 y = readFloat32();

    osg::Vec2f vec(x,y);

    return vec;
}


osg::Vec3f DataInputStream::readVec3f() const
{
    float32 x = readFloat32();
    float32 y = readFloat32();
    float32 z = readFloat32();

    osg::Vec3f vec(x,y,z);

    return vec;
}


osg::Vec3d DataInputStream::readVec3d() const
{
    float64 x = readFloat64();
    float64 y = readFloat64();
    float64 z = readFloat64();

    osg::Vec3d vec(x,y,z);

    return vec;
}


int16 DataInputStream::peekInt16() const
{
    // Get current read position in stream.
    std::istream::pos_type pos = _istream->tellg();

    int16 value = readInt16();

    // Restore position
    _istream->seekg(pos, std::ios_base::beg);

    return value;
}


void DataInputStream::forward(std::istream::off_type _Off) const
{
    seekg(_Off, std::ios_base::cur);
}


std::istream& DataInputStream::read(std::istream::char_type *_Str, std::streamsize _Count) const
{
    return _istream->read(_Str, _Count);
}


std::istream& DataInputStream::seekg(std::istream::off_type _Off, std::ios_base::seekdir _Way) const
{
    return _istream->seekg(_Off, _Way);
}

