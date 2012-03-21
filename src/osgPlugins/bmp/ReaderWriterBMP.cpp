// -*-c++-*-

/*
 * $Id$
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <osg/Image>
#include <osg/Notify>
#include <osg/Image>
#include <osg/GL>
#include <osg/Endian>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <vector>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


/****************************************************************************
 *
 * Follows is code written by GWM and translated to fit with the OSG Ethos.
 *
 * Ported into the OSG as a plugin, Geoff Michel October 2001.
 * For patches, bugs and new features
 * please send them direct to the OSG dev team.
 *
 **********************************************************************/

// find least-significant (lowest) bit position in 16-bit mask
static unsigned int findLeastSignificantBit(unsigned short mask)
{
    unsigned int shift = 1;
    while ((mask & 0x01) == 0)
    {
        mask >>= 1;
        ++shift;
    }
    return shift;
}

// find most-significant (highest) bit position in 16-bit mask
static unsigned int findMostSignificantBit(unsigned short mask)
{
    unsigned int shift = 16;
    while ((mask & 0x8000) == 0)
    {
        mask <<= 1;
        --shift;
    }
    return shift;
}


/*
 * BMP header
 */
const unsigned short BMP_MAGIC_BM = 0x424D; // 'BM'
const unsigned short BMP_MAGIC_MB = 0x4D42; // 'MB'

struct BMPHeader {
    unsigned short magic; // stored 'BM', but read as ushort
    unsigned int fileSize;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int imageOffset;
};

/*
 * Windows v3 header
 */
enum BMPCOMPRESSION {
    BI_RGB = 0,
    BI_RLE8,
    BI_RLE4,
    BI_BITFIELDS,
    BI_JPEG,
    BI_PNG
};
struct BITMAPINFOHEADER {
    //unsigned int hdrSize;
    int width, height;
    unsigned short colorPlanes;
    unsigned short bitsPerPixel;
    unsigned int compression;
    unsigned int imageSize;
    int horizontalPixelPerMeter;
    int verticalPixelPerMeter;
    unsigned int numColorsInPalette;
    unsigned int numImportantColors;
};

/*
 * OS/2 v1 header
 */
struct BITMAPCOREHEADER {
    //unsigned int hdrSize;
    unsigned short width, height;
    unsigned short colorPlanes;
    unsigned short bitsPerPixel;
};

static unsigned char* bmp_load(std::istream& fin,
        int& width_ret, int& height_ret, int& numComponents_ret)
{
    // actual file size
    fin.seekg(0, std::ios::end);
    size_t actFileSize = fin.tellg();
    fin.seekg(0, std::ios::beg);

    bool swap;

    BMPHeader bmp;
    {
        /*
         * read each part individually to avoid struct packing issues (and #pragma)
         */
        fin.read((char*) &bmp.magic, sizeof(bmp.magic));
        fin.read((char*) &bmp.fileSize, sizeof(bmp.fileSize));
        fin.read((char*) &bmp.reserved1, sizeof(bmp.reserved1));
        fin.read((char*) &bmp.reserved2, sizeof(bmp.reserved2));
        fin.read((char*) &bmp.imageOffset, sizeof(bmp.imageOffset));

        if (bmp.magic != BMP_MAGIC_BM && bmp.magic != BMP_MAGIC_MB)
        {
            OSG_WARN << "Invalid BMP magic\n";
            return 0;
        }

        swap = (bmp.magic == BMP_MAGIC_BM); // means machine is big-endian and must swap
        if (swap)
        {
            OSG_DEBUG << "swap=" << swap << std::endl;
            osg::swapBytes4((char*) &bmp.fileSize);
            osg::swapBytes4((char*) &bmp.imageOffset);
        }

        if (bmp.fileSize != actFileSize)
        {
            OSG_DEBUG << "Stored BMP fileSize=" << bmp.fileSize << " != actual=" << actFileSize << std::endl;
            bmp.fileSize = actFileSize;
        }
    }

    BITMAPINFOHEADER dib;
    unsigned int dibHdrSize;

    /*
     * read DIB header
     */
    fin.read((char*) &dibHdrSize, sizeof(dibHdrSize));
    if (swap)
        osg::swapBytes4((char*) &dibHdrSize);

    if (dibHdrSize == 12)
    {
        /*
         * OS/2 v1
         */
        BITMAPCOREHEADER hdr;

        unsigned int expectHdrSize = sizeof(hdr) + sizeof(dibHdrSize);
        if (expectHdrSize != dibHdrSize)
        {
            OSG_WARN << "Invalid BMP OS/2 v1 header size " << expectHdrSize << " != " << dibHdrSize << std::endl;
            return 0;
        }

        fin.read((char*) &hdr, sizeof(hdr));
        if (swap)
        {
            osg::swapBytes2((char*) &hdr.width);
            osg::swapBytes2((char*) &hdr.height);
            osg::swapBytes2((char*) &hdr.colorPlanes);
            osg::swapBytes2((char*) &hdr.bitsPerPixel);
        }

        // store to BITMAPINFOHEADER
        memset(&dib, 0, sizeof(dib));
        dib.width = hdr.width;
        dib.height = hdr.height;
        dib.colorPlanes = hdr.colorPlanes;
        dib.bitsPerPixel = hdr.bitsPerPixel;
    }
    else if (dibHdrSize == 40 || dibHdrSize == 108 || dibHdrSize == 124)
    {
        /*
         * Windows v3/v4/v5 header
         * reads only the common part (i.e. v3) and skips over the rest
         */
        fin.read((char*) &dib, sizeof(dib));
        if (swap)
        {
            osg::swapBytes4((char*) &dib.width);
            osg::swapBytes4((char*) &dib.height);
            osg::swapBytes2((char*) &dib.colorPlanes);
            osg::swapBytes2((char*) &dib.bitsPerPixel);
            osg::swapBytes4((char*) &dib.compression);
            osg::swapBytes4((char*) &dib.imageSize);
            osg::swapBytes4((char*) &dib.numColorsInPalette);
            osg::swapBytes4((char*) &dib.numImportantColors);
        }
    }
    else
    {
        OSG_WARN << "Unsupported BMP/DIB header size=" << dibHdrSize << std::endl;
        return 0;
    }

    // sanity checks
    if (dib.height < 0)
    {
        OSG_DEBUG << "BMP Image is upside-down\n";
        dib.height *= -1;
    }
    if (dib.colorPlanes != 1)
    {
        OSG_WARN << "Invalid BMP number of color planes=" << dib.colorPlanes << std::endl;
        return 0;
    }
    if (dib.bitsPerPixel == 0)
    {
        OSG_WARN << "Invalid BMP bits/pixel=" << dib.bitsPerPixel << std::endl;
        return 0;
    }
    if (dib.compression != BI_RGB && dib.compression != BI_BITFIELDS)
    {
        OSG_WARN << "Unsupported BMP compression=" << dib.compression << std::endl;
        return 0;
    }

    /*
     * color masks
     */
    unsigned int redMask, greenMask, blueMask;
    if (dib.bitsPerPixel == 16 && dib.compression == BI_BITFIELDS)
    {
        fin.read((char*) &redMask, sizeof(redMask));
        fin.read((char*) &greenMask, sizeof(greenMask));
        fin.read((char*) &blueMask, sizeof(blueMask));
        if (swap)
        {
            osg::swapBytes4((char*) &redMask);
            osg::swapBytes4((char*) &greenMask);
            osg::swapBytes4((char*) &blueMask);
        }
    }
    else
    {
        redMask = 0x7c00;
        greenMask = 0x03e0;
        blueMask = 0x001f;
    }

    // determine shift width...
    unsigned int redShift = findLeastSignificantBit(redMask) - 1;
    unsigned int greenShift = findLeastSignificantBit(greenMask) - 1;
    unsigned int blueShift = findLeastSignificantBit(blueMask) - 1;

    // determine mask width
    unsigned int redMaskWidth = findMostSignificantBit(redMask) - redShift;
    unsigned int greenMaskWidth = findMostSignificantBit(greenMask) - greenShift;
    unsigned int blueMaskWidth = findMostSignificantBit(blueMask) - blueShift;

#if 0
    printf("redMask=%04x/%d/%d greenMask=%04x/%d/%d blueMask=%04x/%d/%d\n",
            redMask, redMaskWidth, redShift,
            greenMask, greenMaskWidth, greenShift,
            blueMask, blueMaskWidth, blueShift);
#endif

    unsigned int imageBytesPerPixel = 0;

    /*
     * color palette
     */
    std::vector<unsigned char> colorPalette;
    if (dib.bitsPerPixel < 16)
    {
        // defaults to 2^n
        if (dib.numColorsInPalette == 0)
            dib.numColorsInPalette = 1 << dib.bitsPerPixel;

        // allocate/read color palette
        imageBytesPerPixel = (dibHdrSize == 12 ? 3 : 4); // OS/2 v1 stores RGB, else RGBA
        colorPalette.resize(dib.numColorsInPalette * imageBytesPerPixel);
        fin.read((char*) &*colorPalette.begin(), colorPalette.size());
    }
    else
    {
        if (dib.bitsPerPixel == 16)
            imageBytesPerPixel = 3;
        else if (dib.bitsPerPixel == 24 || dib.bitsPerPixel == 32)
            imageBytesPerPixel = dib.bitsPerPixel / 8;
        else
        {
            OSG_WARN << "Unsupported BMP bit depth " << dib.bitsPerPixel << std::endl;
            return 0;
        }
    }

    unsigned int where = fin.tellg();
    if (where != bmp.imageOffset)
    {
        // this can happen because we don't fully parse v4/v5 headers
        OSG_DEBUG << "BMP streampos out-of-sync where=" << where << " imageOffset=" << bmp.imageOffset << std::endl;
        fin.seekg(bmp.imageOffset, std::ios::beg); // seek to imageOffset and hope for the best
    }

    /*
     * image data
     */
    const unsigned int imageBytesPerRow = dib.width * imageBytesPerPixel;
    const unsigned int imageBufferSize = imageBytesPerRow * dib.height;
    unsigned char* imageBuffer = new unsigned char[imageBufferSize];
    //printf("imageBytesPerPixel=%u imageBytesPerRow=%u\n", imageBytesPerPixel, imageBytesPerRow);

    // byte/row in BMP image data
    unsigned int bytesPerPixel;
    unsigned int bytesPerRow;
    if (dib.bitsPerPixel >= 8)
    {
        bytesPerPixel = dib.bitsPerPixel / 8;
        bytesPerRow = dib.width * bytesPerPixel;
    }
    else
    {
        bytesPerPixel = 1;
        bytesPerRow = (unsigned int) ceilf(dib.width * (dib.bitsPerPixel / 8.0f));
    }
    const unsigned int bytesPerRowAlign = (unsigned int) ceilf(bytesPerRow / 4.0f) * 4;
    //printf("bytesPerPixel=%u bytesPerRow=%u bytesPerRowAlign=%u\n", bytesPerPixel, bytesPerRow, bytesPerRowAlign);

    std::vector<unsigned char> rowBuffer;
    rowBuffer.resize(bytesPerRowAlign);

    if (dib.bitsPerPixel >= 16)
    {
        unsigned char* imgp = imageBuffer;
        for (int i = 0; i < dib.height; ++i)
        {
            // read row
            unsigned char* rowp = &*rowBuffer.begin();
            fin.read((char*) rowp, rowBuffer.size());

            // copy to image buffer, swap/unpack BGR to RGB(A)
            for (unsigned int j = 0; j < bytesPerRow; j += bytesPerPixel)
            {
                if (dib.bitsPerPixel == 16)
                {
                    // 16-bit RGB -> 24-bit RGB
                    unsigned short rgb16 = (rowp[1] << 8) | rowp[0];
                    if (swap)
                        osg::swapBytes2((char*) &rgb16);

                    imgp[0] = (rgb16 & redMask) >> redShift;
                    imgp[1] = (rgb16 & greenMask) >> greenShift;
                    imgp[2] = (rgb16 & blueMask) >> blueShift;

                    // expand range
                    imgp[0] <<= (8-redMaskWidth);
                    imgp[1] <<= (8-greenMaskWidth);
                    imgp[2] <<= (8-blueMaskWidth);
                }
                else
                {
                    // BGR -> RGB(A)
                    imgp[0] = rowp[2];
                    imgp[1] = rowp[1];
                    imgp[2] = rowp[0];
                    if (imageBytesPerPixel == 4)
                    {
                        imgp[3] = 0xff;
                    }
                }
                imgp += imageBytesPerPixel;
                rowp += bytesPerPixel;
            }
        }
    }
    else
    {
        const int idxPerByte = 8 / dib.bitsPerPixel; // color indices per byte
        const int idxMask = (1 << dib.bitsPerPixel) - 1; // index mask
        //printf("idxPerByte=%d idxMask=%02x\n", idxPerByte, idxMask);

        unsigned char* imgp = imageBuffer;
        for (int i = 0; i < dib.height; ++i)
        {
            // read row
            unsigned char* rowp = &*rowBuffer.begin();
            fin.read((char*) rowp, rowBuffer.size());

            int j = 0;
            while (j < dib.width)
            {
                // unpack bytes/indices to image buffer
                unsigned char val = rowp[0];
                for (int k = 0; k < idxPerByte && j < dib.width; ++k, ++j)
                {
                    unsigned int idx = (val >> ((idxPerByte-1-k) * dib.bitsPerPixel)) & idxMask;
                    idx *= imageBytesPerPixel;
                    imgp[0] = colorPalette[idx+2];
                    imgp[1] = colorPalette[idx+1];
                    imgp[2] = colorPalette[idx+0];
                    if (imageBytesPerPixel == 4)
                    {
                        imgp[3] = 0xff;
                    }
                    imgp += imageBytesPerPixel;
                }
                ++rowp;
            }
        }
    }

    // return result
    width_ret = dib.width;
    height_ret = dib.height;
    numComponents_ret = imageBytesPerPixel;

    return imageBuffer;
}

static bool bmp_save(const osg::Image& img, std::ostream& fout)
{
    BMPHeader bmp;
    const unsigned int bmpHdrSize = 14;

    BITMAPINFOHEADER dib;
    assert(sizeof(dib) == 36);
    const unsigned int dibHdrSize = sizeof(dib) + 4;

    const unsigned int bytesPerRowAlign = ((img.s() * 3 + 3) / 4) * 4;

    bool swap = (osg::getCpuByteOrder() == osg::BigEndian);

    // BMP header
    {
        bmp.magic = BMP_MAGIC_BM;
        bmp.reserved1 = bmp.reserved2 = 0;
        bmp.imageOffset = bmpHdrSize + dibHdrSize;
        bmp.fileSize = bmp.imageOffset + bytesPerRowAlign * img.t();
#if 0
        printf("sizeof(bmp)=%u sizeof(dib)=%u dibHdrSize=%u\n", sizeof(bmp), sizeof(dib), dibHdrSize);
        printf("fileSize=%u imageOffset=%u\n", bmp.fileSize, bmp.imageOffset);
        printf("s=%u t=%u bytesPerRowAlign=%u\n", img.s(), img.t(), bytesPerRowAlign);
#endif

        if (swap)
        {
            // big-endian must swap everything except magic
            osg::swapBytes4((char*) &bmp.fileSize);
            osg::swapBytes4((char*) &bmp.imageOffset);
        }
        else
        {
            // little-endian must swap the magic
            osg::swapBytes2((char*) &bmp.magic);
        }

        fout.write((char*) &bmp.magic, sizeof(bmp.magic));
        fout.write((char*) &bmp.fileSize, sizeof(bmp.fileSize));
        fout.write((char*) &bmp.reserved1, sizeof(bmp.reserved1));
        fout.write((char*) &bmp.reserved2, sizeof(bmp.reserved2));
        fout.write((char*) &bmp.imageOffset, sizeof(bmp.imageOffset));
    }

    // DIB header
    {
        dib.width = img.s();
        dib.height = img.t();
        dib.colorPlanes = 1;
        dib.bitsPerPixel = 24;
        dib.compression = BI_RGB;
        dib.imageSize = bytesPerRowAlign * img.t();
        dib.horizontalPixelPerMeter = 1000;
        dib.verticalPixelPerMeter = 1000;
        dib.numColorsInPalette = 0;
        dib.numImportantColors = 0;

        if (swap) {
            osg::swapBytes4((char*) &dibHdrSize);
            osg::swapBytes4((char*) &dib.width);
            osg::swapBytes4((char*) &dib.height);
            osg::swapBytes2((char*) &dib.colorPlanes);
            osg::swapBytes2((char*) &dib.bitsPerPixel);
            osg::swapBytes4((char*) &dib.imageSize);
            osg::swapBytes4((char*) &dib.horizontalPixelPerMeter);
            osg::swapBytes4((char*) &dib.verticalPixelPerMeter);
        }

        fout.write((char*) &dibHdrSize, sizeof(dibHdrSize));
        fout.write((char*) &dib, sizeof(dib));
    }

    unsigned int pixelFormat = img.getPixelFormat();

    unsigned int r = 0, g = 1, b = 2;
    if ( pixelFormat == GL_BGR || pixelFormat == GL_BGRA )
    {
        r = 2;
        b = 0;
    }

    const unsigned int channelsPerPixel = img.computeNumComponents(pixelFormat);

    std::vector<unsigned char> rowBuffer(bytesPerRowAlign);
    for (int y = 0; y < img.t(); ++y)
    {
        const unsigned char* imgp = img.data(0, y);
        for (int x = 0; x < img.s(); ++x)
        {
            // RGB -> BGR
            unsigned int rowOffs = x * 3, imgOffs = x * channelsPerPixel;
            rowBuffer[rowOffs + 2] = imgp[imgOffs + r];
            rowBuffer[rowOffs + 1] = imgp[imgOffs + g];
            rowBuffer[rowOffs + 0] = imgp[imgOffs + b];
        }
        fout.write((char*) &*rowBuffer.begin(), rowBuffer.size());
    }

    return true;
}


class ReaderWriterBMP : public osgDB::ReaderWriter
{
    public:

        ReaderWriterBMP()
        {
            supportsExtension("bmp","BMP Image format");
        }

        const char* className() const { return "BMP Image Reader"; }


        ReadResult readObject(std::istream& fin, const Options* options = 0) const
        {
            return readImage(fin, options);
        }

        ReadResult readObject(const std::string& file, const Options* options = 0) const
        {
            return readImage(file, options);
        }


        ReadResult readImage(std::istream& fin, const Options* = 0) const
        {
            return readBMPStream(fin);
        }

        ReadResult readImage(const std::string& file, const Options* options = 0) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile(file, options);
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osgDB::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
            if(!istream) return ReadResult::FILE_NOT_HANDLED;

            ReadResult rr = readBMPStream(istream);
            if(rr.validImage()) rr.getImage()->setFileName(file);

            return rr;
        }


        WriteResult writeImage(const osg::Image& image, std::ostream& fout, const Options* = 0) const
        {
            if (bmp_save(image, fout))
                return WriteResult::FILE_SAVED;
            else
                return WriteResult::ERROR_IN_WRITING_FILE;
        }

        WriteResult writeImage(const osg::Image& img, const std::string& fileName, const Options* options = 0) const
        {
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

            osgDB::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
            if (!fout) return WriteResult::ERROR_IN_WRITING_FILE;

            return writeImage(img, fout, options);
        }

    private:
        static ReadResult readBMPStream(std::istream& fin)
        {
            int s, t;
            int internalFormat;

            unsigned char *imageData = bmp_load(fin, s, t, internalFormat);
            if (imageData == 0) return ReadResult::ERROR_IN_READING_FILE;

            unsigned int pixelFormat;
            switch (internalFormat)
            {
            case 1:
                pixelFormat = GL_LUMINANCE;
                break;
            case 2:
                pixelFormat = GL_LUMINANCE_ALPHA;
                break;
            case 3:
                pixelFormat = GL_RGB;
                break;
            default:
                pixelFormat = GL_RGBA;
                break;
            }

            osg::Image* img = new osg::Image;
            img->setImage(s, t, 1,
                internalFormat, pixelFormat, GL_UNSIGNED_BYTE, imageData,
                osg::Image::USE_NEW_DELETE);

            return img;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(bmp, ReaderWriterBMP)
