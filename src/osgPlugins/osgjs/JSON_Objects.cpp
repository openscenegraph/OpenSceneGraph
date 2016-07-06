/*  -*-c++-*-
 *  Copyright (C) 2010 Cedric Pinson <cedric.pinson@plopbyte.net>
 */

#include "JSON_Objects"
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osg/Material>
#include <osg/BlendFunc>
#include <osg/BlendColor>
#include <osg/Texture>
#include <osg/CullFace>
#include <osg/Texture2D>
#include <osg/Texture1D>
#include <osg/Image>
#include <sstream>
#include "WriteVisitor"

int JSONObjectBase::level = 0;
unsigned int JSONObject::uniqueID = 0;


std::string JSONObjectBase::indent()
{
    std::string str;
    for (int i = 0; i < JSONObjectBase::level; ++i) {
        str += "  ";
    }
    return str;
}


void JSONMatrix::write(json_stream& str, WriteVisitor& visitor)
{
    str << "[ ";
    for (unsigned int i = 0; i < _array.size(); i++) {
        _array[i]->write(str, visitor);
        if (i != _array.size() -1)
            str << ", ";
    }
    str <<  " ]";
}


JSONObject::JSONObject(const unsigned int id, const std::string& bufferName)
{
    _bufferName = bufferName;
    _maps["UniqueID"] = new JSONValue<unsigned int>(id);
}

void JSONObject::addUniqueID()
{
    if(_maps.find("UniqueID") == _maps.end()) {
        _maps["UniqueID"] = new JSONValue<unsigned int>(JSONObject::uniqueID ++);
    }
}

unsigned int JSONObject::getUniqueID() const
{
    JSONMap::const_iterator iterator = _maps.find("UniqueID");
    if(iterator == _maps.end()) {
        return 0xffffffff;
    }
    const JSONValue<unsigned int>* uid = dynamic_cast<JSONValue<unsigned int>*>(iterator->second.get());
    return uid ? uid->getValue() : 0xffffffff;
}

void JSONObject::addChild(const std::string& type, JSONObject* child)
{
    if (!getMaps()["Children"])
        getMaps()["Children"] = new JSONArray;

    JSONObject* jsonObject = new JSONObject();
    jsonObject->getMaps()[type] = child;
    getMaps()["Children"]->asArray()->getArray().push_back(jsonObject);
}

bool JSONObject::isVarintableIntegerBuffer(osg::Array const* array) const
{
    // Return true for buffers representing integer values and that therefore
    // can be binary encoded compactly using the varint protocol.
    // Note: as Byte/UByte array are already compact we do not consider
    //       them as compactable
    bool isInteger = false;
    switch(static_cast<int>(array->getType()))
    {
        case osg::Array::IntArrayType:
            isInteger = (dynamic_cast<osg::IntArray const*>(array) != NULL);
            break;
        case osg::Array::ShortArrayType:
            isInteger = (dynamic_cast<osg::ShortArray const*>(array) != NULL);
            break;

        case osg::Array::UIntArrayType:
            isInteger = (dynamic_cast<osg::UIntArray const*>(array) != NULL);
            break;
        case osg::Array::UShortArrayType:
            isInteger = (dynamic_cast<osg::UShortArray const*>(array) != NULL);
            break;

        case osg::Array::Vec2iArrayType:
            isInteger = (dynamic_cast<osg::Vec2iArray const*>(array) != NULL);
            break;
        case osg::Array::Vec3iArrayType:
            isInteger = (dynamic_cast<osg::Vec3iArray const*>(array) != NULL);
            break;
        case osg::Array::Vec4iArrayType:
            isInteger = (dynamic_cast<osg::Vec4iArray const*>(array) != NULL);
            break;

        case osg::Array::Vec2uiArrayType:
            isInteger = (dynamic_cast<osg::Vec2uiArray const*>(array) != NULL);
            break;
        case osg::Array::Vec3uiArrayType:
            isInteger = (dynamic_cast<osg::Vec3uiArray const*>(array) != NULL);
            break;
        case osg::Array::Vec4uiArrayType:
            isInteger = (dynamic_cast<osg::Vec4uiArray const*>(array) != NULL);
            break;

        case osg::Array::Vec2sArrayType:
            isInteger = (dynamic_cast<osg::Vec2sArray const*>(array) != NULL);
            break;
        case osg::Array::Vec3sArrayType:
            isInteger = (dynamic_cast<osg::Vec3sArray const*>(array) != NULL);
            break;
        case osg::Array::Vec4sArrayType:
            isInteger = (dynamic_cast<osg::Vec4sArray const*>(array) != NULL);
            break;

        case osg::Array::Vec2usArrayType:
            isInteger = (dynamic_cast<osg::Vec2usArray const*>(array) != NULL);
            break;
        case osg::Array::Vec3usArrayType:
            isInteger = (dynamic_cast<osg::Vec3usArray const*>(array) != NULL);
            break;
        case osg::Array::Vec4usArrayType:
            isInteger = (dynamic_cast<osg::Vec4usArray const*>(array) != NULL);
            break;

        default:
            isInteger = false;
            break;
    }
    return isInteger;
}

void JSONObject::encodeArrayAsVarintBuffer(osg::Array const* array, std::vector<uint8_t>& buffer) const
{
    switch(static_cast<int>(array->getType()))
    {
        case osg::Array::IntArrayType:
            dumpVarintValue<osg::IntArray>(buffer, dynamic_cast<osg::IntArray const*>(array), false);
            break;
        case osg::Array::ShortArrayType:
            dumpVarintValue<osg::ShortArray>(buffer, dynamic_cast<osg::ShortArray const*>(array), false);
            break;

        case osg::Array::UIntArrayType:
            dumpVarintValue<osg::UIntArray>(buffer, dynamic_cast<osg::UIntArray const*>(array), true);
            break;
        case osg::Array::UShortArrayType:
            dumpVarintValue<osg::UShortArray>(buffer, dynamic_cast<osg::UShortArray const*>(array), true);
            break;

        case osg::Array::Vec2iArrayType:
            dumpVarintVector<osg::Vec2iArray>(buffer, dynamic_cast<osg::Vec2iArray const*>(array), false);
            break;
        case osg::Array::Vec3iArrayType:
            dumpVarintVector<osg::Vec3iArray>(buffer, dynamic_cast<osg::Vec3iArray const*>(array), false);
            break;
        case osg::Array::Vec4iArrayType:
            dumpVarintVector<osg::Vec4iArray>(buffer, dynamic_cast<osg::Vec4iArray const*>(array), false);
            break;

        case osg::Array::Vec2sArrayType:
            dumpVarintVector<osg::Vec2sArray>(buffer, dynamic_cast<osg::Vec2sArray const*>(array), false);
            break;
        case osg::Array::Vec3sArrayType:
            dumpVarintVector<osg::Vec3sArray>(buffer, dynamic_cast<osg::Vec3sArray const*>(array), false);
            break;
        case osg::Array::Vec4sArrayType:
            dumpVarintVector<osg::Vec4sArray>(buffer, dynamic_cast<osg::Vec4sArray const*>(array), false);
            break;


        case osg::Array::Vec2uiArrayType:
            dumpVarintVector<osg::Vec2uiArray>(buffer, dynamic_cast<osg::Vec2uiArray const*>(array), true);
            break;
        case osg::Array::Vec3uiArrayType:
            dumpVarintVector<osg::Vec3uiArray>(buffer, dynamic_cast<osg::Vec3uiArray const*>(array), true);
            break;
        case osg::Array::Vec4uiArrayType:
            dumpVarintVector<osg::Vec4uiArray>(buffer, dynamic_cast<osg::Vec4uiArray const*>(array), true);
            break;

        case osg::Array::Vec2usArrayType:
            dumpVarintVector<osg::Vec2usArray>(buffer, dynamic_cast<osg::Vec2usArray const*>(array), true);
            break;
        case osg::Array::Vec3usArrayType:
            dumpVarintVector<osg::Vec3usArray>(buffer, dynamic_cast<osg::Vec3usArray const*>(array), true);
            break;
        case osg::Array::Vec4usArrayType:
            dumpVarintVector<osg::Vec4usArray>(buffer, dynamic_cast<osg::Vec4usArray const*>(array), true);
            break;

        default:
            break;
    }
}

template<typename T>
void JSONObject::dumpVarintVector(std::vector<uint8_t>& oss, T const* buffer, bool isUnsigned) const
{
    if (!buffer) return;

    unsigned int n = buffer->getDataSize();
    for(typename T::const_iterator it = buffer->begin() ; it != buffer->end() ; ++ it) {
        for(unsigned int i = 0 ; i < n ; ++ i) {
            unsigned int value = isUnsigned ? (*it)[i] : JSONObject::toVarintUnsigned((*it)[i]);

            std::vector<uint8_t> encoding = varintEncoding(value);
            oss.insert(oss.end(), encoding.begin(), encoding.end());
        }
    }
}

template<typename T>
void JSONObject::dumpVarintValue(std::vector<uint8_t>& oss, T const* buffer, bool isUnsigned) const
{
    if (!buffer) return;

    for(typename T::const_iterator it = buffer->begin() ; it != buffer->end() ; ++ it) {
        unsigned int value = isUnsigned ? (*it) : JSONObject::toVarintUnsigned(*it);

        std::vector<uint8_t> encoding = varintEncoding(value);
        oss.insert(oss.end(), encoding.begin(), encoding.end());
    }
}

// varint encoding adapted from http://stackoverflow.com/questions/19758270/read-varint-from-linux-sockets
std::vector<uint8_t> JSONObject::varintEncoding(unsigned int value) const
{
    std::vector<uint8_t> buffer;

    do {
        uint8_t next_byte = value & 0x7F;
        value >>= 7;
        if (value) {
            next_byte |= 0x80;
        }
        buffer.push_back(next_byte);
    }
    while (value);

    return buffer;
}

static void writeEntry(json_stream& str, const std::string& key, JSONObject::JSONMap& map, WriteVisitor& visitor)
{
    if (key.empty())
        return;

    JSONObject::JSONMap::iterator keyValue = map.find(key);
    if ( keyValue != map.end() && keyValue->second.valid() ) {

        str << JSONObjectBase::indent() << '"' << key << '"' << ": ";
        keyValue->second->write(str, visitor);
        map.erase(keyValue);

        if (!map.empty()) {
            str << ",\n";
        }
    }
}

void JSONObject::writeOrder(json_stream& str, const std::vector<std::string>& order, WriteVisitor& visitor)
{
    str << "{" << std::endl;
    JSONObjectBase::level++;
    for (unsigned int i = 0; i < order.size(); i++) {
        writeEntry(str, order[i], _maps, visitor);
    }

    while(!_maps.empty()) {
        std::string key = _maps.begin()->first;
        writeEntry(str, key, _maps, visitor);
    }

    JSONObjectBase::level--;
    str << std::endl << JSONObjectBase::indent() << "}";
}

void JSONObject::write(json_stream& str, WriteVisitor& visitor)
{
    OrderList defaultOrder;
    defaultOrder.push_back("UniqueID");
    defaultOrder.push_back("Name");
    defaultOrder.push_back("TargetName");
    writeOrder(str, defaultOrder, visitor);
}


std::pair<unsigned int,unsigned int> JSONVertexArray::writeMergeData(const osg::Array* array,
                                                                     WriteVisitor &visitor,
                                                                     const std::string& filename,
                                                                     std::string& encoding)
{
    std::ofstream& output = visitor.getBufferFile(filename);
    unsigned int offset = output.tellp();

    if(visitor._varint && isVarintableIntegerBuffer(array))
    {
        std::vector<uint8_t> varintByteBuffer;
        encodeArrayAsVarintBuffer(array, varintByteBuffer);
        output.write((char*)&varintByteBuffer[0], varintByteBuffer.size() * sizeof(uint8_t));
        encoding = std::string("varint");
    }
    else
    {
        const char* b = static_cast<const char*>(array->getDataPointer());
        size_t totalDataSize = array->getTotalDataSize();
        output.write(b, totalDataSize);
    }

    unsigned int fsize = output.tellp();

    // pad to 4 bytes
    unsigned int remainder = fsize % 4;
    if (remainder) {
        unsigned int null = 0;
        output.write((char*) (&null), 4 - remainder);
        fsize = output.tellp();
    }
    return std::pair<unsigned int, unsigned int>(offset, fsize - offset);
}

void JSONVertexArray::write(json_stream& str, WriteVisitor& visitor)
{
    bool _useExternalBinaryArray = visitor._useExternalBinaryArray;
    bool _mergeAllBinaryFiles = visitor._mergeAllBinaryFiles;
    std::string basename = visitor._baseName;

    addUniqueID();

    std::stringstream url;
    if (visitor._useExternalBinaryArray) {
        std::string bufferName = getBufferName();
        if(bufferName.empty())
            bufferName = visitor.getBinaryFilename();

        if (visitor._mergeAllBinaryFiles)
            url << bufferName;
        else
            url << basename << "_" << getUniqueID() << ".bin";
    }

    std::string type;

    osg::ref_ptr<const osg::Array> array = _arrayData;

    switch (array->getType()) {
    case osg::Array::QuatArrayType:
    {
        osg::ref_ptr<osg::Vec4Array> converted = new osg::Vec4Array;
        converted->reserve(array->getNumElements());
        const osg::QuatArray* a = static_cast<const osg::QuatArray*>(array.get());
        for (unsigned int i = 0; i < array->getNumElements(); ++i) {
            converted->push_back(osg::Vec4(static_cast<float>((*a)[i][0]),
                                           static_cast<float>((*a)[i][1]),
                                           static_cast<float>((*a)[i][2]),
                                           static_cast<float>((*a)[i][3])));
        }
        array = converted;
        type = "Float32Array";
        break;
    }
    case osg::Array::FloatArrayType:
    case osg::Array::Vec2ArrayType:
    case osg::Array::Vec3ArrayType:
    case osg::Array::Vec4ArrayType:
        type = "Float32Array";
        break;
    case osg::Array::Vec4ubArrayType:
    {
        osg::ref_ptr<osg::Vec4Array> converted = new osg::Vec4Array;
        converted->reserve(array->getNumElements());

        const osg::Vec4ubArray* a = static_cast<const osg::Vec4ubArray*>(array.get());
        for (unsigned int i = 0; i < a->getNumElements(); ++i) {
            converted->push_back(osg::Vec4( (*a)[i][0]/255.0,
                                            (*a)[i][1]/255.0,
                                            (*a)[i][2]/255.0,
                                            (*a)[i][3]/255.0));
        }
        array = converted;
        type = "Float32Array";
    }
    break;
    case osg::Array::UByteArrayType:
    case osg::Array::Vec2ubArrayType:
    case osg::Array::Vec3ubArrayType:
        type = "Uint8Array";
        break;
    case osg::Array::UShortArrayType:
    case osg::Array::Vec2usArrayType:
    case osg::Array::Vec3usArrayType:
    case osg::Array::Vec4usArrayType:
        type = "Uint16Array";
        break;
    case osg::Array::UIntArrayType:
    case osg::Array::Vec2uiArrayType:
    case osg::Array::Vec3uiArrayType:
    case osg::Array::Vec4uiArrayType:
        type = "Uint32Array";
        break;
    case osg::Array::ByteArrayType:
    case osg::Array::Vec2bArrayType:
    case osg::Array::Vec3bArrayType:
    case osg::Array::Vec4bArrayType:
        type = "Int8Array";
        break;
    case osg::Array::ShortArrayType:
    case osg::Array::Vec2sArrayType:
    case osg::Array::Vec3sArrayType:
    case osg::Array::Vec4sArrayType:
        type = "Int16Array";
        break;
    case osg::Array::IntArrayType:
    case osg::Array::Vec2iArrayType:
    case osg::Array::Vec3iArrayType:
    case osg::Array::Vec4iArrayType:
        type = "Int32Array";
        break;
    default:
        osg::notify(osg::WARN) << "Array of type " << array->getType() << " not supported" << std::endl;
        break;
    }

    str << "{ " << std::endl;
    JSONObjectBase::level++;
    str << JSONObjectBase::indent() << "\"" << type << "\"" << ": { " << std::endl;
    JSONObjectBase::level++;
    if (_useExternalBinaryArray) {
        str << JSONObjectBase::indent() << "\"File\": \"" << osgDB::getSimpleFileName(url.str()) << "\","<< std::endl;
    } else {
        if (array->getNumElements() == 0) {
            str << JSONObjectBase::indent() << "\"Elements\": [ ],";

        } else {

            switch (array->getType()) {
            case osg::Array::FloatArrayType:
            case osg::Array::Vec2ArrayType:
            case osg::Array::Vec3ArrayType:
            case osg::Array::Vec4ArrayType:
            {
                const float* a = static_cast<const float*>(array->getDataPointer());
                unsigned int size = array->getNumElements() * array->getDataSize();
                writeInlineArrayReal<float>(str, size, a);
            }
            break;
            case osg::Array::DoubleArrayType:
            case osg::Array::Vec2dArrayType:
            case osg::Array::Vec3dArrayType:
            case osg::Array::Vec4dArrayType:
            case osg::Array::QuatArrayType:
            {
                const double* a = static_cast<const double*>(array->getDataPointer());
                unsigned int size = array->getNumElements() * array->getDataSize();
                writeInlineArrayReal<double>(str, size, a);
            }
            break;
            case osg::Array::ByteArrayType:
            case osg::Array::Vec2bArrayType:
            case osg::Array::Vec3bArrayType:
            case osg::Array::Vec4bArrayType:
            {
                const char* a = static_cast<const char*>(array->getDataPointer());
                unsigned int size = array->getNumElements() * array->getDataSize();
                writeInlineArray<char, short>(str, size, a); // using short to write readable numbers and not `char`s
            }
            break;
            case osg::Array::UByteArrayType:
            case osg::Array::Vec2ubArrayType:
            case osg::Array::Vec3ubArrayType:
            case osg::Array::Vec4ubArrayType:
            {
                const unsigned char* a = static_cast<const unsigned char*>(array->getDataPointer());
                unsigned int size = array->getNumElements() * array->getDataSize();
                writeInlineArray<unsigned char, unsigned short>(str, size, a); // using short to write readable numbers and not `char`s
            }
            break;
            case osg::Array::ShortArrayType:
            case osg::Array::Vec2sArrayType:
            case osg::Array::Vec3sArrayType:
            case osg::Array::Vec4sArrayType:
            {
                const short* a = static_cast<const short*>(array->getDataPointer());
                unsigned int size = array->getNumElements() * array->getDataSize();
                writeInlineArray<short>(str, size, a);
            }
            break;
            case osg::Array::UShortArrayType:
            case osg::Array::Vec2usArrayType:
            case osg::Array::Vec3usArrayType:
            case osg::Array::Vec4usArrayType:
            {
                const unsigned short* a = static_cast<const unsigned short*>(array->getDataPointer());
                unsigned int size = array->getNumElements() * array->getDataSize();
                writeInlineArray<unsigned short>(str, size, a);
            }
            break;
            case osg::Array::IntArrayType:
            case osg::Array::Vec2iArrayType:
            case osg::Array::Vec3iArrayType:
            case osg::Array::Vec4iArrayType:
            {
                const int* a = static_cast<const int*>(array->getDataPointer());
                unsigned int size = array->getNumElements() * array->getDataSize();
                writeInlineArray<int>(str, size, a);
            }
            break;
            case osg::Array::UIntArrayType:
            case osg::Array::Vec2uiArrayType:
            case osg::Array::Vec3uiArrayType:
            case osg::Array::Vec4uiArrayType:
            {
                const unsigned int* a = static_cast<const unsigned int*>(array->getDataPointer());
                unsigned int size = array->getNumElements() * array->getDataSize();
                writeInlineArray<unsigned int>(str, size, a);
            }
            break;

            case osg::Array::MatrixdArrayType:
            default:
                break;
            }
        }
    }

    str << JSONObjectBase::indent() << "\"Size\": " << array->getNumElements();
    if (_useExternalBinaryArray) {
        str << "," << std::endl;
    } else {
        str << std::endl;
    }

    if (_useExternalBinaryArray) {
        unsigned int size;
        if (_mergeAllBinaryFiles) {
            std::pair<unsigned int, unsigned int> result;
            std::string encoding;
            result = writeMergeData(array.get(), visitor, url.str(), encoding);
            unsigned int offset = result.first;
            size = result.second;
            if(!encoding.empty()) {
                str << JSONObjectBase::indent() << "\"Offset\": " << offset << "," << std::endl;
                str << JSONObjectBase::indent() << "\"Encoding\": \"" << encoding << "\"" << std::endl;
            }
            else {
                str << JSONObjectBase::indent() << "\"Offset\": " << offset << std::endl;
            }
        } else {
            size = writeData(array.get(), url.str());
            str << JSONObjectBase::indent() << "\"Offset\": " << 0 << std::endl;
        }
        std::stringstream b;
        osg::notify(osg::NOTICE) << "TypedArray " << type << " " << url.str() << " ";
        if (size/1024.0 < 1.0) {
            osg::notify(osg::NOTICE) << size << " bytes" << std::endl;
        } else if (size/(1024.0*1024.0) < 1.0) {
            osg::notify(osg::NOTICE) << size/1024.0 << " kb" << std::endl;
        } else {
            osg::notify(osg::NOTICE) << size/(1024.0*1024.0) << " mb" << std::endl;
        }

    }

    JSONObjectBase::level--;
    str << JSONObjectBase::indent() << "}" << std::endl;
    JSONObjectBase::level--;

    str << JSONObjectBase::indent() << "}";
}


JSONVec4Array::JSONVec4Array(const osg::Vec4& v) : JSONVec3Array()
{
    for (int i = 0; i < 4; ++i) {
        _array.push_back(new JSONValue<float>(v[i]));
    }
}

JSONVec2Array::JSONVec2Array(const osg::Vec2& v) : JSONVec3Array()
{
    for (int i = 0; i < 2; ++i) {
        _array.push_back(new JSONValue<float>(v[i]));
    }
}

JSONVec3Array::JSONVec3Array(const osg::Vec3& v)
{
    for (int i = 0; i < 3; ++i) {
        _array.push_back(new JSONValue<float>(v[i]));
    }
}

void JSONVec3Array::write(json_stream& str,WriteVisitor& visitor)
{
    str << "[ ";
    for (unsigned int i = 0; i < _array.size(); i++) {
        if (_array[i].valid()) {
            _array[i]->write(str, visitor);
        } else {
            str << "undefined";
        }
        if (i != _array.size()-1)
            str << ", ";
    }
    str << "]";
}

void JSONArray::write(json_stream& str,WriteVisitor& visitor)
{
    str << "[ ";
    for (unsigned int i = 0; i < _array.size(); i++) {
        if (_array[i].valid()) {
            _array[i]->write(str, visitor);
        } else {
            str << "undefined";
        }
        if (i != _array.size() -1) {
            str << ",";
            str << "\n" << JSONObjectBase::indent();
        }
    }
    //str << std::endl << JSONObjectBase::indent() << "]";
    str << " ]";
}






JSONObject* getDrawMode(GLenum mode)
{
    JSONObject* result = 0;
    switch (mode) {
    case GL_POINTS:
        result = new JSONValue<std::string>("POINTS");
        break;
    case GL_LINES:
        result = new JSONValue<std::string>("LINES");
        break;
    case GL_LINE_LOOP:
        result = new JSONValue<std::string>("LINE_LOOP");
        break;
    case GL_LINE_STRIP:
        result = new JSONValue<std::string>("LINE_STRIP");
        break;
    case GL_TRIANGLES:
        result = new JSONValue<std::string>("TRIANGLES");
        break;
    case GL_POLYGON:
        result = new JSONValue<std::string>("TRIANGLE_FAN");
        break;
    case GL_QUAD_STRIP:
    case GL_TRIANGLE_STRIP:
        result = new JSONValue<std::string>("TRIANGLE_STRIP");
        break;
    case GL_TRIANGLE_FAN:
        result = new JSONValue<std::string>("TRIANGLE_FAN");
        break;
    case GL_QUADS:
        osg::notify(osg::WARN) << "exporting quads will not be able to work on opengl es" << std::endl;
        break;
    }
    return result;
}
