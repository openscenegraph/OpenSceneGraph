#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/Image>
#include <osg/GL>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>

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
    unsigned short siz[2]; // a dword for whole file size - make unsigned Feb 2002
    short Reserved1, Reserved2; //reserved for future purposes
    unsigned short offset[2]; //offset to image in bytes
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
//    unsigned char rgbquad[4];
 //   long os2stuff[6]; // storage for os2.1 with 64 bytes to be read.  Dont know what these are yet.
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
    unsigned char *buffer=NULL; // returned to sender & as read from the disk

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
        unsigned char *hdr=new unsigned char[infsize]; // to hold the new header
        fread((char *)hdr, 1,infsize-sizeof(long), fp);
        long hsiz=sizeof(inf); // minimum of structure size & 
        if(infsize<=hsiz) hsiz=infsize;
        memcpy(&inf,hdr, hsiz/*-sizeof(long)*/); // copy only the bytes I can cope with
        delete [] hdr;
        osg::notify(osg::INFO) << "loading "<<filename <<" "<<swap<<" "<<infsize<< " "<<sizeof(inf) << " "<<sizeof(bmpheader) << std::endl;
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
            inf.ColorUsed=(long)pow(2.0,(double)inf.Colorbits); // infer the colours
        }
        osg::notify(osg::INFO) << "readbmp " <<inf.width<< " "<<inf.height << std::endl;
        
        // previous size calculation, see new calcs below.
        long size_prev = hd.siz[1]+hd.siz[0]*65536;
        osg::notify(osg::INFO) << "previous size calc = "<<size_prev<<"  hd.siz[1]="<<hd.siz[1]<<"  hd.siz[0]="<<hd.siz[0]<<std::endl;
        
        // order of size calculation swapped, by Christo Zietsman to fix size bug.
        long size = hd.siz[1]*65536+hd.siz[0];
        osg::notify(osg::INFO) << "new size calc = "<<size<<"  hd.siz[1]="<<hd.siz[1]<<"  hd.siz[0]="<<hd.siz[0]<<std::endl;


        int ncpal=4; // default number of colours per palette entry
        size -= sizeof(bmpheader)+infsize;
        if (inf.ImageSize<size) inf.ImageSize=size;
        imbuff = new unsigned char [ inf.ImageSize]; // read from disk
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

        if (ncomp>0) buffer = new unsigned char [(ncomp==BW?3:ncomp)*inf.width*inf.height]; // to be returned
        else buffer = new unsigned char [ 3*inf.width*inf.height]; // default full colour to be returned
        
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
        virtual bool acceptsExtension(const std::string& extension) { return osgDB::equalCaseInsensitive(extension,"bmp"); }

        virtual ReadResult readImage(const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {

            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            unsigned char *imageData = NULL;
            int width_ret;
            int height_ret;
            int numComponents_ret;

            imageData = bmp_load(fileName.c_str(),&width_ret,&height_ret,&numComponents_ret);

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
        virtual WriteResult writeImage(const osg::Image &img,const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

            FILE *fp = fopen(fileName.c_str(), "wb");
            if (!fp) return WriteResult::ERROR_IN_WRITING_FILE;
            // its easier for me to write a binary write using stdio than streams
            struct bmpheader hd;
            unsigned long nx=img.s(),ny=img.t(); //  unsigned long ndep=img.r();
            unsigned long size, wordsPerScan;
            long infsize;    //size of BMPinfo in bytes
            wordsPerScan=(nx*3+3)/4; // rounds up to next 32 bit boundary
            size=4*ny*wordsPerScan; // rounded to 4bytes * number of scan lines
            hd.FileType=MB;
            hd.Reserved1=hd.Reserved2=0; // offset to image
            hd.offset[0]=sizeof(long)+sizeof(BMPInfo)+sizeof(hd); // 26; // offset to image
            hd.offset[1]=0; // offset to image
  
            // previous way round.          
            // hd.siz[0]=(size&0xffff0000)>>16; // high word
            // hd.siz[1]=(size&0xffff); // low word
            
            // new round to be consistent with the swap in the size calclation in the reading code.
            hd.siz[0]=(size&0xffff); // low word
            hd.siz[1]=(size&0xffff0000)>>16; // high word
            
            fwrite(&hd, sizeof(hd), 1, fp);
            struct BMPInfo inf;
            osg::notify(osg::INFO) << "sizes "<<size << " "<<sizeof(inf)<< std::endl;
            inf.width=nx;   //width of the image in pixels
            inf.height=ny;    //   height of the image in pixels
            inf.planes=1;       //:word: number of planes (always 1)
            inf.Colorbits=24;       //word: number of bits used to describe color in each pixel
            inf.compression=0;  //compression used windows says 0= no compression
            inf.ImageSize=size;    //nx*ny*3; //image size in bytes
            inf.XpixPerMeter=1000; //pixels per meter in X
            inf.YpixPerMeter=1000; //pixels per meter in Y
            inf.ColorUsed=0;   //number of colors used
            inf.Important=0;   //number of "important" colors
   // inf.os2stuff[6]; // allows os2.1 with 64 bytes to be read.  Dont know what these are yet.
            infsize=sizeof(BMPInfo)+sizeof(long);
            fwrite(&infsize, sizeof(long), 1, fp);
            fwrite(&inf, sizeof(inf), 1, fp); // one dword shorter than the structure defined by MS
              // the infsize value (above) completes the structure.
            osg::notify(osg::INFO) << "save screen "<<fileName <<inf.width<< " "<<inf.height << std::endl;
            osg::notify(osg::INFO) << "sizes "<<size << " "<<infsize <<" "<<sizeof(inf)<< std::endl;
            // now output the bitmap
            // 1) swap Blue with Red - needed for Windoss.
            unsigned char *dta=new unsigned char[size];
            unsigned char tmp;
            memcpy(dta,img.data(),size*sizeof(unsigned char));
            for(unsigned int i=0;i<ny;i++) { // per scanline
                int ioff=4*wordsPerScan*i;
                for(unsigned int j=0;j<nx;j++) {
                tmp=dta[3*j+ioff]; // swap r with b,  thanks to good ole Bill - 
  //"Let's use BGR it's more logical than rgb which everyone else uses."
                dta[3*j+ioff]=dta[3*j+ioff+2];
                dta[3*j+ioff+2]=tmp;
                }
            }
            fwrite(dta, sizeof(unsigned char), size, fp);
            delete [] dta;

            fclose(fp);
            return WriteResult::FILE_SAVED;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterBMP> g_readerWriter_BMP_Proxy;
