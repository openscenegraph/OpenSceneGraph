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

enum { ERROR_NO_ERROR =0,ERROR_READING_HEADER,ERROR_READING_PALETTE, ERROR_MEMORY, ERROR_READ_ERROR,
ERROR_NO_FILE,ERROR_READING_COLORS};

static int bmperror = ERROR_NO_ERROR;

// BMP format bits - at start of file is 512 bytes of pure garbage
enum ftype {MB=19778}; // magic number identifies a bmp file; actually chars 'B''M'
// allowed ftypes are 'BM'  for windoze;  OS2 allows:
//'BA' - Bitmap Array
//'CI' - Color Icon
//'CP' - Color Pointer (mouse cursor)
//'IC' - Icon
//'PT' - Pointer (mouse cursor)

enum ncol { BW=1, IA, RGB, RGBA};

struct bmpheader {
    short FileType; //always MB
    short siz[2]; // a dword for whole file size
    short Reserved1, Reserved2; //reserved for future purposes
    short offset[2]; //offset to image in bytes
};
struct BMPInfo {
    long width;   //width of the image in pixels
    long height;    //   height of the image in pixels
    short planes;       //:word: number of planes (always 1)
    short Colorbits;       //word: number of bits used to describe color in each pixel
    long compression;  //compression used
    long ImageSize;    //image size in bytes
    long XpixPerMeter; //pixels per meter in X
    long YpixPerMeter; //pixels per meter in Y
    long ColorUsed;   //number of colors used
    long Important;   //number of "important" colors
    long os2stuff[6]; // allows os2.1 with 64 bytes to be read.  Dont know what these are yet.
};

int
bmp_error(char *buffer, int bufferlen)
{
    switch (bmperror)
    {
        case ERROR_READING_COLORS:
            strncpy(buffer, "BMP loader: Error reading colours", bufferlen);
            break;
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
void swapbyte(long *i)
{
    char *vv=(char *)i;
    char tmp=vv[0];
    vv[0]=vv[3];
    vv[3]=tmp;
    tmp=vv[1];
    vv[1]=vv[2];
    vv[2]=tmp;
}
void swapbyte(unsigned long *i)
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
void swapbyte(unsigned short *i)
{
    char *vv=(char *)i;
    char tmp=vv[0];
    vv[0]=vv[1];
    vv[1]=tmp;
}
void swapbyte(short *i)
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
    unsigned char *buffer; // returned to sender & as read from the disk

    bmperror = ERROR_NO_FILE;

    FILE *fp = fopen(filename, "rb");
    if (!fp) return NULL;

    int ncolours;
    int ncomp=0;
    bool swap=false; // dont need to swap bytes
     // actual size of the bitmap header; 12=os2; 40 = normal; 64=os2.1
    
    struct bmpheader hd;
    struct BMPInfo inf;
    bmperror = ERROR_NO_ERROR;
    fread((char *)&hd, sizeof(bmpheader), 1, fp);
    if (hd.FileType != MB) {
        swapbyte(&(hd.FileType));
        swap=true;
        if (hd.FileType != MB) {
            bmperror=ERROR_READING_HEADER;
        }
    }
    if (hd.FileType == MB) {
        long infsize;    //size of BMPinfo in bytes
        unsigned char *cols=NULL; // dynamic colour palette
        unsigned char *imbuff; // returned to sender & as read from the disk
        fread((char *)&infsize, sizeof(long), 1, fp); // insert inside 'the file is bmp' clause
        if (swap) swapbyte(&infsize);
        if ((infsize-sizeof(long))<sizeof(inf)) { // then the next bytes appear to be valid - actually infsize must be 12,40,64
            fread((char *)&inf, infsize-sizeof(long), 1, fp);
            if (swap) { // inverse the field of the header which need swapping
                swapbyte(&hd.siz[0]);
                swapbyte(&hd.siz[1]);
                swapbyte(&inf.Colorbits);
                swapbyte(&inf.width);
                swapbyte(&inf.height);
                swapbyte(&inf.ImageSize);
            }
            if (infsize==12) { // os2, protect us from our friends ? || infsize==64
                int wd = inf.width&0xffff; // shorts replace longs
                int ht = inf.width>>16;
                int npln = inf.height&0xffff; // number of planes
                int cbits = inf.height>>16;
                inf.width=wd;
                inf.height=ht;
                inf.planes=npln;
                inf.Colorbits=cbits;
                inf.ColorUsed=pow(2.0,inf.Colorbits); // infer the colours
            }
            long size = hd.siz[1]*65536+hd.siz[0];
            int ncpal=4; // default number of colours per palette entry
            size -= sizeof(bmpheader)+infsize;
            if (inf.ImageSize<size) inf.ImageSize=size;
            imbuff = (unsigned char *)malloc( inf.ImageSize); // read from disk
            fread((char *)imbuff, sizeof(unsigned char),inf.ImageSize, fp);
            ncolours=inf.Colorbits/8;
            switch (ncolours) {
            case 1:
                ncomp = BW; // actually this is a 256 colour, paletted image
                inf.Colorbits=8; // so this is how many bits there are per index
                inf.ColorUsed=256; // and number of colours used
                cols=imbuff; // colour palette address - uses 4 bytes/colour
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
            default:
                cols=imbuff; // colour palette address - uses 4 bytes/colour
                if (infsize==12 || infsize==64) ncpal=3; // OS2 - uses 3 colours per palette entry
                else ncpal=4; // Windoze uses 4!
            }
            if (ncomp>0) buffer = (unsigned char *)malloc( (ncomp==BW?3:ncomp)*inf.width*inf.height*sizeof(unsigned char)); // to be returned
            else buffer = (unsigned char *)malloc( 3*inf.width*inf.height*sizeof(unsigned char)); // default full colour to be returned
            
            unsigned long off=0;
            unsigned long rowbytes=ncomp*sizeof(unsigned char)*inf.width;
            unsigned long doff=(rowbytes)/4;
            if ((rowbytes%4)) doff++; // round up if needed
            doff*=4; // to find dword alignment
            for(int j=0; j<inf.height; j++) {
                if (ncomp>BW) memcpy(buffer+j*rowbytes, imbuff+off, rowbytes); // pack bytes closely
                else { // find from the palette..
                    unsigned char *imptr=imbuff+inf.ColorUsed*ncpal; // add size of the palette- start of image
                    int npixperbyte=8/inf.Colorbits; // no of pixels per byte
                    for (int ii=0; ii<inf.width/npixperbyte; ii++) {
                        unsigned char mask=0x00; // masked with index to extract colorbits bits
                        unsigned char byte=imptr[(j*inf.width/npixperbyte)+ii];
                        int jj;
                        for (jj=0; jj<inf.Colorbits; jj++) mask |= (0x80>>jj); // fill N High end bits
                        for (jj=0; jj<npixperbyte; jj++) {
                            int colidx=(byte&mask)>>((npixperbyte-1-jj)*inf.Colorbits);
                            buffer[3*(j*inf.width+ii*npixperbyte+jj)+0]=cols[ncpal*colidx+2];
                            buffer[3*(j*inf.width+ii*npixperbyte+jj)+1]=cols[ncpal*colidx+1];
                            buffer[3*(j*inf.width+ii*npixperbyte+jj)+2]=cols[ncpal*colidx];
                            mask>>=inf.Colorbits;
                        }
                    }
                }
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
        }
        
        fclose(fp);

    } 
    else // else error in header
    {
        fclose(fp);
        return NULL;        
    }
    *width_ret = inf.width;
    *height_ret = inf.height;
    switch (ncomp) {
    case BW:
        *numComponents_ret = 3;
        break;
    case IA:
    case RGB:
    case RGBA:
        *numComponents_ret = ncomp;
        break;
    default:
        *numComponents_ret = 3;
        break;
    }

    return buffer;
}


class ReaderWriterBMP : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() { return "BMP Image Reader"; }
        virtual bool acceptsExtension(const std::string& extension) { return extension=="bmp"; }

        virtual osg::Image* readImage(const std::string& fileName, const osgDB::ReaderWriter::Options*)
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
