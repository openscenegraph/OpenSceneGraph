#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>

#include <osgDB/Registry>

/****************************************************************************
 *
 * Follows is code written by GWM and translated to fit with the OSG Ethos.
 *
 *
 * Ported into the OSG as a plugin, Geoff Michel October 2001.
 * For patches, bugs and new features
 * please send them direct to the OSG dev team.
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
#define ERROR_NO_FILE           5

static int bmperror = ERROR_NO_ERROR;

// BMP format bits - at start of file is 512 bytes of pure garbage
enum ftype {MB=19778}; // magic number identifies a bmp file
enum ncol { BW=1, IA, RGB, RGBA};

struct bmpheader {
    short FileType; //always MB
    short siz[2];
    short Reserved1, Reserved2; //reserved for future purposes
    short offset[2]; //offset to image in bytes
};
struct BMPInfo {
    long size;    //size of BMPinfo in bytes
    long width;   //width of the image in pixels
    long height;    //   height of the image in pixels
    short planes;       //:word: number of planes (always 1)
    short Colorbits;       //word: number of bits used to describe color in each pixel
    long compression;  //compression used
    long ImageSize;    //image size in bytes
    long XpixPerMeter; //pixels per meter in X
    long YpixPerMeter; //pixels per meter in Y
    long ColorUsed;   //number of the color used ¿¿¿???
    long Important;   //number of "important" colors
};

int
bmp_error(char *buffer, int bufferlen)
{
    switch (bmperror)
    {
        case ERROR_READING_HEADER:
            strncpy(buffer, "BMP loader: Error reading header", bufferlen);
            break;
        case ERROR_READING_PALETTE:
            strncpy(buffer, "BMP loader: Error reading palette", bufferlen);
            break;
        case ERROR_MEMORY:
            strncpy(buffer, "BMP loader: Out of memory error", bufferlen);
            break;
        case ERROR_READ_ERROR:
            strncpy(buffer, "BMP loader: Read error", bufferlen);
            break;
    }
    return bmperror;
}

/* byte order workarounds *sigh* */
void swapbyte(long &i)
{
    char *vv=(char *)i;
    char tmp=vv[0];
    vv[0]=vv[3];
    vv[3]=tmp;
    tmp=vv[1];
    vv[1]=vv[2];
    vv[2]=tmp;
}
void swapbyte(unsigned long &i)
{
    char *vv=(char *)i;
    char tmp=vv[0];
    vv[0]=vv[3];
    vv[3]=tmp;
    tmp=vv[1];
    vv[1]=vv[2];
    vv[2]=tmp;
}
void swapbyte(float *i)
{
    char *vv=(char *)i;
    char tmp=vv[0];
    vv[0]=vv[3];
    vv[3]=tmp;
    tmp=vv[1];
    vv[1]=vv[2];
    vv[2]=tmp;
}
void swapbyte(unsigned short &i)
{
    char *vv=(char *)i;
    char tmp=vv[0];
    vv[0]=vv[1];
    vv[1]=tmp;
}
void swapbyte(short &i)
{
    char *vv=(char *)i;
    char tmp=vv[0];
    vv[0]=vv[1];
    vv[1]=tmp;
}

unsigned char *
bmp_load(const char *filename,
int *width_ret,
int *height_ret,
int *numComponents_ret)
{ // the main area of changes from the pic format loader.
    // reads filename, and returns the buffer
    // bmp is very very simple format
    // even Master Gates could have invented it.
    // It is extremely expensive on disk space - every RGB pixel uses 3 bytes plus a header!
    // BMP - sponsored by Seagate.
 //   unsigned char palette[256][3];
    unsigned char *buffer, *imbuff; // returned to sender & as read from the disk

    bmperror = ERROR_NO_FILE;

    FILE *fp = fopen(filename, "rb");
    if (!fp) return NULL;

    int ncolours;
    int ncomp=0;
    bool swap=false; // dont need to swap bytes
    
    struct bmpheader hd;
    struct BMPInfo inf;
    bmperror = ERROR_NO_ERROR;
    fread((char *)&hd, sizeof(bmpheader), 1, fp);
    fread((char *)&inf, sizeof(BMPInfo), 1, fp);
    if (hd.FileType != MB) {
        swapbyte((hd.FileType));
        swap=true;
        if (hd.FileType != MB) {
            bmperror=ERROR_READING_HEADER;
            return NULL;
        }
    }
    if (hd.FileType == MB) {
        if (swap) { // inverse the field of the header which need swapping
            swapbyte(hd.siz[0]);
            swapbyte(hd.siz[1]);
            swapbyte(inf.Colorbits);
            swapbyte(inf.width);
            swapbyte(inf.height);
            swapbyte(inf.ImageSize);
        }
        long size = hd.siz[1]*65536+hd.siz[0];
        size -= sizeof(bmpheader)+sizeof(BMPInfo);
        ncolours=inf.Colorbits/8;
        switch (ncolours) {
        case 1:
            ncomp = BW;
            break;
        case 2:
            ncomp = IA;
            break;
        case 3:
            ncomp = RGB; 
            break;
        case 4:
            ncomp = RGBA;
            break;
        }
        imbuff = (unsigned char *)malloc( inf.ImageSize); // read from disk
        buffer = (unsigned char *)malloc( ncomp*inf.width*inf.height*sizeof(unsigned char)); //
        if (ncomp==BW) {
            osg::notify(osg::NOTICE)<<"BMP file: "<<filename  << " sz " << inf.ImageSize <<endl;
            osg::notify(osg::NOTICE)<<"sizes: "<< inf.width << " " << inf.height << " = " << ncomp*inf.width*inf.height << endl;
        }

        unsigned long off=0;
        unsigned long rowbytes=ncomp*sizeof(unsigned char)*inf.width;
        unsigned long doff=(rowbytes)/4;
        if ((rowbytes%4)) doff++; // round up if needed
        doff*=4; // to find dword alignment
        fread((char *)imbuff, sizeof(unsigned char),inf.ImageSize, fp);
        for(int j=0; j<inf.height; j++) {
            memcpy(buffer+j*rowbytes, imbuff+off, rowbytes); // pack bytes closely
            off+=doff;
            if (ncomp>2) { // yes bill, colours are usually BGR aren't they
                for(int i=0; i<inf.width; i++) {
                    int ijw=i+j*inf.width;
                    unsigned char blu=buffer[3*ijw+0];
                    buffer[3*ijw+0]=buffer[3*ijw+2]; // swap order of colours
                    buffer[3*ijw+2]=blu;
                }
            }
        }
        delete [] imbuff; // free the on-disk storage
        
    } // else error in header
    fclose(fp);
    *width_ret = inf.width;
    *height_ret = inf.height;
    *numComponents_ret = ncomp;

    return buffer;
}


class ReaderWriterBMP : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() { return "BMP Image Reader"; }
        virtual bool acceptsExtension(const std::string& extension) { return extension=="bmp"; }

        virtual osg::Image* readImage(const std::string& fileName)
        {

            unsigned char *imageData = NULL;
            int width_ret;
            int height_ret;
            int numComponents_ret;

            imageData = bmp_load(fileName.c_str(),&width_ret,&height_ret,&numComponents_ret);

            if (imageData==NULL) return NULL;

            int s = width_ret;
            int t = height_ret;
            int r = 1;

            int internalFormat = numComponents_ret;
            

            unsigned int pixelFormat =
                numComponents_ret == 1 ? GL_LUMINANCE :
                numComponents_ret == 2 ? GL_LUMINANCE_ALPHA :
                numComponents_ret == 3 ? GL_RGB :
                numComponents_ret == 4 ? GL_RGBA : (GLenum)-1;

            cout << "internalFormat="<<internalFormat<<endl;
            cout << "pixelFormat="<<pixelFormat<<endl;

            unsigned int dataType = GL_UNSIGNED_BYTE;

            osg::Image* pOsgImage = new osg::Image;
            pOsgImage->setFileName(fileName.c_str());
            pOsgImage->setImage(s,t,r,
                internalFormat,
                pixelFormat,
                dataType,
                imageData);

            return pOsgImage;

        }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterBMP> g_readerWriter_BMP_Proxy;
