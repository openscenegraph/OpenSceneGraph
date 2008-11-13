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
 * Ported into the OSG as a plugin, Robert Osfield Decemeber 2000.
 * Note, reference above to license of simage_rgb is not relevent to the OSG
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
    dest[0] = (unsigned char) (t0 & 0x1f) << 2;
                                 /* g */
    dest[1] = (unsigned char) (t1 & 0x7c) >> 2;
                                 /*b */
    dest[2] = (unsigned char) ((t1 & 0x3)<<3) | ((t0&0xe)>>5);
}


static void
convert_16_to_32(const unsigned char * const src, unsigned char * const dest)
{
    /* RGBA for opengl, lo-hi 16 bit for TGA */
    unsigned int t0 = src[0];
    unsigned int t1 = src[1];
                                 /* r */
    dest[0] = (unsigned char) (t0 & 0x1f) << 2;
                                 /* g */
    dest[1] = (unsigned char) (t1 & 0x7c) >> 2;
                                 /*b */
    dest[2] = (unsigned char) ((t1 & 0x3)<<3) | ((t0&0xe)>>5);
    dest[3] = (t1&0x70)?255:0;   /* a */
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
    int indexsize;
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
    if ((type != 2 && type != 10) ||
        (width < 0 || width > 4096) ||
        (height < 0 || height > 4096) ||
        (depth < 2 || depth > 4))
    {
        tgaerror = ERR_UNSUPPORTED;
        return NULL;
    }

    if (header[0])               /* skip identification field */
        fin.seekg(header[0],std::ios::cur);

    colormap = NULL;
    if (header[1] == 1)          /* there is a colormap */
    {
        int len = getInt16(&header[5]);
        indexsize = header[7]>>3;
        colormap = new unsigned char [len*indexsize];
        fin.read((char*)colormap,len*indexsize);
    }

    if (depth == 2)              /* 16 bits */
    {
        if (flags & 1) format = 4;
        else format = 3;
    }
    else format = depth;

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
            /* FIXME: write code */
            /* should never get here because simage_tga_identify returns 0 */
            /* for this filetype */
            /* I need example files in this format to write the code... */
            tgaerror = ERR_UNSUPPORTED;
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
        case 9:                  /* colormap, compressed */
        {
            /* FIXME: write code */
            /* should never get here because simage_tga_identify returns 0 */
            /* for this filetype */
            /* I need example files in this format to write the code... */
            tgaerror = ERR_UNSUPPORTED;
        }
        break;
        case 10:                 /* RGB, compressed */
        {
            int size, x, y;
            unsigned char *buf;
            unsigned char *src;
            int pos = fin.tellg();
            fin.seekg(0,std::ios::end);
            size = (int)fin.tellg() - pos;
            fin.seekg(pos,std::ios::beg);
            buf = new unsigned char [size];
            if (buf == NULL)
            {
                tgaerror = ERR_MEM;
                break;
            }
            src = buf;
            fin.read((char*)buf,size);
            if (fin.gcount() != (std::streamsize) size)
            {
                tgaerror = ERR_READ;
                break;
            }
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
            if (buf) delete [] buf;
        }
        break;
        default:
            tgaerror = ERR_UNSUPPORTED;
    }

    if (linebuf) delete [] linebuf;

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

            int internalFormat = numComponents_ret;

            unsigned int pixelFormat =
                numComponents_ret == 1 ? GL_LUMINANCE :
            numComponents_ret == 2 ? GL_LUMINANCE_ALPHA :
            numComponents_ret == 3 ? GL_RGB :
            numComponents_ret == 4 ? GL_RGBA : (GLenum)-1;

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
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(tga, ReaderWriterTGA)
