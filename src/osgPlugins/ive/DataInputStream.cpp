/**********************************************************************
 *
 *	FILE:			DataInputStream.cpp
 *
 *	DESCRIPTION:	Implements methods to read simpel datatypes from an
 *					input stream.
 *
 *	CREATED BY:		Rune Schmidt Jensen
 *
 *	HISTORY:		Created 11.03.2003
 *
 *	Copyright 2003 VR-C
 **********************************************************************/

#include "DataInputStream.h"
#include "StateSet.h"
#include <osgDB/ReadFile>

using namespace ive;
using namespace std;

DataInputStream::DataInputStream(std::istream* istream){
	_istream = istream;
	_peeking = false;
	_peekValue = 0;

	if(!istream){
		throw Exception("DataInputStream::DataInputStream(): null pointer exception in argument.");	
	}

	_version = readInt();

	// Are we trying to open a binary .ive file which version are newer than this library.
	if(_version>VERSION){
		throw Exception("DataInputStream::DataInputStream(): The version found in the file is newer than this library can handle.");
	}
}

DataInputStream::~DataInputStream(){}

bool DataInputStream::readBool(){
	bool b;
	_istream->read((char*)&b, BOOLSIZE);
	if (_istream->rdstate() & _istream->failbit)
		throw Exception("DataInputStream::readBool(): Failed to read boolean value.");
	return b;
}

char DataInputStream::readChar(){
	char c;
	_istream->read(&c, CHARSIZE);
	if (_istream->rdstate() & _istream->failbit)
		throw Exception("DataInputStream::readChar(): Failed to read char value.");
	return c;
}

unsigned short DataInputStream::readUShort(){
	unsigned short s;
	_istream->read((char*)&s, SHORTSIZE);
	if (_istream->rdstate() & _istream->failbit)
		throw Exception("DataInputStream::readUShort(): Failed to read unsigned short value.");
	return s;
}

int DataInputStream::readInt(){
	if(_peeking){
		_peeking = false;
		return _peekValue;
	}
	int i;
	_istream->read((char*)&i, INTSIZE);
	if (_istream->rdstate() & _istream->failbit)
		throw Exception("DataInputStream::readInt(): Failed to read int value.");
	return i;
}

/**
 * Read an integer from the stream, but
 * save it such that the next readInt call will
 * return the same integer.
 */
int DataInputStream::peekInt(){
	if(_peeking){
		return _peekValue;
	}
	_peekValue  = readInt();
	_peeking = true;
	return _peekValue;
}

float DataInputStream::readFloat(){
	float f;
	_istream->read((char*)&f, FLOATSIZE);
	if (_istream->rdstate() & _istream->failbit)
		throw Exception("DataInputStream::readFloat(): Failed to read float value.");
	return f;
}

long DataInputStream::readLong(){
	long l;
	_istream->read((char*)&l, LONGSIZE);
	if (_istream->rdstate() & _istream->failbit)
		throw Exception("DataInputStream::readLong(): Failed to read long value.");
	return l;
}

double DataInputStream::readDouble(){
	double d;
	_istream->read((char*)&d, DOUBLESIZE);
	if (_istream->rdstate() & _istream->failbit)
		throw Exception("DataInputStream::readDouble(): Failed to read double value.");
	return d;
}

std::string DataInputStream::readString(){
	std::string s;
	int size = readInt();
	s.resize(size);
	_istream->read((char*)s.c_str(), size);
	if (_istream->rdstate() & _istream->failbit)
		throw Exception("DataInputStream::readString(): Failed to read string value.");
	return s;
}

void DataInputStream::readCharArray(char* data, int size){
	_istream->read(data, size);
	if (_istream->rdstate() & _istream->failbit)
		throw Exception("DataInputStream::readCharArray(): Failed to read char value.");
}

osg::Vec2 DataInputStream::readVec2(){
	osg::Vec2 v;
	v.set(readFloat(), readFloat());
	return v;
}

osg::Vec3 DataInputStream::readVec3(){
	osg::Vec3 v;
	v.set(readFloat(),readFloat(),readFloat());
	return v;
}

osg::Vec4 DataInputStream::readVec4(){
	osg::Vec4 v;
	v.set(readFloat(), readFloat(), readFloat(), readFloat());
	return v;
}

osg::UByte4 DataInputStream::readUByte4(){
	osg::UByte4 v;
	v.set(readChar(), readChar(), readChar(), readChar());
	return v;
}

osg::Quat DataInputStream::readQuat(){
	osg::Quat q;
	q.set(readFloat(), readFloat(), readFloat(), readFloat());
	return q;
}

/**
 * 
 */
osg::Image* DataInputStream::readImage(std::string filename){
	// If image is already read and in list 
	// then just return pointer to this.
	for(ImageList::iterator mitr=_imageList.begin(); 
		mitr!=_imageList.end(); ++mitr){
			if(mitr->first.compare(filename) == 0){
				return mitr->second.get();
			}
	}
	// Image is not in list. 
	// Read it from disk, 
	osg::Image* image = osgDB::readImageFile(filename.c_str());
	// add it to the imageList,
	_imageList.push_back(ImagePair(filename, image));
	// and return image pointer.
	return image;
}

osg::StateSet* DataInputStream::readStateSet(){
	// Read statesets unique ID.
	int id = readInt();
	// See if stateset is already in the list.
	for(StateSetList::iterator itr=_statesetList.begin(); 
		itr!=_statesetList.end(); ++itr){
			if(itr->first == id){
				return itr->second.get();
			}
	}
	// StateSet is not in list.
	// Create a new stateset,
	osg::StateSet* stateset = new osg::StateSet();
	// read its properties from stream
	((ive::StateSet*)(stateset))->read(this);
	// and add it to the stateset list,
	_statesetList.push_back(StateSetPair(id, stateset));
	return stateset;
}



osg::Geometry::AttributeBinding DataInputStream::readBinding(){
	char c = readChar();
	switch((int)c){
		case 0:	return osg::Geometry::BIND_OFF;
		case 1: return osg::Geometry::BIND_OVERALL;
		case 2: return osg::Geometry::BIND_PER_PRIMITIVE;
		case 3: return osg::Geometry::BIND_PER_PRIMITIVE_SET;
		case 4: return osg::Geometry::BIND_PER_VERTEX;
		default: throw Exception("Unknown binding type in DataInputStream::readBinding()");
	}
}

osg::Array* DataInputStream::readArray(){
	char c = readChar();
	switch((int)c){
		case 0: return readIntArray();
		case 1: return readUByteArray();
		case 2: return readUShortArray();
		case 3: return readUIntArray();
		case 4: return readUByte4Array();
		case 5: return readFloatArray();
		case 6:	return readVec2Array();
		case 7:	return readVec3Array();
		case 8:	return readVec4Array();
		default: throw Exception("Unknown array type in DataInputStream::readArray()");
	}
}

osg::IntArray* DataInputStream::readIntArray(){
	int size = readInt();
	osg::IntArray* a = new osg::IntArray();
	a->reserve(size);
	for(int i =0; i<size;i++){
		a->push_back(readInt());
	}
	return a;
}

osg::UByteArray* DataInputStream::readUByteArray(){
	int size = readInt();
	osg::UByteArray* a = new osg::UByteArray();
	a->reserve(size);
	for(int i =0; i<size;i++){
		a->push_back(readChar());
	}
	return a;
}

osg::UShortArray* DataInputStream::readUShortArray(){
	int size = readInt();
	osg::UShortArray* a = new osg::UShortArray();
	a->reserve(size);
	for(int i =0; i<size;i++){
		a->push_back(readUShort());
	}
	return a;
}

osg::UIntArray* DataInputStream::readUIntArray(){
	int size = readInt();
	osg::UIntArray* a = new osg::UIntArray();
	a->reserve(size);
	for(int i =0; i<size;i++){
		a->push_back((unsigned int)readInt());
	}
	return a;
}

osg::UByte4Array* DataInputStream::readUByte4Array(){
	int size = readInt();
	osg::UByte4Array* a = new osg::UByte4Array();
	a->reserve(size);
	for(int i =0; i<size;i++){
		a->push_back(readUByte4());
	}
	return a;
}

osg::FloatArray* DataInputStream::readFloatArray(){
	int size = readInt();
	osg::FloatArray* a = new osg::FloatArray();
	a->reserve(size);
	for(int i =0; i<size;i++){
		a->push_back(readFloat());
	}
	return a;
}

osg::Vec2Array* DataInputStream::readVec2Array(){
	int size = readInt();
	osg::Vec2Array* a = new osg::Vec2Array(size);
	for(int i = 0; i < size; i++){
		(*a)[i] = (readVec2());
	}
	return a;
}

osg::Vec3Array* DataInputStream::readVec3Array(){
	int size = readInt();
	osg::Vec3Array* a = new osg::Vec3Array(size);
	for(int i = 0; i < size; i++){
		(*a)[i] = readVec3();
	}
	return a;
}

osg::Vec4Array* DataInputStream::readVec4Array(){
	int size = readInt();
	osg::Vec4Array* a = new osg::Vec4Array(size);
	for(int i = 0; i < size; i++){
		(*a)[i] = (readVec4());
	}
	return a;
}

osg::Matrix DataInputStream::readMatrix(){
	osg::Matrix mat;
	float* p = mat.ptr();
	for(int i=0;i<16;i++){
		p[i] = readFloat();
	}
	return mat;
}
