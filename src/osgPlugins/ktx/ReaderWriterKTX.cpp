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

#include "ReaderWriterKTX.h"
#include <osg/Endian>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <istream>

const unsigned char ReaderWriterKTX::FileSignature[12] = {
    0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
};

ReaderWriterKTX::ReaderWriterKTX()
{
    supportsExtension("ktx", "KTX image format");
}

const char* ReaderWriterKTX::className() const { return "KTX Image Reader/Writer"; }

bool ReaderWriterKTX::correctByteOrder(KTXTexHeader& header) const
{
    if (header.endianness == MyEndian)
        return true;

    if (header.endianness != NotMyEndian)
        return false;

    for (uint32_t* ptr = &header.glType; ptr <= &header.bytesOfKeyValueData; ++ptr) {
        osg::swapBytes4(reinterpret_cast<char*>(ptr));
    }

    return true;
}

osgDB::ReaderWriter::ReadResult ReaderWriterKTX::readKTXStream(std::istream& fin) const
{
    KTXTexHeader header;
    fin.seekg(0, std::ios::end);
    uint32_t fileLength = fin.tellg();
    fin.seekg(0, std::ios::beg);

    //read in the data for the header and store it
    fin.read((char*)&header, sizeof(KTXTexHeader));
    if(!fin.good())
    {
        OSG_WARN << "Failed to read KTX header." << std::endl;
        return ReadResult(ReadResult::ERROR_IN_READING_FILE);
    }

    //verify that the file is a ktx file from its identifier
    if (memcmp(header.identifier, FileSignature, sizeof(FileSignature)))
    {
        OSG_WARN << "Failed to verify KTX header." << std::endl;
        return ReadResult(ReadResult::FILE_NOT_HANDLED);
    }

    //correct the byte order if the endianess doesn't match
    if(correctByteOrder(header) == false)
    {
        OSG_WARN << "Corrupt KTX header (invalid endianness marker)" << std::endl;
        return ReadResult(ReadResult::FILE_NOT_HANDLED);
    }

    if(header.glFormat == 0)
        header.glFormat = header.glInternalFormat;

    // KTX sets height to 0 for 1D textures, and depth to 0 for both 1D and 2D textures.
    // OpenSceneGraph expects textures to have non-zero dimensions
    if (header.pixelHeight == 0)
        header.pixelHeight = 1;
    if (header.pixelDepth == 0)
        header.pixelDepth = 1;

    if(header.numberOfArrayElements != 0)
    {
        OSG_WARN << "Array textures in KTX files are not supported." << std::endl;
        return ReadResult(ReadResult::FILE_NOT_HANDLED);
    }

    if(header.numberOfFaces != 1) //if this is a cube map
    {
        OSG_WARN << "Cube maps cannot be read directly from KTX files." << std::endl;
        return ReadResult(ReadResult::FILE_NOT_HANDLED);
    }

    if (header.numberOfMipmapLevels == 0)
        header.numberOfMipmapLevels = 1;

    //read keyvalue data. Will be ignoring for now
    fin.ignore(header.bytesOfKeyValueData);

    uint32_t imageSize;
    uint32_t totalImageSize = fileLength -
            (sizeof(KTXTexHeader) + header.bytesOfKeyValueData +
                    (sizeof(imageSize) * header.numberOfMipmapLevels));

    unsigned char* totalImageData = new unsigned char[totalImageSize];
    if (!totalImageData)
        return ReadResult::INSUFFICIENT_MEMORY_TO_LOAD;

    char* imageData = (char*)totalImageData;
    bool byteswapImageData = (header.glTypeSize > 1) && (header.endianness != MyEndian);

    uint32_t totalOffset = 0;
    osg::Image::MipmapDataType mipmapData;

    for(uint32_t mipmapLevel = 0; mipmapLevel < header.numberOfMipmapLevels; mipmapLevel++)
    {
        fin.read((char*)&imageSize, sizeof(imageSize));
        if(!fin.good())
        {
            OSG_WARN << "Failed to read Image Data." << std::endl;
            delete[] totalImageData;
            return ReadResult::ERROR_IN_READING_FILE;
        }
        if (header.endianness != MyEndian)
            osg::swapBytes4(reinterpret_cast<char*>(&imageSize));

        fin.read(imageData, imageSize);
        if(!fin.good())
        {
            OSG_WARN << "Failed to read Image Data." << std::endl;
            delete[] totalImageData;
            return ReadResult::ERROR_IN_READING_FILE;
        }

        if (byteswapImageData)
        {
            char* endData = imageData + imageSize;
            if (header.glTypeSize == 4)
            {
                for(char* longData = imageData; longData < endData; longData += 4)
                {
                    osg::swapBytes4(longData);
                }
            }
            else if (header.glTypeSize == 2)
            {
                for(char* shortData = imageData; shortData < endData; shortData += 2)
                {
                    osg::swapBytes2(shortData);
                }
            }
        }

        if(mipmapLevel > 0)
            mipmapData.push_back(totalOffset);

        //move the offset to the next imageSize data
        totalOffset += imageSize;

        // advance buffer pointer to read next mipmap level
        imageData += imageSize;

        if (mipmapLevel < (header.numberOfMipmapLevels - 1))
        {
            uint32_t mipPadding = 3 - (imageSize + 3) % 4;
            if (mipPadding > 0)
            {
                fin.read(imageData, mipPadding);
                imageData += mipPadding;
                totalOffset += mipPadding;
            }
        }
    }

    osg::ref_ptr<osg::Image> image = new osg::Image;
    if (!image.valid())
    {
        delete[] totalImageData;
        return ReadResult::INSUFFICIENT_MEMORY_TO_LOAD;
    }

    image->setImage(header.pixelWidth, header.pixelHeight, header.pixelDepth,
        header.glInternalFormat, header.glFormat,
        header.glType, totalImageData, osg::Image::USE_NEW_DELETE);

    if (header.numberOfMipmapLevels > 1)
        image->setMipmapLevels(mipmapData);

    return image.get();
}


osgDB::ReaderWriter::ReadResult ReaderWriterKTX::readImage(std::istream& fin,const osgDB::ReaderWriter::Options*) const
{
    return readKTXStream(fin);
}


osgDB::ReaderWriter::ReadResult ReaderWriterKTX::readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if(!acceptsExtension(ext))
        return ReadResult::FILE_NOT_HANDLED;
        
    std::string fileName = osgDB::findDataFile(file, options);
    if(fileName.empty())
        return ReadResult::FILE_NOT_FOUND;
        
    std::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
    if(!istream)
        return ReadResult::ERROR_IN_READING_FILE;

    ReadResult rr = readKTXStream(istream);
    if(rr.validImage())
        rr.getImage()->setFileName(file);

    return rr;
}

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(ktx, ReaderWriterKTX)
