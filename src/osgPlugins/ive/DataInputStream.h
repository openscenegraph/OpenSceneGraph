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
#include <osg/Image>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Uniform>
#include <osg/ref_ptr>

#include <osgTerrain/TerrainTile>
#include <osgVolume/VolumeTile>

#include <osgDB/ReaderWriter>

#include "IveVersion.h"
#include "DataTypeSize.h"
#include "Exception.h"



namespace ive{

class DataInputStream{

public:
    DataInputStream(std::istream* istream, const osgDB::ReaderWriter::Options* options);
    ~DataInputStream();

    const osgDB::ReaderWriter::Options* getOptions() const { return _options.get(); }

    inline unsigned int getVersion() const { return _version; }
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
    deprecated_osg::Geometry::AttributeBinding readBinding();
    osg::Array* readArray();
    osg::IntArray* readIntArray();
    osg::UByteArray* readUByteArray();
    osg::UShortArray* readUShortArray();
    osg::UIntArray* readUIntArray();
    osg::Vec4ubArray* readVec4ubArray();
    bool readPackedFloatArray(osg::FloatArray* floatArray);
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
    osg::Vec2dArray* readVec2dArray();
    osg::Vec3dArray* readVec3dArray();
    osg::Vec4dArray* readVec4dArray();

    osg::Image* readImage(std::string s);
    osg::Image* readImage(IncludeImageMode mode);
    osg::Image* readImage();
    osg::StateSet* readStateSet();
    osg::StateAttribute* readStateAttribute();
    osg::Uniform* readUniform();
    osg::Shader* readShader();
    osg::Drawable* readDrawable();
    osg::Shape* readShape();
    osg::Node* readNode();

    osgTerrain::Layer* readLayer();
    osgTerrain::Locator* readLocator();

    osgVolume::Layer* readVolumeLayer();
    osgVolume::Locator* readVolumeLocator();
    osgVolume::Property* readVolumeProperty();

    osg::Object* readObject();

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
    typedef std::map<int,osg::ref_ptr<osgTerrain::Layer> >      LayerMap;
    typedef std::map<int,osg::ref_ptr<osgTerrain::Locator> >    LocatorMap;
    typedef std::map<int,osg::ref_ptr<osgVolume::Layer> >       VolumeLayerMap;
    typedef std::map<int,osg::ref_ptr<osgVolume::Locator> >     VolumeLocatorMap;
    typedef std::map<int,osg::ref_ptr<osgVolume::Property> >    VolumePropertyMap;

    bool                _verboseOutput;
    std::istream*       _istream;
    int                 _byteswap;

    bool                _owns_istream;

    bool uncompress(std::istream& fin, std::string& destination) const;

    void throwException(const std::string& message) { _exception = new Exception(message); }
    void throwException(Exception* exception) { _exception = exception; }
    const Exception* getException() const { return _exception.get(); }

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
    LayerMap            _layerMap;
    LocatorMap          _locatorMap;
    VolumeLayerMap      _volumeLayerMap;
    VolumeLocatorMap    _volumeLocatorMap;
    VolumePropertyMap   _volumePropertyMap;

    bool _loadExternalReferenceFiles;

    osg::ref_ptr<const osgDB::ReaderWriter::Options> _options;

    osg::ref_ptr<Exception> _exception;
};

}
#endif // IVE_DATAINPUTSTREAM
