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
#include <osg/Shape>

#include "IveVersion.h"
#include "DataTypeSize.h"
#include "Exception.h"

#include <osg/StateSet>
#include <osg/ref_ptr>

namespace ive {					   

class DataOutputStream{

public:
	DataOutputStream(std::ostream* ostream);
	~DataOutputStream();
	void writeBool(bool b);
	void writeChar(char c);
	void writeUChar(unsigned char c);
	void writeUShort(unsigned short s);
	void writeUInt(unsigned int s);
	void writeInt(int i);
	void writeFloat(float f);
	void writeLong(long l);
	void writeULong(unsigned long l);
	void writeDouble(double d);
	void writeString(const std::string& s);
	void writeCharArray(const char* data, int size);
	void writeVec2(const osg::Vec2& v);
	void writeVec3(const osg::Vec3& v);
	void writeVec4(const osg::Vec4& v);
	void writeUByte4(const osg::UByte4& v);
	void writeQuat(const osg::Quat& q);
	void writeBinding(osg::Geometry::AttributeBinding b);
	void writeArray(const osg::Array* a);
	void writeIntArray(const osg::IntArray* a);
	void writeUByteArray(const osg::UByteArray* a);
	void writeUShortArray(const osg::UShortArray* a);
	void writeUIntArray(const osg::UIntArray* a);
	void writeUByte4Array(const osg::UByte4Array* a);
	void writeFloatArray(const osg::FloatArray* a);
	void writeVec2Array(const osg::Vec2Array* a);
	void writeVec3Array(const osg::Vec3Array* a);
	void writeVec4Array(const osg::Vec4Array* a);
	void writeMatrix(const osg::Matrix& mat);

	void writeStateSet(const osg::StateSet* stateset);
	void writeStateAttribute(const osg::StateAttribute* sa);
	void writeDrawable(const osg::Drawable* sa);
	void writeShape(const osg::Shape* sa);
	void writeNode(const osg::Node* sa);

	// Set and get include image data in stream
	void setIncludeImageData(bool b) {_includeImageData=b;};
	bool getIncludeImageData() {return _includeImageData;};

        bool                _verboseOutput;

private:
	std::ostream* _ostream;

 	// Container to map stateset uniques to their respective stateset.
	typedef std::map<const osg::StateSet*,int>        StateSetMap;
	typedef std::map<const osg::StateAttribute*,int>  StateAttributeMap;
	typedef std::map<const osg::Drawable*,int>        DrawableMap;
	typedef std::map<const osg::Shape*,int>        ShapeMap;
	typedef std::map<const osg::Node*,int>            NodeMap;
        
        StateSetMap         _stateSetMap;
        StateAttributeMap   _stateAttributeMap;
        DrawableMap         _drawableMap;
        ShapeMap            _shapeMap;
        NodeMap             _nodeMap;

	bool                _includeImageData;
	
};

}
#endif // IVE_DATAOUTPUTSTREAM
