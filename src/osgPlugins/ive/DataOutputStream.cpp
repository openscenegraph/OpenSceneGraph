/**********************************************************************
 *
 *    FILE:           DataOutputStream.cpp
 *
 *    DESCRIPTION:    Implements methods to write simple datatypes to an
 *                    output stream.
 *
 *    CREATED BY:     Rune Schmidt Jensen
 *
 *    HISTORY:        Created 11.03.2003
 *                    Updated for 1D textures - Don Burns 27.1.2004
 *                      Updated for light model - Stan Blinov at 25 august 7512 from World Creation (7.09.2004)
 *
 *    Copyright 2003 VR-C
 **********************************************************************/

#include "DataOutputStream.h"
#include "Exception.h"

#include "StateSet.h"
#include "AlphaFunc.h"
#include "BlendFunc.h"
#include "Material.h"
#include "CullFace.h"
#include "Depth.h"
#include "ClipPlane.h"
#include "PolygonOffset.h"
#include "PolygonMode.h"
#include "ShadeModel.h"
#include "Point.h"
#include "LineWidth.h"
#include "Texture1D.h"
#include "Texture2D.h"
#include "Texture3D.h"
#include "TextureCubeMap.h"
#include "TextureRectangle.h"
#include "TexEnv.h"
#include "TexEnvCombine.h"
#include "TexGen.h"
#include "TexMat.h"
#include "FragmentProgram.h"
#include "VertexProgram.h"
#include "LightModel.h"
#include "ProxyNode.h"
#include "FrontFace.h"
#include "Program.h"
#include "Uniform.h"
#include "Shader.h"
#include "Viewport.h"
#include "Scissor.h"
#include "Image.h"

#include "Group.h"
#include "MatrixTransform.h"
#include "CameraNode.h"
#include "CameraView.h"
#include "Geode.h"
#include "LightSource.h"
#include "TexGenNode.h"
#include "ClipNode.h"
#include "Billboard.h"
#include "Sequence.h"
#include "LOD.h"
#include "PagedLOD.h"
#include "PositionAttitudeTransform.h"
#include "AutoTransform.h"
#include "DOFTransform.h"
#include "Transform.h"
#include "Switch.h"
#include "OccluderNode.h"
#include "Impostor.h"
#include "CoordinateSystemNode.h"

#include "LightPointNode.h"
#include "MultiSwitch.h"
#include "VisibilityGroup.h"

#include "MultiTextureControl.h"

#include "Geometry.h"
#include "ShapeDrawable.h"

#include "Shape.h"

#include "Text.h"

#include <osg/Notify>
#include <osg/io_utils>
#include <osgDB/FileUtils>

#include <fstream>
#include <sstream>

using namespace ive;


void DataOutputStream::setOptions(const osgDB::ReaderWriter::Options* options) 
{ 
    _options = options; 

    if (_options.get())
    {
        if(_options->getOptionString().find("noTexturesInIVEFile")!=std::string::npos) {
            setIncludeImageMode(IMAGE_REFERENCE_FILE);
        } else if(_options->getOptionString().find("includeImageFileInIVEFile")!=std::string::npos) {
            setIncludeImageMode(IMAGE_INCLUDE_FILE);
        } else if(_options->getOptionString().find("compressImageData")!=std::string::npos) {
            setIncludeImageMode(IMAGE_COMPRESS_DATA);
        }
        osg::notify(osg::DEBUG_INFO) << "ive::DataOutpouStream.setIncludeImageMode()=" << getIncludeImageMode() << std::endl;

        setIncludeExternalReferences(_options->getOptionString().find("inlineExternalReferencesInIVEFile")!=std::string::npos);
        osg::notify(osg::DEBUG_INFO) << "ive::DataOutpouStream.setIncludeExternalReferences()=" << getIncludeExternalReferences() << std::endl;

        setWriteExternalReferenceFiles(_options->getOptionString().find("noWriteExternalReferenceFiles")==std::string::npos);
        osg::notify(osg::DEBUG_INFO) << "ive::DataOutpouStream.setWriteExternalReferenceFiles()=" << getWriteExternalReferenceFiles() << std::endl;

        setUseOriginalExternalReferences(_options->getOptionString().find("useOriginalExternalReferences")!=std::string::npos);
        osg::notify(osg::DEBUG_INFO) << "ive::DataOutpouStream.setUseOriginalExternalReferences()=" << getUseOriginalExternalReferences() << std::endl;
    }
}

DataOutputStream::DataOutputStream(std::ostream * ostream)
{
    _verboseOutput = false;

    _includeImageMode = IMAGE_INCLUDE_DATA;

    _includeExternalReferences     = false;
    _writeExternalReferenceFiles   = false;
    _useOriginalExternalReferences = true;
    
    
    _ostream = ostream;
    if(!_ostream)
        throw Exception("DataOutputStream::DataOutputStream(): null pointer exception in argument.");
    writeUInt(ENDIAN_TYPE) ;
    writeUInt(getVersion());
}

DataOutputStream::~DataOutputStream(){}

void DataOutputStream::writeBool(bool b)
{
    char c = b?1:0;
    _ostream->write(&c, CHARSIZE);
    
    if (_verboseOutput) std::cout<<"read/writeBool() ["<<(int)c<<"]"<<std::endl;
}

void DataOutputStream::writeChar(char c){
    _ostream->write(&c, CHARSIZE);
    
    if (_verboseOutput) std::cout<<"read/writeChar() ["<<(int)c<<"]"<<std::endl;
}

void DataOutputStream::writeUChar(unsigned char c){
    _ostream->write((char*)&c, CHARSIZE);
    
    if (_verboseOutput) std::cout<<"read/writeUChar() ["<<(int)c<<"]"<<std::endl;
}

void DataOutputStream::writeUShort(unsigned short s){
    _ostream->write((char*)&s, SHORTSIZE);
    
    if (_verboseOutput) std::cout<<"read/writeUShort() ["<<s<<"]"<<std::endl;
}

void DataOutputStream::writeShort(short s){
    _ostream->write((char*)&s, SHORTSIZE);

    if (_verboseOutput) std::cout<<"read/writeShort() ["<<s<<"]"<<std::endl;
}

void DataOutputStream::writeUInt(unsigned int s){
    _ostream->write((char*)&s, INTSIZE);
    
    if (_verboseOutput) std::cout<<"read/writeUInt() ["<<s<<"]"<<std::endl;
}

void DataOutputStream::writeInt(int i){
    _ostream->write((char*)&i, INTSIZE);
    
    if (_verboseOutput) std::cout<<"read/writeInt() ["<<i<<"]"<<std::endl;
}

void DataOutputStream::writeFloat(float f){
    _ostream->write((char*)&f, FLOATSIZE);
    
    if (_verboseOutput) std::cout<<"read/writeFloat() ["<<f<<"]"<<std::endl;
}

void DataOutputStream::writeLong(long l){
    _ostream->write((char*)&l, LONGSIZE);
    
    if (_verboseOutput) std::cout<<"read/writeLong() ["<<l<<"]"<<std::endl;
}

void DataOutputStream::writeULong(unsigned long l){
    _ostream->write((char*)&l, LONGSIZE);
    
    if (_verboseOutput) std::cout<<"read/writeULong() ["<<l<<"]"<<std::endl;
}

void DataOutputStream::writeDouble(double d){
    _ostream->write((char*)&d, DOUBLESIZE);
    
    if (_verboseOutput) std::cout<<"read/writeDouble() ["<<d<<"]"<<std::endl;
}

void DataOutputStream::writeString(const std::string& s){
    writeInt(s.size());
    _ostream->write(s.c_str(), s.size());
    
    if (_verboseOutput) std::cout<<"read/writeString() ["<<s<<"]"<<std::endl;
}

void DataOutputStream::writeCharArray(const char* data, int size){
    _ostream->write(data, size);
    
    if (_verboseOutput) std::cout<<"read/writeCharArray() ["<<data<<"]"<<std::endl;
}

void DataOutputStream::writeVec2(const osg::Vec2& v){
    writeFloat(v.x());
    writeFloat(v.y());
    
    if (_verboseOutput) std::cout<<"read/writeVec2() ["<<v<<"]"<<std::endl;
}

void DataOutputStream::writeVec3(const osg::Vec3& v){
    writeFloat(v.x());
    writeFloat(v.y());
    writeFloat(v.z());
    
    if (_verboseOutput) std::cout<<"read/writeVec3() ["<<v<<"]"<<std::endl;
}

void DataOutputStream::writeVec4(const osg::Vec4& v){
    writeFloat(v.x());
    writeFloat(v.y());
    writeFloat(v.z());
    writeFloat(v.w());
    
    if (_verboseOutput) std::cout<<"read/writeVec4() ["<<v<<"]"<<std::endl;
}

void DataOutputStream::writeVec2d(const osg::Vec2d& v){
    writeDouble(v.x());
    writeDouble(v.y());
    
    if (_verboseOutput) std::cout<<"read/writeVec2() ["<<v<<"]"<<std::endl;
}

void DataOutputStream::writeVec3d(const osg::Vec3d& v){
    writeDouble(v.x());
    writeDouble(v.y());
    writeDouble(v.z());
    
    if (_verboseOutput) std::cout<<"read/writeVec3d() ["<<v<<"]"<<std::endl;
}

void DataOutputStream::writeVec4d(const osg::Vec4d& v){
    writeDouble(v.x());
    writeDouble(v.y());
    writeDouble(v.z());
    writeDouble(v.w());
    
    if (_verboseOutput) std::cout<<"read/writeVec4d() ["<<v<<"]"<<std::endl;
}

void DataOutputStream::writePlane(const osg::Plane& v)
{
    writeFloat(v[0]);
    writeFloat(v[1]);
    writeFloat(v[2]);
    writeFloat(v[3]);
    
    if (_verboseOutput) std::cout<<"read/writePlane() ["<<v<<"]"<<std::endl;
}

void DataOutputStream::writeVec4ub(const osg::Vec4ub& v){
    writeChar(v.r());
    writeChar(v.g());
    writeChar(v.b());
    writeChar(v.a());
    
    if (_verboseOutput) std::cout<<"read/writeVec4ub() ["<<v<<"]"<<std::endl;
}

void DataOutputStream::writeVec2b(const osg::Vec2b& v){
    writeChar(v.r());
    writeChar(v.g());    

    if (_verboseOutput) std::cout<<"read/writeVec2b() ["<<v<<"]"<<std::endl;
}

void DataOutputStream::writeVec3b(const osg::Vec3b& v){
    writeChar(v.r());
    writeChar(v.g());
    writeChar(v.b());

    if (_verboseOutput) std::cout<<"read/writeVec3b() ["<<v<<"]"<<std::endl;
}

void DataOutputStream::writeVec4b(const osg::Vec4b& v){
    writeChar(v.r());
    writeChar(v.g());
    writeChar(v.b());
    writeChar(v.a());

    if (_verboseOutput) std::cout<<"read/writeVec4b() ["<<v<<"]"<<std::endl;
}

void DataOutputStream::writeQuat(const osg::Quat& q){
    writeFloat(q.x());
    writeFloat(q.y());
    writeFloat(q.z());
    writeFloat(q.w());
    
    if (_verboseOutput) std::cout<<"read/writeQuat() ["<<q<<"]"<<std::endl;
}

void DataOutputStream::writeBinding(osg::Geometry::AttributeBinding b){
    switch(b){
        case osg::Geometry::BIND_OFF:                writeChar((char) 0); break;
        case osg::Geometry::BIND_OVERALL:            writeChar((char) 1); break;
        case osg::Geometry::BIND_PER_PRIMITIVE:        writeChar((char) 2); break;
        case osg::Geometry::BIND_PER_PRIMITIVE_SET:    writeChar((char) 3); break;
        case osg::Geometry::BIND_PER_VERTEX:        writeChar((char) 4); break;
        default: throw Exception("Unknown binding in DataOutputStream::writeBinding()");
    }
    
    if (_verboseOutput) std::cout<<"read/writeBinding() ["<<b<<"]"<<std::endl;
}

void DataOutputStream::writeArray(const osg::Array* a){
    switch(a->getType()){
        case osg::Array::IntArrayType: 
            writeChar((char)0);
            writeIntArray(static_cast<const osg::IntArray*>(a));
            break;
        case osg::Array::UByteArrayType:
            writeChar((char)1);
            writeUByteArray(static_cast<const osg::UByteArray*>(a));
            break;
        case osg::Array::UShortArrayType:
            writeChar((char)2);
            writeUShortArray(static_cast<const osg::UShortArray*>(a));
            break;
        case osg::Array::UIntArrayType:
            writeChar((char)3);
            writeUIntArray(static_cast<const osg::UIntArray*>(a));
            break;
        case osg::Array::Vec4ubArrayType:
            writeChar((char)4);
            writeVec4ubArray(static_cast<const osg::Vec4ubArray*>(a));
            break;
        case osg::Array::FloatArrayType:
            writeChar((char)5);
            writeFloatArray(static_cast<const osg::FloatArray*>(a));
            break;
        case osg::Array::Vec2ArrayType: 
            writeChar((char)6);
            writeVec2Array(static_cast<const osg::Vec2Array*>(a));
            break;
        case osg::Array::Vec3ArrayType: 
            writeChar((char)7);
            writeVec3Array(static_cast<const osg::Vec3Array*>(a));
            break;
         case osg::Array::Vec4ArrayType: 
            writeChar((char)8);
            writeVec4Array(static_cast<const osg::Vec4Array*>(a));
            break;
         case osg::Array::Vec2sArrayType:
             writeChar((char)9);
             writeVec2sArray(static_cast<const osg::Vec2sArray*>(a));
             break;
         case osg::Array::Vec3sArrayType:
             writeChar((char)10);
             writeVec3sArray(static_cast<const osg::Vec3sArray*>(a));
             break;
         case osg::Array::Vec4sArrayType:
             writeChar((char)11);
             writeVec4sArray(static_cast<const osg::Vec4sArray*>(a));
             break;
         case osg::Array::Vec2bArrayType:
             writeChar((char)12);
             writeVec2bArray(static_cast<const osg::Vec2bArray*>(a));
             break;
         case osg::Array::Vec3bArrayType:
             writeChar((char)13);
             writeVec3bArray(static_cast<const osg::Vec3bArray*>(a));
             break;
         case osg::Array::Vec4bArrayType:
             writeChar((char)14);
             writeVec4bArray(static_cast<const osg::Vec4bArray*>(a));
             break;            
        default: throw Exception("Unknown array type in DataOutputStream::writeArray()");
    }
}


void DataOutputStream::writeIntArray(const osg::IntArray* a)
{
    int size = a->getNumElements();
    writeInt(size);
    for(int i =0; i<size ;i++){
        writeInt(a->index(i));
    }
    
    if (_verboseOutput) std::cout<<"read/writeIntArray() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeUByteArray(const osg::UByteArray* a)
{
    int size = a->getNumElements(); 
    writeInt(size);
    for(int i =0; i<size ;i++){
        writeChar((*a)[i]);
    }
    
    if (_verboseOutput) std::cout<<"read/writeUByteArray() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeUShortArray(const osg::UShortArray* a)
{
    int size = a->getNumElements(); 
    writeInt(size);
    for(int i =0; i<size ;i++){
        writeUShort((*a)[i]);
    }
    
    if (_verboseOutput) std::cout<<"read/writeUShortArray() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeUIntArray(const osg::UIntArray* a)
{
    int size = a->getNumElements(); 
    writeInt(size);
    for(int i =0; i<size ;i++){
        writeInt((*a)[i]);
    }
    
    if (_verboseOutput) std::cout<<"read/writeUIntArray() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeVec4ubArray(const osg::Vec4ubArray* a)
{
    int size = a->getNumElements(); 
    writeInt(size);
    for(int i =0; i<size ;i++){
        writeVec4ub((*a)[i]);
    }
    
    if (_verboseOutput) std::cout<<"read/writeVec4ubArray() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeFloatArray(const osg::FloatArray* a)
{
    int size = a->getNumElements(); 
    writeInt(size);
    for(int i =0; i<size ;i++){
        writeFloat((*a)[i]);
    }
    
    if (_verboseOutput) std::cout<<"read/writeFloatArray() ["<<size<<"]"<<std::endl;
}


void DataOutputStream::writeVec2Array(const osg::Vec2Array* a)
{
    int size = a->size();
    writeInt(size);
    for(int i=0;i<size;i++){
        writeVec2((*a)[i]);
    }
    
    if (_verboseOutput) std::cout<<"read/writeVec2Array() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeVec3Array(const osg::Vec3Array* a)
{
    int size = a->size();
    writeInt(size);
    for(int i = 0; i < size; i++){
        writeVec3((*a)[i]);
    }
    
    if (_verboseOutput) std::cout<<"read/writeVec3Array() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeVec4Array(const osg::Vec4Array* a)
{
    int size = a->size();
    writeInt(size);
    for(int i=0;i<size;i++){
        writeVec4((*a)[i]);
    }
    
    if (_verboseOutput) std::cout<<"read/writeVec4Array() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeVec2sArray(const osg::Vec2sArray* a)
{
    int size = a->getNumElements(); 
    writeInt(size);
    for(int i =0; i<size ;i++){
        writeShort((*a)[i].x());
        writeShort((*a)[i].y());
    }

    if (_verboseOutput) std::cout<<"read/writeVec2sArray() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeVec3sArray(const osg::Vec3sArray* a)
{
    int size = a->getNumElements(); 
    writeInt(size);
    for(int i =0; i<size ;i++){
        writeShort((*a)[i].x());
        writeShort((*a)[i].y());
        writeShort((*a)[i].z());
    }

    if (_verboseOutput) std::cout<<"read/writeVec3sArray() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeVec4sArray(const osg::Vec4sArray* a)
{
    int size = a->getNumElements(); 
    writeInt(size);
    for(int i =0; i<size ;i++){
        writeShort((*a)[i].x());
        writeShort((*a)[i].y());
        writeShort((*a)[i].z());
        writeShort((*a)[i].w());
    }

    if (_verboseOutput) std::cout<<"read/writeVec4sArray() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeVec2bArray(const osg::Vec2bArray* a)
{
    int size = a->getNumElements(); 
    writeInt(size);
    for(int i =0; i<size ;i++){
        writeVec2b((*a)[i]);
    }

    if (_verboseOutput) std::cout<<"read/writeVec2bArray() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeVec3bArray(const osg::Vec3bArray* a)
{
    int size = a->getNumElements(); 
    writeInt(size);
    for(int i =0; i<size ;i++){
        writeVec3b((*a)[i]);
    }

    if (_verboseOutput) std::cout<<"read/writeVec3bArray() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeVec4bArray(const osg::Vec4bArray* a)
{
    int size = a->getNumElements(); 
    writeInt(size);
    for(int i =0; i<size ;i++){
        writeVec4b((*a)[i]);
    }

    if (_verboseOutput) std::cout<<"read/writeVec4bArray() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeMatrixf(const osg::Matrixf& mat)
{
    for(int r=0;r<4;r++)
    {
        for(int c=0;c<4;c++)
        {
            writeFloat(mat(r,c));
        }
    }
    
    if (_verboseOutput) std::cout<<"read/writeMatrix() ["<<mat<<"]"<<std::endl;
}

void DataOutputStream::writeMatrixd(const osg::Matrixd& mat)
{
    for(int r=0;r<4;r++)
    {
        for(int c=0;c<4;c++)
        {
            writeDouble(mat(r,c));
        }
    }
    
    if (_verboseOutput) std::cout<<"read/writeMatrix() ["<<mat<<"]"<<std::endl;
}


void DataOutputStream::writeStateSet(const osg::StateSet* stateset)
{
    StateSetMap::iterator itr = _stateSetMap.find(stateset);
    if (itr!=_stateSetMap.end())
    {
        // Id already exists so just write ID.
        writeInt(itr->second);

        if (_verboseOutput) std::cout<<"read/writeStateSet() ["<<itr->second<<"]"<<std::endl;
    }
    else
    {
        // id doesn't exist so create a new ID and
        // register the stateset.

        int id = _stateSetMap.size();
        _stateSetMap[stateset] = id;

        // write the id.
        writeInt(id);

        // write the stateset.
        ((ive::StateSet*)(stateset))->write(this);

        if (_verboseOutput) std::cout<<"read/writeStateSet() ["<<id<<"]"<<std::endl;

    }
}

void DataOutputStream::writeStateAttribute(const osg::StateAttribute* attribute)
{
    StateAttributeMap::iterator itr = _stateAttributeMap.find(attribute);
    if (itr!=_stateAttributeMap.end())
    {
        // Id already exists so just write ID.
        writeInt(itr->second);
        if (_verboseOutput) std::cout<<"read/writeStateAttribute() ["<<itr->second<<"]"<<std::endl;
    }
    else
    {
        // id doesn't exist so create a new ID and
        // register the stateset.

        int id = _stateAttributeMap.size();
        _stateAttributeMap[attribute] = id;

        // write the id.
        writeInt(id);

        // write the stateset.
        if(dynamic_cast<const osg::AlphaFunc*>(attribute)){
            ((ive::AlphaFunc*)(attribute))->write(this);
        }
        else if(dynamic_cast<const osg::BlendFunc*>(attribute)){
            ((ive::BlendFunc*)(attribute))->write(this);
        }
        else if(dynamic_cast<const osg::Depth*>(attribute)){
            ((ive::Depth*)(attribute))->write(this);
        }
        else if(dynamic_cast<const osg::Viewport*>(attribute)){
            ((ive::Viewport*)(attribute))->write(this);
        }
        else if(dynamic_cast<const osg::Scissor*>(attribute)){
            ((ive::Scissor*)(attribute))->write(this);
        }
        // This is a Material
        else if(dynamic_cast<const osg::Material*>(attribute)){
            ((ive::Material*)(attribute))->write(this);
        }
        // This is a CullFace
        else if(dynamic_cast<const osg::CullFace*>(attribute)){
            ((ive::CullFace*)(attribute))->write(this);
        }
        // this is a Cliplane
        else if(dynamic_cast<const osg::ClipPlane*>(attribute)){
            ((ive::ClipPlane*)(attribute))->write(this);
        }
        // This is a PolygonOffset
        else if(dynamic_cast<const osg::PolygonOffset*>(attribute)){
            ((ive::PolygonOffset*)(attribute))->write(this);
        }
        // This is a PolygonMode
        else if(dynamic_cast<const osg::PolygonMode*>(attribute)){
            ((ive::PolygonMode*)(attribute))->write(this);
        }
        else if(dynamic_cast<const osg::ShadeModel*>(attribute)){
            ((ive::ShadeModel*)(attribute))->write(this);
        }
        else if(dynamic_cast<const osg::Point*>(attribute)){
            ((ive::Point*)(attribute))->write(this);
        }
        else if(dynamic_cast<const osg::LineWidth*>(attribute)){
            ((ive::LineWidth*)(attribute))->write(this);
        }
        else if(dynamic_cast<const osg::LineWidth*>(attribute)){
            ((ive::LineWidth*)(attribute))->write(this);
        }
        // This is a Texture1D
        else if(dynamic_cast<const osg::Texture1D*>(attribute)){
            ((ive::Texture1D*)(attribute))->write(this);
        }
        // This is a Texture2D
        else if(dynamic_cast<const osg::Texture2D*>(attribute)){
            ((ive::Texture2D*)(attribute))->write(this);
        }
        // This is a Texture2D
        else if(dynamic_cast<const osg::Texture3D*>(attribute)){
            ((ive::Texture3D*)(attribute))->write(this);
        }
        // This is a TextureCubeMap
        else if(dynamic_cast<const osg::TextureCubeMap*>(attribute)){
            ((ive::TextureCubeMap*)(attribute))->write(this);
        }
        // This is a TextureRectangle
        else if(dynamic_cast<const osg::TextureRectangle*>(attribute)){
            ((ive::TextureRectangle*)(attribute))->write(this);
        }
        // This is a TexEnv
        else if(dynamic_cast<const osg::TexEnv*>(attribute)){
            ((ive::TexEnv*)(attribute))->write(this);
        }
        // This is a TexEnvCombine
        else if(dynamic_cast<const osg::TexEnvCombine*>(attribute)){
            ((ive::TexEnvCombine*)(attribute))->write(this);
        }
        // This is a TexGen
        else if(dynamic_cast<const osg::TexGen*>(attribute)){
            ((ive::TexGen*)(attribute))->write(this);
        }
        // This is a TexMat
        else if(dynamic_cast<const osg::TexMat*>(attribute)){
            ((ive::TexMat*)(attribute))->write(this);
        }
        // This is a FragmentProgram
        else if(dynamic_cast<const osg::FragmentProgram*>(attribute)){
            ((ive::FragmentProgram*)(attribute))->write(this);
        }
        // This is a VertexProgram
        else if(dynamic_cast<const osg::VertexProgram*>(attribute)){
            ((ive::VertexProgram*)(attribute))->write(this);
        }
        // This is a LightModel
        else if(dynamic_cast<const osg::LightModel*>(attribute)){
            ((ive::LightModel*)(attribute))->write(this);
        }
        // This is a FrontFace
        else if(dynamic_cast<const osg::FrontFace*>(attribute)){
            ((ive::FrontFace*)(attribute))->write(this);
        }
        // This is a FrontFace
        else if(dynamic_cast<const osg::Program*>(attribute)){
            ((ive::Program*)(attribute))->write(this);
        }

        else{
            std::string className = attribute->className();
            throw Exception(std::string("StateSet::write(): Unknown StateAttribute: ").append(className));
        }
        if (_verboseOutput) std::cout<<"read/writeStateAttribute() ["<<id<<"]"<<std::endl;
    }
}

void DataOutputStream::writeUniform(const osg::Uniform* uniform)
{
    UniformMap::iterator itr = _uniformMap.find(uniform);
    if (itr!=_uniformMap.end())
    {
        // Id already exists so just write ID.
        writeInt(itr->second);

        if (_verboseOutput) std::cout<<"read/writeUniform() ["<<itr->second<<"]"<<std::endl;
    }
    else
    {
        // id doesn't exist so create a new ID and
        // register the uniform.

        int id = _uniformMap.size();
        _uniformMap[uniform] = id;

        // write the id.
        writeInt(id);

        // write the stateset.
        ((ive::Uniform*)(uniform))->write(this);

        if (_verboseOutput) std::cout<<"read/writeUniform() ["<<id<<"]"<<std::endl;

    }
}

void DataOutputStream::writeShader(const osg::Shader* shader)
{
    ShaderMap::iterator itr = _shaderMap.find(shader);
    if (itr!=_shaderMap.end())
    {
        // Id already exists so just write ID.
        writeInt(itr->second);

        if (_verboseOutput) std::cout<<"read/writeShader() ["<<itr->second<<"]"<<std::endl;
    }
    else
    {
        // id doesn't exist so create a new ID and
        // register the shader.

        int id = _shaderMap.size();
        _shaderMap[shader] = id;

        // write the id.
        writeInt(id);

        // write the stateset.
        ((ive::Shader*)(shader))->write(this);

        if (_verboseOutput) std::cout<<"read/writeShader() ["<<id<<"]"<<std::endl;

    }
}

void DataOutputStream::writeDrawable(const osg::Drawable* drawable)
{
    DrawableMap::iterator itr = _drawableMap.find(drawable);
    if (itr!=_drawableMap.end())
    {
        // Id already exists so just write ID.
        writeInt(itr->second);

        if (_verboseOutput) std::cout<<"read/writeDrawable() ["<<itr->second<<"]"<<std::endl;
    }
    else
    {
        // id doesn't exist so create a new ID and
        // register the stateset.

        int id = _drawableMap.size();
        _drawableMap[drawable] = id;

        // write the id.
        writeInt(id);
        
        if(dynamic_cast<const osg::Geometry*>(drawable))
            ((ive::Geometry*)(drawable))->write(this);
        else if(dynamic_cast<const osg::ShapeDrawable*>(drawable))
            ((ive::ShapeDrawable*)(drawable))->write(this);
        else if(dynamic_cast<const osgText::Text*>(drawable))
            ((ive::Text*)(drawable))->write(this);
        else
        {
            throw Exception("Unknown drawable in DataOutputStream::writeDrawable()");
        }
        if (_verboseOutput) std::cout<<"read/writeDrawable() ["<<id<<"]"<<std::endl;
    }
}

void DataOutputStream::writeShape(const osg::Shape* shape)
{
    ShapeMap::iterator itr = _shapeMap.find(shape);
    if (itr!=_shapeMap.end())
    {
        // Id already exists so just write ID.
        writeInt(itr->second);

        if (_verboseOutput) std::cout<<"read/writeShape() ["<<itr->second<<"]"<<std::endl;
    }
    else
    {
        // id doesn't exist so create a new ID and
        // register the stateset.

        int id = _shapeMap.size();
        _shapeMap[shape] = id;

        // write the id.
        writeInt(id);
        
        if(dynamic_cast<const osg::Sphere*>(shape))
            ((ive::Sphere*)(shape))->write(this);
        else if(dynamic_cast<const osg::Box*>(shape))
            ((ive::Box*)(shape))->write(this);
        else if(dynamic_cast<const osg::Cone*>(shape))
            ((ive::Cone*)(shape))->write(this);
        else if(dynamic_cast<const osg::Cylinder*>(shape))
            ((ive::Cylinder*)(shape))->write(this);
        else if(dynamic_cast<const osg::Capsule*>(shape))
            ((ive::Capsule*)(shape))->write(this);
        else if(dynamic_cast<const osg::HeightField*>(shape))
            ((ive::HeightField*)(shape))->write(this);
        else
        {
            throw Exception("Unknown shape in DataOutputStream::writeShape()");
        }
        if (_verboseOutput) std::cout<<"read/writeShape() ["<<id<<"]"<<std::endl;
    }
}

void DataOutputStream::writeNode(const osg::Node* node)
{
    NodeMap::iterator itr = _nodeMap.find(node);
    if (itr!=_nodeMap.end())
    {
        // Id already exists so just write ID.
        writeInt(itr->second);

        if (_verboseOutput) std::cout<<"read/writeNode() ["<<itr->second<<"]"<<std::endl;
    }
    else
    {
        // id doesn't exist so create a new ID and
        // register the stateset.

        int id = _nodeMap.size();
        _nodeMap[node] = id;

        // write the id.
        writeInt(id);

        // this follow code *really* should use a NodeVisitor... Robert Osfield August 2003.

        if(dynamic_cast<const osg::MatrixTransform*>(node)){
            ((ive::MatrixTransform*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::CameraNode*>(node)){
            ((ive::CameraNode*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::CameraView*>(node)){
            ((ive::CameraView*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::PositionAttitudeTransform*>(node)){
            ((ive::PositionAttitudeTransform*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::AutoTransform*>(node)){
            ((ive::AutoTransform*)(node))->write(this);
        }
        else if(dynamic_cast<const osgSim::DOFTransform*>(node)){
            ((ive::DOFTransform*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::LightSource*>(node)){
            ((ive::LightSource*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::TexGenNode*>(node)){
            ((ive::TexGenNode*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::ClipNode*>(node)){
            ((ive::ClipNode*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::Sequence*>(node)){
            ((ive::Sequence*)(node))->write(this);
        }
        else if(dynamic_cast<const osgSim::Impostor*>(node)){
            ((ive::Impostor*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::PagedLOD*>(node)){
            ((ive::PagedLOD*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::LOD*>(node)){
            ((ive::LOD*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::Switch*>(node)){
            ((ive::Switch*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::CoordinateSystemNode*>(node)){
            ((ive::CoordinateSystemNode*)(node))->write(this);
        }
        else if(dynamic_cast<const osgSim::MultiSwitch*>(node)){
            ((ive::MultiSwitch*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::OccluderNode*>(node)){
            ((ive::OccluderNode*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::Transform*>(node)){
            ((ive::Transform*)(node))->write(this);
        }
        else if(dynamic_cast<const osgSim::VisibilityGroup*>(node)){
            ((ive::VisibilityGroup*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::ProxyNode*>(node)){
            ((ive::ProxyNode*)(node))->write(this);
        }
        else if(dynamic_cast<const osgFX::MultiTextureControl*>(node)){
            ((ive::MultiTextureControl*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::Group*>(node)){
            ((ive::Group*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::Billboard*>(node)){
            ((ive::Billboard*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::Geode*>(node)){
            ((ive::Geode*)(node))->write(this);
        }
        else if(dynamic_cast<const osgSim::LightPointNode*>(node)){
            ((ive::LightPointNode*)(node))->write(this);
        }
        else
            throw Exception("Unknown node in Group::write()");

        if (_verboseOutput) std::cout<<"read/writeNode() ["<<id<<"]"<<std::endl;
    }
}

void DataOutputStream::writeImage(IncludeImageMode mode, osg::Image *image)
{
    switch(mode) {
        case IMAGE_INCLUDE_DATA:
            // Include image data in stream
            writeBool(image!=0);
            if(image)
                ((ive::Image*)image)->write(this);
            break;
        case IMAGE_REFERENCE_FILE:
            // Only include image name in stream
            if (image && !(image->getFileName().empty())){
                writeString(image->getFileName());
            }
            else{ 
                writeString("");
            }    
            break;
        case IMAGE_INCLUDE_FILE:
            // Include image file in stream
            if(image && !(image->getFileName().empty())) {
                std::string fullPath = osgDB::findDataFile(image->getFileName(),_options.get());
                std::ifstream infile(fullPath.c_str(), std::ios::in | std::ios::binary);
                if(infile) {

                    //Write filename
                    writeString(image->getFileName());

                    //Get size of file
                    infile.seekg(0,std::ios::end);
                    int size = infile.tellg();
                    infile.seekg(0,std::ios::beg);

                    //Write file size
                    writeInt(size);

                    //Read file data
                    char *buffer = new char[size];
                    infile.read(buffer,size);

                    //Write file data
                    writeCharArray(buffer,size);

                    //Delete buffer
                    delete [] buffer;

                    //Close file
                    infile.close();
                   
                } else {
                    writeString("");
                    writeInt(0);
                }
            }
            else{ 
                writeString("");
                writeInt(0);
            }    
            break;
        case IMAGE_COMPRESS_DATA:
            if(image)
            {
                //Get ReaderWriter for jpeg images
                
                std::string extension = "png";
                if (image->getPixelFormat()==GL_RGB) extension = "jpg";
                
                osgDB::ReaderWriter* writer = osgDB::Registry::instance()->getReaderWriterForExtension(extension);

                if(writer)
                {
                    //Attempt to write the image to an output stream.
                    //The reason this isn't performed directly on the internal _ostream
                    //is because the writer might perform seek operations which could
                    //corrupt the output stream.
                    std::stringstream outputStream;
                    osgDB::ReaderWriter::WriteResult wr;
                    wr = writer->writeImage(*image,outputStream,_options.get());

                    if(wr.success()) {

                        //Write file format. Do this for two reasons:
                        // 1 - Same code can be used to read in as with IMAGE_INCLUDE_FILE mode
                        // 2 - Maybe in future version user can specify which format to use
                        writeString(std::string(".")+extension); //Need to add dot so osgDB::getFileExtension will work
                        
                        //Write size of stream
                        int size = outputStream.tellp();
                        writeInt(size);
                       
                        //Write stream
                        writeCharArray(outputStream.str().c_str(),size);

                        return;
                    }
                }
            }
            //Image compression failed, write blank data
            writeString("");
            writeInt(0);
            break;
        default:
            throw Exception("DataOutputStream::writeImage(): Invalid IncludeImageMode value.");
            break;
    }
}
