/**********************************************************************
 *
 *	FILE:			DataOutputStream.cpp
 *
 *	DESCRIPTION:	Implements methods to write simpel datatypes to an
 *					output stream.
 *
 *	CREATED BY:		Rune Schmidt Jensen
 *
 *	HISTORY:		Created 11.03.2003
 *
 *	Copyright 2003 VR-C
 **********************************************************************/

#include "DataOutputStream.h"
#include "Exception.h"
#include "StateSet.h"

using namespace ive;

DataOutputStream::DataOutputStream(std::ostream * ostream){

	_includeImageData= true;
	_ostream = ostream;
	if(!_ostream)
		throw Exception("DataOutputStream::DataOutputStream(): null pointer exception in argument.");
	writeInt(VERSION);
}

DataOutputStream::~DataOutputStream(){}

void DataOutputStream::writeBool(bool b){
	_ostream->write((char*)&b, BOOLSIZE);
}

void DataOutputStream::writeChar(char c){
	_ostream->write(&c, CHARSIZE);
}

void DataOutputStream::writeUShort(unsigned short s){
	_ostream->write((char*)&s, SHORTSIZE);
}

void DataOutputStream::writeInt(int i){
	_ostream->write((char*)&i, INTSIZE);
}

void DataOutputStream::writeFloat(float f){
	_ostream->write((char*)&f, FLOATSIZE);
}

void DataOutputStream::writeLong(long l){
	_ostream->write((char*)&l, LONGSIZE);
}

void DataOutputStream::writeDouble(double d){
	_ostream->write((char*)&d, DOUBLESIZE);
}

void DataOutputStream::writeString(std::string s){
	writeInt(s.size());
	_ostream->write(s.c_str(), s.size());
}

void DataOutputStream::writeCharArray(char* data, int size){
	_ostream->write(data, size);
}

void DataOutputStream::writeVec2(osg::Vec2 v){
	writeFloat(v.y());
	writeFloat(v.x());
}

void DataOutputStream::writeVec3(osg::Vec3 v){
	writeFloat(v.z());
	writeFloat(v.y());
	writeFloat(v.x());
}

void DataOutputStream::writeVec4(osg::Vec4 v){
	writeFloat(v.w());
	writeFloat(v.z());
	writeFloat(v.y());
	writeFloat(v.x());
}

void DataOutputStream::writeUByte4(osg::UByte4 v){
	writeChar(v.a());
	writeChar(v.b());
	writeChar(v.g());
	writeChar(v.r());
}

void DataOutputStream::writeQuat(osg::Quat q){
	writeFloat(q.w());
	writeFloat(q.z());
	writeFloat(q.y());
	writeFloat(q.x());
}

void DataOutputStream::writeStateSet(osg::StateSet* stateset){
	// If stateset is already in list we do not write it again.
	// We just writes its unique ID.
	for(StateSetList::iterator itr=_statesetList.begin(); 
		itr!=_statesetList.end(); ++itr){
			if((*itr) == (int)stateset){
				writeInt((*itr));
				return;
			}
	}
	// StateSet is not in list. 
	// We write its unique ID,
	writeInt((int)stateset);
	// add it to the stateset list,
	_statesetList.push_back((int)stateset);
	// and write it to stream.
	((ive::StateSet*)(stateset))->write(this);
}

void DataOutputStream::writeBinding(osg::Geometry::AttributeBinding b){
	switch(b){
		case osg::Geometry::BIND_OFF:				writeChar((char) 0); break;
		case osg::Geometry::BIND_OVERALL:			writeChar((char) 1); break;
		case osg::Geometry::BIND_PER_PRIMITIVE:		writeChar((char) 2); break;
		case osg::Geometry::BIND_PER_PRIMITIVE_SET:	writeChar((char) 3); break;
		case osg::Geometry::BIND_PER_VERTEX:		writeChar((char) 4); break;
		default: throw Exception("Unknown binding in DataOutputStream::writeBinding()");
	}
}

void DataOutputStream::writeArray(osg::Array* a){
	switch(a->getType()){
		case osg::Array::IntArrayType: 
			writeChar((char)0);
			writeIntArray(static_cast<osg::IntArray*>(a));
			break;
		case osg::Array::UByteArrayType:
			writeChar((char)1);
			writeUByteArray(static_cast<osg::UByteArray*>(a));
			break;
		case osg::Array::UShortArrayType:
			writeChar((char)2);
			writeUShortArray(static_cast<osg::UShortArray*>(a));
			break;
		case osg::Array::UIntArrayType:
			writeChar((char)3);
			writeUIntArray(static_cast<osg::UIntArray*>(a));
			break;
		case osg::Array::UByte4ArrayType:
			writeChar((char)4);
			writeUByte4Array(static_cast<osg::UByte4Array*>(a));
			break;
		case osg::Array::FloatArrayType:
			writeChar((char)5);
			writeFloatArray(static_cast<osg::FloatArray*>(a));
			break;
		case osg::Array::Vec2ArrayType: 
			writeChar((char)6);
			writeVec2Array(static_cast<osg::Vec2Array*>(a));
			break;
		case osg::Array::Vec3ArrayType: 
			writeChar((char)7);
			writeVec3Array(static_cast<osg::Vec3Array*>(a));
			break;
 		case osg::Array::Vec4ArrayType: 
			writeChar((char)8);
			writeVec4Array(static_cast<osg::Vec4Array*>(a));
			break;
		default: throw Exception("Unknown array type in DataOutputStream::writeArray()");
	}
}


void DataOutputStream::writeIntArray(osg::IntArray* a){
	int size = a->getNumElements();
	writeInt(size);
	for(int i =0; i<size ;i++){
		writeInt(a->index(i));
	}
}

void DataOutputStream::writeUByteArray(osg::UByteArray* a){
	int size = a->getNumElements(); 
	writeInt(size);
	for(int i =0; i<size ;i++){
		writeChar((*a)[i]);
	}
}

void DataOutputStream::writeUShortArray(osg::UShortArray* a){
	int size = a->getNumElements(); 
	writeInt(size);
	for(int i =0; i<size ;i++){
		writeUShort((*a)[i]);
	}
}

void DataOutputStream::writeUIntArray(osg::UIntArray* a){
	int size = a->getNumElements(); 
	writeInt(size);
	for(int i =0; i<size ;i++){
		writeInt((*a)[i]);
	}
}

void DataOutputStream::writeUByte4Array(osg::UByte4Array* a){
	int size = a->getNumElements(); 
	writeInt(size);
	for(int i =0; i<size ;i++){
		writeUByte4((*a)[i]);
	}
}

void DataOutputStream::writeFloatArray(osg::FloatArray* a){
	int size = a->getNumElements(); 
	writeInt(size);
	for(int i =0; i<size ;i++){
		writeFloat((*a)[i]);
	}
}


void DataOutputStream::writeVec2Array(osg::Vec2Array* a){
	int size = a->size();
	writeInt(size);
	for(int i=0;i<size;i++){
		writeVec2((*a)[i]);
	}
}

void DataOutputStream::writeVec3Array(osg::Vec3Array* a){
	int size = a->size();
	writeInt(size);
	for(int i = 0; i < size; i++){
        writeVec3((*a)[i]);
	}
}

void DataOutputStream::writeVec4Array(osg::Vec4Array* a){
	int size = a->size();
	writeInt(size);
	for(int i=0;i<size;i++){
		writeVec4((*a)[i]);
	}
}

void DataOutputStream::writeMatrix(osg::Matrix mat){
	float* p = mat.ptr();
	for(int i=0;i<16;i++){
		writeFloat(p[i]);
	}
}
