/**********************************************************************
 *
 *    FILE:            DataInputStream.cpp
 *
 *    DESCRIPTION:    Implements methods to read simple datatypes from an
 *                    input stream.
 *
 *    CREATED BY:        Rune Schmidt Jensen
 *
 *    HISTORY:        Created 11.03.2003
 *                    Updated for texture1D by Don Burns, 27.1.2004
 *                      Updated for light model - Stan Blinov at 25 august 7512 from World Creation (7.09.2004)
 *
 *
 *    Copyright 2003 VR-C
 **********************************************************************/

#include "DataInputStream.h"
#include "StateSet.h"
#include "AlphaFunc.h"
#include "BlendFunc.h"
#include "Depth.h"
#include "Material.h"
#include "CullFace.h"
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
#include "Uniform.h"
#include "Shader.h"

#include "LightPointNode.h"
#include "MultiSwitch.h"
#include "VisibilityGroup.h"

#include "MultiTextureControl.h"

#include "Geometry.h"
#include "ShapeDrawable.h"
#include "Shape.h"

#include "Text.h"

#include <osg/Endian>
#include <osg/Notify>
#include <osg/io_utils>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>

#include <stdio.h>
#include <sstream>

using namespace ive;
using namespace std;

void DataInputStream::setOptions(const osgDB::ReaderWriter::Options* options) 
{ 
    _options = options; 

    if (_options.get())
    {
        setLoadExternalReferenceFiles(_options->getOptionString().find("noLoadExternalReferenceFiles")==std::string::npos);
        osg::notify(osg::DEBUG_INFO) << "ive::DataInputStream.setLoadExternalReferenceFiles()=" << getLoadExternalReferenceFiles() << std::endl;
    }
}

DataInputStream::DataInputStream(std::istream* istream)
{
    unsigned int endianType ;

    _loadExternalReferenceFiles = false;
    
    _verboseOutput = false;

    _istream = istream;
    _peeking = false;
    _peekValue = 0;
    _byteswap = 0;

    if(!istream){
        throw Exception("DataInputStream::DataInputStream(): null pointer exception in argument.");    
    }

    endianType = readUInt() ;
    
    if ( endianType != ENDIAN_TYPE) {
      // Make sure the file is simply swapped
      if ( endianType != OPPOSITE_ENDIAN_TYPE ) {
         throw Exception("DataInputStream::DataInputStream(): This file has an unreadable endian type.") ;
      }
      osg::notify(osg::INFO)<<"DataInputStream::DataInputStream: Reading a byteswapped file" << std::endl ;
      _byteswap = 1 ;
   }
    
    _version = readUInt();
        
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

unsigned int DataInputStream::getVersion()
{
    return( _version );
}

char DataInputStream::readChar(){
    char c;
    _istream->read(&c, CHARSIZE);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readChar(): Failed to read char value.");

    if (_verboseOutput) std::cout<<"read/writeChar() ["<<(int)c<<"]"<<std::endl;
    
    return c;
}

unsigned char DataInputStream::readUChar(){
    unsigned char c;
    _istream->read((char*)&c, CHARSIZE);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readUChar(): Failed to read unsigned char value.");

    if (_verboseOutput) std::cout<<"read/writeUChar() ["<<(int)c<<"]"<<std::endl;
    
    return c;
}

unsigned short DataInputStream::readUShort(){
    unsigned short s;
    _istream->read((char*)&s, SHORTSIZE);
    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readUShort(): Failed to read unsigned short value.");

    if (_verboseOutput) std::cout<<"read/writeUShort() ["<<s<<"]"<<std::endl;
    
    if (_byteswap) osg::swapBytes((char *)&s,SHORTSIZE);
    
    return s;
}

unsigned int DataInputStream::readUInt(){
    unsigned int s;
    _istream->read((char*)&s, INTSIZE);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readUInt(): Failed to read unsigned int value.");

    if (_byteswap) osg::swapBytes((char *)&s,INTSIZE) ;
    
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

    // comment out for time being as this check seems to eroneously cause a
    // premature exit when reading .ive files under OSX!#?:!
    // Robet Osfield, September 12th 2003.
    // if (_istream->rdstate() & _istream->failbit)
    //    throw Exception("DataInputStream::readInt(): Failed to read int value.");

    
    if (_byteswap) osg::swapBytes((char *)&i,INTSIZE) ;
    
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

    if (_byteswap) osg::swapBytes((char *)&f,FLOATSIZE) ;

    if (_verboseOutput) std::cout<<"read/writeFloat() ["<<f<<"]"<<std::endl;
    return f;
}

long DataInputStream::readLong(){
    long l;
    _istream->read((char*)&l, LONGSIZE);
    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readLong(): Failed to read long value.");

    if (_byteswap) osg::swapBytes((char *)&l,LONGSIZE) ;

    if (_verboseOutput) std::cout<<"read/writeLong() ["<<l<<"]"<<std::endl;
    return l;
}

unsigned long DataInputStream::readULong(){
    unsigned long l;
    _istream->read((char*)&l, LONGSIZE);
    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readULong(): Failed to read unsigned long value.");

    if (_byteswap) osg::swapBytes((char *)&l,LONGSIZE) ;

    if (_verboseOutput) std::cout<<"read/writeULong() ["<<l<<"]"<<std::endl;
    
    return l;
}

double DataInputStream::readDouble(){
    double d;
    _istream->read((char*)&d, DOUBLESIZE);
    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readDouble(): Failed to read double value.");

    if (_byteswap) osg::swapBytes((char *)&d,DOUBLESIZE) ;
    if (_verboseOutput) std::cout<<"read/writeDouble() ["<<d<<"]"<<std::endl;
    return d;
}

std::string DataInputStream::readString(){
    std::string s;
    int size = readInt();
    s.resize(size);
    _istream->read((char*)s.c_str(), size);
    //if (_istream->rdstate() & _istream->failbit)
    //   throw Exception("DataInputStream::readString(): Failed to read string value.");

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
osg::Vec2d DataInputStream::readVec2d()
{
    osg::Vec2d v;
    v.x()=readDouble();
    v.y()=readDouble();

    if (_verboseOutput) std::cout<<"read/writeVec2d() ["<<v<<"]"<<std::endl;
    
    return v;
}

osg::Vec3d DataInputStream::readVec3d(){
    osg::Vec3d v;
    v.x()=readDouble();
    v.y()=readDouble();
    v.z()=readDouble();

    if (_verboseOutput) std::cout<<"read/writeVec3d() ["<<v<<"]"<<std::endl;

    return v;
}

osg::Vec4d DataInputStream::readVec4d(){
    osg::Vec4d v;
    v.x()=readDouble();
    v.y()=readDouble();
    v.z()=readDouble();
    v.w()=readDouble();

    if (_verboseOutput) std::cout<<"read/writeVec4d() ["<<v<<"]"<<std::endl;
    
    return v;
}

osg::Plane DataInputStream::readPlane(){
    osg::Plane v;
    v[0]=readFloat();
    v[1]=readFloat();
    v[2]=readFloat();
    v[3]=readFloat();

    if (_verboseOutput) std::cout<<"read/writePlane() ["<<v<<"]"<<std::endl;
    
    return v;
}

osg::Vec4ub DataInputStream::readVec4ub(){
    osg::Vec4ub v;
    v.r()=readChar();
    v.g()=readChar();
    v.b()=readChar();
    v.a()=readChar();

    if (_verboseOutput) std::cout<<"read/writeVec4ub() ["<<v<<"]"<<std::endl;
    
    return v;
}

osg::Quat DataInputStream::readQuat(){
    osg::Quat q;
    q.x()=readFloat();
    q.y()=readFloat();
    q.z()=readFloat();
    q.w()=readFloat();

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
        case 4: return readVec4ubArray();
        case 5: return readFloatArray();
        case 6:    return readVec2Array();
        case 7:    return readVec3Array();
        case 8:    return readVec4Array();
        case 9:    return readVec2sArray();
        case 10:   return readVec3sArray();
        case 11:   return readVec4sArray();
        case 12:   return readVec2bArray();        
        case 13:   return readVec3bArray();        
        case 14:   return readVec4bArray();        
        default: throw Exception("Unknown array type in DataInputStream::readArray()");
    }
}

osg::IntArray* DataInputStream::readIntArray()
{
    int size = readInt();
    osg::IntArray* a = new osg::IntArray(size);
    
    _istream->read((char*)&((*a)[0]), INTSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readIntArray(): Failed to read Int array.");

    if (_verboseOutput) std::cout<<"read/writeIntArray() ["<<size<<"]"<<std::endl;  

    if (_byteswap) {
       for (int  i = 0 ; i < size ; i++ ) osg::swapBytes((char *)&((*a)[i]),INTSIZE) ;
    }
       
    return a;
}

osg::UByteArray* DataInputStream::readUByteArray()
{
    int size = readInt();
    osg::UByteArray* a = new osg::UByteArray(size);

    _istream->read((char*)&((*a)[0]), CHARSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readUByteArray(): Failed to read UByte array.");

    if (_verboseOutput) std::cout<<"read/writeUByteArray() ["<<size<<"]"<<std::endl;
    
    return a;
}

osg::UShortArray* DataInputStream::readUShortArray()
{
    int size = readInt();
    osg::UShortArray* a = new osg::UShortArray(size);

    _istream->read((char*)&((*a)[0]), SHORTSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readUShortArray(): Failed to read UShort array.");

    if (_verboseOutput) std::cout<<"read/writeUShortArray() ["<<size<<"]"<<std::endl;
    
    if (_byteswap)
    {
        for (int i = 0 ; i < size ; i++ ) 
            osg::swapBytes((char *)&((*a)[i]),SHORTSIZE) ;
    }
    return a;
}

osg::UIntArray* DataInputStream::readUIntArray()
{
    int size = readInt();
    osg::UIntArray* a = new osg::UIntArray(size);

    _istream->read((char*)&((*a)[0]), INTSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readUIntArray(): Failed to read UInt array.");

    if (_verboseOutput) std::cout<<"read/writeUIntArray() ["<<size<<"]"<<std::endl;
    
    if (_byteswap)
    {
        for (int i = 0 ; i < size ; i++ )
            osg::swapBytes((char *)&((*a)[i]),INTSIZE) ;
    }
    return a;
}

osg::Vec4ubArray* DataInputStream::readVec4ubArray()
{
    int size = readInt();
    osg::Vec4ubArray* a = new osg::Vec4ubArray(size);

    _istream->read((char*)&((*a)[0]), INTSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readVec4ubArray(): Failed to read Vec4ub array.");

    if (_verboseOutput) std::cout<<"read/writeVec4ubArray() ["<<size<<"]"<<std::endl;
    
    return a;
}

osg::FloatArray* DataInputStream::readFloatArray()
{
    int size = readInt();
    
    osg::FloatArray* a = new osg::FloatArray(size);
    
    _istream->read((char*)&((*a)[0]), FLOATSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readFloatArray(): Failed to read float array.");

    if (_verboseOutput) std::cout<<"read/writeFloatArray() ["<<size<<"]"<<std::endl;
    
    if (_byteswap)
    {
        for (int i = 0 ; i < size ; i++ )
            osg::swapBytes((char *)&((*a)[i]),FLOATSIZE) ;
    }
    return a;
}

osg::Vec2Array* DataInputStream::readVec2Array()
{
    int size = readInt();
    osg::Vec2Array* a = new osg::Vec2Array(size);
    
    _istream->read((char*)&((*a)[0]), FLOATSIZE*2*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readVec2Array(): Failed to read Vec2 array.");

    if (_verboseOutput) std::cout<<"read/writeVec2Array() ["<<size<<"]"<<std::endl;
    
    if (_byteswap)
    {
       float *ptr = (float*)&((*a)[0]) ;
       for (int i = 0 ; i < size*2 ; i++ )
       {
          osg::swapBytes((char *)&(ptr[i]), FLOATSIZE) ;
       }
    }
    return a;
}

osg::Vec3Array* DataInputStream::readVec3Array()
{
    int size = readInt();
    osg::Vec3Array* a = new osg::Vec3Array(size);

    _istream->read((char*)&((*a)[0]), FLOATSIZE*3*size);
    
    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readVec3Array(): Failed to read Vec3 array.");

    if (_verboseOutput) std::cout<<"read/writeVec3Array() ["<<size<<"]"<<std::endl;
    

    if (_byteswap)
    {
       float *ptr = (float*)&((*a)[0]) ;
       for (int i = 0 ; i < size*3 ; i++ )
       {
          osg::swapBytes((char *)&(ptr[i]),FLOATSIZE) ;
       }
    }
    return a;
}

osg::Vec4Array* DataInputStream::readVec4Array(){
    int size = readInt();
    osg::Vec4Array* a = new osg::Vec4Array(size);

    _istream->read((char*)&((*a)[0]), FLOATSIZE*4*size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readVec4Array(): Failed to read Vec4 array.");

    if (_verboseOutput) std::cout<<"read/writeVec4Array() ["<<size<<"]"<<std::endl;
    
    if (_byteswap) {
       float *ptr = (float*)&((*a)[0]) ;
       for (int i = 0 ; i < size*4 ; i++ ) {
          osg::swapBytes((char *)&(ptr[i]),FLOATSIZE) ;
       }
    }
    return a;
}

osg::Vec2bArray* DataInputStream::readVec2bArray()
{
    int size = readInt();
    osg::Vec2bArray* a = new osg::Vec2bArray(size);

    _istream->read((char*)&((*a)[0]), CHARSIZE * 2 * size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readVec2bArray(): Failed to read Vec2b array.");

    if (_verboseOutput) std::cout<<"read/writeVec2bArray() ["<<size<<"]"<<std::endl;

    return a;
}

osg::Vec3bArray* DataInputStream::readVec3bArray()
{
    int size = readInt();
    osg::Vec3bArray* a = new osg::Vec3bArray(size);

    _istream->read((char*)&((*a)[0]), CHARSIZE * 3 * size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readVec3bArray(): Failed to read Vec3b array.");

    if (_verboseOutput) std::cout<<"read/writeVec3bArray() ["<<size<<"]"<<std::endl;

    return a;
}

osg::Vec4bArray* DataInputStream::readVec4bArray()
{
    int size = readInt();
    osg::Vec4bArray* a = new osg::Vec4bArray(size);

    _istream->read((char*)&((*a)[0]), CHARSIZE * 4 * size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readVec4bArray(): Failed to read Vec4b array.");

    if (_verboseOutput) std::cout<<"read/writeVec4bArray() ["<<size<<"]"<<std::endl;

    return a;
}

osg::Vec2sArray* DataInputStream::readVec2sArray()
{
    int size = readInt();
    osg::Vec2sArray* a = new osg::Vec2sArray(size);

    _istream->read((char*)&((*a)[0]), SHORTSIZE * 2 * size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readVec2sArray(): Failed to read Vec2s array.");

    if (_verboseOutput) std::cout<<"read/writeVec2sArray() ["<<size<<"]"<<std::endl;

    if (_byteswap)
    {
       short *ptr = (short*)&((*a)[0]) ;
       for (int i = 0 ; i < size*2 ; i++ )
       {
          osg::swapBytes((char *)&(ptr[i]), SHORTSIZE) ;
       }
    }

    return a;
}

osg::Vec3sArray* DataInputStream::readVec3sArray()
{
    int size = readInt();
    osg::Vec3sArray* a = new osg::Vec3sArray(size);

    _istream->read((char*)&((*a)[0]), SHORTSIZE * 3 * size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readVec3sArray(): Failed to read Vec3s array.");

    if (_verboseOutput) std::cout<<"read/writeVec3sArray() ["<<size<<"]"<<std::endl;


    if (_byteswap)
    {
       short *ptr = (short*)&((*a)[0]) ;
       for (int i = 0 ; i < size*3 ; i++ )
       {
          osg::swapBytes((char *)&(ptr[i]), SHORTSIZE) ;
       }
    }

    return a;
}

osg::Vec4sArray* DataInputStream::readVec4sArray()
{
    int size = readInt();
    osg::Vec4sArray* a = new osg::Vec4sArray(size);

    _istream->read((char*)&((*a)[0]), SHORTSIZE * 4 * size);

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readVec4sArray(): Failed to read Vec4s array.");

    if (_verboseOutput) std::cout<<"read/writeVec4sArray() ["<<size<<"]"<<std::endl;

    if (_byteswap)
    {
       short *ptr = (short*)&((*a)[0]) ;
       for (int i = 0 ; i < size*4 ; i++ )
       {
          osg::swapBytes((char *)&(ptr[i]), SHORTSIZE) ;
       }
    }

    return a;
}

osg::Matrixf DataInputStream::readMatrixf()
{
    osg::Matrixf mat;
    for(int r=0;r<4;r++)
    {
        for(int c=0;c<4;c++)
        {
            mat(r,c) = readFloat();
        }
    }

    if (_istream->rdstate() & _istream->failbit)
        throw Exception("DataInputStream::readMatrix(): Failed to read Matrix array.");

    if (_verboseOutput) std::cout<<"read/writeMatrix() ["<<mat<<"]"<<std::endl;
    

    return mat;
}

osg::Matrixd DataInputStream::readMatrixd()
{
    osg::Matrixd mat;
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
    if (mitr!=_imageMap.end()) return mitr->second.get();
        
    // Image is not in list. 
    // Read it from disk, 
    osg::Image* image = osgDB::readImageFile(filename.c_str(),_options.get());
        
    // add it to the imageList,
    _imageMap[filename] = image;
    // and return image pointer.

    if (_verboseOutput) std::cout<<"read/writeImage() ["<<image<<"]"<<std::endl;
    
    return image;
}

osg::Image* DataInputStream::readImage(IncludeImageMode mode)
{
    switch(mode) {
        case IMAGE_INCLUDE_DATA:
            // Read image data from stream
            if(readBool())
            {
                osg::Image* image = new osg::Image();
                ((ive::Image*)image)->read(this);
                return image;
            }
            break;
        case IMAGE_REFERENCE_FILE:
            // Only read image name from stream.
            {
                std::string filename = readString();
                if(filename.compare("")!=0){
                    return readImage(filename);
                }
            }
            break;
        case IMAGE_INCLUDE_FILE:
        case IMAGE_COMPRESS_DATA:
            // Read image file from stream
            {
                std::string filename = readString();
                int size = readInt();
                if(filename.compare("")!=0 && size > 0){

                    //Read in file
                    char *buffer = new char[size];
                    readCharArray(buffer,size);

                    //Get ReaderWriter from file extension
                    std::string ext = osgDB::getFileExtension(filename);
                    osgDB::ReaderWriter *reader = osgDB::Registry::instance()->getReaderWriterForExtension(ext);

                    osgDB::ReaderWriter::ReadResult rr;
                    if(reader) {
                        //Convert data to istream
                        std::stringstream inputStream;
                        inputStream.write(buffer,size);

                        //Attempt to read the image
                        rr = reader->readImage(inputStream,_options.get());
                    }

                    //Delete buffer
                    delete [] buffer;

                    //Return result
                    if(rr.validImage()) {
                        return rr.takeImage();
                    }
                }
            }
            break;
        default:
            throw Exception("DataInputStream::readImage(): Invalid IncludeImageMode value.");
            break;
    }
    return 0;
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
    if(attributeID == IVEALPHAFUNC){
        attribute = new osg::AlphaFunc();
        ((ive::AlphaFunc*)(attribute))->read(this);
    }
    else if(attributeID == IVEBLENDFUNC ||
            attributeID == IVEBLENDFUNCSEPARATE){
        attribute = new osg::BlendFunc();
        ((ive::BlendFunc*)(attribute))->read(this);
    }
    else if(attributeID == IVEDEPTH){
        attribute = new osg::Depth();
        ((ive::Depth*)(attribute))->read(this);
    }
    else if(attributeID == IVEVIEWPORT){
        attribute = new osg::Viewport();
        ((ive::Viewport*)(attribute))->read(this);
    }
    else if(attributeID == IVESCISSOR){
        attribute = new osg::Scissor();
        ((ive::Scissor*)(attribute))->read(this);
    }
    else if(attributeID == IVEMATERIAL){
        attribute = new osg::Material();
        ((ive::Material*)(attribute))->read(this);
    }
    else if(attributeID == IVECULLFACE){
        attribute = new osg::CullFace();
        ((ive::CullFace*)(attribute))->read(this);
    }
    else if(attributeID == IVECLIPPLANE){
        attribute = new osg::ClipPlane();
        ((ive::ClipPlane*)(attribute))->read(this);
    }
    else if(attributeID == IVEPOLYGONOFFSET){
        attribute = new osg::PolygonOffset();
        ((ive::PolygonOffset*)(attribute))->read(this);
    }
    else if(attributeID == IVEPOLYGONMODE){
        attribute = new osg::PolygonMode();
        ((ive::PolygonMode*)(attribute))->read(this);
    }
    else if(attributeID == IVESHADEMODEL){
        attribute = new osg::ShadeModel();
        ((ive::ShadeModel*)(attribute))->read(this);
    }
    else if(attributeID == IVEPOINT){
        attribute = new osg::Point();
        ((ive::Point*)(attribute))->read(this);
    }
    else if(attributeID == IVELINEWIDTH){
        attribute = new osg::LineWidth();
        ((ive::LineWidth*)(attribute))->read(this);
    }
    else if(attributeID == IVETEXTURE1D){
        attribute = new osg::Texture1D();
        ((ive::Texture1D*)(attribute))->read(this);
    }
    else if(attributeID == IVETEXTURE2D){
        attribute = new osg::Texture2D();
        ((ive::Texture2D*)(attribute))->read(this);
    }
    else if(attributeID == IVETEXTURE3D){
        attribute = new osg::Texture3D();
        ((ive::Texture3D*)(attribute))->read(this);
    }
    else if(attributeID == IVETEXTURECUBEMAP){
        attribute = new osg::TextureCubeMap();
        ((ive::TextureCubeMap*)(attribute))->read(this);
    }
    else if(attributeID == IVETEXTURERECTANGLE){
        attribute = new osg::TextureRectangle();
        ((ive::TextureRectangle*)(attribute))->read(this);
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
    else if(attributeID == IVETEXMAT){
        attribute = new osg::TexMat();
        ((ive::TexMat*)(attribute))->read(this);
    }
    else if(attributeID == IVEFRAGMENTPROGRAM){
        attribute = new osg::FragmentProgram();
        ((ive::FragmentProgram*)(attribute))->read(this);
    }
    else if(attributeID == IVEVERTEXPROGRAM){
        attribute = new osg::VertexProgram();
        ((ive::VertexProgram*)(attribute))->read(this);
    }
    else if(attributeID == IVELIGHTMODEL){
        attribute = new osg::LightModel();
        ((ive::LightModel*)(attribute))->read(this);
    }
    else if(attributeID == IVEFRONTFACE){
        attribute = new osg::FrontFace();
        ((ive::FrontFace*)(attribute))->read(this);
    }
    else if(attributeID == IVEPROGRAM){
        attribute = new osg::Program();
        ((ive::Program*)(attribute))->read(this);
    }
    else{
        throw Exception("Unknown StateAttribute in StateSet::read()");
    }
       
    // and add it to the stateattribute map,
    _stateAttributeMap[id] = attribute;
        

    if (_verboseOutput) std::cout<<"read/writeStateAttribute() ["<<id<<"]"<<std::endl;
    
    return attribute;
}

osg::Uniform* DataInputStream::readUniform()
{
    // Read uniforms unique ID.
    int id = readInt();
    // See if uniform is already in the list.
    UniformMap::iterator itr= _uniformMap.find(id);
    if (itr!=_uniformMap.end()) return itr->second.get();

    // Uniform is not in list.
    // Create a new uniform,
    osg::Uniform* uniform = new osg::Uniform();

    // read its properties from stream
    ((ive::Uniform*)(uniform))->read(this);
        
    // and add it to the uniform map,
    _uniformMap[id] = uniform;
        

    if (_verboseOutput) std::cout<<"read/writeUniform() ["<<id<<"]"<<std::endl;
    
    return uniform;
}


osg::Shader* DataInputStream::readShader()
{
    // Read shaders unique ID.
    int id = readInt();
    // See if shader is already in the list.
    ShaderMap::iterator itr= _shaderMap.find(id);
    if (itr!=_shaderMap.end()) return itr->second.get();

    // Shader is not in list.
    // Create a new shader,
    osg::Shader* shader = new osg::Shader();

    // read its properties from stream
    ((ive::Shader*)(shader))->read(this);
        
    // and add it to the shader map,
    _shaderMap[id] = shader;
        

    if (_verboseOutput) std::cout<<"read/writeShader() ["<<id<<"]"<<std::endl;
    
    return shader;
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
    if(drawableTypeID == IVEGEOMETRY)
    {
        drawable = new osg::Geometry();                
        ((Geometry*)(drawable))->read(this);
    }
    else if(drawableTypeID == IVESHAPEDRAWABLE)
    {
        drawable = new osg::ShapeDrawable();                
        ((ShapeDrawable*)(drawable))->read(this);
    }
    else if(drawableTypeID == IVETEXT){
        drawable = new osgText::Text();
        ((Text*)(drawable))->read(this);
    }    
    else
        throw Exception("Unknown drawable drawableTypeIDentification in Geode::read()");

       
    // and add it to the stateattribute map,
    _drawableMap[id] = drawable;
        

    if (_verboseOutput) std::cout<<"read/writeDrawable() ["<<id<<"]"<<std::endl;
    
    return drawable;
}

osg::Shape* DataInputStream::readShape()
{
    // Read stateattributes unique ID.
    int id = readInt();
    // See if stateattribute is already in the list.
    ShapeMap::iterator itr= _shapeMap.find(id);
    if (itr!=_shapeMap.end()) return itr->second.get();

    // stateattribute is not in list.
    // Create a new stateattribute,

    int shapeTypeID = peekInt();
    osg::Shape* shape;
    if(shapeTypeID == IVESPHERE)
    {
        shape = new osg::Sphere();                
        ((Sphere*)(shape))->read(this);
    }
    else if(shapeTypeID == IVEBOX)
    {
        shape = new osg::Box();
        ((Box*)(shape))->read(this);
    }
    else if(shapeTypeID == IVECONE)
    {
        shape = new osg::Cone();
        ((Cone*)(shape))->read(this);
    }
    else if(shapeTypeID == IVECYLINDER)
    {
        shape = new osg::Cylinder();
        ((Cylinder*)(shape))->read(this);
    }
    else if(shapeTypeID == IVECAPSULE)
    {
        shape = new osg::Capsule();
        ((Capsule*)(shape))->read(this);
    }
    else if(shapeTypeID == IVEHEIGHTFIELD)
    {
        shape = new osg::HeightField();                
        ((HeightField*)(shape))->read(this);
    }
    else
        throw Exception("Unknown shape shapeTypeIDentification in Shape::read()");

       
    // and add it to the stateattribute map,
    _shapeMap[id] = shape;
        

    if (_verboseOutput) std::cout<<"read/writeShape() ["<<id<<"]"<<std::endl;
    
    return shape;
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
    else if(nodeTypeID== IVECAMERANODE){
        node = new osg::CameraNode();
        ((ive::CameraNode*)(node))->read(this);
    }
    else if(nodeTypeID== IVECAMERAVIEW){
        node = new osg::CameraView();
        ((ive::CameraView*)(node))->read(this);
    }
    else if(nodeTypeID== IVEPOSITIONATTITUDETRANSFORM){
        node = new osg::PositionAttitudeTransform();
        ((ive::PositionAttitudeTransform*)(node))->read(this);
    }
    else if(nodeTypeID== IVEAUTOTRANSFORM){
        node = new osg::AutoTransform();
        ((ive::AutoTransform*)(node))->read(this);
    }
    else if(nodeTypeID== IVEDOFTRANSFORM){
        node = new osgSim::DOFTransform();
        ((ive::DOFTransform*)(node))->read(this);
    }
    else if(nodeTypeID== IVETRANSFORM){
        node = new osg::Transform();
        ((ive::Transform*)(node))->read(this);
    }
    else if(nodeTypeID== IVELIGHTSOURCE){
        node = new osg::LightSource();
        ((ive::LightSource*)(node))->read(this);
    }
    else if(nodeTypeID== IVETEXGENNODE){
        node = new osg::TexGenNode();
        ((ive::TexGenNode*)(node))->read(this);
    }
    else if(nodeTypeID== IVECLIPNODE){
        node = new osg::ClipNode();
        ((ive::ClipNode*)(node))->read(this);
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
    else if(nodeTypeID== IVECOORDINATESYSTEMNODE){
        node = new osg::CoordinateSystemNode();
        ((ive::CoordinateSystemNode*)(node))->read(this);
    }
    else if(nodeTypeID== IVESWITCH){
        node = new osg::Switch();
        ((ive::Switch*)(node))->read(this);
    }
    else if(nodeTypeID== IVEMULTISWITCH){
        node = new osgSim::MultiSwitch();
        ((ive::MultiSwitch*)(node))->read(this);
    }
    else if(nodeTypeID== IVEIMPOSTOR){
        node = new osgSim::Impostor();
        ((ive::Impostor*)(node))->read(this);
    }
    else if(nodeTypeID== IVEOCCLUDERNODE){
        node = new osg::OccluderNode();
        ((ive::OccluderNode*)(node))->read(this);
    }
    else if(nodeTypeID== IVEVISIBILITYGROUP){
        node = new osgSim::VisibilityGroup();
        ((ive::VisibilityGroup*)(node))->read(this);
    }
    else if(nodeTypeID== IVEPROXYNODE){
        node = new osg::ProxyNode();
        ((ive::ProxyNode*)(node))->read(this);
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
    else if(nodeTypeID== IVELIGHTPOINTNODE){
        node = new osgSim::LightPointNode();
        ((ive::LightPointNode*)(node))->read(this);
    }
    else if(nodeTypeID== IVEMULTITEXTURECONTROL){
        node = new osgFX::MultiTextureControl();
        ((ive::MultiTextureControl*)(node))->read(this);
    }
    else{
        throw Exception("Unknown node identification in DataInputStream::readNode()");
    }

    // and add it to the node map,
    _nodeMap[id] = node;
        

    if (_verboseOutput) std::cout<<"read/writeNode() ["<<id<<"]"<<std::endl;
    
    return node;
}


