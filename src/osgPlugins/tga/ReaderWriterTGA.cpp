#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

/****************************************************************************
 *
 * Follows is code extracted from the simage library.  Original Authors:
 *
 *      Systems in Motion,
 *      <URL:http://www.sim.no>
 *
 *      Peder Blekken <pederb@sim.no>
 *      Morten Eriksen <mortene@sim.no>
 *      Marius Bugge Monsen <mariusbu@sim.no>
 *
 * The original COPYING notice
 *
 *      All files in this library are public domain, except simage_rgb.cpp which is
 *      Copyright (c) Mark J Kilgard <mjk@nvidia.com>. I will contact Mark
 *      very soon to hear if this source also can become public domain.
 *
 *      Please send patches for bugs and new features to: <pederb@sim.no>.
 *
 *      Peder Blekken
 *
 *
 * Ported into the OSG as a plugin, Robert Osfield December 2000.
 * Note, reference above to license of simage_rgb is not relevant to the OSG
 * as the OSG does not use it.  Also for patches, bugs and new features
 * please send them direct to the OSG dev team rather than address above.
 *
 **********************************************************************/

/* */
/* Supported types: */
/*   */
/*  1 (Uncompressed, color-mapped images) */
/*  2 (RGB uncompressed) */
/*  9 RLE color-mapped */
/* 10 RLE RGB */
/* */

#define ERR_NO_ERROR     0
#define ERR_OPEN         1
#define ERR_READ         2
#define ERR_MEM          3
#define ERR_UNSUPPORTED  4

static int tgaerror = ERR_NO_ERROR;
int
simage_tga_error(char * buffer, int buflen)
{
    switch (tgaerror)
    {
        case ERR_OPEN:
            strncpy(buffer, "TGA loader: Error opening file", buflen);
            break;
        case ERR_READ:
            strncpy(buffer, "TGA loader: Error reading file", buflen);
            break;
        case ERR_MEM:
            strncpy(buffer, "TGA loader: Out of memory error", buflen);
            break;
    }
    return tgaerror;
}


/* TODO: */
/* - bottom-up images */
/* - huffman, delta encoding */
static void
convert_16_to_24(const unsigned char * const src, unsigned char * const dest)
{
    /* RGB for opengl, lo-hi 16 bit for TGA */
    unsigned int t0 = src[0];
    unsigned int t1 = src[1];
                                 /* r */
    dest[0] = (unsigned char) (t1 & 0x7c) << 1;
                                 /* g */
    dest[1] = (unsigned char) ((t1 & 0x03) << 6) | ((t0 & 0xe0) >> 2);
                                 /*b */
    dest[2] = (unsigned char) (t0 & 0x1f) << 3;
}


static void
convert_16_to_32(const unsigned char * const src, unsigned char * const dest)
{
    /* RGBA for opengl, lo-hi 16 bit for TGA */
    unsigned int t0 = src[0];
    unsigned int t1 = src[1];
                                 /* r */
    dest[0] = (unsigned char) (t1 & 0x7c) << 1;
                                 /* g */
    dest[1] = (unsigned char) ((t1 & 0x03) << 6) | ((t0 & 0xe0) >> 2);
                                 /*b */
    dest[2] = (unsigned char) (t0 & 0x1f) << 3;
    dest[3] = (t1 & 0x80)?255:0;   /* a */
}


static void
convert_24_to_24(const unsigned char * const src, unsigned char * const dest)
{
    /* RGB for opengl */
    /* BGR for TGA */
    dest[0] = src[2];
    dest[1] = src[1];
    dest[2] = src[0];
}


static void
convert_32_to_32(const unsigned char * const src, unsigned char * const dest)
{
    /* opengl image format is RGBA, not ARGB */
    /* TGA image format is BGRA for 32 bit */
    dest[0] = src[2];
    dest[1] = src[1];
    dest[2] = src[0];
    dest[3] = src[3];
}


static void
convert_data(const unsigned char * const src, unsigned char * const dest,
const int x, const int srcformat,
const int destformat)
{
    if (srcformat == 2)
    {
        if (destformat == 3)
            convert_16_to_24(src+x*srcformat,
                dest+x*destformat);
        else
        {
            assert(destformat == 4);
            convert_16_to_32(src+x*srcformat,
                dest+x*destformat);
        }
    }
    else if (srcformat == 3)
    {
        assert(destformat == 3);
        convert_24_to_24(src+x*srcformat,
            dest+x*destformat);
    }
    else
    {
        assert(srcformat == 4 && destformat == 4);
        convert_32_to_32(src+x*srcformat,
            dest+x*destformat);
    }
}


/* Intel byte order workaround */
static int getInt16(unsigned char *ptr)
{
    int res = ptr[0];
    int tmp = ptr[1];
    return res | (tmp<<8);
}

static int getInt24(unsigned char *ptr)
{
    int temp1 = ptr[0];
    int temp2 = ptr[1];
    int temp3 = ptr[2];
    return temp1 | (temp2 << 8) | (temp3 << 16);
}

static int getInt32(unsigned char *ptr)
{
    int temp1 = ptr[0];
    int temp2 = ptr[1];
    int temp3 = ptr[2];
    int temp4 = ptr[3];
    return temp1 | (temp2 << 8) | (temp3 << 16) | (temp4 << 24);
}

/* */
/* decode a new rle packet */
/* */
static void
rle_new_packet(unsigned char ** src,
int * rleRemaining,
int * rleIsCompressed,
unsigned char *rleCurrent,
const int rleEntrySize)
{
    int i;
    unsigned char code = *(*src)++;
                                 /* number of bytes left in this packet */
    *rleRemaining = (code & 127) + 1;
    if (code & 128)              /* rle */
    {
        *rleIsCompressed = 1;
        for (i = 0; i < rleEntrySize; i++)
            rleCurrent[i] = *(*src)++;
    }
    else                         /* uncompressed */
    {
        *rleIsCompressed = 0;
    }
}


/* */
/* decode the # of specified bytes */
/* */
static void
rle_decode(unsigned char ** src,
unsigned char *dest,
const int numbytes,
int * rleRemaining,
int * rleIsCompressed,
unsigned char *rleCurrent,
const int rleEntrySize)
{
    int i;
    int size = rleEntrySize;
    unsigned char *stop = dest + numbytes;
    while (dest < stop)
    {
        if (*rleRemaining == 0)  /* start new packet */
            rle_new_packet(src, rleRemaining, rleIsCompressed,
                rleCurrent, rleEntrySize);

        if (*rleIsCompressed)
        {
            for (i = 0; i < size; i++)
            {
                *dest++ = rleCurrent[i];
            }
        }
        else
        {
            for (i = 0; i < size; i++)
            {
                *dest++ = *(*src)++;
            }
        }
        // original code : *rleRemaining)--;
        (*rleRemaining)--;
    }
}


unsigned char *
simage_tga_load(std::istream& fin,
int *width_ret,
int *height_ret,
int *numComponents_ret)
{
    unsigned char header[18];
    int type;
    int width;
    int height;
    int depth;
    int flags;
    int format;
    unsigned char *colormap;
    int colormapLen = 0;
    int indexsize = 0;
    int rleIsCompressed;
    int rleRemaining;
    int rleEntrySize;
    unsigned char rleCurrent[4];
    unsigned char *buffer;
    unsigned char *dest;
    int bpr;
    unsigned char *linebuf;

    tgaerror = ERR_NO_ERROR;     /* clear error */

    fin.read((char*)header,18);
    if (fin.gcount() != 18)
    {
        tgaerror = ERR_READ;
        return NULL;
    }

    type = header[2];
    width = getInt16(&header[12]);
    height = getInt16(&header[14]);
    depth = header[16] >> 3;
    flags = header[17];

    /* check for reasonable values in case this is not a tga file */
    if ((type != 1 && type != 2 && type != 10) ||
        (width < 0 || width > 4096) ||
        (height < 0 || height > 4096) ||
        (depth < 1 || depth > 4))
    {
        tgaerror = ERR_UNSUPPORTED;
        return NULL;
    }

    if (header[0])               /* skip identification field */
        fin.seekg(header[0],std::ios::cur);

    colormap = NULL;
    if (header[1] == 1)          /* there is a colormap */
    {
        colormapLen = getInt16(&header[5]);
        indexsize = header[7]>>3;
        colormap = new unsigned char [colormapLen*indexsize];
        fin.read((char*)colormap,colormapLen*indexsize);

        if (indexsize == 2)          /* 16 bits */
        {
            if (flags & 1) format = 4;
            else format = 3;
        }
        else
            format = indexsize;
    }
    else
    {
        if (depth == 2)              /* 16 bits */
        {
            if (flags & 1) format = 4;
            else format = 3;
        }
        else format = depth;
    }

    /*    SoDebugError::postInfo("simage_tga_load", "TARGA file: %d %d %d %d %d\n",  */
    /*               type, width, height, depth, format); */

    rleIsCompressed = 0;
    rleRemaining = 0;
    rleEntrySize = depth;
    buffer = new unsigned char [width*height*format];
    dest = buffer;
    bpr = format * width;
    linebuf = new unsigned char [width*depth];

    //check the intended image orientation
    bool bLeftToRight = (flags&0x10)==0;
    bool bTopToBottom = (flags&0x20)!=0;
    int lineoffset = bTopToBottom ? -bpr : bpr;
    if (bTopToBottom) //move start point to last line in buffer
        dest += (bpr*(height-1));

    switch(type)
    {
        case 1:                  /* colormap, uncompressed */
        {
            if (colormapLen == 0 || indexsize == 0)
            {
                tgaerror = ERR_UNSUPPORTED; /* colormap missing or empty */

                if (colormap) delete [] colormap;
                delete [] buffer;
                delete [] linebuf;

                return NULL;
            }
            unsigned char * formattedMap = new unsigned char[colormapLen * format];
            for (int i = 0; i < colormapLen; i++)
            {
                convert_data(colormap, formattedMap, i, indexsize, format);
            }

            int x, y;
            for (y = 0; y < height; y++)
            {
                fin.read((char*)linebuf, width*depth);
                if (fin.gcount() != (std::streamsize) (width*depth))
                {
                    tgaerror = ERR_READ;
                    break;
                }

                for (x = 0; x < width; x++)
                {
                    int index;
                    switch (depth)
                    {
                    case 1:
                        index = linebuf[x];
                        break;
                    case 2:
                        index = getInt16(linebuf + x * 2);
                        break;
                    case 3:
                        index = getInt24(linebuf + x * 3);
                        break;
                    case 4:
                        index = getInt32(linebuf + x * 4);
                        break;
                    default:
                        tgaerror = ERR_UNSUPPORTED;
                        delete [] formattedMap;
                        return NULL; /* unreachable code - (depth < 1 || depth > 4) rejected by "check for reasonable values in case this is not a tga file" near the start of this function*/
                    }

                    int adjustedX = bLeftToRight ? x : (width - 1) - x;
                    for (int i = 0; i < format; i++)
                        (dest + adjustedX * format)[i] = (formattedMap + index * format)[i];
                }
                dest += lineoffset;
            }

            delete [] formattedMap;
        }
        break;
        case 2:                  /* RGB, uncompressed */
        {
            int x, y;
            for (y = 0; y < height; y++)
            {
                fin.read((char*)linebuf,width*depth);
                if (fin.gcount() != (std::streamsize) (width*depth))
                {
                    tgaerror = ERR_READ;
                    break;
                }
                for (x = 0; x < width; x++)
                {
                    convert_data(linebuf, dest, bLeftToRight ? x : (width-1) - x, depth, format);
                }
                dest += lineoffset;
            }
        }
        break;
        case 10:                 /* RGB, compressed */
        {
            int size, x, y;
            int pos = fin.tellg();

            fin.seekg(0,std::ios::end);
            size = (int)fin.tellg() - pos;
            fin.seekg(pos,std::ios::beg);
            unsigned char* buf = new unsigned char [size];
            if (buf == NULL)
            {
                tgaerror = ERR_MEM;
                break;
            }
            unsigned char* src = buf;

            fin.read((char*)buf,size);
            if (fin.gcount() == (std::streamsize) size)
            {
                for (y = 0; y < height; y++)
                {
                    rle_decode(&src, linebuf, width*depth, &rleRemaining,
                        &rleIsCompressed, rleCurrent, rleEntrySize);
                    assert(src <= buf + size);
                    for (x = 0; x < width; x++)
                    {
                        convert_data(linebuf, dest,  bLeftToRight ? x : (width-1) - x, depth, format);
                    }
                    dest += lineoffset;
                }
            }
            else
            {
                tgaerror = ERR_READ;
            }
            if (buf) delete [] buf;
        }
        break;
        default:
            tgaerror = ERR_UNSUPPORTED;
    }

    if (linebuf) delete [] linebuf;
    if (colormap) delete [] colormap;

    if (tgaerror)
    {
        if (buffer) delete [] buffer;
        return NULL;
    }

    *width_ret = width;
    *height_ret = height;
    *numComponents_ret = format;
    return buffer;
}


int
simage_tga_identify(const char *filename,
const unsigned char *buf,
int headerlen)
{
    char * ptr;
    if (headerlen < 18) return 0;
    ptr = (char *)strrchr(filename, '.');
    if (!ptr) return 0;          /* TGA files must end with .tga|.TGA */

    if (strcmp(ptr, ".tga") && strcmp(ptr, ".TGA")) return 0;

    if (buf[1] == 1 && buf[2] == 1 && buf[17] < 64)
    {
        /*      SoDebugError::postInfo("simage_tga_identify", */
        /*                 "TARGA colormap file: %s\n", filename); */
        return 0;
    }
    if ((buf[1] == 0 || buf[1] == 1) && buf[2] == 2 && buf[17] < 64) return 1;
    if (buf[1] == 1 && buf[2] == 9 && buf[17] < 64)
    {
        /*      SoDebugError::postInfo("simage_tga_identity", */
        /*                 "TARGA RLE and colormap file: %s\n", filename);  */

        /* will soon be supported */
        return 0;
    }
    if ((buf[1] == 0 || buf[1] == 1) && buf[2] == 10 && buf[17] < 64)
    {
        /* RLE and RGB */
        return 1;
    }
    else                         /* unsupported */
    {
        /*      SoDebugError::postInfo("simage_tga_identify", */
        /*                 "Unsupported TARGA type.\n"); */
    }
    /* not a TGA, or not supported type */
    return 0;
}


class ReaderWriterTGA : public osgDB::ReaderWriter
{
    public:

        ReaderWriterTGA()
        {
            supportsExtension("tga","Tga Image format");
        }

        virtual const char* className() const { return "TGA Image Reader"; }

        ReadResult readTGAStream(std::istream& fin) const
        {
            unsigned char *imageData = NULL;
            int width_ret;
            int height_ret;
            int numComponents_ret;

            imageData = simage_tga_load(fin,&width_ret,&height_ret,&numComponents_ret);

            if (imageData==NULL) return ReadResult::FILE_NOT_HANDLED;

            int s = width_ret;
            int t = height_ret;
            int r = 1;

            unsigned int pixelFormat =
                numComponents_ret == 1 ? GL_LUMINANCE :
                numComponents_ret == 2 ? GL_LUMINANCE_ALPHA :
                numComponents_ret == 3 ? GL_RGB :
                numComponents_ret == 4 ? GL_RGBA : (GLenum)-1;

            int internalFormat = pixelFormat;

            unsigned int dataType = GL_UNSIGNED_BYTE;


            osg::Image* pOsgImage = new osg::Image;
            pOsgImage->setImage(s,t,r,
                internalFormat,
                pixelFormat,
                dataType,
                imageData,
                osg::Image::USE_NEW_DELETE);

            return pOsgImage;

        }

        virtual ReadResult readObject(std::istream& fin,const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return readImage(fin, options);
        }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return readImage(file, options);
        }

        virtual ReadResult readImage(std::istream& fin,const Options* =NULL) const
        {
            return readTGAStream(fin);
        }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osgDB::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
            if(!istream) return ReadResult::FILE_NOT_HANDLED;
            ReadResult rr = readTGAStream(istream);
            if(rr.validImage()) rr.getImage()->setFileName(file);
            return rr;
        }

        bool saveTGAStream(const osg::Image& image, std::ostream& fout) const
        {
            if (!image.data()) return false;

            // At present, I will only save the image to unmapped RGB format
            // Other data types can be added soon with different options
            // The format description can be found at:
            // http://local.wasp.uwa.edu.au/~pbourke/dataformats/tga/
            unsigned int pixelFormat = image.getPixelFormat();
            int width = image.s(), height = image.t();
            int numPerPixel = image.computeNumComponents(pixelFormat);
            int pixelMultiplier = (image.getDataType()==GL_FLOAT ? 255 : 1);

            // Headers
            fout.put(0);  // Identification field size
            fout.put(0);  // Color map type
            fout.put(2);  // Image type
            fout.put(0); fout.put(0);  // Color map origin
            fout.put(0); fout.put(0);  // Color map length
            fout.put(0);  // Color map entry size
            fout.put(0); fout.put(0);  // X origin of image
            fout.put(0); fout.put(0);  // Y origin of image
            fout.put(width&0xff); fout.put((width&0xff00)>>8);  // Width of image
            fout.put(height&0xff); fout.put((height&0xff00)>>8);  // Height of image
            fout.put(numPerPixel * 8);  // Image pixel size
            fout.put(0);  // Image descriptor

            // Swap red/blue channels for BGR images
            int r = 0, g = 1, b = 2;
            if( pixelFormat == GL_BGR || pixelFormat == GL_BGRA )
            {
                r = 2;
                b = 0;
            }

            // Data
            for (int y=0; y<height; ++y)
            {
                const unsigned char* ptr = image.data(0,y);
                for (int x=0; x<width; ++x)
                {
                    int off = x * numPerPixel;
                    switch ( numPerPixel )
                    {
                    case 3:  // BGR
                        fout.put(ptr[off+b] * pixelMultiplier); fout.put(ptr[off+g] * pixelMultiplier);
                        fout.put(ptr[off+r] * pixelMultiplier);
                        break;
                    case 4:  // BGRA
                        fout.put(ptr[off+b] * pixelMultiplier); fout.put(ptr[off+g] * pixelMultiplier);
                        fout.put(ptr[off+r] * pixelMultiplier); fout.put(ptr[off+3] * pixelMultiplier);
                        break;
                    default:
                        return false;
                    }
                }
            }
            return true;
        }

        virtual WriteResult writeImage(const osg::Image& image, std::ostream& fout, const Options*) const
        {
            if (saveTGAStream(image, fout))
                return WriteResult::FILE_SAVED;
            else
                return WriteResult::ERROR_IN_WRITING_FILE;
        }

        virtual WriteResult writeImage(const osg::Image& image, const std::string& fileName, const Options* options) const
        {
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

            osgDB::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
            if (!fout) return WriteResult::ERROR_IN_WRITING_FILE;
            return writeImage(image, fout, options);
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(tga, ReaderWriterTGA)
