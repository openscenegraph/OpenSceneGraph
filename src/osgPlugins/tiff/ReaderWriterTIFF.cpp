#include <tiffio.h>

#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>

#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <stdio.h>

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


#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>

#define ERR_NO_ERROR    0
#define ERR_OPEN        1
#define ERR_READ        2
#define ERR_MEM         3
#define ERR_UNSUPPORTED 4
#define ERR_TIFFLIB     5

static int tifferror = ERR_NO_ERROR;

int
simage_tiff_error(char * buffer, int buflen)
{
    switch (tifferror)
    {
        case ERR_OPEN:
            strncpy(buffer, "TIFF loader: Error opening file", buflen);
            break;
        case ERR_MEM:
            strncpy(buffer, "TIFF loader: Out of memory error", buflen);
            break;
        case ERR_UNSUPPORTED:
            strncpy(buffer, "TIFF loader: Unsupported image type", buflen);
            break;
        case ERR_TIFFLIB:
            strncpy(buffer, "TIFF loader: Illegal tiff file", buflen);
            break;
    }
    return tifferror;
}


static void
tiff_error(const char*, const char*, va_list)
{
    // values are (const char* module, const char* fmt, va_list list)
    /* FIXME: store error message ? */
}


static void
tiff_warn(const char *, const char *, va_list)
{
    // values are (const char* module, const char* fmt, va_list list)
    /* FIXME: notify? */
}


static int
checkcmap(int n, uint16* r, uint16* g, uint16* b)
{
    while (n-- > 0)
        if (*r++ >= 256 || *g++ >= 256 || *b++ >= 256)
            return (16);
    /* Assuming 8-bit colormap */
    return (8);
}


static void
invert_row(unsigned char *ptr, unsigned char *data, int n, int invert)
{
    while (n--)
    {
        if (invert) *ptr++ = 255 - *data++;
        else *ptr++ = *data++;
    }
}


static void
remap_row(unsigned char *ptr, unsigned char *data, int n,
unsigned short *rmap, unsigned short *gmap, unsigned short *bmap)
{
    unsigned int ix;
    while (n--)
    {
        ix = *data++;
        *ptr++ = (unsigned char) rmap[ix];
        *ptr++ = (unsigned char) gmap[ix];
        *ptr++ = (unsigned char) bmap[ix];
    }
}


static void
copy_row(unsigned char *ptr, unsigned char *data, int n, int numSamples)
{
    while (n--)
    {
        for(int i=0;i<numSamples;++i)
        {
            *ptr++ = *data++;
        }
    }
}


static void
interleave_row(unsigned char *ptr,
unsigned char *red, unsigned char *blue, unsigned char *green,
int n)
{
    while (n--)
    {
        *ptr++ = *red++;
        *ptr++ = *green++;
        *ptr++ = *blue++;
    }
}


int
simage_tiff_identify(const char *,
const unsigned char *header,
int headerlen)
{
    static unsigned char tifcmp[] = {0x4d, 0x4d, 0x0, 0x2a};
    static unsigned char tifcmp2[] = {0x49, 0x49, 0x2a, 0};

    if (headerlen < 4) return 0;
    if (memcmp((const void*)header, (const void*)tifcmp, 4) == 0) return 1;
    if (memcmp((const void*)header, (const void*)tifcmp2, 4) == 0) return 1;
    return 0;
}


/* useful defines (undef'ed below) */
#define CVT(x)      (((x) * 255L) / ((1L<<16)-1))
#define pack(a,b)   ((a)<<8 | (b))

unsigned char *
simage_tiff_load(const char *filename,
int *width_ret,
int *height_ret,
int *numComponents_ret)
{
    TIFF *in;
    uint16 samplesperpixel;
    uint16 bitspersample;
    uint16 photometric;
    uint32 w, h;
    uint16 config;
    uint16* red;
    uint16* green;
    uint16* blue;
    unsigned char *inbuf = NULL;
    tsize_t rowsize;
    uint32 row;
    int format;
    unsigned char *buffer;
    int width;
    int height;
    unsigned char *currPtr;

    TIFFSetErrorHandler(tiff_error);
    TIFFSetWarningHandler(tiff_warn);

    in = TIFFOpen(filename, "r");
    if (in == NULL)
    {
        tifferror = ERR_OPEN;
        return NULL;
    }
    if (TIFFGetField(in, TIFFTAG_PHOTOMETRIC, &photometric) == 1)
    {
        if (photometric != PHOTOMETRIC_RGB && photometric != PHOTOMETRIC_PALETTE &&
            photometric != PHOTOMETRIC_MINISWHITE &&
            photometric != PHOTOMETRIC_MINISBLACK)
        {
            /*Bad photometric; can only handle Grayscale, RGB and Palette images :-( */
            TIFFClose(in);
            tifferror = ERR_UNSUPPORTED;
            return NULL;
        }
    }
    else
    {
        tifferror = ERR_READ;
        TIFFClose(in);
        return NULL;
    }

    if (TIFFGetField(in, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel) == 1)
    {
        if (samplesperpixel != 1 &&
            samplesperpixel != 2 &&
            samplesperpixel != 3 &&
            samplesperpixel != 4)
        {
            /* Bad samples/pixel */
            tifferror = ERR_UNSUPPORTED;
            TIFFClose(in);
            return NULL;
        }
    }
    else
    {
        tifferror = ERR_READ;
        TIFFClose(in);
        return NULL;
    }

    if (TIFFGetField(in, TIFFTAG_BITSPERSAMPLE, &bitspersample) == 1)
    {
        if (bitspersample != 8)
        {
            /* can only handle 8-bit samples. */
            TIFFClose(in);
            tifferror = ERR_UNSUPPORTED;
            return NULL;
        }
    }
    else
    {
        tifferror = ERR_READ;
        TIFFClose(in);
        return NULL;
    }

    if (TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &w) != 1 ||
        TIFFGetField(in, TIFFTAG_IMAGELENGTH, &h) != 1 ||
        TIFFGetField(in, TIFFTAG_PLANARCONFIG, &config) != 1)
    {
        TIFFClose(in);
        tifferror = ERR_READ;
        return NULL;
    }

    /*
    if (photometric == PHOTOMETRIC_MINISWHITE ||
        photometric == PHOTOMETRIC_MINISBLACK)
        format = 1;
    else
        format = 3;
    */
    format = samplesperpixel;

    buffer = new unsigned char [w*h*format];
    for(unsigned char* ptr=buffer;ptr<buffer+w*h*format;++ptr) *ptr = 0;

    if (!buffer)
    {
        tifferror = ERR_MEM;
        TIFFClose(in);
        return NULL;
    }

    width = w;
    height = h;

    currPtr = buffer + (h-1)*w*format;

    tifferror = ERR_NO_ERROR;

    switch (pack(photometric, config))
    {
        case pack(PHOTOMETRIC_MINISWHITE, PLANARCONFIG_CONTIG):
        case pack(PHOTOMETRIC_MINISBLACK, PLANARCONFIG_CONTIG):
        case pack(PHOTOMETRIC_MINISWHITE, PLANARCONFIG_SEPARATE):
        case pack(PHOTOMETRIC_MINISBLACK, PLANARCONFIG_SEPARATE):
            inbuf = new unsigned char [TIFFScanlineSize(in)];
            for (row = 0; row < h; row++)
            {
                if (TIFFReadScanline(in, inbuf, row, 0) < 0)
                {
                    tifferror = ERR_READ;
                    break;
                }
                invert_row(currPtr, inbuf, w, photometric == PHOTOMETRIC_MINISWHITE);
                currPtr -= format*w;
            }
            break;

        case pack(PHOTOMETRIC_PALETTE, PLANARCONFIG_CONTIG):
        case pack(PHOTOMETRIC_PALETTE, PLANARCONFIG_SEPARATE):
            if (TIFFGetField(in, TIFFTAG_COLORMAP, &red, &green, &blue) != 1)
                tifferror = ERR_READ;
            /* */
            /* Convert 16-bit colormap to 8-bit (unless it looks */
            /* like an old-style 8-bit colormap). */
            /* */
            if (!tifferror && checkcmap(1<<bitspersample, red, green, blue) == 16)
            {
                int i;
                for (i = (1<<bitspersample)-1; i >= 0; i--)
                {
                    red[i] = CVT(red[i]);
                    green[i] = CVT(green[i]);
                    blue[i] = CVT(blue[i]);
                }
            }

            inbuf = new unsigned char [TIFFScanlineSize(in)];
            for (row = 0; row < h; row++)
            {
                if (TIFFReadScanline(in, inbuf, row, 0) < 0)
                {
                    tifferror = ERR_READ;
                    break;
                }
                remap_row(currPtr, inbuf, w, red, green, blue);
                currPtr -= format*w;
            }
            break;

        case pack(PHOTOMETRIC_RGB, PLANARCONFIG_CONTIG):
            inbuf = new unsigned char [TIFFScanlineSize(in)];
            for (row = 0; row < h; row++)
            {
                if (TIFFReadScanline(in, inbuf, row, 0) < 0)
                {
                    tifferror = ERR_READ;
                    break;
                }
                copy_row(currPtr, inbuf, w,samplesperpixel);
                currPtr -= format*w;
            }
            break;

        case pack(PHOTOMETRIC_RGB, PLANARCONFIG_SEPARATE):
            osg::notify(osg::NOTICE)<<"case 4"<<std::endl;
            rowsize = TIFFScanlineSize(in);
            inbuf = new unsigned char [format*rowsize];
            for (row = 0; !tifferror && row < h; row++)
            {
                int s;
                for (s = 0; s < format; s++)
                {
                    if (TIFFReadScanline(in, (tdata_t)(inbuf+s*rowsize), (uint32)row, (tsample_t)s) < 0)
                    {
                        tifferror = ERR_READ; break;
                    }
                }
                if (!tifferror)
                {
                    interleave_row(currPtr, inbuf, inbuf+rowsize, inbuf+2*rowsize, w);
                    currPtr -= format*w;
                }
            }
            break;
        default:
            tifferror = ERR_UNSUPPORTED;
            break;
    }

    if (inbuf) delete [] inbuf;
    TIFFClose(in);

    if (tifferror)
    {
        if (buffer) delete [] buffer;
        return NULL;
    }
    *width_ret = width;
    *height_ret = height;
    *numComponents_ret = format;
    return buffer;
}


#undef CVT
#undef pack

class ReaderWriterTIFF : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() const { return "TIFF Image Reader"; }
        virtual bool acceptsExtension(const std::string& extension) 
        { 
            if( osgDB::equalCaseInsensitive(extension,"tiff")) return true;
            if( osgDB::equalCaseInsensitive(extension,"tif") ) return true;
            return false;
        }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options*)
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            unsigned char *imageData = NULL;
            int width_ret;
            int height_ret;
            int numComponents_ret;

            imageData = simage_tiff_load(fileName.c_str(),&width_ret,&height_ret,&numComponents_ret);

            if (imageData==NULL) 
            {
                char err_msg[256];
                simage_tiff_error( err_msg, sizeof(err_msg)); 
                osg::notify(osg::WARN) << err_msg << std::endl;
                return ReadResult::FILE_NOT_HANDLED;
            }

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
            pOsgImage->setFileName(fileName.c_str());
            pOsgImage->setImage(s,t,r,
                internalFormat,
                pixelFormat,
                dataType,
                imageData,
                osg::Image::USE_NEW_DELETE);

            return pOsgImage;

        }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterTIFF> g_readerWriter_TIFF_Proxy;
