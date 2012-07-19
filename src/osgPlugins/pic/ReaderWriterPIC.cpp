#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ERROR_NO_ERROR         0
#define ERROR_READING_HEADER   1
#define ERROR_READING_PALETTE  2
#define ERROR_MEMORY           3
#define ERROR_READ_ERROR       4

static int picerror = ERROR_NO_ERROR;

int
simage_pic_error(char *buffer, int bufferlen)
{
    switch (picerror)
    {
        case ERROR_READING_HEADER:
            strncpy(buffer, "PIC loader: Error reading header", bufferlen);
            break;
        case ERROR_READING_PALETTE:
            strncpy(buffer, "PIC loader: Error reading palette", bufferlen);
            break;
        case ERROR_MEMORY:
            strncpy(buffer, "PIC loader: Out of memory error", bufferlen);
            break;
        case ERROR_READ_ERROR:
            strncpy(buffer, "PIC loader: Read error", bufferlen);
            break;
    }
    return picerror;
}


/* byte order workaround *sigh* */

static int
readint16(FILE *fp, int * res)
{
    unsigned char tmp = 0;
    unsigned int tmp2;
    if (fread(&tmp, 1, 1, fp) != 1) return 0;
    *res = tmp;
    if (fread(&tmp, 1, 1, fp) != 1) return 0;
    tmp2 = tmp;
    tmp2 <<= 8;
    *res |= tmp2;
    return 1;
}


int
simage_pic_identify(const char *, const unsigned char *header, int headerlen)
{
    static unsigned char piccmp[] = {0x19, 0x91};
    if (headerlen < 2) return 0;
    if (memcmp((const void*)header,
        (const void*)piccmp, 2) == 0) return 1;
    return 0;
}


unsigned char *
simage_pic_load(const char *filename,
int *width_ret,
int *height_ret,
int *numComponents_ret)
{
    int w, h, width, height, i, j, format;
    unsigned char palette[256][3];
    unsigned char * tmpbuf, * buffer, * ptr;

    FILE *fp = osgDB::fopen(filename, "rb");
    if (!fp) return NULL;

    picerror = ERROR_NO_ERROR;

    fseek(fp, 2, SEEK_SET);
    if (!readint16(fp, &w))
    {
        picerror = ERROR_READING_HEADER;
        fclose(fp);
        return NULL;
    }

    fseek(fp, 4, SEEK_SET);
    if (!readint16(fp, &h))
    {
        picerror = ERROR_READING_HEADER;
        fclose(fp);
        return NULL;
    }

    width = w;
    height = h;

    if (width <= 0 || height <= 0)
    {
        fclose(fp);
        return NULL;
    }
    fseek(fp, 32, SEEK_SET);

    if (fread(&palette, 3, 256, fp) != 256)
    {
        picerror = ERROR_READING_PALETTE;
    }

    tmpbuf = new unsigned char [width];
    buffer = new unsigned char [3*width*height];
    if (tmpbuf == NULL || buffer == NULL)
    {
        picerror = ERROR_MEMORY;
        if (tmpbuf) delete [] tmpbuf;
        if (buffer) delete [] buffer;
        fclose(fp);
        return NULL;
    }
    ptr = buffer;
    for (i = 0; i < height; i++)
    {
        if (fread(tmpbuf, 1, width, fp) != (size_t) width)
        {
            picerror = ERROR_READ_ERROR;
            fclose(fp);
            if (tmpbuf) delete [] tmpbuf;
            if (buffer) delete [] buffer;
            buffer = NULL;
            width = height = 0;
            return NULL;
        }
        for (j = 0; j < width; j++)
        {
            int idx = tmpbuf[j];
            *ptr++ = palette[idx][0];
            *ptr++ = palette[idx][1];
            *ptr++ = palette[idx][2];
        }
    }
    format = 3;
    fclose(fp);

    *width_ret = width;
    *height_ret = height;
    *numComponents_ret = format;

    if (tmpbuf) delete [] tmpbuf;

    return buffer;
}


class ReaderWriterPIC : public osgDB::ReaderWriter
{
    public:
        ReaderWriterPIC()
        {
            supportsExtension("pic","PIC Image format");
        }

        virtual const char* className() const { return "PIC Image Reader"; }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return readImage(file, options);
        }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            unsigned char *imageData = NULL;
            int width_ret;
            int height_ret;
            int numComponents_ret;

            imageData = simage_pic_load(fileName.c_str(),&width_ret,&height_ret,&numComponents_ret);

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
REGISTER_OSGPLUGIN(pic, ReaderWriterPIC)
