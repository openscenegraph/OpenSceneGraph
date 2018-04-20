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

// Macro similar to what's in FLT/TRP plugins (except it uses wide char under Windows if OSG_USE_UTF8_FILENAME)
#if defined(_WIN32)
#include <windows.h>
#include <osg/Config>
#include <osgDB/ConvertUTF>
#ifdef OSG_USE_UTF8_FILENAME
#define DELETEFILE(file) DeleteFileW(osgDB::convertUTF8toUTF16((file)).c_str())
#else
#define DELETEFILE(file) DeleteFileA((file))
#endif

#else   // Unix

#include <stdio.h>
#define DELETEFILE(file) remove((file))

#endif

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

    //correct the byte order if the endianness doesn't match
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

        if (totalOffset + imageSize > totalImageSize) {
            OSG_WARN << "Failed to read mipmap: " << mipmapLevel << " not enough bytes in file." << std::endl;
            delete[] totalImageData;
            return ReadResult::ERROR_IN_READING_FILE;
        }

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

bool ReaderWriterKTX::writeKTXStream(const osg::Image *img, std::ostream& fout) const {
    KTXTexHeader header;
    memcpy(header.identifier, FileSignature, sizeof(FileSignature));
    header.endianness = MyEndian;
    header.glType = img->getDataType();
    header.glTypeSize = 1;
    if (!img->isCompressed()) {
        header.glTypeSize = img->getPixelSizeInBits() / 8;
    }
    header.glFormat = img->getPixelFormat();
    header.glInternalFormat = img->getInternalTextureFormat();
    header.glBaseInternalFormat = img->computePixelFormat(header.glType);
    header.pixelWidth =img->s();
    header.pixelHeight = img->t() > 1 ? img->t() : 0;
    header.pixelDepth = img->r() > 1 ? img->r() : 0;
    header.numberOfArrayElements = 0;
    header.numberOfFaces = 1;
    header.numberOfMipmapLevels = img->getNumMipmapLevels();
    header.bytesOfKeyValueData = 0;

    fout.write(reinterpret_cast<char*>(&header), sizeof(header)); /* write file header */
    uint32_t imageSize;
    int s = img->s();
    int t = img->t();
    int r = img->r();

    osg::Image::DataIterator imgData(img);
    unsigned int imgDataOffset = 0;
    //write main image: imageSize bytes
    for (uint32_t mipmapLevel = 0; mipmapLevel < header.numberOfMipmapLevels; mipmapLevel++)
    {
        imageSize = osg::Image::computeImageSizeInBytes(s, t, r, img->getPixelFormat(), img->getDataType(), img->getPacking());
        fout.write(reinterpret_cast<char*>(&imageSize), sizeof(imageSize));
        {
            unsigned int bytesWritten = 0;
            unsigned int bytesToWrite = imageSize - bytesWritten;
            while (imgData.valid() && (bytesWritten < imageSize)) {
                unsigned int blockSize = osg::minimum(imgData.size() - imgDataOffset, bytesToWrite);
                fout.write(reinterpret_cast<const char*>(imgData.data()), blockSize);
                bytesWritten += blockSize;
                imgDataOffset += blockSize;
                if (imgData.size() == imgDataOffset) {
                    ++imgData;
                    imgDataOffset = 0;
                }
            }
        }
        if (s > 1) s >>= 1;
        if (t > 1) t >>= 1;
        if (r > 1) r >>= 1;
    }
    // Check for correct saving
    if (fout.fail())
        return false;//report failure

    // If we get that far the file was saved properly
    return true;
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
///////////////////
osgDB::ReaderWriter::WriteResult ReaderWriterKTX::writeObject(const osg::Object& object, const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
    const osg::Image* image = dynamic_cast<const osg::Image*>(&object);
    if (!image) return WriteResult::FILE_NOT_HANDLED;

    return writeImage(*image, file, options);
}

osgDB::ReaderWriter::WriteResult ReaderWriterKTX::writeObject(const osg::Object& object, std::ostream& fout, const Options* options) const
{
    const osg::Image* image = dynamic_cast<const osg::Image*>(&object);
    if (!image) return WriteResult::FILE_NOT_HANDLED;

    return writeImage(*image, fout, options);
}


osgDB::ReaderWriter::WriteResult ReaderWriterKTX::writeImage(const osg::Image &image, const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
    std::string ext = osgDB::getFileExtension(file);
    if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

    osgDB::ofstream fout(file.c_str(), std::ios::out | std::ios::binary);
    if (!fout) return WriteResult::ERROR_IN_WRITING_FILE;

    WriteResult res(writeImage(image, fout, options));
    if (!res.success()) {
        // Remove file on failure
        fout.close();
        DELETEFILE(file.c_str());
        OSG_WARN << "ReaderWriterKTX::writeImage Failed to write " << file << "." << std::endl;
    }
    OSG_INFO << "ReaderWriterKTX::writeImage write " << file << " success;" << image.s() << "x" << image.t() << "x" << image.r() << std::endl;
    return res;
}

osgDB::ReaderWriter::WriteResult ReaderWriterKTX::writeImage(const osg::Image& image, std::ostream& fout, const Options* /*options*/) const
{
//    bool noAutoFlipDDSWrite = options && options->getOptionString().find("ddsNoAutoFlipWrite") != std::string::npos; //maybe for ktx too?
    bool success = writeKTXStream(&image, fout);

    if (success)
        return WriteResult::FILE_SAVED;
    else
        return WriteResult::ERROR_IN_WRITING_FILE;
}
// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(ktx, ReaderWriterKTX)
