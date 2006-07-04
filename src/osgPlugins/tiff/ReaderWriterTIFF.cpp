
#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>

#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <stdio.h>
#include <tiffio.h>

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

/* Functions to read TIFF image from memory
 *
 */

tsize_t libtiffStreamReadProc(thandle_t fd, tdata_t buf, tsize_t size)
{
    std::istream *fin = (std::istream*)fd;

    fin->read((char*)buf,size);

    if(fin->bad())
        return -1;

    if(fin->gcount() < size)
        return 0;

    return size;
}

tsize_t libtiffStreamWriteProc(thandle_t, tdata_t, tsize_t)
{
    return 0;
}

toff_t libtiffStreamSeekProc(thandle_t fd, toff_t off, int i)
{
    std::istream *fin = (std::istream*)fd;

    toff_t ret;
    switch(i)
    {
        case SEEK_SET:
            fin->seekg(off,std::ios::beg);
            ret = fin->tellg();
            if(fin->bad())
                ret = 0;
            break;

        case SEEK_CUR:
            fin->seekg(off,std::ios::cur);
            ret = fin->tellg();
            if(fin->bad())
                ret = 0;
            break;

        case SEEK_END:
            fin->seekg(off,std::ios::end);
            ret = fin->tellg();
            if(fin->bad())
                ret = 0;
            break;
        default:
            ret = 0;
            break;
    }
    return ret;
}

int libtiffStreamCloseProc(thandle_t)
{
    return 0;
}

toff_t libtiffStreamSizeProc(thandle_t fd)
{
    std::istream *fin = (std::istream*)fd;

    std::streampos curPos = fin->tellg();

    fin->seekg(0, std::ios::end);
    toff_t size = fin->tellg();
    fin->seekg(curPos, std::ios::beg);

    return size;
}

int libtiffStreamMapProc(thandle_t, tdata_t*, toff_t*)
{
    return 0;
}

void libtiffStreamUnmapProc(thandle_t, tdata_t, toff_t)
{
}

/* Functions to write TIFF image from memory
 *
 */

tsize_t libtiffOStreamReadProc(thandle_t, tdata_t, tsize_t)
{
    return 0;
}

tsize_t libtiffOStreamWriteProc(thandle_t fd, tdata_t buf, tsize_t size)
{
    std::ostream *fout = (std::ostream*)fd;

    fout->write((const char*)buf,size);

    if(fout->bad()) {
        return -1;
    }

    return size;
}

toff_t libtiffOStreamSizeProc(thandle_t fd)
{
    std::ostream *fout = (std::ostream*)fd;

    std::streampos curPos = fout->tellp();

    fout->seekp(0, std::ios::end);
    toff_t size = fout->tellp();
    fout->seekp(curPos, std::ios::beg);

    return size;
}

toff_t libtiffOStreamSeekProc(thandle_t fd, toff_t off, int i)
{
    std::ostream *fout = (std::ostream*)fd;

    toff_t ret;
    switch(i)
    {
        case SEEK_SET:
            fout->seekp(off,std::ios::beg);
            ret = fout->tellp();
            if(fout->bad())
                ret = 0;
            break;

        case SEEK_CUR:
            fout->seekp(off,std::ios::cur);
            ret = fout->tellp();
            if(fout->bad())
                ret = 0;
            break;

        case SEEK_END:
            fout->seekp(off,std::ios::end);
            ret = fout->tellp();
            if(fout->bad())
                ret = 0;
            break;
        default:
            ret = 0;
            break;
    }
    return ret;
}

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
invert_row(unsigned char *ptr, unsigned char *data, int n, int invert, uint16 bitspersample)
{
    if (bitspersample == 8)
    {
        while (n--)
        {
            if (invert) *ptr++ = 255 - *data++;
            else *ptr++ = *data++;
        }
    }
    else
    {
        unsigned short *ptr1 = (unsigned short *)ptr;
        unsigned short *data1 = (unsigned short *)data;
        
        while (n--)
        {
            if (invert) *ptr1++ = 65535 - *data1++;
            else *ptr1++ = *data1++;
        }
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


static void interleave_row(unsigned char *ptr,
                           unsigned char *red, unsigned char *green, unsigned char *blue,
                           int n, int numSamples)
{
    while (n--)
    {
        *ptr++ = *red++;
        *ptr++ = *green++;
        *ptr++ = *blue++;
        if (numSamples==4) *ptr++ = 255;
    }
}

static void interleave_row(unsigned char *ptr,
                           unsigned char *red, unsigned char *green, unsigned char *blue, unsigned char *alpha,
                           int n, int numSamples)
{
    while (n--)
    {
        *ptr++ = *red++;
        *ptr++ = *green++;
        *ptr++ = *blue++;
        if (numSamples==4) *ptr++ = *alpha++;
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
simage_tiff_load(std::istream& fin,
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

    in = TIFFClientOpen("inputstream", "r", (thandle_t)&fin,
            libtiffStreamReadProc, //Custom read function
            libtiffStreamWriteProc, //Custom write function
            libtiffStreamSeekProc, //Custom seek function
            libtiffStreamCloseProc, //Custom close function
            libtiffStreamSizeProc, //Custom size function
            libtiffStreamMapProc, //Custom map function
            libtiffStreamUnmapProc); //Custom unmap function

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
         if (bitspersample != 8 && bitspersample != 16)
        {
            /* can only handle 8 and 16 bit samples. */
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
    format = samplesperpixel * bitspersample / 8;

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
                invert_row(currPtr, inbuf, w, photometric == PHOTOMETRIC_MINISWHITE, bitspersample);
                //invert_row(currPtr, inbuf, w, photometric == PHOTOMETRIC_MINISWHITE);
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
                    if (format==3) interleave_row(currPtr, inbuf, inbuf+rowsize, inbuf+2*rowsize, w, format);
                    else if (format==4) interleave_row(currPtr, inbuf, inbuf+rowsize, inbuf+2*rowsize, inbuf+3*rowsize, w, format);
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
        virtual bool acceptsExtension(const std::string& extension) const
        { 
            if( osgDB::equalCaseInsensitive(extension,"tiff")) return true;
            if( osgDB::equalCaseInsensitive(extension,"tif") ) return true;
            return false;
        }

        ReadResult readTIFStream(std::istream& fin) const
        {
            unsigned char *imageData = NULL;
            int width_ret;
            int height_ret;
            int numComponents_ret;

            imageData = simage_tiff_load(fin,&width_ret,&height_ret,&numComponents_ret);

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
            pOsgImage->setImage(s,t,r,
                internalFormat,
                pixelFormat,
                dataType,
                imageData,
                osg::Image::USE_NEW_DELETE);

            return pOsgImage;
        }

        WriteResult::WriteStatus writeTIFStream(std::ostream& fout, const osg::Image& img) const
        {
            //Code is based from the following article on CodeProject.com
            //http://www.codeproject.com/bitmap/BitmapsToTiffs.asp

            TIFF *image;
            int samplesPerPixel;
            uint16 photometric;

            image = TIFFClientOpen("outputstream", "w", (thandle_t)&fout,
                                    libtiffOStreamReadProc, //Custom read function
                                    libtiffOStreamWriteProc, //Custom write function
                                    libtiffOStreamSeekProc, //Custom seek function
                                    libtiffStreamCloseProc, //Custom close function
                                    libtiffOStreamSizeProc, //Custom size function
                                    libtiffStreamMapProc, //Custom map function
                                    libtiffStreamUnmapProc); //Custom unmap function
            
            if(image == NULL)
            {
                return WriteResult::ERROR_IN_WRITING_FILE;
            }

            switch(img.getPixelFormat()) {
                case GL_LUMINANCE:
                case GL_ALPHA:
                    photometric = PHOTOMETRIC_MINISBLACK;
                    samplesPerPixel = 1;
                    break;
                case GL_LUMINANCE_ALPHA:
                    photometric = PHOTOMETRIC_MINISBLACK;
                    samplesPerPixel = 2;
                    break;
                case GL_RGB:
                    photometric = PHOTOMETRIC_RGB;
                    samplesPerPixel = 3;
                    break;
                case GL_RGBA:
                    photometric = PHOTOMETRIC_RGB;
                    samplesPerPixel = 4;
                    break;
                default:
                    return WriteResult::ERROR_IN_WRITING_FILE;
                    break;
            }

            TIFFSetField(image, TIFFTAG_IMAGEWIDTH,img.s());
            TIFFSetField(image, TIFFTAG_IMAGELENGTH,img.t());
            TIFFSetField(image, TIFFTAG_BITSPERSAMPLE,8);
            TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL,samplesPerPixel);
            TIFFSetField(image, TIFFTAG_PHOTOMETRIC, photometric);
            TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS); 
            TIFFSetField(image, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
            TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

            //uint32 rowsperstrip = TIFFDefaultStripSize(image, -1); 
            //TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
            
            // Write the information to the file
            for(int i = 0; i < img.t(); ++i) {
                TIFFWriteScanline(image,(tdata_t)img.data(0,img.t()-i-1),i,0);
            }

            // Close the file
            TIFFClose(image);

            return WriteResult::FILE_SAVED;
        }

        virtual ReadResult readImage(std::istream& fin,const osgDB::ReaderWriter::Options* =NULL) const
        {
            return readTIFStream(fin);
        }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            std::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
            if(!istream) return ReadResult::FILE_NOT_HANDLED;
            ReadResult rr = readTIFStream(istream);
            if(rr.validImage()) rr.getImage()->setFileName(file);
            return rr;
        }

        virtual WriteResult writeImage(const osg::Image& img,std::ostream& fout,const osgDB::ReaderWriter::Options* /*options*/) const
        {
            WriteResult::WriteStatus ws = writeTIFStream(fout,img);
            return ws;
        }

        virtual WriteResult writeImage(const osg::Image &img,const std::string& fileName, const osgDB::ReaderWriter::Options *options) const
        {
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

            std::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
            if(!fout) return WriteResult::ERROR_IN_WRITING_FILE;

            return writeImage(img,fout,options);
        }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterTIFF> g_readerWriter_TIFF_Proxy;
