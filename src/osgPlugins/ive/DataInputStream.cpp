/**********************************************************************
 *
 *    FILE:            DataInputStream.cpp
 *
 *    DESCRIPTION:    Implements methods to read simpel datatypes from an
 *                    input stream.
 *
 *    CREATED BY:        Rune Schmidt Jensen
 *
 *    HISTORY:        Created 11.03.2003
 *
 *    Copyright 2003 VR-C
 **********************************************************************/

#include "DataInputStream.h"
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

#include <osgDB/ReadFile>

using namespace ive;
using namespace std;

DataInputStream::DataInputStream(std::istream* istream)
{
    _verboseOutput = true;

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
    char c;
    _istream->read(&c, CHARSIZE);
    
    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readBool(): Failed to read boolean value.");

    if (_verboseOutput) std::cout<<"read/writeBool() ["<<(int)c<<"]"<<std::endl;
    
    return c!=0;
}

char DataInputStream::readChar(){
    char c;
    _istream->read(&c, CHARSIZE);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readChar(): Failed to read char value.");

    if (_verboseOutput) std::cout<<"read/writeChar() ["<<(int)c<<"]"<<std::endl;
    
    return c;
}

unsigned short DataInputStream::readUShort(){
    unsigned short s;
    _istream->read((char*)&s, SHORTSIZE);
    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readUShort(): Failed to read unsigned short value.");

    if (_verboseOutput) std::cout<<"read/writeUShort() ["<<s<<"]"<<std::endl;
    
    return s;
}

unsigned int DataInputStream::readUInt(){
    unsigned int s;
    _istream->read((char*)&s, INTSIZE);
    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readUInt(): Failed to read unsigned int value.");

    if (_verboseOutput) std::cout<<"read/writeUInt() ["<<s<<"]"<<std::endl;
    
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

    if (_verboseOutput) std::cout<<"read/writeInt() ["<<i<<"]"<<std::endl;
    
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

    if (_verboseOutput) std::cout<<"read/writeFloat() ["<<f<<"]"<<std::endl;
    
    return f;
}

long DataInputStream::readLong(){
    long l;
    _istream->read((char*)&l, LONGSIZE);
    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readLong(): Failed to read long value.");

    if (_verboseOutput) std::cout<<"read/writeLong() ["<<l<<"]"<<std::endl;
    
    return l;
}

double DataInputStream::readDouble(){
    double d;
    _istream->read((char*)&d, DOUBLESIZE);
    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readDouble(): Failed to read double value.");

    if (_verboseOutput) std::cout<<"read/writeDouble() ["<<d<<"]"<<std::endl;
    
    return d;
}

std::string DataInputStream::readString(){
    std::string s;
    int size = readInt();
    s.resize(size);
    _istream->read((char*)s.c_str(), size);
    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readString(): Failed to read string value.");

    if (_verboseOutput) std::cout<<"read/writeString() ["<<s<<"]"<<std::endl;
    
    return s;
}

void DataInputStream::readCharArray(char* data, int size){
    _istream->read(data, size);
    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readCharArray(): Failed to read char value.");

    if (_verboseOutput) std::cout<<"read/writeCharArray() ["<<data<<"]"<<std::endl;
}

osg::Vec2 DataInputStream::readVec2()
{
    osg::Vec2 v;
    v.x()=readFloat();
    v.y()=readFloat();

    if (_verboseOutput) std::cout<<"read/writeVec2() ["<<v<<"]"<<std::endl;
    
    return v;
}

osg::Vec3 DataInputStream::readVec3(){
    osg::Vec3 v;
    v.x()=readFloat();
    v.y()=readFloat();
    v.z()=readFloat();

    if (_verboseOutput) std::cout<<"read/writeVec3() ["<<v<<"]"<<std::endl;

    return v;
}

osg::Vec4 DataInputStream::readVec4(){
    osg::Vec4 v;
    v.x()=readFloat();
    v.y()=readFloat();
    v.z()=readFloat();
    v.w()=readFloat();

    if (_verboseOutput) std::cout<<"read/writeVec4() ["<<v<<"]"<<std::endl;
    
    return v;
}

osg::UByte4 DataInputStream::readUByte4(){
    osg::UByte4 v;
    v.r()=readChar();
    v.g()=readChar();
    v.b()=readChar();
    v.a()=readChar();

    if (_verboseOutput) std::cout<<"read/writeUByte4() ["<<v<<"]"<<std::endl;
    
    return v;
}

osg::Quat DataInputStream::readQuat(){
    osg::Quat q;
    q.set(readFloat(), readFloat(), readFloat(), readFloat());

    if (_verboseOutput) std::cout<<"read/writeQuat() ["<<q<<"]"<<std::endl;
    
    return q;
}




osg::Geometry::AttributeBinding DataInputStream::readBinding(){
    char c = readChar();

    if (_verboseOutput) std::cout<<"read/writeBinding() ["<<(int)c<<"]"<<std::endl;
    
    switch((int)c){
        case 0:    return osg::Geometry::BIND_OFF;
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
        case 6:    return readVec2Array();
        case 7:    return readVec3Array();
        case 8:    return readVec4Array();
        default: throw Exception("Unknown array type in DataInputStream::readArray()");
    }
}

osg::IntArray* DataInputStream::readIntArray(){
    int size = readInt();
    osg::IntArray* a = new osg::IntArray(size);
    
    _istream->read((char*)&((*a)[0]), INTSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readIntArray(): Failed to read Int array.");

    if (_verboseOutput) std::cout<<"read/writeIntArray() ["<<size<<"]"<<std::endl;  

    return a;
}

osg::UByteArray* DataInputStream::readUByteArray(){
    int size = readInt();
    osg::UByteArray* a = new osg::UByteArray(size);

    _istream->read((char*)&((*a)[0]), CHARSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readUByteArray(): Failed to read UByte array.");

    if (_verboseOutput) std::cout<<"read/writeUByteArray() ["<<size<<"]"<<std::endl;
    
    return a;
}

osg::UShortArray* DataInputStream::readUShortArray(){
    int size = readInt();
    osg::UShortArray* a = new osg::UShortArray(size);

    _istream->read((char*)&((*a)[0]), SHORTSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readUShortArray(): Failed to read UShort array.");

    if (_verboseOutput) std::cout<<"read/writeUShortArray() ["<<size<<"]"<<std::endl;
    
    return a;
}

osg::UIntArray* DataInputStream::readUIntArray(){
    int size = readInt();
    osg::UIntArray* a = new osg::UIntArray(size);

    _istream->read((char*)&((*a)[0]), INTSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readUIntArray(): Failed to read UInt array.");

    if (_verboseOutput) std::cout<<"read/writeUIntArray() ["<<size<<"]"<<std::endl;
    
    return a;
}

osg::UByte4Array* DataInputStream::readUByte4Array(){
    int size = readInt();
    osg::UByte4Array* a = new osg::UByte4Array(size);

    _istream->read((char*)&((*a)[0]), INTSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readUbyte4Array(): Failed to read UByte4 array.");

    if (_verboseOutput) std::cout<<"read/writeUByte4Array() ["<<size<<"]"<<std::endl;
    
    return a;
}

osg::FloatArray* DataInputStream::readFloatArray(){
    int size = readInt();
    osg::FloatArray* a = new osg::FloatArray(size);
    
    _istream->read((char*)&((*a)[0]), FLOATSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readFloatArray(): Failed to read float array.");

    if (_verboseOutput) std::cout<<"read/writeFloatArray() ["<<size<<"]"<<std::endl;
    
    return a;
}

osg::Vec2Array* DataInputStream::readVec2Array(){
    int size = readInt();
    osg::Vec2Array* a = new osg::Vec2Array(size);
    
    _istream->read((char*)&((*a)[0]), FLOATSIZE*2*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readVec2Array(): Failed to read Vec2 array.");

    if (_verboseOutput) std::cout<<"read/writeVec2Array() ["<<size<<"]"<<std::endl;
    
    return a;
}

osg::Vec3Array* DataInputStream::readVec3Array(){
    int size = readInt();
    osg::Vec3Array* a = new osg::Vec3Array(size);

    _istream->read((char*)&((*a)[0]), FLOATSIZE*3*size);
    
    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readVec3Array(): Failed to read Vec3 array.");

    if (_verboseOutput) std::cout<<"read/writeVec3Array() ["<<size<<"]"<<std::endl;
    

    return a;
}

osg::Vec4Array* DataInputStream::readVec4Array(){
    int size = readInt();
    osg::Vec4Array* a = new osg::Vec4Array(size);

    _istream->read((char*)&((*a)[0]), FLOATSIZE*4*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readVec4Array(): Failed to read Vec4 array.");

    if (_verboseOutput) std::cout<<"read/writeVec4Array() ["<<size<<"]"<<std::endl;
    
    return a;
}

osg::Matrix DataInputStream::readMatrix()
{
    osg::Matrix mat;
    for(int r=0;r<4;r++)
    {
        for(int c=0;c<4;c++)
        {
            mat(r,c) = readDouble();
        }
    }

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readMatrix(): Failed to read Matrix array.");

    if (_verboseOutput) std::cout<<"read/writeMatrix() ["<<mat<<"]"<<std::endl;
    

    return mat;
}

osg::Image* DataInputStream::readImage(std::string filename)
{
    // If image is already read and in list 
    // then just return pointer to this.
    ImageMap::iterator mitr=_imageMap.find(filename);
    if (mitr!=_imageMap.end()) mitr->second.get();
        
    // Image is not in list. 
    // Read it from disk, 
    osg::Image* image = osgDB::readImageFile(filename.c_str());
        
    // add it to the imageList,
    _imageMap[filename] = image;
    // and return image pointer.

    if (_verboseOutput) std::cout<<"read/writeImage() ["<<image<<"]"<<std::endl;
    
    return image;
}

osg::StateSet* DataInputStream::readStateSet()
{
    // Read statesets unique ID.
    int id = readInt();
    // See if stateset is already in the list.
    StateSetMap::iterator itr= _statesetMap.find(id);
    if (itr!=_statesetMap.end()) return itr->second.get();

    // StateSet is not in list.
    // Create a new stateset,
    osg::StateSet* stateset = new osg::StateSet();

    // read its properties from stream
    ((ive::StateSet*)(stateset))->read(this);
        
    // and add it to the stateset map,
    _statesetMap[id] = stateset;
        

    if (_verboseOutput) std::cout<<"read/writeStateSet() ["<<id<<"]"<<std::endl;
    
    return stateset;
}

osg::StateAttribute* DataInputStream::readStateAttribute()
{
    // Read stateattributes unique ID.
    int id = readInt();
    // See if stateattribute is already in the list.
    StateAttributeMap::iterator itr= _stateAttributeMap.find(id);
    if (itr!=_stateAttributeMap.end()) return itr->second.get();

    // stateattribute is not in list.
    // Create a new stateattribute,


    osg::StateAttribute* attribute;
    int attributeID = peekInt();
    if(attributeID == IVEBLENDFUNC){
        attribute = new osg::BlendFunc();
        ((ive::BlendFunc*)(attribute))->read(this);
    }
    else if(attributeID == IVEMATERIAL){
        attribute = new osg::Material();
        ((ive::Material*)(attribute))->read(this);
    }
    else if(attributeID == IVECULLFACE){
        attribute = new osg::CullFace();
        ((ive::CullFace*)(attribute))->read(this);
    }
    else if(attributeID == IVEPOLYGONOFFSET){
        attribute = new osg::PolygonOffset();
        ((ive::PolygonOffset*)(attribute))->read(this);
    }
    else if(attributeID == IVESHADEMODEL){
        attribute = new osg::ShadeModel();
        ((ive::ShadeModel*)(attribute))->read(this);
    }
    else if(attributeID == IVEPOINT){
        attribute = new osg::Point();
        ((ive::Point*)(attribute))->read(this);
    }
    else if(attributeID == IVETEXTURE2D){
        attribute = new osg::Texture2D();
        ((ive::Texture2D*)(attribute))->read(this);
    }
    else if(attributeID == IVETEXTURECUBEMAP){
        attribute = new osg::TextureCubeMap();
        ((ive::TextureCubeMap*)(attribute))->read(this);
    }
    else if(attributeID == IVETEXENV){
        attribute = new osg::TexEnv();
        ((ive::TexEnv*)(attribute))->read(this);
    }
    else if(attributeID == IVETEXENVCOMBINE){
        attribute = new osg::TexEnvCombine();
        ((ive::TexEnvCombine*)(attribute))->read(this);
    }
    else if(attributeID == IVETEXGEN){
        attribute = new osg::TexGen();
        ((ive::TexGen*)(attribute))->read(this);
    }
    else{
        throw Exception("Unkown StateAttribute in StateSet::read()");
    }
       
    // and add it to the stateattribute map,
    _stateAttributeMap[id] = attribute;
        

    if (_verboseOutput) std::cout<<"read/writeStateAttribute() ["<<id<<"]"<<std::endl;
    
    return attribute;
}

osg::Drawable* DataInputStream::readDrawable()
{
    // Read stateattributes unique ID.
    int id = readInt();
    // See if stateattribute is already in the list.
    DrawableMap::iterator itr= _drawableMap.find(id);
    if (itr!=_drawableMap.end()) return itr->second.get();

    // stateattribute is not in list.
    // Create a new stateattribute,

    int drawableTypeID = peekInt();
    osg::Drawable* drawable;
    if(drawableTypeID == IVEGEOMETRY){
        drawable = new osg::Geometry();                
        ((Geometry*)(drawable))->read(this);
    }
    else
        throw Exception("Unknown drawable drawableTypeIDentification in Geode::read()");

       
    // and add it to the stateattribute map,
    _drawableMap[id] = drawable;
        

    if (_verboseOutput) std::cout<<"read/writeDrawable() ["<<id<<"]"<<std::endl;
    
    return drawable;
}

osg::Node* DataInputStream::readNode()
{
    // Read node unique ID.
    int id = readInt();
    // See if node is already in the list.
    NodeMap::iterator itr= _nodeMap.find(id);
    if (itr!=_nodeMap.end()) return itr->second.get();

    // stateattribute is not in list.
    // Create a new node,

    osg::Node* node;
    int nodeTypeID= peekInt();
    
    if(nodeTypeID== IVEMATRIXTRANSFORM){
        node = new osg::MatrixTransform();
        ((ive::MatrixTransform*)(node))->read(this);
    }
//             else if(nodeTypeID== IVEVIEWPOINT){
//                 node = new osgfIVE::ViewPoint();
//                 ((ive::ViewPoint*)(node))->read(this);
//             }
    else if(nodeTypeID== IVEPOSITIONATTITUDETRANSFORM){
        node = new osg::PositionAttitudeTransform();
        ((ive::PositionAttitudeTransform*)(node))->read(this);
    }
    else if(nodeTypeID== IVETRANSFORM){
        node = new osg::Transform();
        ((ive::Transform*)(node))->read(this);
    }
    else if(nodeTypeID== IVELIGHTSOURCE){
        node = new osg::LightSource();
        ((ive::LightSource*)(node))->read(this);
    }
    else if(nodeTypeID== IVESEQUENCE){
        node = new osg::Sequence();
        ((ive::Sequence*)(node))->read(this);
    }
    else if(nodeTypeID== IVELOD){
        node = new osg::LOD();
        ((ive::LOD*)(node))->read(this);
    }
    else if(nodeTypeID== IVEPAGEDLOD){
        node = new osg::PagedLOD();
        ((ive::PagedLOD*)(node))->read(this);
    }
    else if(nodeTypeID== IVESWITCH){
        node = new osg::Switch();
        ((ive::Switch*)(node))->read(this);
    }
    else if(nodeTypeID== IVEIMPOSTOR){
        node = new osg::Impostor();
        ((ive::Impostor*)(node))->read(this);
    }
    else if(nodeTypeID== IVEOCCLUDERNODE){
        node = new osg::OccluderNode();
        ((ive::OccluderNode*)(node))->read(this);
    }
    else if(nodeTypeID== IVEGROUP){
        node = new osg::Group();
        ((ive::Group*)(node))->read(this);
    }
    else if(nodeTypeID== IVEBILLBOARD){
        node = new osg::Billboard();
        ((ive::Billboard*)(node))->read(this);
    }
    else if(nodeTypeID== IVEGEODE){
        node = new osg::Geode();
        ((ive::Geode*)(node))->read(this);
    }
    else{
        throw Exception("Unknown node identification in DataInputStream::readNode()");
    }

    // and add it to the stateattribute map,
    _nodeMap[id] = node;
        

    if (_verboseOutput) std::cout<<"read/writeNode() ["<<id<<"]"<<std::endl;
    
    return node;
}
