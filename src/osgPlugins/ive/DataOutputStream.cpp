/**********************************************************************
 *
 *    FILE:            DataOutputStream.cpp
 *
 *    DESCRIPTION:    Implements methods to write simpel datatypes to an
 *                    output stream.
 *
 *    CREATED BY:        Rune Schmidt Jensen
 *
 *    HISTORY:        Created 11.03.2003
 *
 *    Copyright 2003 VR-C
 **********************************************************************/

#include "DataOutputStream.h"
#include "Exception.h"

#include "StateSet.h"
#include "BlendFunc.h"
#include "Material.h"
#include "CullFace.h"
#include "PolygonOffset.h"
#include "ShadeModel.h"
#include "Point.h"
#include "Texture2D.h"
#include "TextureCubeMap.h"
#include "TexEnv.h"
#include "TexEnvCombine.h"
#include "TexGen.h"

#include "Group.h"
#include "MatrixTransform.h"
#include "Geode.h"
#include "LightSource.h"
#include "Billboard.h"
#include "Sequence.h"
#include "LOD.h"
#include "PagedLOD.h"
//#include "ViewPoint.h"
#include "PositionAttitudeTransform.h"
#include "Transform.h"
#include "Switch.h"
#include "OccluderNode.h"
#include "Impostor.h"

#include "Geometry.h"

using namespace ive;

DataOutputStream::DataOutputStream(std::ostream * ostream)
{
    _verboseOutput = false;

    _includeImageData= true;
    _ostream = ostream;
    if(!_ostream)
        throw Exception("DataOutputStream::DataOutputStream(): null pointer exception in argument.");
    writeInt(VERSION);
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

void DataOutputStream::writeUShort(unsigned short s){
    _ostream->write((char*)&s, SHORTSIZE);
    
    if (_verboseOutput) std::cout<<"read/writeUShort() ["<<s<<"]"<<std::endl;
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

void DataOutputStream::writeUByte4(const osg::UByte4& v){
    writeChar(v.r());
    writeChar(v.g());
    writeChar(v.b());
    writeChar(v.a());
    
    if (_verboseOutput) std::cout<<"read/writeUByte4() ["<<v<<"]"<<std::endl;
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
        case osg::Array::UByte4ArrayType:
            writeChar((char)4);
            writeUByte4Array(static_cast<const osg::UByte4Array*>(a));
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

void DataOutputStream::writeUByte4Array(const osg::UByte4Array* a)
{
    int size = a->getNumElements(); 
    writeInt(size);
    for(int i =0; i<size ;i++){
        writeUByte4((*a)[i]);
    }
    
    if (_verboseOutput) std::cout<<"read/writeUByte4Array() ["<<size<<"]"<<std::endl;
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

void DataOutputStream::writeMatrix(const osg::Matrix& mat)
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
        if(dynamic_cast<const osg::BlendFunc*>(attribute)){
            ((ive::BlendFunc*)(attribute))->write(this);
        }
        // This is a Material
        else if(dynamic_cast<const osg::Material*>(attribute)){
            ((ive::Material*)(attribute))->write(this);
        }
        // This is a CullFace
        else if(dynamic_cast<const osg::CullFace*>(attribute)){
            ((ive::CullFace*)(attribute))->write(this);
        }
        // This is a PolygonOffset
        else if(dynamic_cast<const osg::PolygonOffset*>(attribute)){
            ((ive::PolygonOffset*)(attribute))->write(this);
        }
        else if(dynamic_cast<const osg::ShadeModel*>(attribute)){
            ((ive::ShadeModel*)(attribute))->write(this);
        }
        else if(dynamic_cast<const osg::Point*>(attribute)){
            ((ive::Point*)(attribute))->write(this);
        }
        // This is a Texture2D
        else if(dynamic_cast<const osg::Texture2D*>(attribute)){
            ((ive::Texture2D*)(attribute))->write(this);
        }
        // This is a TextureCubeMap
        else if(dynamic_cast<const osg::TextureCubeMap*>(attribute)){
            ((ive::TextureCubeMap*)(attribute))->write(this);
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
        else{
            std::string className = attribute->className();
            throw Exception(std::string("StateSet::write(): Unknown StateAttribute: ").append(className));
        }
        if (_verboseOutput) std::cout<<"read/writeStateAttribute() ["<<id<<"]"<<std::endl;
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
        else{
            throw Exception("Unknown drawable in DataOutputStream::writeDrawable()");
        }
        if (_verboseOutput) std::cout<<"read/writeDrawable() ["<<id<<"]"<<std::endl;
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
//         else if(dynamic_cast<osgfIVE::ViewPoint*>(node)){
//             ((ive::ViewPoint*)(node))->write(this);
//         }
        else if(dynamic_cast<const osg::PositionAttitudeTransform*>(node)){
            ((ive::PositionAttitudeTransform*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::LightSource*>(node)){
            ((ive::LightSource*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::Sequence*>(node)){
            ((ive::Sequence*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::Impostor*>(node)){
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
        else if(dynamic_cast<const osg::OccluderNode*>(node)){
            ((ive::OccluderNode*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::Transform*>(node)){
            ((ive::Transform*)(node))->write(this);
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
        else
            throw Exception("Unknown node in Group::write()");

        if (_verboseOutput) std::cout<<"read/writeNode() ["<<id<<"]"<<std::endl;
    }
}
