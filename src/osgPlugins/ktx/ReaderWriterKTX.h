/* -*-c++-*- OpenSceneGraph - Copyright (C) 2012 APX Labs, LLC
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include <osgDB/ReaderWriter>

#ifdef _MSC_VER
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

struct KTXTexHeader
{
    uint8_t  identifier[12];
    uint32_t endianness;
    uint32_t glType;
    uint32_t glTypeSize;
    uint32_t glFormat;
    uint32_t glInternalFormat;
    uint32_t glBaseInternalFormat;
    uint32_t pixelWidth;
    uint32_t pixelHeight;
    uint32_t pixelDepth;
    uint32_t numberOfArrayElements;
    uint32_t numberOfFaces;
    uint32_t numberOfMipmapLevels;
    uint32_t bytesOfKeyValueData;
};


class ReaderWriterKTX : public osgDB::ReaderWriter
{
public:
    static const unsigned char FileSignature[12];
    static const uint32_t MyEndian = 0x04030201;
    static const uint32_t NotMyEndian = 0x01020304;

    ReaderWriterKTX();

    virtual const char* className() const;
    virtual ReadResult readImage(std::istream& fin,const osgDB::ReaderWriter::Options* =NULL) const;
    virtual ReadResult readKTXStream(std::istream& fin) const;
    virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const;

private:
    bool correctByteOrder(KTXTexHeader& header) const;
};
