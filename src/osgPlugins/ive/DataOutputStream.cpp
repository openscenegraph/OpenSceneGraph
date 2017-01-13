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
 *                    Updated for light model - Stan Blinov at 25 august 7512 from World Creation (7.09.2004)
 *
 *    Copyright 2003 VR-C
 **********************************************************************/

#include "DataOutputStream.h"
#include "Exception.h"

#include "StateSet.h"
#include "AlphaFunc.h"
#include "BlendColor.h"
#include "Stencil.h"
#include "StencilTwoSided.h"
#include "BlendFunc.h"
#include "BlendEquation.h"
#include "Material.h"
#include "CullFace.h"
#include "ColorMask.h"
#include "Depth.h"
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
#include "Uniform.h"
#include "Shader.h"
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

#include "Volume.h"
#include "VolumeTile.h"
#include "VolumeImageLayer.h"
#include "VolumeCompositeLayer.h"
#include "VolumeLocator.h"
#include "VolumeCompositeProperty.h"
#include "VolumeSwitchProperty.h"
#include "VolumeScalarProperty.h"
#include "VolumeTransferFunctionProperty.h"

#include <osg/Notify>
#include <osg/io_utils>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/fstream>
#include <osgDB/WriteFile>

#include <stdlib.h>
#include <sstream>

using namespace ive;


DataOutputStream::DataOutputStream(std::ostream * ostream, const osgDB::ReaderWriter::Options* options)
{
    _verboseOutput = false;

    _includeImageMode = IMAGE_INCLUDE_DATA;

    _includeExternalReferences     = false;
    _writeExternalReferenceFiles   = false;
    _useOriginalExternalReferences = true;
    _maximumErrorToSizeRatio       = 0.001;

    _outputTextureFiles = false;
    _textureFileNameNumber = 0;

    _options = options;

    _compressionLevel = 0;

    if (options) _filename = options->getPluginStringData("filename");

    if (_filename.empty())
    {
        // initialize _filename to a unique identifier in case a real filename is not supplied
        std::ostringstream filenameBuilder;
        filenameBuilder << "file" << ostream; // use address of ostream to formulate unique filename
        _filename = filenameBuilder.str();
    }

    if (_options.get())
    {
        std::string optionsString = _options->getOptionString();

        if(optionsString.find("noTexturesInIVEFile")!=std::string::npos) {
            setIncludeImageMode(IMAGE_REFERENCE_FILE);
        } else if(optionsString.find("includeImageFileInIVEFile")!=std::string::npos) {
            setIncludeImageMode(IMAGE_INCLUDE_FILE);
        } else if(optionsString.find("compressImageData")!=std::string::npos) {
            setIncludeImageMode(IMAGE_COMPRESS_DATA);
        }
        OSG_DEBUG << "ive::DataOutputStream.setIncludeImageMode()=" << getIncludeImageMode() << std::endl;

        setIncludeExternalReferences(optionsString.find("inlineExternalReferencesInIVEFile")!=std::string::npos);
        OSG_DEBUG << "ive::DataOutputStream.setIncludeExternalReferences()=" << getIncludeExternalReferences() << std::endl;

        setWriteExternalReferenceFiles(optionsString.find("noWriteExternalReferenceFiles")==std::string::npos);
        OSG_DEBUG << "ive::DataOutputStream.setWriteExternalReferenceFiles()=" << getWriteExternalReferenceFiles() << std::endl;

        setUseOriginalExternalReferences(optionsString.find("useOriginalExternalReferences")!=std::string::npos);
        OSG_DEBUG << "ive::DataOutputStream.setUseOriginalExternalReferences()=" << getUseOriginalExternalReferences() << std::endl;

        setOutputTextureFiles(optionsString.find("OutputTextureFiles")!=std::string::npos);
        OSG_DEBUG << "ive::DataOutputStream.setOutputTextureFiles()=" << getOutputTextureFiles() << std::endl;

        _compressionLevel =  (optionsString.find("compressed")!=std::string::npos) ? 1 : 0;
        OSG_DEBUG << "ive::DataOutputStream._compressionLevel=" << _compressionLevel << std::endl;

        std::string::size_type terrainErrorPos = optionsString.find("TerrainMaximumErrorToSizeRatio=");
        if (terrainErrorPos!=std::string::npos)
        {
            std::string::size_type endOfToken = optionsString.find_first_of('=', terrainErrorPos);
            std::string::size_type endOfNumber = optionsString.find_first_of(' ', endOfToken);
            std::string::size_type numOfCharInNumber = (endOfNumber != std::string::npos) ?
                    endOfNumber-endOfToken-1 :
                    optionsString.size()-endOfToken-1;

            if (numOfCharInNumber>0)
            {
                std::string numberString = optionsString.substr(endOfToken+1, numOfCharInNumber);
                _maximumErrorToSizeRatio = osg::asciiToDouble(numberString.c_str());

                OSG_DEBUG<<"TerrainMaximumErrorToSizeRatio = "<<_maximumErrorToSizeRatio<<std::endl;
            }
            else
            {
                OSG_DEBUG<<"Error no value to TerrainMaximumErrorToSizeRatio assigned"<<std::endl;
            }
        }
    }

    #ifndef USE_ZLIB
    if (_compressionLevel>0)
    {
        OSG_NOTICE << "Compression not supported in this .ive version." << std::endl;
        _compressionLevel = 0;
    }
    #endif

    _output_ostream = _ostream = ostream;

    if(!_ostream)
    {
        throwException("DataOutputStream::DataOutputStream(): null pointer exception in argument.");
        return;
    }

    writeUInt(ENDIAN_TYPE) ;
    writeUInt(getVersion());

    writeInt(_compressionLevel);

    if (_compressionLevel>0)
    {

        _ostream = &_compressionStream;
    }
}

DataOutputStream::~DataOutputStream()
{
    if (_compressionLevel>0)
    {
        _ostream = _output_ostream;

        std::string compressionString(_compressionStream.str());
        writeUInt(compressionString.size());

        compress(*_output_ostream, compressionString);
    }
}

#ifdef USE_ZLIB

#include <zlib.h>

#define CHUNK 16384
bool DataOutputStream::compress(std::ostream& fout, const std::string& source) const
{
    int ret, flush = Z_FINISH;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

    int level = 6;
    int stategy = Z_DEFAULT_STRATEGY; // looks to be the best for .osg/.ive files
    //int stategy = Z_FILTERED;
    //int stategy = Z_HUFFMAN_ONLY;
    //int stategy = Z_RLE;

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit2(&strm,
                       level,
                       Z_DEFLATED,
                       15+16, // +16 to use gzip encoding
                       8, // default
                       stategy);
    if (ret != Z_OK)
    return false;

    strm.avail_in = source.size();
    strm.next_in = (Bytef*)(&(*source.begin()));

    /* run deflate() on input until output buffer not full, finish
       compression if all of source has been read in */
    do {
        strm.avail_out = CHUNK;
        strm.next_out = out;
        ret = deflate(&strm, flush);    /* no bad return value */

        if (ret == Z_STREAM_ERROR)
        {
            OSG_NOTICE<<"Z_STREAM_ERROR"<<std::endl;
            return false;
        }

        have = CHUNK - strm.avail_out;

        if (have>0) fout.write((const char*)out, have);

        if (fout.fail())
        {
            (void)deflateEnd(&strm);
            return false;
        }
    } while (strm.avail_out == 0);

    /* clean up and return */
    (void)deflateEnd(&strm);
    return true;
}
#else
bool DataOutputStream::compress(std::ostream& fout, const std::string& source) const
{
    return false;
}
#endif

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
    writeDouble(v[0]);
    writeDouble(v[1]);
    writeDouble(v[2]);
    writeDouble(v[3]);

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
void DataOutputStream::writeUInt64(unsigned long long ull){
    _ostream->write((char*)&ull, INT64SIZE);

    if (_verboseOutput) std::cout<<"read/writeUInt64() ["<<ull<<"]"<<std::endl;
}
void DataOutputStream::writeInt64(long long ll){
    _ostream->write((char*)&ll, INT64SIZE);

    if (_verboseOutput) std::cout<<"read/writeInt64() ["<<ll<<"]"<<std::endl;
}
void DataOutputStream::writeUInt64Array(const osg::UInt64Array* a){
    int size = a->getNumElements();
    writeUInt64(size);
    for(int i =0; i<size ;i++){
        writeInt((*a)[i]);
    }

    if (_verboseOutput) std::cout<<"read/writeUInt64Array() ["<<size<<"]"<<std::endl;
}
void DataOutputStream::writeInt64Array(const osg::Int64Array* a){
    int size = a->getNumElements();
    writeInt64(size);
    for(int i =0; i<size ;i++){
        writeInt((*a)[i]);
    }

    if (_verboseOutput) std::cout<<"read/writeInt64Array() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeQuat(const osg::Quat& q){
    writeFloat(q.x());
    writeFloat(q.y());
    writeFloat(q.z());
    writeFloat(q.w());

    if (_verboseOutput) std::cout<<"read/writeQuat() ["<<q<<"]"<<std::endl;
}

void DataOutputStream::writeBinding(osg::Array::Binding b){
    switch(b){
        case osg::Array::BIND_OFF:                           writeChar((char) 0); break;
        case osg::Array::BIND_OVERALL:                       writeChar((char) 1); break;
        case osg::Array::BIND_PER_PRIMITIVE_SET:             writeChar((char) 3); break;
        case osg::Array::BIND_PER_VERTEX:                    writeChar((char) 4); break;
        default: throwException("Unknown binding in DataOutputStream::writeBinding()");
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
         case osg::Array::Vec2dArrayType:
             writeChar((char)15);
             writeVec2dArray(static_cast<const osg::Vec2dArray*>(a));
             break;
         case osg::Array::Vec3dArrayType:
             writeChar((char)16);
             writeVec3dArray(static_cast<const osg::Vec3dArray*>(a));
             break;
         case osg::Array::Vec4dArrayType:
             writeChar((char)17);
             writeVec4dArray(static_cast<const osg::Vec4dArray*>(a));
             break;
         case osg::Array::UInt64ArrayType:
             writeChar((char)18);
             writeUInt64Array(static_cast<const osg::UInt64Array*>(a));
             break;
        default: throwException("Unknown array type in DataOutputStream::writeArray()");
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

void DataOutputStream::writePackedFloatArray(const osg::FloatArray* a, float maxError)
{
    int size = a->getNumElements();
    writeInt(size);
    if (size==0) return;

    float minValue = (*a)[0];
    float maxValue = minValue;
    for(int i=1; i<size; ++i)
    {
        if ((*a)[i]<minValue) minValue = (*a)[i];
        if ((*a)[i]>maxValue) maxValue = (*a)[i];
    }

    if (minValue==maxValue)
    {
        OSG_DEBUG<<"Writing out "<<size<<" same values "<<minValue<<std::endl;

        writeBool(true);
        writeFloat(minValue);
        return;
    }

    writeBool(false);

    int packingSize = 4;
    if (maxError>0.0f)
    {

        //float byteError = 0.0f;
        float byteMultiplier = 255.0f/(maxValue-minValue);
        float byteInvMultiplier = 1.0f/byteMultiplier;

        //float shortError = 0.0f;
        float shortMultiplier = 65535.0f/(maxValue-minValue);
        float shortInvMultiplier = 1.0f/shortMultiplier;

        float max_error_byte = 0.0f;
        float max_error_short = 0.0f;

        for(int i=0; i<size; ++i)
        {
            float value = (*a)[i];
            unsigned char byteValue = (unsigned char)((value-minValue)*byteMultiplier);
            unsigned short shortValue = (unsigned short)((value-minValue)*shortMultiplier);
            float value_byte = minValue + float(byteValue)*byteInvMultiplier;
            float value_short = minValue + float(shortValue)*shortInvMultiplier;

            float error_byte = fabsf(value_byte - value);
            float error_short = fabsf(value_short - value);

            if (error_byte>max_error_byte) max_error_byte = error_byte;
            if (error_short>max_error_short) max_error_short = error_short;
        }

        OSG_DEBUG<<"maxError "<<maxError<<std::endl;
        OSG_DEBUG<<"Values to write "<<size<<" max_error_byte = "<<max_error_byte<<" max_error_short="<<max_error_short<<std::endl;


        if (max_error_byte < maxError) packingSize = 1;
        else if (max_error_short < maxError) packingSize = 2;

        OSG_DEBUG<<"packingSize "<<packingSize<<std::endl;

    }

    if (packingSize==1)
    {
        writeInt(1);

        writeFloat(minValue);
        writeFloat(maxValue);

        float byteMultiplier = 255.0f/(maxValue-minValue);

        for(int i=0; i<size; ++i)
        {
            unsigned char currentValue = (unsigned char)(((*a)[i]-minValue)*byteMultiplier);
            writeUChar(currentValue);
        }
    }
    else if (packingSize==2)
    {
        writeInt(2);

        writeFloat(minValue);
        writeFloat(maxValue);

        float shortMultiplier = 65535.0f/(maxValue-minValue);

        for(int i=0; i<size; ++i)
        {
            unsigned short currentValue = (unsigned short)(((*a)[i]-minValue)*shortMultiplier);
            writeUShort(currentValue);
        }
    }
    else
    {
        writeInt(4);

        for(int i=0; i<size; ++i)
        {
            writeFloat((*a)[i]);
        }

    }

    if (_verboseOutput) std::cout<<"read/writePackedFloatArray() ["<<size<<"]"<<std::endl;
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

void DataOutputStream::writeVec2dArray(const osg::Vec2dArray* a)
{
    int size = a->size();
    writeInt(size);
    for(int i=0;i<size;i++){
        writeVec2d((*a)[i]);
    }

    if (_verboseOutput) std::cout<<"read/writeVec2dArray() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeVec3dArray(const osg::Vec3dArray* a)
{
    int size = a->size();
    writeInt(size);
    for(int i = 0; i < size; i++){
        writeVec3d((*a)[i]);
    }

    if (_verboseOutput) std::cout<<"read/writeVec3dArray() ["<<size<<"]"<<std::endl;
}

void DataOutputStream::writeVec4dArray(const osg::Vec4dArray* a)
{
    int size = a->size();
    writeInt(size);
    for(int i=0;i<size;i++){
        writeVec4d((*a)[i]);
    }

    if (_verboseOutput) std::cout<<"read/writeVec4dArray() ["<<size<<"]"<<std::endl;
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
        else if(dynamic_cast<const osg::BlendColor*>(attribute)){
            ((ive::BlendColor*)(attribute))->write(this);
        }
        else if(dynamic_cast<const osg::Stencil*>(attribute)){
            ((ive::Stencil*)(attribute))->write(this);
        }
        else if(dynamic_cast<const osg::StencilTwoSided*>(attribute)){
            ((ive::StencilTwoSided*)(attribute))->write(this);
        }
        else if(dynamic_cast<const osg::BlendFunc*>(attribute)){
            ((ive::BlendFunc*)(attribute))->write(this);
        }
        else if(dynamic_cast<const osg::BlendEquation*>(attribute)){
            ((ive::BlendEquation*)(attribute))->write(this);
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
        // This is a ColorMask
        else if(dynamic_cast<const osg::ColorMask*>(attribute)){
            ((ive::ColorMask*)(attribute))->write(this);
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
        // This is a LineStipple
        else if(dynamic_cast<const osg::LineStipple*>(attribute)){
            ((ive::LineStipple*)(attribute))->write(this);
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
        // This is a Texture2DArray
        else if(dynamic_cast<const osg::Texture2DArray*>(attribute)){
            ((ive::Texture2DArray*)(attribute))->write(this);
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
        // This is a Program
        else if(dynamic_cast<const osg::Program*>(attribute)){
            ((ive::Program*)(attribute))->write(this);
        }
        // This is a PointSprite
        else if(dynamic_cast<const osg::PointSprite*>(attribute)){
            ((ive::PointSprite*)(attribute))->write(this);
        }
        // This is a Multisample
        else if(dynamic_cast<const osg::Multisample*>(attribute)){
            ((ive::Multisample*)(attribute))->write(this);
        }
        // This is a Fog
        else if(dynamic_cast<const osg::Fog*>(attribute)){
            ((ive::Fog*)(attribute))->write(this);
        }
        // This is a Light
        else if(dynamic_cast<const osg::Light*>(attribute)){
            ((ive::Light*)(attribute))->write(this);
        }
        // This is a PolygonStipple
        else if(dynamic_cast<const osg::PolygonStipple*>(attribute)){
            ((ive::PolygonStipple*)(attribute))->write(this);
        }

        else{
            std::string className = attribute->className();
            throwException(std::string("StateSet::write(): Unknown StateAttribute: ").append(className));
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
            throwException("Unknown drawable in DataOutputStream::writeDrawable()");
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
            throwException("Unknown shape in DataOutputStream::writeShape()");
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
        // register the node.

        int id = _nodeMap.size();
        _nodeMap[node] = id;

        // write the id.
        writeInt(id);

        // this follow code *really* should use a NodeVisitor... Robert Osfield August 2003.

        if(dynamic_cast<const osg::MatrixTransform*>(node)){
            ((ive::MatrixTransform*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::Camera*>(node)){
            ((ive::Camera*)(node))->write(this);
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
        else if(dynamic_cast<const osgSim::MultiSwitch*>(node)){
            ((ive::MultiSwitch*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::OccluderNode*>(node)){
            ((ive::OccluderNode*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::OcclusionQueryNode*>(node)){
            ((ive::OcclusionQueryNode*)(node))->write(this);
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


        else if(dynamic_cast<const osgFX::AnisotropicLighting*>(node)){
            ((ive::AnisotropicLighting*)(node))->write(this);
        }
        else if(dynamic_cast<const osgFX::BumpMapping*>(node)){
            ((ive::BumpMapping*)(node))->write(this);
        }
        else if(dynamic_cast<const osgFX::Cartoon*>(node)){
            ((ive::Cartoon*)(node))->write(this);
        }
        else if(dynamic_cast<const osgFX::Scribe*>(node)){
            ((ive::Scribe*)(node))->write(this);
        }
        else if(dynamic_cast<const osgFX::SpecularHighlights*>(node)){
            ((ive::SpecularHighlights*)(node))->write(this);
        }

        else if(dynamic_cast<const osgTerrain::TerrainTile*>(node)){
            ((ive::TerrainTile*)(node))->write(this);
        }
        else if(dynamic_cast<const osgTerrain::Terrain*>(node)){
            ((ive::Terrain*)(node))->write(this);
        }
        else if(dynamic_cast<const osgVolume::Volume*>(node)){
            ((ive::Volume*)(node))->write(this);
        }

        else if(dynamic_cast<const osg::CoordinateSystemNode*>(node)){
            ((ive::CoordinateSystemNode*)(node))->write(this);
        }

        else if(dynamic_cast<const osgVolume::VolumeTile*>(node)){
            ((ive::VolumeTile*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::Billboard*>(node)){
            ((ive::Billboard*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::Geode*>(node)){
            ((ive::Geode*)(node))->write(this);
        }
        else if(dynamic_cast<const osg::Group*>(node)){
            ((ive::Group*)(node))->write(this);
        }
        else if(dynamic_cast<const osgSim::LightPointNode*>(node)){
            ((ive::LightPointNode*)(node))->write(this);
        }
        else
        {
            if (typeid(node)!=typeid(osg::Node))
            {
                OSG_WARN<<"Unknown node in DataOutputStream::writeNode(..), className()="<<node->className()<<std::endl;
            }

            ((ive::Node*)(node))->write(this);

            // throwException(std::string("Unknown node in Group::write(), className()=")+node->className());
        }

        if (_verboseOutput) std::cout<<"read/writeNode() ["<<id<<"]"<<std::endl;
    }
}

IncludeImageMode DataOutputStream::getIncludeImageMode(const osg::Image* image) const
{
    if (image)
    {
        if (image->getWriteHint()==osg::Image::STORE_INLINE)
        {
            return IMAGE_INCLUDE_DATA;
        }
        else if (image->getWriteHint()==osg::Image::EXTERNAL_FILE)
        {
            return IMAGE_REFERENCE_FILE;
        }
    }
    return getIncludeImageMode();
}


void DataOutputStream::writeImage(osg::Image *image)
{
    IncludeImageMode mode = getIncludeImageMode(image);

    if ( getVersion() >= VERSION_0029)
    {
        osg::ImageSequence* is = dynamic_cast<osg::ImageSequence*>(image);
        if (is)
        {
            ((ive::ImageSequence*)(is))->write(this);
        }
        else
        {
            writeInt(IVEIMAGE);
            writeChar(mode);
            writeImage(mode,image);
        }
    }
    else
    {
        writeChar(mode);
        writeImage(mode,image);
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
        {
            if (image)
            {
                // Only include image name in stream
                std::string fileName = image->getFileName();
                // Export an image, if requested
                if (getOutputTextureFiles())
                {
                    if (fileName.empty())
                    { // synthesize a new faux filename
                        fileName = getTextureFileNameForOutput();
                    }
                    osgDB::writeImageFile(*image, fileName);
                }
                writeString(fileName);
            }
            else
            {
                writeString("");
            }
            break;
        }
        case IMAGE_INCLUDE_FILE:
            // Include image file in stream
            if(image && !(image->getFileName().empty())) {
                std::string fullPath = osgDB::findDataFile(image->getFileName(),_options.get());
                osgDB::ifstream infile(fullPath.c_str(), std::ios::in | std::ios::binary);
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
            throwException("DataOutputStream::writeImage(): Invalid IncludeImageMode value.");
            break;
    }
}


void DataOutputStream::writeLayer(const osgTerrain::Layer* layer)
{
    if (layer==0)
    {
        writeInt(-1);
        return;
    }

    LayerMap::iterator itr = _layerMap.find(layer);
    if (itr!=_layerMap.end())
    {
        // Id already exists so just write ID.
        writeInt(itr->second);

        if (_verboseOutput) std::cout<<"read/writeLayer() ["<<itr->second<<"]"<<std::endl;
    }
    else
    {
        // id doesn't exist so create a new ID and
        // register the stateset.

        int id = _layerMap.size();
        _layerMap[layer] = id;

        // write the id.
        writeInt(id);

        if (dynamic_cast<const osgTerrain::HeightFieldLayer*>(layer))
        {
            ((ive::HeightFieldLayer*)(layer))->write(this);
        }
        else if (dynamic_cast<const osgTerrain::ImageLayer*>(layer))
        {
            ((ive::ImageLayer*)(layer))->write(this);
        }
        else if (dynamic_cast<const osgTerrain::SwitchLayer*>(layer))
        {
            ((ive::SwitchLayer*)(layer))->write(this);
        }
        else if (dynamic_cast<const osgTerrain::CompositeLayer*>(layer))
        {
            ((ive::CompositeLayer*)(layer))->write(this);
        }
        else if (dynamic_cast<const osgTerrain::ProxyLayer*>(layer))
        {
            writeInt(IVEPROXYLAYER);
            writeString(layer->getFileName());

            const osgTerrain::Locator* locator = layer->getLocator();
            bool writeOutLocator = locator && !locator->getDefinedInFile();
            writeLocator(writeOutLocator ? locator : 0 );

            writeUInt(layer->getMinLevel());
            writeUInt(layer->getMaxLevel());
        }
        else
        {
            throwException("Unknown layer in DataOutputStream::writeLayer()");
        }
        if (_verboseOutput) std::cout<<"read/writeLayer() ["<<id<<"]"<<std::endl;
    }
}


void DataOutputStream::writeLocator(const osgTerrain::Locator* locator)
{
    if (locator==0)
    {
        writeInt(-1);
        return;
    }

    LocatorMap::iterator itr = _locatorMap.find(locator);
    if (itr!=_locatorMap.end())
    {
        // Id already exists so just write ID.
        writeInt(itr->second);

        if (_verboseOutput) std::cout<<"read/writeLocator() ["<<itr->second<<"]"<<std::endl;
    }
    else
    {
        // id doesn't exist so create a new ID and
        // register the locator.

        int id = _locatorMap.size();
        _locatorMap[locator] = id;

        // write the id.
        writeInt(id);

        // write the locator.
        ((ive::Locator*)(locator))->write(this);

        if (_verboseOutput) std::cout<<"read/writeLocator() ["<<id<<"]"<<std::endl;

    }
}

void DataOutputStream::writeVolumeLayer(const osgVolume::Layer* layer)
{
    if (layer==0)
    {
        writeInt(-1);
        return;
    }

    VolumeLayerMap::iterator itr = _volumeLayerMap.find(layer);
    if (itr!=_volumeLayerMap.end())
    {
        // Id already exists so just write ID.
        writeInt(itr->second);

        if (_verboseOutput) std::cout<<"read/writeLayer() ["<<itr->second<<"]"<<std::endl;
    }
    else
    {
        // id doesn't exist so create a new ID and
        // register the stateset.

        int id = _volumeLayerMap.size();
        _volumeLayerMap[layer] = id;

        // write the id.
        writeInt(id);

        if (dynamic_cast<const osgVolume::ImageLayer*>(layer))
        {
            ((ive::VolumeImageLayer*)(layer))->write(this);
        }
        else if (dynamic_cast<const osgVolume::CompositeLayer*>(layer))
        {
            ((ive::VolumeCompositeLayer*)(layer))->write(this);
        }
        else
        {
            throwException("Unknown layer in DataOutputStream::writeLayer()");
        }
        if (_verboseOutput) std::cout<<"read/writeLayer() ["<<id<<"]"<<std::endl;
    }
}


void DataOutputStream::writeVolumeLocator(const osgVolume::Locator* locator)
{
    if (locator==0)
    {
        writeInt(-1);
        return;
    }

    VolumeLocatorMap::iterator itr = _volumeLocatorMap.find(locator);
    if (itr!=_volumeLocatorMap.end())
    {
        // Id already exists so just write ID.
        writeInt(itr->second);

        if (_verboseOutput) std::cout<<"read/writeVolumeLocator() ["<<itr->second<<"]"<<std::endl;
    }
    else
    {
        // id doesn't exist so create a new ID and
        // register the locator.

        int id = _volumeLocatorMap.size();
        _volumeLocatorMap[locator] = id;

        // write the id.
        writeInt(id);

        // write the locator.
        ((ive::VolumeLocator*)(locator))->write(this);

        if (_verboseOutput) std::cout<<"read/writeVolumeLocator() ["<<id<<"]"<<std::endl;

    }
}

void DataOutputStream::writeVolumeProperty(const osgVolume::Property* property)
{
    if (property==0)
    {
        writeInt(-1);
        return;
    }

    VolumePropertyMap::iterator itr = _volumePropertyMap.find(property);
    if (itr!=_volumePropertyMap.end())
    {
        // Id already exists so just write ID.
        writeInt(itr->second);

        if (_verboseOutput) std::cout<<"read/writeVolumeLocator() ["<<itr->second<<"]"<<std::endl;
    }
    else
    {
        // id doesn't exist so create a new ID and
        // register the locator.

        int id = _volumePropertyMap.size();
        _volumePropertyMap[property] = id;

        // write the id.
        writeInt(id);

        // write the propery
         if (dynamic_cast<const osgVolume::SwitchProperty*>(property))
        {
            ((ive::VolumeSwitchProperty*)(property))->write(this);
        }
        else if (dynamic_cast<const osgVolume::CompositeProperty*>(property))
        {
            ((ive::VolumeCompositeProperty*)(property))->write(this);
        }
        else if (dynamic_cast<const osgVolume::TransferFunctionProperty*>(property))
        {
            ((ive::VolumeTransferFunctionProperty*)(property))->write(this);
        }
        else if (dynamic_cast<const osgVolume::MaximumIntensityProjectionProperty*>(property))
        {
            writeInt(IVEVOLUMEMAXIMUMINTENSITYPROPERTY);
        }
        else if (dynamic_cast<const osgVolume::LightingProperty*>(property))
        {
            writeInt(IVEVOLUMELIGHTINGPROPERTY);
        }
        else if (dynamic_cast<const osgVolume::IsoSurfaceProperty*>(property))
        {
            writeInt(IVEVOLUMEISOSURFACEPROPERTY);
            ((ive::VolumeScalarProperty*)(property))->write(this);
        }
        else if (dynamic_cast<const osgVolume::AlphaFuncProperty*>(property))
        {
            writeInt(IVEVOLUMEALPHAFUNCPROPERTY);
            ((ive::VolumeScalarProperty*)(property))->write(this);
        }
        else if (dynamic_cast<const osgVolume::SampleDensityProperty*>(property))
        {
            writeInt(IVEVOLUMESAMPLEDENSITYPROPERTY);
            ((ive::VolumeScalarProperty*)(property))->write(this);
        }
        else if (dynamic_cast<const osgVolume::TransparencyProperty*>(property))
        {
            writeInt(IVEVOLUMETRANSPARENCYPROPERTY);
            ((ive::VolumeScalarProperty*)(property))->write(this);
        }
        else
        {
            throwException("Unknown layer in DataOutputStream::writVolumeProperty()");
        }

        if (_verboseOutput) std::cout<<"read/writeVolumeProperty() ["<<id<<"]"<<std::endl;

    }
}

void DataOutputStream::writeObject(const osg::Object* object)
{
    const osg::Node* node = dynamic_cast<const osg::Node*>(object);
    if (node)
    {
        writeInt(IVENODE);
        writeNode(node);
        return;
    }

    const osg::StateSet* stateset = dynamic_cast<const osg::StateSet*>(object);
    if (stateset)
    {
        writeInt(IVESTATESET);
        writeStateSet(stateset);
        return;
    }

    const osg::StateAttribute* sa = dynamic_cast<const osg::StateAttribute*>(object);
    if (sa)
    {
        writeInt(IVESTATEATTRIBUTE);
        writeStateAttribute(sa);
        return;
    }

    const osg::Drawable* drawable = dynamic_cast<const osg::Drawable*>(object);
    if (drawable)
    {
        writeInt(IVEDRAWABLE);
        writeDrawable(drawable);
        return;
    }

    const osgSim::ShapeAttributeList* sal = dynamic_cast<const osgSim::ShapeAttributeList*>(object);
    if (sal)
    {
        writeInt(IVESHAPEATTRIBUTELIST);
        ((ive::ShapeAttributeList*)sal)->write(this);
        return;
    }

    // fallback, osg::Object type not supported, so can't write out
    writeInt(-1);
}

std::string DataOutputStream::getTextureFileNameForOutput()
{
    std::string fileName = osgDB::getNameLessExtension(_filename);
    if (_textureFileNameNumber>0)
    {
        std::ostringstream o;
        o << '_' << _textureFileNameNumber;
        fileName += o.str();
    }

    fileName += ".dds";
    ++_textureFileNameNumber;

    return fileName;
}



void DataOutputStream::setExternalFileWritten(const std::string& filename, bool hasBeenWritten)
{
    _externalFileWritten[filename] = hasBeenWritten;
}

bool DataOutputStream::getExternalFileWritten(const std::string& filename) const
{
    ExternalFileWrittenMap::const_iterator itr = _externalFileWritten.find(filename);
    if (itr != _externalFileWritten.end()) return itr->second;
    return false;
}
