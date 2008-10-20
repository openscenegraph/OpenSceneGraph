#ifndef IVE_DATAOUTPUTSTREAM
#define IVE_DATAOUTPUTSTREAM 1



#include <iostream>        // for ofstream
#include <string>
#include <sstream>

#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Quat>
#include <osg/Array>
#include <osg/Matrix>
#include <osg/Geometry>
#include <osg/Shape>
#include <osg/Uniform>
#include <osgDB/ReaderWriter>

#include <osgTerrain/TerrainTile>

#include "IveVersion.h"
#include "DataTypeSize.h"
#include "Exception.h"

#include <osg/StateSet>
#include <osg/ref_ptr>

namespace ive {                       

class DataOutputStream{

public:
    DataOutputStream(std::ostream* ostream, const osgDB::ReaderWriter::Options* options);
    ~DataOutputStream();

    const osgDB::ReaderWriter::Options* getOptions() const { return _options.get(); }

    unsigned int getVersion() { return VERSION; }
        
    void writeBool(bool b);
    void writeChar(char c);
    void writeUChar(unsigned char c);
    void writeUShort(unsigned short s);
    void writeShort(short s);    
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
    void writeVec2d(const osg::Vec2d& v);
    void writeVec3d(const osg::Vec3d& v);
    void writeVec4d(const osg::Vec4d& v);
    void writePlane(const osg::Plane& v);
    void writeVec4ub(const osg::Vec4ub& v);
    void writeQuat(const osg::Quat& q);
    void writeBinding(osg::Geometry::AttributeBinding b);
    void writeArray(const osg::Array* a);
    void writeIntArray(const osg::IntArray* a);
    void writeUByteArray(const osg::UByteArray* a);
    void writeUShortArray(const osg::UShortArray* a);
    void writeUIntArray(const osg::UIntArray* a);
    void writeVec4ubArray(const osg::Vec4ubArray* a);
    void writeVec2b(const osg::Vec2b& v);    
    void writeVec3b(const osg::Vec3b& v);    
    void writeVec4b(const osg::Vec4b& v);    
    
    void writePackedFloatArray(const osg::FloatArray* a, float maxError);
    
    void writeFloatArray(const osg::FloatArray* a);
    void writeVec2Array(const osg::Vec2Array* a);
    void writeVec3Array(const osg::Vec3Array* a);
    void writeVec4Array(const osg::Vec4Array* a);
    void writeVec2sArray(const osg::Vec2sArray* a);
    void writeVec3sArray(const osg::Vec3sArray* a);
    void writeVec4sArray(const osg::Vec4sArray* a);    
    void writeVec2bArray(const osg::Vec2bArray* a);
    void writeVec3bArray(const osg::Vec3bArray* a);
    void writeVec4bArray(const osg::Vec4bArray* a);
    void writeVec2dArray(const osg::Vec2dArray* a);
    void writeVec3dArray(const osg::Vec3dArray* a);
    void writeVec4dArray(const osg::Vec4dArray* a);  
    void writeMatrixf(const osg::Matrixf& mat);
    void writeMatrixd(const osg::Matrixd& mat);

    void writeStateSet(const osg::StateSet* stateset);
    void writeStateAttribute(const osg::StateAttribute* sa);
    void writeUniform(const osg::Uniform* uniform);
    void writeShader(const osg::Shader* shader);
    void writeDrawable(const osg::Drawable* sa);
    void writeShape(const osg::Shape* sa);
    void writeNode(const osg::Node* sa);
    void writeImage(IncludeImageMode mode, osg::Image *image);
    void writeImage(osg::Image *image);

    void writeLayer(const osgTerrain::Layer* layer);
    void writeLocator(const osgTerrain::Locator* locator);
    
    void writeObject(const osg::Object* object);

    void setWriteDirectory(const std::string& directoryName) { _writeDirectory = directoryName; }
    const std::string& getWriteDirectory() const { return _writeDirectory; }

    // Set and get include image data in stream
    void setIncludeImageMode(IncludeImageMode mode) {_includeImageMode=mode;};
    IncludeImageMode getIncludeImageMode() const {return _includeImageMode;};
    IncludeImageMode getIncludeImageMode(const osg::Image* image) const;

    // Set and get include external references in stream
    void setIncludeExternalReferences(bool b) {_includeExternalReferences=b;};
    bool getIncludeExternalReferences() const {return _includeExternalReferences;};

    // Set and get if must be generated external reference ive files
    void setWriteExternalReferenceFiles(bool b) {_writeExternalReferenceFiles=b;};
    bool getWriteExternalReferenceFiles() const {return _writeExternalReferenceFiles;};

    // Set and get if must be used original external reference files
    void setUseOriginalExternalReferences(bool b) {_useOriginalExternalReferences=b;};
    bool getUseOriginalExternalReferences() const {return _useOriginalExternalReferences;};

    void setTerrainMaximumErrorToSizeRatio(double ratio) { _maximumErrorToSizeRatio = ratio; }
    double getTerrainMaximumErrorToSizeRatio() const { return _maximumErrorToSizeRatio; }


    bool                _verboseOutput;

    bool compress(std::ostream& fout, const std::string& source) const;

private:

    std::ostream* _ostream;
    std::ostream* _output_ostream;
    
    std::stringstream _compressionStream;
    int _compressionLevel;

     // Container to map stateset uniques to their respective stateset.
    typedef std::map<const osg::StateSet*,int>          StateSetMap;
    typedef std::map<const osg::StateAttribute*,int>    StateAttributeMap;
    typedef std::map<const osg::Uniform*,int>           UniformMap;
    typedef std::map<const osg::Shader*,int>            ShaderMap;
    typedef std::map<const osg::Drawable*,int>          DrawableMap;
    typedef std::map<const osg::Shape*,int>             ShapeMap;
    typedef std::map<const osg::Node*,int>              NodeMap;
    typedef std::map<const osgTerrain::Layer*,int>      LayerMap;
    typedef std::map<const osgTerrain::Locator*,int>    LocatorMap;
        
    StateSetMap         _stateSetMap;
    StateAttributeMap   _stateAttributeMap;
    UniformMap          _uniformMap;
    ShaderMap           _shaderMap;
    DrawableMap         _drawableMap;
    ShapeMap            _shapeMap;
    NodeMap             _nodeMap;
    LayerMap            _layerMap;
    LocatorMap          _locatorMap;

    std::string         _writeDirectory;
    bool                _includeExternalReferences;
    bool                _writeExternalReferenceFiles;
    bool                _useOriginalExternalReferences;
    double              _maximumErrorToSizeRatio;

    IncludeImageMode    _includeImageMode;
    
    osg::ref_ptr<const osgDB::ReaderWriter::Options> _options;
};

}
#endif // IVE_DATAOUTPUTSTREAM
