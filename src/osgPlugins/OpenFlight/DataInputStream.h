//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#ifndef FLT_DATAINPUTSTREAM
#define FLT_DATAINPUTSTREAM 1

#include <iostream>		// for ifstream
#include <string>
#include <osg/Vec3f>
#include <osg/Vec3d>
#include <osg/Vec4f>
#include "types.h"	

namespace flt {

class Record;

class DataInputStream
{
public:

	DataInputStream(std::istream* istream);
	virtual ~DataInputStream();

	int8 readInt8(int8 def=0) const;
	uint8 readUInt8(uint8 def=0) const;
	int16 readInt16(int16 def=0) const;
	uint16 readUInt16(uint16 def=0) const;
	int32 readInt32(int32 def=0) const;
	uint32 readUInt32(uint32 def=0) const;
	float32 readFloat32(float32 def=0) const;
	float64 readFloat64(float64 def=0) const;
	void readCharArray(char* data, int size) const;
    std::string readString(int size) const;
    osg::Vec4f readColor32() const;
    osg::Vec2f readVec2f() const;
    osg::Vec3f readVec3f() const;
	osg::Vec3d readVec3d() const;

    void forward(std::istream::off_type _Off) const;

    int16 peekInt16() const;

    inline std::istream& operator() () { return *_istream; }

protected:

    virtual std::istream& read(std::istream::char_type *_Str, std::streamsize _Count) const;
    virtual std::istream& seekg(std::istream::off_type _Off, std::ios_base::seekdir _Way) const;

    bool                _byteswap;
	std::istream*       _istream;
};

} // end namespace

#endif

