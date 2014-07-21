// ReaderWriter for pvr images

#if defined(_MSC_VER)
#include <windows.h>
#endif

#include <osg/Image>
#include <osg/Notify>

#include <osg/Geode>

#include <osg/GL>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <osg/Types>

using namespace osg;

#define PVR_TEXTURE_FLAG_TYPE_MASK    0xff

static char gPVRTexIdentifier[5] = "PVR!";

enum
{
  kPVRTextureFlagTypePVRTC_2 = 12,
  kPVRTextureFlagTypePVRTC_4,
  kPVRTextureFlagTypeOGLPVRTC_2 = 24,
  kPVRTextureFlagTypeOGLPVRTC_4,
  kPVRTextureFlagTypeETC = 54
};

typedef struct _PVRTexHeader
{
  uint32_t headerLength;
  uint32_t height;
  uint32_t width;
  uint32_t numMipmaps;
  uint32_t flags;
  uint32_t dataLength;
  uint32_t bpp;
  uint32_t bitmaskRed;
  uint32_t bitmaskGreen;
  uint32_t bitmaskBlue;
  uint32_t bitmaskAlpha;
  uint32_t pvrTag;
  uint32_t numSurfs;

  typedef unsigned char * BytePtr;

  bool needsBytesSwapped()
  {
    union {
      int testWord;
      char testByte[sizeof(int)];
    }endianTest;
    endianTest.testWord = 1;
    if( endianTest.testByte[0] == 1 )
      return false;
    else
      return true;
  }

  template <class T>
  inline void swapBytes(  T &s )
  {
    if( sizeof( T ) == 1 )
      return;

    T d = s;
    BytePtr sptr = (BytePtr)&s;
    BytePtr dptr = &(((BytePtr)&d)[sizeof(T)-1]);

    for( unsigned int i = 0; i < sizeof(T); i++ )
      *(sptr++) = *(dptr--);
  }

  void swapBytes()
  {
    swapBytes(headerLength);
    swapBytes(height);
    swapBytes(width);
    swapBytes(numMipmaps);
    swapBytes(flags);
    swapBytes(dataLength);
    swapBytes(bpp);
    swapBytes(bitmaskRed);
    swapBytes(bitmaskGreen);
    swapBytes(bitmaskBlue);
    swapBytes(bitmaskAlpha);
    swapBytes(pvrTag);
    swapBytes(numSurfs);
  }

} PVRTexHeader;


class ReaderWriterPVR : public osgDB::ReaderWriter
{
public:

    ReaderWriterPVR()
    {
        supportsExtension("pvr","PVR image format");
    }

    virtual const char* className() const { return "PVR Image Reader/Writer"; }


    ReadResult readPVRStream(std::istream& fin) const
    {
        PVRTexHeader header;

        fin.read((char*)&header, sizeof(PVRTexHeader));
        if(!fin.good()){
            osg::notify(osg::WARN) << "Failed to read pvr header." << std::endl;
            return ReadResult::ERROR_IN_READING_FILE;
        }

        if(header.needsBytesSwapped())
            header.swapBytes();

        if(gPVRTexIdentifier[0] != static_cast<char>((header.pvrTag >>  0) & 0xff) ||
           gPVRTexIdentifier[1] != static_cast<char>((header.pvrTag >>  8) & 0xff) ||
           gPVRTexIdentifier[2] != static_cast<char>((header.pvrTag >> 16) & 0xff) ||
           gPVRTexIdentifier[3] != static_cast<char>((header.pvrTag >> 24) & 0xff))
            {
                osg::notify(osg::WARN) << "Failed to verify pvr header: " << ((header.pvrTag >>  0) & 0xff) << ", " << ((header.pvrTag >>  8) & 0xff) << ", " << ((header.pvrTag >>  16) & 0xff) << ", " << ((header.pvrTag >>  24) & 0xff) << std::endl;
                return ReadResult::FILE_NOT_HANDLED;
            }


        uint32_t formatFlags = header.flags & PVR_TEXTURE_FLAG_TYPE_MASK;
        GLenum internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
        uint32_t width, height;

        if(formatFlags == kPVRTextureFlagTypePVRTC_4 || formatFlags == kPVRTextureFlagTypePVRTC_2 ||
           formatFlags == kPVRTextureFlagTypeOGLPVRTC_4 || formatFlags == kPVRTextureFlagTypeOGLPVRTC_2 ||
           formatFlags == kPVRTextureFlagTypeETC){
            if(formatFlags == kPVRTextureFlagTypePVRTC_4 || formatFlags == kPVRTextureFlagTypeOGLPVRTC_4)
                internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
            else if(formatFlags == kPVRTextureFlagTypePVRTC_2 || formatFlags == kPVRTextureFlagTypeOGLPVRTC_2)
                internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
            else if(formatFlags == kPVRTextureFlagTypeETC)
                internalFormat = GL_ETC1_RGB8_OES;

            width = header.width;
            height = header.height;

            osg::ref_ptr<osg::Image> image = new osg::Image;
            if (!image) return ReadResult::INSUFFICIENT_MEMORY_TO_LOAD;

            unsigned char *imageData = new unsigned char[header.dataLength];
            if (!imageData) return ReadResult::INSUFFICIENT_MEMORY_TO_LOAD;

            fin.read((char*)imageData, header.dataLength);
            if(!fin.good())
            {
                delete [] imageData;
                return ReadResult::ERROR_IN_READING_FILE;
            }

            image->setImage(header.width, header.height, 1,
                            internalFormat,    internalFormat,
                            GL_UNSIGNED_BYTE,
                            imageData,
                            osg::Image::USE_NEW_DELETE);

            uint32_t dataOffset = 0;
            uint32_t blockSize = 0, widthBlocks = 0, heightBlocks = 0;
            uint32_t bpp = 4;

            osg::Image::MipmapDataType mipmapdata;

            // Calculate the data size for each texture level and respect the minimum number of blocks
            while(dataOffset < header.dataLength){
                if(formatFlags == kPVRTextureFlagTypePVRTC_4 || formatFlags == kPVRTextureFlagTypeOGLPVRTC_4){
                    blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
                    widthBlocks = width / 4;
                    heightBlocks = height / 4;
                    bpp = 4;
                }else if(formatFlags == kPVRTextureFlagTypeETC){
                    blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
                    widthBlocks = width / 4;
                    heightBlocks = height / 4;
                    bpp = 4;
                }else{
                    blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
                    widthBlocks = width / 8;
                    heightBlocks = height / 4;
                    bpp = 2;
                }

                // Clamp to minimum number of blocks
                if(widthBlocks < 2)
                    widthBlocks = 2;
                if(heightBlocks < 2)
                    heightBlocks = 2;

                if(dataOffset > 0)
                    mipmapdata.push_back(dataOffset);

                dataOffset += widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);

                width = osg::maximum(width >> 1, (uint32_t)1);
                height = osg::maximum(height >> 1, (uint32_t)1);
            }

            if(!mipmapdata.empty())
                image->setMipmapLevels(mipmapdata);

            return image.get();
        }

        osg::notify(osg::WARN) << "Failed to read pvr data." << std::endl;
        return ReadResult::FILE_NOT_HANDLED;
    }

    virtual ReadResult readObject(std::istream& fin,const osgDB::ReaderWriter::Options* options =NULL) const
    {
        return readImage(fin, options);
    }

    virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
    {
        return readImage(file, options);
    }

    virtual ReadResult readImage(std::istream& fin,const osgDB::ReaderWriter::Options* =NULL) const
    {
        return readPVRStream(fin);
    }

    virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if(!acceptsExtension(ext))
            return ReadResult::FILE_NOT_HANDLED;

        std::string fileName = osgDB::findDataFile(file, options);
        if(fileName.empty())
            return ReadResult::FILE_NOT_FOUND;

        std::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
        if(!istream) return ReadResult::FILE_NOT_HANDLED;
        ReadResult rr = readPVRStream(istream);
        if(rr.validImage()) rr.getImage()->setFileName(file);
        return rr;
    }

};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(pvr, ReaderWriterPVR)
