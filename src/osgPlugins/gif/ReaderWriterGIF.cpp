#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>


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

/*!
  GIF loader, using libungif
  Based, in part, on source code found in libungif, gif2rgb.c
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern  "C"
{
    #include <gif_lib.h>
};

#define ERR_NO_ERROR     0
#define ERR_OPEN         1
#define ERR_READ         2
#define ERR_MEM          3

#define MY_GIF_DEBUG 1

static int giferror = ERR_NO_ERROR;

int
simage_gif_error(char * buffer, int buflen)
{
    switch (giferror)
    {
        case ERR_OPEN:
            strncpy(buffer, "GIF loader: Error opening file", buflen);
            break;
        case ERR_READ:
            strncpy(buffer, "GIF loader: Error reading file", buflen);
            break;
        case ERR_MEM:
            strncpy(buffer, "GIF loader: Out of memory error", buflen);
            break;
    }
    return giferror;
}


int
simage_gif_identify(const char *,
const unsigned char *header,
int headerlen)
{
    static unsigned char gifcmp[] = {'G', 'I', 'F'};
    if (headerlen < 3) return 0;
    if (memcmp((const void*)header,
        (const void*)gifcmp, 3) == 0) return 1;
    return 0;
}


static void
decode_row(GifFileType * giffile,
unsigned char * buffer,
unsigned char * rowdata,
int x, int y, int len,
int transparent)
{
    GifColorType * cmentry;
    ColorMapObject * colormap;
    int colormapsize;
    unsigned char col;
    unsigned char * ptr;
    y = giffile->SHeight - (y+1);
    ptr = buffer + (giffile->SWidth * y + x) * 4;

    colormap = (giffile->Image.ColorMap
        ? giffile->Image.ColorMap
        : giffile->SColorMap);
    colormapsize = colormap ? colormap->ColorCount : 255;

    while (len--)
    {
        col = *rowdata++;
                                 /* just in case */
        if (col >= colormapsize) col = 0;
        cmentry = colormap ? &colormap->Colors[col] : NULL;
        if (cmentry)
        {
            *ptr++ = cmentry->Red;
            *ptr++ = cmentry->Green;
            *ptr++ = cmentry->Blue;
        }
        else
        {
            *ptr++ = col;
            *ptr++ = col;
            *ptr++ = col;
        }
        *ptr++ = (col == transparent ? 0x00 : 0xff);
    }
}

int gif_read_stream(GifFileType *gfile, GifByteType *gdata, int glength)
{
    std::istream *stream = (std::istream*)gfile->UserData; //Get pointer to istream
    stream->read((char*)gdata,glength); //Read requested amount of data
    return stream->gcount();
}

unsigned char *
simage_gif_load(std::istream& fin,
int *width_ret,
int *height_ret,
int *numComponents_ret)
{
    int i, j, n, row, col, width, height, extcode;
    unsigned char * rowdata;
    unsigned char * buffer, * ptr;
    unsigned char bg;
    int transparent;
    GifRecordType recordtype;
    GifByteType * extension;
    GifFileType * giffile;
    GifColorType * bgcol;

    /* The way an interlaced image should be read - offsets and jumps */
    int interlacedoffset[] = { 0, 4, 2, 1 };
    int interlacedjumps[] = { 8, 8, 4, 2 };

    giffile = DGifOpen(&fin,gif_read_stream);
    if (!giffile)
    {
        giferror = ERR_OPEN;
        return NULL;
    }

    transparent = -1;            /* no transparent color by default */

    n = giffile->SHeight * giffile->SWidth;
    buffer = new unsigned char [n * 4];
    if (!buffer)
    {
        giferror = ERR_MEM;
        return NULL;
    }
    rowdata = new unsigned char [giffile->SWidth];
    if (!rowdata)
    {
        giferror = ERR_MEM;
        delete [] buffer;
        return NULL;
    }

    bg = giffile->SBackGroundColor;
    if (giffile->SColorMap && bg < giffile->SColorMap->ColorCount)
    {
        bgcol = &giffile->SColorMap->Colors[bg];
    }
    else bgcol = NULL;
    ptr = buffer;
    for (i = 0; i < n; i++)
    {
        if (bgcol)
        {
            *ptr++ = bgcol->Red;
            *ptr++ = bgcol->Green;
            *ptr++ = bgcol->Blue;
            *ptr++ = 0xff;
        }
        else
        {
            *ptr++ = 0x00;
            *ptr++ = 0x00;
            *ptr++ = 0x00;
            *ptr++ = 0xff;
        }
    }

    /* Scan the content of the GIF file and load the image(s) in: */
    do
    {
        if (DGifGetRecordType(giffile, &recordtype) == GIF_ERROR)
        {
            giferror = ERR_READ;
            delete [] buffer;
            delete [] rowdata;
            return NULL;
        }
        switch (recordtype)
        {
            case IMAGE_DESC_RECORD_TYPE:
                if (DGifGetImageDesc(giffile) == GIF_ERROR)
                {
                    giferror = ERR_READ;
                    delete [] buffer;
                    delete [] rowdata;
                    return NULL;
                }
                                 /* subimage position in composite image */
                row = giffile->Image.Top;
                col = giffile->Image.Left;
                width = giffile->Image.Width;
                height = giffile->Image.Height;
                if (giffile->Image.Left + giffile->Image.Width > giffile->SWidth ||
                    giffile->Image.Top + giffile->Image.Height > giffile->SHeight)
                {
                    /* image is not confined to screen dimension */
                    giferror = ERR_READ;
                    delete [] buffer;
                    delete [] rowdata;
                    return NULL;
                }
                if (giffile->Image.Interlace)
                {
                    //fprintf(stderr,"interlace\n");
                    /* Need to perform 4 passes on the images: */
                    for (i = 0; i < 4; i++)
                    {
                        for (j = row + interlacedoffset[i]; j < row + height;
                            j += interlacedjumps[i])
                        {
                            if (DGifGetLine(giffile, rowdata, width) == GIF_ERROR)
                            {
                                giferror = ERR_READ;
                                delete [] buffer;
                                delete [] rowdata;
                                return NULL;
                            }
                            else decode_row(giffile, buffer, rowdata, col, j, width, transparent);
                        }
                    }
                }
                else
                {
                    for (i = 0; i < height; i++, row++)
                    {
                        if (DGifGetLine(giffile, rowdata, width) == GIF_ERROR)
                        {
                            giferror = ERR_READ;
                            delete [] buffer;
                            delete [] rowdata;
                            return NULL;
                        }
                        else decode_row(giffile, buffer, rowdata, col, row, width, transparent);
                    }
                }
                break;
            case EXTENSION_RECORD_TYPE:
                /* Skip any extension blocks in file: */
                if (DGifGetExtension(giffile, &extcode, &extension) == GIF_ERROR)
                {
                    giferror = ERR_READ;
                    delete [] buffer;
                    delete [] rowdata;
                    return NULL;
                }
                /* transparent test from the gimp gif-plugin. Open Source rulez! */
                else if (extcode == 0xf9)
                {
                    if (extension[0] >= 4 && extension[1] & 0x1) transparent = extension[4];
                    else transparent = -1;
                }
                while (extension != NULL)
                {
                    if (DGifGetExtensionNext(giffile, &extension) == GIF_ERROR)
                    {
                        giferror = ERR_READ;
                        delete [] buffer;
                        delete [] rowdata;
                        return NULL;
                    }
                }
                break;
            case TERMINATE_RECORD_TYPE:
                break;
            default:             /* Should be trapped by DGifGetRecordType. */
                break;
        }
    }
    while (recordtype != TERMINATE_RECORD_TYPE);

    delete [] rowdata;
    *width_ret = giffile->SWidth;
    *height_ret = giffile->SHeight;
    *numComponents_ret = 4;
    DGifCloseFile(giffile);
    return buffer;
}

class ReaderWriterGIF : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() const { return "GIF Image Reader"; }
        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension,"gif");
        }

        ReadResult readGIFStream(std::istream& fin) const
        {
            unsigned char *imageData = NULL;
            int width_ret;
            int height_ret;
            int numComponents_ret;

            imageData = simage_gif_load(fin,&width_ret,&height_ret,&numComponents_ret);

            switch (giferror)
            {
                case ERR_OPEN:
                    return ReadResult("GIF loader: Error opening file");
                case ERR_READ:
                    return ReadResult("GIF loader: Error reading file");
                case ERR_MEM:
                    return ReadResult("GIF loader: Out of memory error");
            }

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

        virtual ReadResult readImage(std::istream& fin,const osgDB::ReaderWriter::Options* =NULL) const
        {
            return readGIFStream(fin);
        }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            std::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
            if(!istream) return ReadResult::FILE_NOT_HANDLED;
            ReadResult rr = readGIFStream(istream);
            if(rr.validImage()) rr.getImage()->setFileName(file);
            return rr;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterGIF> g_readerWriter_GIF_Proxy;
