#ifndef IVE_DATAOUTPUTSTREAM
#define IVE_DATAOUTPUTSTREAM 1



#include <iostream>		// for ofstream
#include <string>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Quat>
#include <osg/Array>
#include <osg/Matrix>
#include <osg/Geometry>

#include "Export.h"
#include "DataTypeSize.h"
#include "Exception.h"

#include <osg/StateSet>
#include <osg/ref_ptr>

namespace ive {					   

class IVE_EXPORT DataOutputStream{

public:
	DataOutputStream(std::ostream* ostream);
	~DataOutputStream();
	void writeBool(bool b);
	void writeChar(char c);
	void writeUShort(unsigned short s);
	void writeInt(int i);
	void writeFloat(float f);
	void writeLong(long l);
	void writeDouble(double d);
	void writeString(std::string s);
	void writeCharArray(char* data, int size);
	void writeVec2(osg::Vec2 v);
	void writeVec3(osg::Vec3 v);
	void writeVec4(osg::Vec4 v);
	void writeUByte4(osg::UByte4 v);
	void writeQuat(osg::Quat q);
	void writeStateSet(osg::StateSet* stateset);
	void writeBinding(osg::Geometry::AttributeBinding b);
	void writeArray(osg::Array* a);
	void writeIntArray(osg::IntArray* a);
	void writeUByteArray(osg::UByteArray* a);
	void writeUShortArray(osg::UShortArray* a);
	void writeUIntArray(osg::UIntArray* a);
	void writeUByte4Array(osg::UByte4Array* a);
	void writeFloatArray(osg::FloatArray* a);
	void writeVec2Array(osg::Vec2Array* a);
	void writeVec3Array(osg::Vec3Array* a);
	void writeVec4Array(osg::Vec4Array* a);
	void writeMatrix(osg::Matrix mat);

	// Set and get include image data in stream
	void setIncludeImageData(bool b) {_includeImageData=b;};
	bool getIncludeImageData() {return _includeImageData;};

private:
	std::ostream* _ostream;

 	// Container to map stateset uniques to their respective stateset.
	typedef std::vector<int>						StateSetList;
	StateSetList _statesetList;

	bool			_includeImageData;
	
};

}
#endif // IVE_DATAOUTPUTSTREAM
