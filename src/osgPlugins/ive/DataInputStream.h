#ifndef IVE_DATAINPUTSTREAM
#define IVE_DATAINPUTSTREAM 1


#include <iostream>		// for ifstream
#include <string>
#include <map>
#include <vector>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Quat>
#include <osg/Array>
#include <osg/Matrix>
#include <osg/Geometry>					   

#include "DataTypeSize.h"	
#include "Exception.h"


#include <osg/Image>
#include <osg/StateSet>
#include <osg/ref_ptr>

namespace ive{

class DataInputStream{

public:
	DataInputStream(std::istream* istream);
	~DataInputStream();
	bool readBool();
	char readChar();
	unsigned short readUShort();
	int readInt();
	int peekInt();
	float readFloat();
	long readLong();
	double readDouble();
	std::string readString();
	void readCharArray(char* data, int size);
	osg::Vec2 readVec2();
	osg::Vec3 readVec3();
	osg::Vec4 readVec4();
	osg::UByte4 readUByte4();
	osg::Quat readQuat();
	osg::Matrix readMatrix();
	osg::Geometry::AttributeBinding readBinding();
	osg::Image* readImage(std::string s);
	osg::StateSet* readStateSet();
	osg::Array* readArray();
	osg::IntArray* readIntArray();
	osg::UByteArray* readUByteArray();
	osg::UShortArray* readUShortArray();
	osg::UIntArray* readUIntArray();
	osg::UByte4Array* readUByte4Array();
	osg::FloatArray* readFloatArray();
	osg::Vec2Array* readVec2Array();
	osg::Vec3Array* readVec3Array();
	osg::Vec4Array* readVec4Array();

 	// Container to map image filenames to their respective images.
	typedef std::pair<std::string, osg::ref_ptr<osg::Image> >	ImagePair;
	typedef std::vector<ImagePair>								ImageList;

 	// Container to map stateset id to their respective stateset.
	typedef std::pair<int, osg::ref_ptr<osg::StateSet> >		StateSetPair;
	typedef std::vector<StateSetPair>							StateSetList;


private:
	std::istream*	_istream;
	int				_version;
	bool			_peeking;
	int				_peekValue; 
	ImageList		_imageList;
	StateSetList	_statesetList;
};

}
#endif // IVE_DATAINPUTSTREAM
