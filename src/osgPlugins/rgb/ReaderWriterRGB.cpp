#include "osg/Image"
#include "osg/Notify"

#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/Texture>

#include "osg/GL"

#include "osgDB/FileNameUtils"
#include "osgDB/Registry"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SEEK_SET
#  define SEEK_SET 0
#endif

using namespace osg;

typedef struct _rawImageRec
{
    unsigned short imagic;
    unsigned short type;
    unsigned short dim;
    unsigned short sizeX, sizeY, sizeZ;
    unsigned long min, max;
    unsigned long wasteBytes;
    char name[80];
    unsigned long colorMap;
    FILE *file;
    unsigned char *tmp, *tmpR, *tmpG, *tmpB, *tmpA;
    unsigned long rleEnd;
    GLuint *rowStart;
    GLint *rowSize;
} rawImageRec;

static void ConvertShort(unsigned short *array, long length)
{
    unsigned long b1, b2;
    unsigned char *ptr;

    ptr = (unsigned char *)array;
    while (length--)
    {
        b1 = *ptr++;
        b2 = *ptr++;
        *array++ = (unsigned short) ((b1 << 8) | (b2));
    }
}


static void ConvertLong(GLuint *array, long length)
{
    unsigned long b1, b2, b3, b4;
    unsigned char *ptr;

    ptr = (unsigned char *)array;
    while (length--)
    {
        b1 = *ptr++;
        b2 = *ptr++;
        b3 = *ptr++;
        b4 = *ptr++;
        *array++ = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4);
    }
}


static rawImageRec *RawImageOpen(const char *fileName)
{
    union
    {
        int testWord;
        char testByte[4];
    } endianTest;
    rawImageRec *raw;
    GLenum swapFlag;
    int x;

    endianTest.testWord = 1;
    if (endianTest.testByte[0] == 1)
    {
        swapFlag = GL_TRUE;
    }
    else
    {
        swapFlag = GL_FALSE;
    }

    raw = (rawImageRec *)malloc(sizeof(rawImageRec));
    if (raw == NULL)
    {
        notify(WARN)<< "Out of memory!"<<endl;
        return NULL;
    }
    if ((raw->file = fopen(fileName, "rb")) == NULL)
    {
        perror(fileName);
        return NULL;
    }

    fread(raw, 1, 12, raw->file);

    if (swapFlag)
    {
        ConvertShort(&raw->imagic, 6);
    }

    raw->tmp = raw->tmpR = raw->tmpG = raw->tmpB = raw->tmpA = 0L;

    raw->tmp = (unsigned char *)malloc(raw->sizeX*256);
    if (raw->tmp == NULL )
    {
        notify(FATAL)<< "Out of memory!"<<endl;
        return NULL;
    }

    if( raw->sizeZ >= 1 )
    {
        if( (raw->tmpR = (unsigned char *)malloc(raw->sizeX)) == NULL )
        {
            notify(FATAL)<< "Out of memory!"<<endl;
            free( raw->tmp );
            return NULL;
        }
    }
    if( raw->sizeZ >= 2 )
    {
        if( (raw->tmpG = (unsigned char *)malloc(raw->sizeX)) == NULL )
        {
            notify(FATAL)<< "Out of memory!"<<endl;
            free( raw->tmp );
            free( raw->tmpR );
            return NULL;
        }
    }
    if( raw->sizeZ >= 3 )
    {
        if( (raw->tmpB = (unsigned char *)malloc(raw->sizeX)) == NULL )
        {
            notify(FATAL)<< "Out of memory!"<<endl;
            free( raw->tmp );
            free( raw->tmpR );
            free( raw->tmpG );
            return NULL;
        }
    }
    if (raw->sizeZ >= 4)
    {
        if( (raw->tmpA = (unsigned char *)malloc(raw->sizeX)) == NULL )
        {
            notify(FATAL)<< "Out of memory!"<<endl;
            free( raw->tmp );
            free( raw->tmpR );
            free( raw->tmpG );
            free( raw->tmpB );
            return NULL;
        }
    }

    if ((raw->type & 0xFF00) == 0x0100)
    {
        x = raw->sizeY * raw->sizeZ * sizeof(GLuint);
        raw->rowStart = (GLuint *)malloc(x);
        raw->rowSize = (GLint *)malloc(x);
        if (raw->rowStart == NULL || raw->rowSize == NULL)
        {
            notify(FATAL)<< "Out of memory!"<<endl;
            return NULL;
        }
        raw->rleEnd = 512 + (2 * x);
        fseek(raw->file, 512, SEEK_SET);
        fread(raw->rowStart, 1, x, raw->file);
        fread(raw->rowSize, 1, x, raw->file);
        if (swapFlag)
        {
            ConvertLong(raw->rowStart, (long) (x/sizeof(GLuint)));
            ConvertLong((GLuint *)raw->rowSize, (long) (x/sizeof(GLint)));
        }
    }
    return raw;
}


static void RawImageClose(rawImageRec *raw)
{
    fclose(raw->file);
    free(raw->tmp);
    if( raw->tmpR )
        free(raw->tmpR);
    if( raw->tmpG )
        free(raw->tmpG);
    if( raw->tmpB )
        free(raw->tmpB);
    if( raw->tmpA )
        free(raw->tmpA);

    free(raw);
}


static void RawImageGetRow(rawImageRec *raw, unsigned char *buf, int y, int z)
{
    unsigned char *iPtr, *oPtr, pixel;
    int count, done = 0;

    if ((raw->type & 0xFF00) == 0x0100)
    {
        fseek(raw->file, (long) raw->rowStart[y+z*raw->sizeY], SEEK_SET);
        fread(raw->tmp, 1, (unsigned int)raw->rowSize[y+z*raw->sizeY],
            raw->file);

        iPtr = raw->tmp;
        oPtr = buf;
        while (!done)
        {
            pixel = *iPtr++;
            count = (int)(pixel & 0x7F);
            if (!count)
            {
                done = 1;
                return;
            }
            if (pixel & 0x80)
            {
                while (count--)
                {
                    *oPtr++ = *iPtr++;
                }
            }
            else
            {
                pixel = *iPtr++;
                while (count--)
                {
                    *oPtr++ = pixel;
                }
            }
        }
    }
    else
    {
        fseek(raw->file, 512+(y*raw->sizeX)+(z*raw->sizeX*raw->sizeY),
            SEEK_SET);
        fread(buf, 1, raw->sizeX, raw->file);
    }
}


static void RawImageGetData(rawImageRec *raw, unsigned char **data )
{
    unsigned char *ptr;
    int i, j;

    //     // round the width to a factor 4
    //     int width = (int)(floorf((float)raw->sizeX/4.0f)*4.0f);
    //     if (width!=raw->sizeX) width += 4;

    *data = (unsigned char *)malloc(2 * (raw->sizeX+1)*(raw->sizeY+1)*4);
    //    *data = (unsigned char *)malloc(2 * (width+1)*(raw->sizeY+1)*4);

    ptr = *data;
    for (i = 0; i < (int)(raw->sizeY); i++)
    {
        if( raw->sizeZ >= 1 )
            RawImageGetRow(raw, raw->tmpR, i, 0);
        if( raw->sizeZ >= 2 )
            RawImageGetRow(raw, raw->tmpG, i, 1);
        if( raw->sizeZ >= 3 )
            RawImageGetRow(raw, raw->tmpB, i, 2);
        if( raw->sizeZ >= 4 )
            RawImageGetRow(raw, raw->tmpA, i, 3);
        for (j = 0; j < (int)(raw->sizeX); j++)
        {
            if( raw->sizeZ >= 1 )
                *ptr++ = *(raw->tmpR + j);
            if( raw->sizeZ >= 2 )
                *ptr++ = *(raw->tmpG + j);
            if( raw->sizeZ >= 3 )
                *ptr++ = *(raw->tmpB + j);
            if( raw->sizeZ >= 4 )
                *ptr++ = *(raw->tmpA + j);
        }
        //         // pad the image width with blanks to bring it up to the rounded width.
        //         for(;j<width;++j) *ptr++ = 0;
    }
}


class ReaderWriterRGB : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() { return "Default RGB Image Reader/Writer"; }
        
        virtual bool acceptsExtension(const std::string& extension)
        {
            return osgDB::equalCaseInsensitive(extension,"rgb") ||
                osgDB::equalCaseInsensitive(extension,"rgba") ||
                osgDB::equalCaseInsensitive(extension,"int") || 
                osgDB::equalCaseInsensitive(extension,"inta") ||
                osgDB::equalCaseInsensitive(extension,"bw");
        }

        virtual Image* readImage(const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return NULL;
            
            rawImageRec *raw;

            if( (raw = RawImageOpen(fileName.c_str())) == NULL )
            {
                notify(FATAL)<< "Unable to open \""<<fileName<<"\""<<endl;
                return NULL;
            }

            int s = raw->sizeX;
            int t = raw->sizeY;
            int r = 1;

        #if 0
            int internalFormat = raw->sizeZ == 3 ? GL_RGB5 :
            raw->sizeZ == 4 ? GL_RGB5_A1 : GL_RGB;
        #else
            int internalFormat = raw->sizeZ;
        #endif
            unsigned int pixelFormat =
                raw->sizeZ == 1 ? GL_LUMINANCE :
                raw->sizeZ == 2 ? GL_LUMINANCE_ALPHA :
                raw->sizeZ == 3 ? GL_RGB :
                raw->sizeZ == 4 ? GL_RGBA : (GLenum)-1;

            unsigned int dataType = GL_UNSIGNED_BYTE;

            unsigned char *data;
            RawImageGetData(raw, &data);
            RawImageClose(raw);

            Image* image = new Image();
            image->setFileName(fileName.c_str());
            image->setImage(s,t,r,
                internalFormat,
                pixelFormat,
                dataType,
                data);

            notify(INFO) << "image read ok "<<s<<"  "<<t<<endl;
            return image;

        }

};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterRGB> g_readerWriter_RGB_Proxy;
