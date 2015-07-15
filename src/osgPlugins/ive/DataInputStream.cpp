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
#include "BlendColor.h"
#include "Stencil.h"
#include "StencilTwoSided.h"
#include "BlendFunc.h"
#include "BlendEquation.h"
#include "Depth.h"
#include "Material.h"
#include "CullFace.h"
#include "ColorMask.h"
#include "ClipPlane.h"
#include "PolygonOffset.h"
#include "PolygonMode.h"
#include "ShadeModel.h"
#include "Point.h"
#include "LineWidth.h"
#include "LineStipple.h"
#include "Texture1D.h"
#include "Texture2D.h"
#include "Texture2DArray.h"
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
#include "ImageSequence.h"
#include "PointSprite.h"
#include "Multisample.h"
#include "Fog.h"
#include "Light.h"
#include "PolygonStipple.h"

#include "Node.h"
#include "Group.h"
#include "MatrixTransform.h"
#include "Camera.h"
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
#include "OcclusionQueryNode.h"
#include "Impostor.h"
#include "CoordinateSystemNode.h"
#include "Uniform.h"
#include "Shader.h"

#include "LightPointNode.h"
#include "MultiSwitch.h"
#include "VisibilityGroup.h"

#include "MultiTextureControl.h"
#include "ShapeAttributeList.h"
#include "Effect.h"
#include "AnisotropicLighting.h"
#include "BumpMapping.h"
#include "Cartoon.h"
#include "Scribe.h"
#include "SpecularHighlights.h"

#include "Volume.h"
#include "VolumeTile.h"
#include "VolumeImageLayer.h"
#include "VolumeCompositeLayer.h"
#include "VolumeLocator.h"
#include "VolumeCompositeProperty.h"
#include "VolumeSwitchProperty.h"
#include "VolumeScalarProperty.h"
#include "VolumeTransferFunctionProperty.h"

#include "Geometry.h"
#include "ShapeDrawable.h"
#include "Shape.h"

#include "Text.h"

#include "Terrain.h"
#include "TerrainTile.h"
#include "Locator.h"
#include "ImageLayer.h"
#include "HeightFieldLayer.h"
#include "CompositeLayer.h"
#include "SwitchLayer.h"

#include "FadeText.h"
#include "Text3D.h"

#include <osg/Endian>
#include <osg/Notify>
#include <osg/io_utils>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>

#include <stdio.h>
#include <sstream>

using namespace ive;
using namespace std;

DataInputStream::DataInputStream(std::istream* istream, const osgDB::ReaderWriter::Options* options)
{
    unsigned int endianType ;

    _loadExternalReferenceFiles = false;

    _verboseOutput = false;

    _istream = istream;
    _owns_istream = false;
    _peeking = false;
    _peekValue = 0;
    _byteswap = 0;

    _options = options;

    if (_options.get())
    {
        setLoadExternalReferenceFiles(_options->getOptionString().find("noLoadExternalReferenceFiles")==std::string::npos);
        OSG_DEBUG << "ive::DataInputStream.setLoadExternalReferenceFiles()=" << getLoadExternalReferenceFiles() << std::endl;
    }

    if(!istream){
        throwException("DataInputStream::DataInputStream(): null pointer exception in argument.");
    }

    endianType = readUInt() ;

    if ( endianType != ENDIAN_TYPE) {
      // Make sure the file is simply swapped
      if ( endianType != OPPOSITE_ENDIAN_TYPE ) {
         throwException("DataInputStream::DataInputStream(): This file has an unreadable endian type.") ;
      }
      OSG_INFO<<"DataInputStream::DataInputStream: Reading a byteswapped file" << std::endl ;
      _byteswap = 1 ;
   }

    _version = readUInt();

    // Are we trying to open a binary .ive file which version are newer than this library.
    if(_version>VERSION){
        throwException("DataInputStream::DataInputStream(): The version found in the file is newer than this library can handle.");
    }

    if (_version>=VERSION_0033)
    {
        int compressionLevel = readInt();

        if (compressionLevel>0)
        {
            OSG_INFO<<"compressed ive stream"<<std::endl;

            unsigned int maxSize = readUInt();

            std::string data;
            data.reserve(maxSize);

            if (!uncompress(*istream, data))
            {
                throwException("Error in uncompressing .ive");
                return;
            }

            _istream = new std::stringstream(data);
            _owns_istream = true;
        }
        else
        {
            OSG_INFO<<"uncompressed ive stream"<<std::endl;
        }
    }
}

DataInputStream::~DataInputStream()
{
    if (_owns_istream) delete _istream;
}

#ifdef USE_ZLIB

#include <zlib.h>

bool DataInputStream::uncompress(std::istream& fin, std::string& destination) const
{
    //#define CHUNK 16384
    #define CHUNK 32768

    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm,
                       15 + 32 // autodected zlib or gzip header
                       );
    if (ret != Z_OK)
    {
        OSG_INFO<<"failed to init"<<std::endl;
        return ret != 0;
    }

    /* decompress until deflate stream ends or end of file */
    do {
        //strm.avail_in = fin.readsome((char*)in, CHUNK);
        fin.read((char *)in, CHUNK);
        strm.avail_in = fin.gcount();

        if (strm.avail_in == 0)
        {
            break;
        }
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);

            switch (ret) {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return false;
            }
            have = CHUNK - strm.avail_out;

            destination.append((char*)out, have);

        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);

    return ret == Z_STREAM_END ? true : false;
}
#else
bool DataInputStream::uncompress(std::istream& fin, std::string& destination) const
{
    return false;
}
#endif

bool DataInputStream::readBool(){
    char c=0;
    _istream->read(&c, CHARSIZE);

    if (_istream->rdstate() & _istream->failbit)
        throwException("DataInputStream::readBool(): Failed to read boolean value.");

    if (_verboseOutput) std::cout<<"read/writeBool() ["<<(int)c<<"]"<<std::endl;

    return c!=0;
}

char DataInputStream::readChar(){
    char c=0;
    _istream->read(&c, CHARSIZE);

    if (_istream->rdstate() & _istream->failbit)
        throwException("DataInputStream::readChar(): Failed to read char value.");

    if (_verboseOutput) std::cout<<"read/writeChar() ["<<(int)c<<"]"<<std::endl;

    return c;
}

unsigned char DataInputStream::readUChar(){
    unsigned char c=0;
    _istream->read((char*)&c, CHARSIZE);

    if (_istream->rdstate() & _istream->failbit)
        throwException("DataInputStream::readUChar(): Failed to read unsigned char value.");

    if (_verboseOutput) std::cout<<"read/writeUChar() ["<<(int)c<<"]"<<std::endl;

    return c;
}

unsigned short DataInputStream::readUShort(){
    unsigned short s=0;
    _istream->read((char*)&s, SHORTSIZE);
    if (_istream->rdstate() & _istream->failbit)
        throwException("DataInputStream::readUShort(): Failed to read unsigned short value.");

    if (_verboseOutput) std::cout<<"read/writeUShort() ["<<s<<"]"<<std::endl;

    if (_byteswap) osg::swapBytes((char *)&s,SHORTSIZE);

    return s;
}

unsigned int DataInputStream::readUInt(){
    unsigned int s=0;
    _istream->read((char*)&s, INTSIZE);

    if (_istream->rdstate() & _istream->failbit)
        throwException("DataInputStream::readUInt(): Failed to read unsigned int value.");

    if (_byteswap) osg::swapBytes((char *)&s,INTSIZE) ;

    if (_verboseOutput) std::cout<<"read/writeUInt() ["<<s<<"]"<<std::endl;

    return s;
}

int DataInputStream::readInt(){
    if(_peeking){
        _peeking = false;
        return _peekValue;
    }
    int i=0;
    _istream->read((char*)&i, INTSIZE);

    if (_istream->rdstate() & _istream->failbit)
        throwException("DataInputStream::readInt(): Failed to read int value.");

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
    float f=0.0f;
    _istream->read((char*)&f, FLOATSIZE);
    if (_istream->rdstate() & _istream->failbit)
        throwException("DataInputStream::readFloat(): Failed to read float value.");

    if (_byteswap) osg::swapBytes((char *)&f,FLOATSIZE) ;

    if (_verboseOutput) std::cout<<"read/writeFloat() ["<<f<<"]"<<std::endl;
    return f;
}

long DataInputStream::readLong(){
    long l=0;
    _istream->read((char*)&l, LONGSIZE);
    if (_istream->rdstate() & _istream->failbit)
        throwException("DataInputStream::readLong(): Failed to read long value.");

    if (_byteswap) osg::swapBytes((char *)&l,LONGSIZE) ;

    if (_verboseOutput) std::cout<<"read/writeLong() ["<<l<<"]"<<std::endl;
    return l;
}

unsigned long DataInputStream::readULong(){
    unsigned long l=0;
    _istream->read((char*)&l, LONGSIZE);
    if (_istream->rdstate() & _istream->failbit)
        throwException("DataInputStream::readULong(): Failed to read unsigned long value.");

    if (_byteswap) osg::swapBytes((char *)&l,LONGSIZE) ;

    if (_verboseOutput) std::cout<<"read/writeULong() ["<<l<<"]"<<std::endl;

    return l;
}

double DataInputStream::readDouble()
{
    double d=0.0;
    _istream->read((char*)&d, DOUBLESIZE);
    if (_istream->rdstate() & _istream->failbit)
        throwException("DataInputStream::readDouble(): Failed to read double value.");

    if (_byteswap) osg::swapBytes((char *)&d,DOUBLESIZE) ;
    if (_verboseOutput) std::cout<<"read/writeDouble() ["<<d<<"]"<<std::endl;
    return d;
}

std::string DataInputStream::readString()
{
    std::string s;
    int size = readInt();
    if (size != 0)
    {
        s.resize(size);
        _istream->read((char*)s.c_str(), size);
        //if (_istream->rdstate() & _istream->failbit)
        //   throwException("DataInputStream::readString(): Failed to read string value.");

        if (_verboseOutput) std::cout<<"read/writeString() ["<<s<<"]"<<std::endl;
    }

    return s;
}

void DataInputStream::readCharArray(char* data, int size)
{
    _istream->read(data, size);
    if (_istream->rdstate() & _istream->failbit)
        throwException("DataInputStream::readCharArray(): Failed to read char value.");

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

    if (getVersion() <= VERSION_0018)
    {
        v[0]=readFloat();
        v[1]=readFloat();
        v[2]=readFloat();
        v[3]=readFloat();
    }
    else
    {
        // assume double for planes even if Plane::value_type is float
        // to ensure that the .ive format does vary.
        v[0]=readDouble();
        v[1]=readDouble();
        v[2]=readDouble();
        v[3]=readDouble();
    }

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




deprecated_osg::Geometry::AttributeBinding DataInputStream::readBinding(){
    char c = readChar();

    if (_verboseOutput) std::cout<<"readBinding() ["<<(int)c<<"]"<<std::endl;

    switch((int)c){
        case 0:    return deprecated_osg::Geometry::BIND_OFF;
        case 1: return deprecated_osg::Geometry::BIND_OVERALL;
        case 2: return deprecated_osg::Geometry::BIND_PER_PRIMITIVE;
        case 3: return deprecated_osg::Geometry::BIND_PER_PRIMITIVE_SET;
        case 4: return deprecated_osg::Geometry::BIND_PER_VERTEX;
        default:
            throwException("Unknown binding type in DataInputStream::readBinding()");
            return deprecated_osg::Geometry::BIND_OFF;
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
        case 15:   return readVec2dArray();
        case 16:   return readVec3dArray();
        case 17:   return readVec4dArray();
        default:
            throwException("Unknown array type in DataInputStream::readArray()");
            return 0;
    }
}

osg::IntArray* DataInputStream::readIntArray()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::IntArray> a = new osg::IntArray(size);

    _istream->read((char*)&((*a)[0]), INTSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readIntArray(): Failed to read Int array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeIntArray() ["<<size<<"]"<<std::endl;

    if (_byteswap) {
       for (int  i = 0 ; i < size ; i++ ) osg::swapBytes((char *)&((*a)[i]),INTSIZE) ;
    }

    return a.release();
}

osg::UByteArray* DataInputStream::readUByteArray()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::UByteArray> a = new osg::UByteArray(size);

    _istream->read((char*)&((*a)[0]), CHARSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readUByteArray(): Failed to read UByte array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeUByteArray() ["<<size<<"]"<<std::endl;

    return a.release();
}

osg::UShortArray* DataInputStream::readUShortArray()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::UShortArray> a = new osg::UShortArray(size);

    _istream->read((char*)&((*a)[0]), SHORTSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readUShortArray(): Failed to read UShort array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeUShortArray() ["<<size<<"]"<<std::endl;

    if (_byteswap)
    {
        for (int i = 0 ; i < size ; i++ )
            osg::swapBytes((char *)&((*a)[i]),SHORTSIZE) ;
    }
    return a.release();
}

osg::UIntArray* DataInputStream::readUIntArray()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::UIntArray> a = new osg::UIntArray(size);

    _istream->read((char*)&((*a)[0]), INTSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readUIntArray(): Failed to read UInt array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeUIntArray() ["<<size<<"]"<<std::endl;

    if (_byteswap)
    {
        for (int i = 0 ; i < size ; i++ )
            osg::swapBytes((char *)&((*a)[i]),INTSIZE) ;
    }
    return a.release();
}

osg::Vec4ubArray* DataInputStream::readVec4ubArray()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::Vec4ubArray> a = new osg::Vec4ubArray(size);

    _istream->read((char*)&((*a)[0]), INTSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readVec4ubArray(): Failed to read Vec4ub array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeVec4ubArray() ["<<size<<"]"<<std::endl;

    return a.release();
}

bool DataInputStream::readPackedFloatArray(osg::FloatArray* a)
{
    int size = readInt();

    a->resize(size);

    if (size == 0)
        return true;

    if (readBool())
    {
        float value = readFloat();

        for(int i=0; i<size; ++i)
        {
            (*a)[i] = value;
        }
    }
    else
    {
        int packingSize = readInt();

        if (packingSize==1)
        {
            float minValue = readFloat();
            float maxValue = readFloat();

            float byteMultiplier = 255.0f/(maxValue-minValue);
            float byteInvMultiplier = 1.0f/byteMultiplier;

            for(int i=0; i<size; ++i)
            {
                unsigned char byte_value = readUChar();
                float value = minValue + float(byte_value)*byteInvMultiplier;
                (*a)[i] = value;
            }
        }
        else if (packingSize==2)
        {
            float minValue = readFloat();
            float maxValue = readFloat();

            float shortMultiplier = 65535.0f/(maxValue-minValue);
            float shortInvMultiplier = 1.0f/shortMultiplier;

            for(int i=0; i<size; ++i)
            {
                unsigned short short_value = readUShort();
                float value = minValue + float(short_value)*shortInvMultiplier;
                (*a)[i] = value;
            }
        }
        else
        {
            for(int i=0; i<size; ++i)
            {
                (*a)[i] = readFloat();
            }
        }
    }

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readFloatArray(): Failed to read float array.");
        return false;
    }

    if (_verboseOutput) std::cout<<"read/writeFloatArray() ["<<size<<"]"<<std::endl;

    return true;
}


osg::FloatArray* DataInputStream::readFloatArray()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::FloatArray> a = new osg::FloatArray(size);

    _istream->read((char*)&((*a)[0]), FLOATSIZE*size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readFloatArray(): Failed to read float array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeFloatArray() ["<<size<<"]"<<std::endl;

    if (_byteswap)
    {
        for (int i = 0 ; i < size ; i++ )
            osg::swapBytes((char *)&((*a)[i]),FLOATSIZE) ;
    }
    return a.release();
}

osg::Vec2Array* DataInputStream::readVec2Array()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::Vec2Array> a = new osg::Vec2Array(size);

    _istream->read((char*)&((*a)[0]), FLOATSIZE*2*size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readVec2Array(): Failed to read Vec2 array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeVec2Array() ["<<size<<"]"<<std::endl;

    if (_byteswap)
    {
       float *ptr = (float*)&((*a)[0]) ;
       for (int i = 0 ; i < size*2 ; i++ )
       {
          osg::swapBytes((char *)&(ptr[i]), FLOATSIZE) ;
       }
    }
    return a.release();
}

osg::Vec3Array* DataInputStream::readVec3Array()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::Vec3Array> a = new osg::Vec3Array(size);

    _istream->read((char*)&((*a)[0]), FLOATSIZE*3*size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readVec3Array(): Failed to read Vec3 array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeVec3Array() ["<<size<<"]"<<std::endl;


    if (_byteswap)
    {
       float *ptr = (float*)&((*a)[0]) ;
       for (int i = 0 ; i < size*3 ; i++ )
       {
          osg::swapBytes((char *)&(ptr[i]),FLOATSIZE) ;
       }
    }
    return a.release();
}

osg::Vec4Array* DataInputStream::readVec4Array(){
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::Vec4Array> a = new osg::Vec4Array(size);

    _istream->read((char*)&((*a)[0]), FLOATSIZE*4*size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readVec4Array(): Failed to read Vec4 array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeVec4Array() ["<<size<<"]"<<std::endl;

    if (_byteswap) {
       float *ptr = (float*)&((*a)[0]) ;
       for (int i = 0 ; i < size*4 ; i++ ) {
          osg::swapBytes((char *)&(ptr[i]),FLOATSIZE) ;
       }
    }
    return a.release();
}

osg::Vec2bArray* DataInputStream::readVec2bArray()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::Vec2bArray> a = new osg::Vec2bArray(size);

    _istream->read((char*)&((*a)[0]), CHARSIZE * 2 * size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readVec2bArray(): Failed to read Vec2b array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeVec2bArray() ["<<size<<"]"<<std::endl;

    return a.release();
}

osg::Vec3bArray* DataInputStream::readVec3bArray()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::Vec3bArray> a = new osg::Vec3bArray(size);

    _istream->read((char*)&((*a)[0]), CHARSIZE * 3 * size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readVec3bArray(): Failed to read Vec3b array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeVec3bArray() ["<<size<<"]"<<std::endl;

    return a.release();
}

osg::Vec4bArray* DataInputStream::readVec4bArray()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::Vec4bArray> a = new osg::Vec4bArray(size);

    _istream->read((char*)&((*a)[0]), CHARSIZE * 4 * size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readVec4bArray(): Failed to read Vec4b array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeVec4bArray() ["<<size<<"]"<<std::endl;

    return a.release();
}

osg::Vec2sArray* DataInputStream::readVec2sArray()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::Vec2sArray> a = new osg::Vec2sArray(size);

    _istream->read((char*)&((*a)[0]), SHORTSIZE * 2 * size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readVec2sArray(): Failed to read Vec2s array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeVec2sArray() ["<<size<<"]"<<std::endl;

    if (_byteswap)
    {
       short *ptr = (short*)&((*a)[0]) ;
       for (int i = 0 ; i < size*2 ; i++ )
       {
          osg::swapBytes((char *)&(ptr[i]), SHORTSIZE) ;
       }
    }

    return a.release();
}

osg::Vec3sArray* DataInputStream::readVec3sArray()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::Vec3sArray> a = new osg::Vec3sArray(size);

    _istream->read((char*)&((*a)[0]), SHORTSIZE * 3 * size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readVec3sArray(): Failed to read Vec3s array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeVec3sArray() ["<<size<<"]"<<std::endl;


    if (_byteswap)
    {
       short *ptr = (short*)&((*a)[0]) ;
       for (int i = 0 ; i < size*3 ; i++ )
       {
          osg::swapBytes((char *)&(ptr[i]), SHORTSIZE) ;
       }
    }

    return a.release();
}

osg::Vec4sArray* DataInputStream::readVec4sArray()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::Vec4sArray> a = new osg::Vec4sArray(size);

    _istream->read((char*)&((*a)[0]), SHORTSIZE * 4 * size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readVec4sArray(): Failed to read Vec4s array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeVec4sArray() ["<<size<<"]"<<std::endl;

    if (_byteswap)
    {
       short *ptr = (short*)&((*a)[0]) ;
       for (int i = 0 ; i < size*4 ; i++ )
       {
          osg::swapBytes((char *)&(ptr[i]), SHORTSIZE) ;
       }
    }

    return a.release();
}

osg::Vec2dArray* DataInputStream::readVec2dArray()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::Vec2dArray> a = new osg::Vec2dArray(size);

    _istream->read((char*)&((*a)[0]), DOUBLESIZE*2*size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readVec2dArray(): Failed to read Vec2d array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeVec2dArray() ["<<size<<"]"<<std::endl;

    if (_byteswap)
    {
       double *ptr = (double*)&((*a)[0]) ;
       for (int i = 0 ; i < size*2 ; i++ )
       {
          osg::swapBytes((char *)&(ptr[i]), DOUBLESIZE) ;
       }
    }
    return a.release();
}

osg::Vec3dArray* DataInputStream::readVec3dArray()
{
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::Vec3dArray> a = new osg::Vec3dArray(size);

    _istream->read((char*)&((*a)[0]), DOUBLESIZE*3*size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readVec3dArray(): Failed to read Vec3d array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeVec3dArray() ["<<size<<"]"<<std::endl;


    if (_byteswap)
    {
       double *ptr = (double*)&((*a)[0]) ;
       for (int i = 0 ; i < size*3 ; i++ )
       {
          osg::swapBytes((char *)&(ptr[i]),DOUBLESIZE) ;
       }
    }
    return a.release();
}

osg::Vec4dArray* DataInputStream::readVec4dArray(){
    int size = readInt();
    if (size == 0)
        return NULL;

    osg::ref_ptr<osg::Vec4dArray> a = new osg::Vec4dArray(size);

    _istream->read((char*)&((*a)[0]), DOUBLESIZE*4*size);

    if (_istream->rdstate() & _istream->failbit)
    {
        throwException("DataInputStream::readVec4dArray(): Failed to read Vec4d array.");
        return 0;
    }

    if (_verboseOutput) std::cout<<"read/writeVec4dArray() ["<<size<<"]"<<std::endl;

    if (_byteswap) {
       double *ptr = (double*)&((*a)[0]) ;
       for (int i = 0 ; i < size*4 ; i++ ) {
          osg::swapBytes((char *)&(ptr[i]),DOUBLESIZE) ;
       }
    }
    return a.release();
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
    {
        throwException("DataInputStream::readMatrix(): Failed to read Matrix array.");
        return osg::Matrixf();
    }

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
    {
        throwException("DataInputStream::readMatrix(): Failed to read Matrix array.");
        return osg::Matrixd();
    }

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
    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(filename.c_str(),_options.get());

    // add it to the imageList,
    _imageMap[filename] = image;
    // and return image pointer.

    if (_verboseOutput) std::cout<<"read/writeImage() ["<<image<<"]"<<std::endl;

    return image.release();
}

osg::Image* DataInputStream::readImage()
{
    if ( getVersion() >= VERSION_0029 )
    {
        int id = peekInt();
        if(id == IVEIMAGESEQUENCE)
        {
            osg::ImageSequence* image = new osg::ImageSequence();
            ((ive::ImageSequence*)image)->read(this);
            return image;
        }
        else
        {
            readInt();
            IncludeImageMode includeImg = (IncludeImageMode)readChar();
            return readImage(includeImg);
        }
    }
    else
    {
        IncludeImageMode includeImg = (IncludeImageMode)readChar();
        return readImage(includeImg);
    }
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
                if(!filename.empty()){
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
            throwException("DataInputStream::readImage(): Invalid IncludeImageMode value.");
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
    osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();

    // read its properties from stream
    ((ive::StateSet*)(stateset.get()))->read(this);

    // exit early if an exception has been set.
    if (getException()) return 0;

    // and add it to the stateset map,
    _statesetMap[id] = stateset;


    if (_verboseOutput) std::cout<<"read/writeStateSet() ["<<id<<"]"<<std::endl;

    return stateset.get();
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


    osg::ref_ptr<osg::StateAttribute> attribute;
    int attributeID = peekInt();
    if(attributeID == IVEALPHAFUNC){
        attribute = new osg::AlphaFunc();
        ((ive::AlphaFunc*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEBLENDCOLOR){
        attribute = new osg::BlendColor();
        ((ive::BlendColor*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEBLENDFUNC ||
            attributeID == IVEBLENDFUNCSEPARATE){
        attribute = new osg::BlendFunc();
        ((ive::BlendFunc*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEBLENDEQUATION){
        attribute = new osg::BlendEquation();
        ((ive::BlendEquation*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEDEPTH){
        attribute = new osg::Depth();
        ((ive::Depth*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEVIEWPORT){
        attribute = new osg::Viewport();
        ((ive::Viewport*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVESCISSOR){
        attribute = new osg::Scissor();
        ((ive::Scissor*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEMATERIAL){
        attribute = new osg::Material();
        ((ive::Material*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVECULLFACE){
        attribute = new osg::CullFace();
        ((ive::CullFace*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVECOLORMASK){
        attribute = new osg::ColorMask();
        ((ive::ColorMask*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVECLIPPLANE){
        attribute = new osg::ClipPlane();
        ((ive::ClipPlane*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEPOLYGONOFFSET){
        attribute = new osg::PolygonOffset();
        ((ive::PolygonOffset*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEPOLYGONMODE){
        attribute = new osg::PolygonMode();
        ((ive::PolygonMode*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVESHADEMODEL){
        attribute = new osg::ShadeModel();
        ((ive::ShadeModel*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEPOINT){
        attribute = new osg::Point();
        ((ive::Point*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVELINEWIDTH){
        attribute = new osg::LineWidth();
        ((ive::LineWidth*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVETEXTURE1D){
        attribute = new osg::Texture1D();
        ((ive::Texture1D*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVETEXTURE2D){
        attribute = new osg::Texture2D();
        ((ive::Texture2D*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVETEXTURE2DARRAY){
        attribute = new osg::Texture2DArray();
        ((ive::Texture2DArray*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVETEXTURE3D){
        attribute = new osg::Texture3D();
        ((ive::Texture3D*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVETEXTURECUBEMAP){
        attribute = new osg::TextureCubeMap();
        ((ive::TextureCubeMap*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVETEXTURERECTANGLE){
        attribute = new osg::TextureRectangle();
        ((ive::TextureRectangle*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVETEXENV){
        attribute = new osg::TexEnv();
        ((ive::TexEnv*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVETEXENVCOMBINE){
        attribute = new osg::TexEnvCombine();
        ((ive::TexEnvCombine*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVETEXGEN){
        attribute = new osg::TexGen();
        ((ive::TexGen*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVETEXMAT){
        attribute = new osg::TexMat();
        ((ive::TexMat*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEFRAGMENTPROGRAM){
        attribute = new osg::FragmentProgram();
        ((ive::FragmentProgram*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEVERTEXPROGRAM){
        attribute = new osg::VertexProgram();
        ((ive::VertexProgram*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVELIGHTMODEL){
        attribute = new osg::LightModel();
        ((ive::LightModel*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEFRONTFACE){
        attribute = new osg::FrontFace();
        ((ive::FrontFace*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEPROGRAM){
        attribute = new osg::Program();
        ((ive::Program*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEPOINTSPRITE){
        attribute = new osg::PointSprite();
        ((ive::PointSprite*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEMULTISAMPLE){
        attribute = new osg::Multisample();
        ((ive::Multisample*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVELINESTIPPLE){
        attribute = new osg::LineStipple();
        ((ive::LineStipple*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVESTENCIL){
        attribute = new osg::Stencil();
        ((ive::Stencil*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVESTENCILTWOSIDED){
        attribute = new osg::StencilTwoSided();
        ((ive::StencilTwoSided*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEFOG){
        attribute = new osg::Fog();
        ((ive::Fog*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVELIGHT){
        attribute = new osg::Light();
        ((ive::Light*)(attribute.get()))->read(this);
    }
    else if(attributeID == IVEPOLYGONSTIPPLE){
        attribute = new osg::PolygonStipple();
        ((ive::PolygonStipple*)(attribute.get()))->read(this);
    }
    else{
        throwException("Unknown StateAttribute in StateSet::read()");
        return 0;
    }

    // exit early if an exception has been set.
    if (getException()) return 0;

    // and add it to the stateattribute map,
    _stateAttributeMap[id] = attribute;


    if (_verboseOutput) std::cout<<"read/writeStateAttribute() ["<<id<<"]"<<std::endl;

    return attribute.get();
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
    osg::ref_ptr<osg::Uniform> uniform = new osg::Uniform();

    // read its properties from stream
    ((ive::Uniform*)(uniform.get()))->read(this);

    // exit early if an exception has been set.
    if (getException()) return 0;

    // and add it to the uniform map,
    _uniformMap[id] = uniform;


    if (_verboseOutput) std::cout<<"read/writeUniform() ["<<id<<"]"<<std::endl;

    return uniform.get();
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
    osg::ref_ptr<osg::Shader> shader = new osg::Shader();

    // read its properties from stream
    ((ive::Shader*)(shader.get()))->read(this);

    // exit early if an exception has been set.
    if (getException()) return 0;

    // and add it to the shader map,
    _shaderMap[id] = shader;


    if (_verboseOutput) std::cout<<"read/writeShader() ["<<id<<"]"<<std::endl;

    return shader.get();
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
    osg::ref_ptr<osg::Drawable> drawable;
    if(drawableTypeID == IVEGEOMETRY)
    {
        drawable = new osg::Geometry();
        ((Geometry*)(drawable.get()))->read(this);
    }
    else if(drawableTypeID == IVESHAPEDRAWABLE)
    {
        drawable = new osg::ShapeDrawable();
        ((ShapeDrawable*)(drawable.get()))->read(this);
    }
    else if(drawableTypeID == IVETEXT){
        drawable = new osgText::Text();
        ((Text*)(drawable.get()))->read(this);
    }
    else if(drawableTypeID == IVEFADETEXT){
        drawable = new osgText::FadeText();
        ((FadeText*)(drawable.get()))->read(this);
    }
    else if(drawableTypeID == IVETEXT3D){
        drawable = new osgText::Text3D();
        ((Text3D*)(drawable.get()))->read(this);
    }
    else
        throwException("Unknown drawable drawableTypeIDentification in Geode::read()");

    // exit early if an exception has been set.
    if (getException()) return 0;


    // and add it to the stateattribute map,
    _drawableMap[id] = drawable;


    if (_verboseOutput) std::cout<<"read/writeDrawable() ["<<id<<"]"<<std::endl;

    return drawable.get();
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
    osg::ref_ptr<osg::Shape> shape;
    if(shapeTypeID == IVESPHERE)
    {
        shape = new osg::Sphere();
        ((Sphere*)(shape.get()))->read(this);
    }
    else if(shapeTypeID == IVEBOX)
    {
        shape = new osg::Box();
        ((Box*)(shape.get()))->read(this);
    }
    else if(shapeTypeID == IVECONE)
    {
        shape = new osg::Cone();
        ((Cone*)(shape.get()))->read(this);
    }
    else if(shapeTypeID == IVECYLINDER)
    {
        shape = new osg::Cylinder();
        ((Cylinder*)(shape.get()))->read(this);
    }
    else if(shapeTypeID == IVECAPSULE)
    {
        shape = new osg::Capsule();
        ((Capsule*)(shape.get()))->read(this);
    }
    else if(shapeTypeID == IVEHEIGHTFIELD)
    {
        shape = new osg::HeightField();
        ((HeightField*)(shape.get()))->read(this);
    }
    else
        throwException("Unknown shape shapeTypeIDentification in Shape::read()");

    // exit early if an exception has been set.
    if (getException()) return 0;

    // and add it to the stateattribute map,
    _shapeMap[id] = shape;


    if (_verboseOutput) std::cout<<"read/writeShape() ["<<id<<"]"<<std::endl;

    return shape.get();
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

    osg::ref_ptr<osg::Node> node;
    int nodeTypeID= peekInt();

    if(nodeTypeID== IVEMATRIXTRANSFORM){
        node = new osg::MatrixTransform();
        ((ive::MatrixTransform*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVECAMERA){
        node = new osg::Camera();
        ((ive::Camera*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVECAMERAVIEW){
        node = new osg::CameraView();
        ((ive::CameraView*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEPOSITIONATTITUDETRANSFORM){
        node = new osg::PositionAttitudeTransform();
        ((ive::PositionAttitudeTransform*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEAUTOTRANSFORM){
        node = new osg::AutoTransform();
        ((ive::AutoTransform*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEDOFTRANSFORM){
        node = new osgSim::DOFTransform();
        ((ive::DOFTransform*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVETRANSFORM){
        node = new osg::Transform();
        ((ive::Transform*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVELIGHTSOURCE){
        node = new osg::LightSource();
        ((ive::LightSource*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVETEXGENNODE){
        node = new osg::TexGenNode();
        ((ive::TexGenNode*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVECLIPNODE){
        node = new osg::ClipNode();
        ((ive::ClipNode*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVESEQUENCE){
        node = new osg::Sequence();
        ((ive::Sequence*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVELOD){
        node = new osg::LOD();
        ((ive::LOD*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEPAGEDLOD){
        node = new osg::PagedLOD();
        ((ive::PagedLOD*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVECOORDINATESYSTEMNODE){
        node = new osg::CoordinateSystemNode();
        ((ive::CoordinateSystemNode*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVESWITCH){
        node = new osg::Switch();
        ((ive::Switch*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEMULTISWITCH){
        node = new osgSim::MultiSwitch();
        ((ive::MultiSwitch*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEIMPOSTOR){
        node = new osgSim::Impostor();
        ((ive::Impostor*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEOCCLUDERNODE){
        node = new osg::OccluderNode();
        ((ive::OccluderNode*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEOCCLUSIONQUERYNODE){
        node = new osg::OcclusionQueryNode();
        ((ive::OcclusionQueryNode*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEVISIBILITYGROUP){
        node = new osgSim::VisibilityGroup();
        ((ive::VisibilityGroup*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEPROXYNODE){
        node = new osg::ProxyNode();
        ((ive::ProxyNode*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEGROUP){
        node = new osg::Group();
        ((ive::Group*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEBILLBOARD){
        node = new osg::Billboard();
        ((ive::Billboard*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEGEODE){
        node = new osg::Geode();
        ((ive::Geode*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVELIGHTPOINTNODE){
        node = new osgSim::LightPointNode();
        ((ive::LightPointNode*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEMULTITEXTURECONTROL){
        node = new osgFX::MultiTextureControl();
        ((ive::MultiTextureControl*)(node.get()))->read(this);
    }

    else if(nodeTypeID== IVEANISOTROPICLIGHTING){
        node = new osgFX::AnisotropicLighting();
        ((ive::AnisotropicLighting*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEBUMPMAPPING){
        node = new osgFX::BumpMapping();
        ((ive::BumpMapping*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVECARTOON){
        node = new osgFX::Cartoon();
        ((ive::Cartoon*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVESCRIBE){
        node = new osgFX::Scribe();
        ((ive::Scribe*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVESPECULARHIGHLIGHTS){
        node = new osgFX::SpecularHighlights();
        ((ive::SpecularHighlights*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVETERRAINTILE){
        node = new osgTerrain::TerrainTile();
        ((ive::TerrainTile*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVETERRAIN){
        node = new osgTerrain::Terrain();
        ((ive::Terrain*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEVOLUME){
        node = new osgVolume::Volume();
        ((ive::Volume*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVEVOLUMETILE){
        node = new osgVolume::VolumeTile();
        ((ive::VolumeTile*)(node.get()))->read(this);
    }
    else if(nodeTypeID== IVENODE){
        node = new osg::Node();
        ((ive::Node*)(node.get()))->read(this);
    }
    else{
        throwException("Unknown node identification in DataInputStream::readNode()");
    }

    // exit early if an exception has been set.
    if (getException()) return 0;

    // and add it to the node map,
    _nodeMap[id] = node;


    if (_verboseOutput) std::cout<<"read/writeNode() ["<<id<<"]"<<std::endl;

    return node.get();
}

osgTerrain::Layer* DataInputStream::readLayer()
{
    // Read node unique ID.
    int id = readInt();
    if (id<0) return 0;

    // See if layer is already in the list.
    LayerMap::iterator itr= _layerMap.find(id);
    if (itr!=_layerMap.end()) return itr->second.get();

    // Layer is not in list.
    // Create a new Layer,

    osg::ref_ptr<osgTerrain::Layer> layer = 0;
    int layerid = peekInt();

    if (layerid==IVEHEIGHTFIELDLAYER)
    {
        layer = new osgTerrain::HeightFieldLayer;
        ((ive::HeightFieldLayer*)(layer.get()))->read(this);
    }
    else if (layerid==IVEIMAGELAYER)
    {
        layer = new osgTerrain::ImageLayer;
        ((ive::ImageLayer*)(layer.get()))->read(this);
    }
    else if (layerid==IVESWITCHLAYER)
    {
        layer = new osgTerrain::SwitchLayer;
        ((ive::SwitchLayer*)(layer.get()))->read(this);
    }
    else if (layerid==IVECOMPOSITELAYER)
    {
        layer = new osgTerrain::CompositeLayer;
        ((ive::CompositeLayer*)(layer.get()))->read(this);
    }
    else if (layerid==IVEPROXYLAYER)
    {
        std::string filename = readString();
        osg::ref_ptr<osg::Object> object = osgDB::readObjectFile(filename+".gdal");
        osgTerrain::ProxyLayer* proxyLayer = dynamic_cast<osgTerrain::ProxyLayer*>(object.get());

        osg::ref_ptr<osgTerrain::Locator> locator = readLocator();
        unsigned int minLevel = readUInt();
        unsigned int maxLevel = readUInt();

        if (proxyLayer)
        {
            if (locator.valid()) proxyLayer->setLocator(locator.get());

            proxyLayer->setMinLevel(minLevel);
            proxyLayer->setMaxLevel(maxLevel);
        }

        layer = proxyLayer;
    }
    else{
        throwException("Unknown layer identification in DataInputStream::readLayer()");
    }

    // exit early if an exception has been set.
    if (getException()) return 0;

    // and add it to the node map,
    _layerMap[id] = layer;


    if (_verboseOutput) std::cout<<"read/writeLayer() ["<<id<<"]"<<std::endl;

    return layer.get();
}

osgTerrain::Locator* DataInputStream::readLocator()
{
    // Read locator unique ID.
    int id = readInt();
    if (id<0) return 0;

    // See if stateset is already in the list.
    LocatorMap::iterator itr= _locatorMap.find(id);
    if (itr!=_locatorMap.end()) return itr->second.get();

    // Locator is not in list.
    // Create a new locator,
    osg::ref_ptr<osgTerrain::Locator> locator = new osgTerrain::Locator();

    // read its properties from stream
    ((ive::Locator*)(locator.get()))->read(this);

    // exit early if an exception has been set.
    if (getException()) return 0;

    // and add it to the locator map,
    _locatorMap[id] = locator;

    if (_verboseOutput) std::cout<<"read/writeLocator() ["<<id<<"]"<<std::endl;

    return locator.get();
}


osgVolume::Layer* DataInputStream::readVolumeLayer()
{
    // Read node unique ID.
    int id = readInt();
    if (id<0) return 0;

    // See if layer is already in the list.
    VolumeLayerMap::iterator itr= _volumeLayerMap.find(id);
    if (itr!=_volumeLayerMap.end()) return itr->second.get();

    // Layer is not in list.
    // Create a new Layer,

    osg::ref_ptr<osgVolume::Layer> layer = 0;
    int layerid = peekInt();

    if (layerid==IVEVOLUMEIMAGELAYER)
    {
        layer = new osgVolume::ImageLayer;
        ((ive::VolumeImageLayer*)(layer.get()))->read(this);
    }
    else if (layerid==IVEVOLUMECOMPOSITELAYER)
    {
        layer = new osgVolume::CompositeLayer;
        ((ive::VolumeCompositeLayer*)(layer.get()))->read(this);
    }
    else{
        throwException("Unknown layer identification in DataInputStream::readLayer()");
    }

    // exit early if an exception has been set.
    if (getException()) return 0;

    // and add it to the node map,
    _volumeLayerMap[id] = layer;


    if (_verboseOutput) std::cout<<"read/writeVolumeLayer() ["<<id<<"]"<<std::endl;

    return layer.get();
}

osgVolume::Locator* DataInputStream::readVolumeLocator()
{
    // Read locator unique ID.
    int id = readInt();
    if (id<0) return 0;

    // See if stateset is already in the list.
    VolumeLocatorMap::iterator itr= _volumeLocatorMap.find(id);
    if (itr!=_volumeLocatorMap.end()) return itr->second.get();

    // Locator is not in list.
    // Create a new locator,
    osg::ref_ptr<osgVolume::Locator> locator = new osgVolume::Locator();

    // read its properties from stream
    ((ive::VolumeLocator*)(locator.get()))->read(this);

    // exit early if an exception has been set.
    if (getException()) return 0;

    // and add it to the locator map,
    _volumeLocatorMap[id] = locator;

    if (_verboseOutput) std::cout<<"read/writeVolumeLocator() ["<<id<<"]"<<std::endl;

    return locator.get();
}

osgVolume::Property* DataInputStream::readVolumeProperty()
{
    // Read property unique ID.
    int id = readInt();
    if (id<0) return 0;

    // See if stateset is already in the list.
    VolumePropertyMap::iterator itr= _volumePropertyMap.find(id);
    if (itr!=_volumePropertyMap.end()) return itr->second.get();

    int layerid = peekInt();
    osg::ref_ptr<osgVolume::Property> property = 0;

    if (layerid==IVEVOLUMECOMPOSITEPROPERTY)
    {
        property = new osgVolume::CompositeProperty;
        ((ive::VolumeCompositeProperty*)(property.get()))->read(this);
    }
    else if (layerid==IVEVOLUMESWITCHPROPERTY)
    {
        property = new osgVolume::SwitchProperty;
        ((ive::VolumeSwitchProperty*)(property.get()))->read(this);
    }
    else if (layerid==IVEVOLUMETRANSFERFUNCTIONPROPERTY)
    {
        property = new osgVolume::TransferFunctionProperty;
        ((ive::VolumeTransferFunctionProperty*)(property.get()))->read(this);
    }
    else if (layerid==IVEVOLUMEMAXIMUMINTENSITYPROPERTY)
    {
        property = new osgVolume::MaximumIntensityProjectionProperty;
        readInt();
    }
    else if (layerid==IVEVOLUMELIGHTINGPROPERTY)
    {
        property = new osgVolume::LightingProperty;
        readInt();
    }
    else if (layerid==IVEVOLUMEISOSURFACEPROPERTY)
    {
        property = new osgVolume::IsoSurfaceProperty;
        readInt();
        ((ive::VolumeScalarProperty*)(property.get()))->read(this);
    }
    else if (layerid==IVEVOLUMEALPHAFUNCPROPERTY)
    {
        property = new osgVolume::AlphaFuncProperty;
        readInt();
        ((ive::VolumeScalarProperty*)(property.get()))->read(this);
    }
    else if (layerid==IVEVOLUMESAMPLEDENSITYPROPERTY)
    {
        property = new osgVolume::SampleDensityProperty;
        readInt();
        ((ive::VolumeScalarProperty*)(property.get()))->read(this);
    }
    else if (layerid==IVEVOLUMETRANSPARENCYPROPERTY)
    {
        property = new osgVolume::TransparencyProperty;
        readInt();
        ((ive::VolumeScalarProperty*)(property.get()))->read(this);
    }
    else{
        throwException("Unknown layer identification in DataInputStream::readVolumeProperty()");
    }

    // exit early if an exception has been set.
    if (getException()) return 0;

    // and add it to the locator map,
    _volumePropertyMap[id] = property;

    if (_verboseOutput) std::cout<<"read/writeVolumeProperty() ["<<id<<"]"<<std::endl;

    return property.get();
}

osg::Object* DataInputStream::readObject()
{
    int id = readInt();
    if (id<0) return 0;

    if (id==IVENODE)
    {
        return readNode();
    }
    else if (id==IVESTATESET)
    {
        return readStateSet();
    }
    else if (id==IVESTATEATTRIBUTE)
    {
        return readStateAttribute();
    }
    else if (id==IVEDRAWABLE)
    {
        return readDrawable();
    }
    else if (id==IVESHAPEATTRIBUTELIST)
    {
        osg::ref_ptr<osgSim::ShapeAttributeList> sal = new osgSim::ShapeAttributeList;
        ((ive::ShapeAttributeList*)sal.get())->read(this);

        // exit early if an exception has been set.
        if (getException()) return 0;

        return sal.release();
    }

    return 0;
}


