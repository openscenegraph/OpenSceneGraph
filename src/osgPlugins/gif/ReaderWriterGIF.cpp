#include <osg/Image>
#include <osg/ImageStream>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include <OpenThreads/Thread>


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
 * Note, reference above to license of simage_rgb is not relevant to the OSG
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
}

#define ERR_NO_ERROR     0
#define ERR_OPEN         1
#define ERR_READ         2
#define ERR_MEM          3

#define MY_GIF_DEBUG 1

// GifImageStream class
class GifImageStream : public osg::ImageStream, public OpenThreads::Thread
{
public:
    GifImageStream() :
        osg::ImageStream(),
        _multiplier(1.0),
        _currentLength(0),
        _length(0),
        _frameNum(0),
        _dataNum(0),
        _done(false)
    {
        _status=PAUSED;
    }

    virtual Object* clone() const { return new GifImageStream; }
    virtual bool isSameKindAs( const Object* obj ) const
    { return dynamic_cast<const GifImageStream*>(obj) != NULL; }
    virtual const char* className() const { return "GifImageStream"; }

    virtual void play()
    {
        if (!isRunning())
            start();
        _status=PLAYING;
    }

    virtual void pause() { _status=PAUSED; }

    virtual void rewind() { setReferenceTime( 0.0 ); }

    virtual void quit( bool waitForThreadToExit=true )
    {
        _done = true;
        if (isRunning() && waitForThreadToExit)
        {
            cancel();
            join();
        }
    }

    StreamStatus getStatus() { return _status; }
    virtual double getLength() const { return _length*0.01*_multiplier; }

    // Go to a specific position of stream
    virtual void setReferenceTime( double time )
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

        int i=1;
        int framePos = static_cast<int>(time*100.0/_multiplier);
        if ( framePos>=(int)_length )
            framePos = _length;

        std::vector<FrameData*>::iterator it;
        for ( it=_dataList.begin(); it!=_dataList.end(); it++,i++ )
        {
            framePos -= (*it)->delay;
            if ( framePos<0 )
                break;
        }
        _dataNum = i-1;
        _frameNum = (*it)->delay+framePos;
        setNewImage();
    }
    virtual double getReferenceTime() const { return _currentLength*0.01*_multiplier; }

    // Speed up, slow down or back to normal (1.0)
    virtual void setTimeMultiplier( double m )
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        if ( m>0 )
            _multiplier = m;
    }
    virtual double getTimeMultiplier() const { return _multiplier; }

    // Not used in GIF animation
    virtual void setVolume(float) {}
    virtual float getVolume() const { return 0.0f; }

    virtual void run()
    {
        _dataIter = _dataList.begin();

        while ( !_done )
        {
            if ( _status==PLAYING && (*_dataIter) )
            {
                if ( _frameNum>=(*_dataIter)->delay )
                {
                    _frameNum = 0;
                    if ( _dataNum>=_dataList.size()-1 )
                    {
                        if ( getLoopingMode()==LOOPING )
                        {
                            _dataNum = 0;
                            _currentLength = 0;
                        }
                    }
                    else
                        _dataNum++;

                    setNewImage();
                }
                else
                {
                    _frameNum++;
                    _currentLength++;
                }

                OpenThreads::Thread::microSleep(static_cast<int>(10000.0f*_multiplier));
            }
            else
                OpenThreads::Thread::microSleep(150000L);
        }
    }

    void addToImageStream( int ss, int tt, int rr, int numComponents, int delayTime, unsigned char* imgData )
    {
        if ( isRunning() )
        {
            OSG_WARN<<"GifImageStream::addToImageStream: thread is running!"<<std::endl;
            return;
        }

        GLint internalFormat = numComponents;
        GLenum dataType = GL_UNSIGNED_BYTE;

        GLenum pixelFormat =
            numComponents == 1 ? GL_LUMINANCE :
            numComponents == 2 ? GL_LUMINANCE_ALPHA :
            numComponents == 3 ? GL_RGB :
            numComponents == 4 ? GL_RGBA : (GLenum)-1;

        if ( _dataList.empty() )
        {
            // Set image texture for the first time
            setImage(ss, tt, rr, internalFormat, pixelFormat, dataType,
                imgData,osg::Image::NO_DELETE,1);
        }

        FrameData* newData = new FrameData;
        newData->delay = delayTime;
        newData->data = imgData;
        _dataList.push_back( newData );
        _length += delayTime;
    }

protected:
    typedef struct
    {
        unsigned int delay;
        unsigned char* data;
    } FrameData;

    void setNewImage()
    {
        _dataIter = _dataList.begin()+_dataNum;

        if ( *_dataIter )
        {
            unsigned char* image = (*_dataIter)->data;
            setImage(_s,_t,_r,_internalTextureFormat,_pixelFormat,_dataType,
                image,osg::Image::NO_DELETE,1);
            dirty();
        }
    }

    virtual ~GifImageStream()
    {
        if( isRunning() )
            quit( true );

        std::vector<FrameData*>::iterator it;
        for ( it=_dataList.begin(); it!=_dataList.end(); it++ )
        {
            delete (*it)->data;
            delete (*it);
        }
    }

    double          _multiplier;
    unsigned int    _currentLength;
    unsigned int    _length;

    unsigned int    _frameNum;
    unsigned int    _dataNum;
    std::vector<FrameData*> _dataList;
    std::vector<FrameData*>::iterator _dataIter;

    bool _done;
    OpenThreads::Mutex _mutex;
};

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

        if ( col == transparent )
        {
            // keep pixels of last image if transparent mode is on
            // this is necessary for GIF animating
            ptr += 3;
        }
        else
        {
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
int *numComponents_ret,
GifImageStream** obj)
{
    int i, j, n, row, col, width, height, extcode;
    unsigned char * rowdata;
    unsigned char * buffer, * ptr;
    unsigned char bg;
    int transparent, delaytime;
    GifRecordType recordtype;
    GifByteType * extension;
    GifFileType * giffile;
    GifColorType * bgcol;

    /* The way an interlaced image should be read - offsets and jumps */
    int interlacedoffset[] = { 0, 4, 2, 1 };
    int interlacedjumps[] = { 8, 8, 4, 2 };
#if (GIFLIB_MAJOR >= 5)
    int Error;
    giffile = DGifOpen(&fin,gif_read_stream, &Error);
#else
    giffile = DGifOpen(&fin,gif_read_stream);
#endif
    if (!giffile)
    {
        giferror = ERR_OPEN;
        return NULL;
    }

    transparent = -1;            /* no transparent color by default */
    delaytime = 8;               /* delay time of a frame  */

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
    int gif_num=0;
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
                /* start recording image stream if more than one image found  */
                gif_num++;
                if ( gif_num==2 )
                {
                    *obj = new GifImageStream;
                    (*obj)->addToImageStream( giffile->SWidth, giffile->SHeight, 1, 4, delaytime, buffer );
                    unsigned char* destbuffer = new unsigned char [n * 4];
                    buffer = (unsigned char*)memcpy( destbuffer, buffer, n*4 );
                }

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

                // Record gif image stream
                if ( *obj && obj )
                {
                    (*obj)->addToImageStream( giffile->SWidth, giffile->SHeight, 1, 4, delaytime, buffer );
                    unsigned char* destbuffer = new unsigned char [n * 4];
                    buffer = (unsigned char*)memcpy( destbuffer, buffer, n*4 );
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

                    delaytime = (extension[3]<<8)+extension[2];    // minimum unit 1/100s, so 8 here means 8/100s
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

    // Delete the last allocated buffer to avoid memory leaks if we using GifImageStream
    if ( obj && *obj )
    {
        delete [] buffer;
	buffer = 0;
    }

    delete [] rowdata;
    *width_ret = giffile->SWidth;
    *height_ret = giffile->SHeight;
    *numComponents_ret = 4;
#if (GIFLIB_MAJOR >= 5&& !(GIFLIB_MAJOR == 5 && GIFLIB_MINOR == 0))
    DGifCloseFile(giffile, &Error);
#else
    DGifCloseFile(giffile);
#endif
    return buffer;
}

class ReaderWriterGIF : public osgDB::ReaderWriter
{
    public:

        ReaderWriterGIF()
        {
            supportsExtension("gif","GIF Image format");
        }

        virtual const char* className() const { return "GIF Image Reader"; }

        ReadResult readGIFStream(std::istream& fin) const
        {
            unsigned char *imageData = NULL;
            int width_ret;
            int height_ret;
            int numComponents_ret;

            GifImageStream* gifStream = NULL;
            imageData = simage_gif_load( fin,&width_ret,&height_ret,&numComponents_ret, &gifStream );

            switch (giferror)
            {
                case ERR_OPEN:
                    return ReadResult("GIF loader: Error opening file");
                case ERR_READ:
                    return ReadResult("GIF loader: Error reading file");
                case ERR_MEM:
                    return ReadResult("GIF loader: Out of memory error");
            }

            // Use GifImageStream to display animate GIFs
            if ( gifStream )
            {
                OSG_DEBUG<<"Using GifImageStream ..."<<std::endl;
                return gifStream;
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

        virtual ReadResult readObject(std::istream& fin,const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return readImage(fin, options);
        }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return readImage(file, options);
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

            osgDB::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
            if(!istream) return ReadResult::FILE_NOT_HANDLED;
            ReadResult rr = readGIFStream(istream);
            if(rr.validImage()) rr.getImage()->setFileName(file);
            return rr;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(gif, ReaderWriterGIF)
