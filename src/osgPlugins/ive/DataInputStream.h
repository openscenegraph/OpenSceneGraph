#ifndef IVE_DATAINPUTSTREAM
#define IVE_DATAINPUTSTREAM 1


#include <iostream>        // for ifstream
#include <string>
#include <map>
#include <vector>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Vec2d>
#include <osg/Vec3d>
#include <osg/Vec4d>
#include <osg/Quat>
#include <osg/Array>
#include <osg/Matrix>
#include <osg/Geometry>                       
#include <osg/Image>
#include <osg/StateSet>
#include <osg/Uniform>
#include <osg/ref_ptr>

#include <osgDB/ReaderWriter>

#include "IveVersion.h"
#include "DataTypeSize.h"    
#include "Exception.h"



namespace ive{

class DataInputStream{

public:
    DataInputStream(std::istream* istream);
    ~DataInputStream();

        void setOptions(const osgDB::ReaderWriter::Options* options);
        const osgDB::ReaderWriter::Options* getOptions() const { return _options.get(); }

    unsigned int getVersion();
    bool readBool();
    char readChar();
    unsigned char readUChar();
    unsigned short readUShort();
    unsigned int readUInt();
    int readInt();
    int peekInt();
    float readFloat();
    long readLong();
    unsigned long readULong();
    double readDouble();
    std::string readString();
    void readCharArray(char* data, int size);
        
    osg::Vec2 readVec2();
    osg::Vec3 readVec3();
    osg::Vec4 readVec4();
    osg::Vec2d readVec2d();
    osg::Vec3d readVec3d();
    osg::Vec4d readVec4d();
    osg::Plane readPlane();
    osg::Vec4ub readVec4ub();
    osg::Quat readQuat();
    osg::Matrixf readMatrixf();
    osg::Matrixd readMatrixd();
    osg::Geometry::AttributeBinding readBinding();
    osg::Array* readArray();
    osg::IntArray* readIntArray();
    osg::UByteArray* readUByteArray();
    osg::UShortArray* readUShortArray();
    osg::UIntArray* readUIntArray();
    osg::Vec4ubArray* readVec4ubArray();
    osg::FloatArray* readFloatArray();
    osg::Vec2Array* readVec2Array();
    osg::Vec3Array* readVec3Array();
    osg::Vec4Array* readVec4Array();
    osg::Vec2bArray* readVec2bArray();
    osg::Vec3bArray* readVec3bArray();
    osg::Vec4bArray* readVec4bArray();
    osg::Vec2sArray* readVec2sArray();
    osg::Vec3sArray* readVec3sArray();
    osg::Vec4sArray* readVec4sArray();    

    osg::Image* readImage(std::string s);
    osg::Image* readImage(IncludeImageMode mode);
    osg::StateSet* readStateSet();
    osg::StateAttribute* readStateAttribute();
    osg::Uniform* readUniform();
    osg::Shader* readShader();
    osg::Drawable* readDrawable();
    osg::Shape* readShape();
    osg::Node* readNode();

    // Set and get if must be generated external reference ive files
    void setLoadExternalReferenceFiles(bool b) {_loadExternalReferenceFiles=b;};
    bool getLoadExternalReferenceFiles() {return _loadExternalReferenceFiles;};

    typedef std::map<std::string, osg::ref_ptr<osg::Image> >    ImageMap;
    typedef std::map<int,osg::ref_ptr<osg::StateSet> >          StateSetMap;
    typedef std::map<int,osg::ref_ptr<osg::StateAttribute> >    StateAttributeMap;
    typedef std::map<int,osg::ref_ptr<osg::Uniform> >           UniformMap;
    typedef std::map<int,osg::ref_ptr<osg::Shader> >            ShaderMap;
    typedef std::map<int,osg::ref_ptr<osg::Drawable> >          DrawableMap;
    typedef std::map<int,osg::ref_ptr<osg::Shape> >             ShapeMap;
    typedef std::map<int,osg::ref_ptr<osg::Node> >              NodeMap;

    bool                _verboseOutput;
    std::istream*       _istream;
    int                 _byteswap;

private:
    int                 _version;
    bool                _peeking;
    int                 _peekValue; 
    ImageMap            _imageMap;
    StateSetMap         _statesetMap;
    StateAttributeMap   _stateAttributeMap;
    UniformMap          _uniformMap;
    ShaderMap           _shaderMap;
    DrawableMap         _drawableMap;
    ShapeMap            _shapeMap;
    NodeMap             _nodeMap;

    bool _loadExternalReferenceFiles;
        
    osg::ref_ptr<const osgDB::ReaderWriter::Options> _options;
   
};

}
#endif // IVE_DATAINPUTSTREAM
