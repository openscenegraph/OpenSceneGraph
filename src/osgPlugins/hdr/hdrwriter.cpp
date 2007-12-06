/* 
 The following code was based on code from the following location:
    http://www.graphics.cornell.edu/online/formats/rgbe/

 It includes the following information :

    "This file contains code to read and write four byte rgbe file format
    developed by Greg Ward.  It handles the conversions between rgbe and
    pixels consisting of floats.  The data is assumed to be an array of floats.
    By default there are three floats per pixel in the order red, green, blue.
    (RGBE_DATA_??? values control this.)  Only the mimimal header reading and 
    writing is implemented.  Each routine does error checking and will return
    a status value as defined below.  This code is intended as a skeleton so
    feel free to modify it to suit your needs.

    (Place notice here if you modified the code.)
    posted to http://www.graphics.cornell.edu/~bjw/
    written by Bruce Walter  (bjw@graphics.cornell.edu)  5/26/95
    based on code written by Greg Ward"

 Modified for OSG September 2007 david.spilling@gmail.com :
    The file format is described fully in http://radsite.lbl.gov/radiance/refer/filefmts.pdf
    For the moment, we don't output most of the header fields

 
*/

#include <stdlib.h>

#include "hdrwriter.h"

#include <sstream>
#include <ostream>
#include <string>
#include <math.h>
#include <ctype.h>


bool HDRWriter::writeRLE(const osg::Image *img, std::ostream& fout)
{
    return writePixelsRLE(fout,(float*) img->data(), img->s(), img->t());
}

bool HDRWriter::writeRAW(const osg::Image *img, std::ostream& fout)
{
    return writePixelsRAW(fout,(unsigned char*) img->data(), img->s() * img->t());
}





/* number of floats per pixel */
#define RGBE_DATA_SIZE   3

/* offsets to red, green, and blue components in a data (float) pixel */
#define R            0
#define G            1
#define B            2
#define E            3

#define  MINELEN    8                // minimum scanline length for encoding
#define  MAXELEN    0x7fff            // maximum scanline length for encoding

/* default minimal header. modify if you want more information in header */
bool HDRWriter::writeHeader(const osg::Image *img, std::ostream& fout)
{
    std::stringstream stream;    // for conversion to strings

    stream << "#?RADIANCE\n";    // Could be RGBE, but some 3rd party software doesn't interpret the file correctly
                                // if it is.
    stream << "FORMAT=32-bit_rle_rgbe\n\n";

    // Our image data is usually arranged row major, with the origin at the bottom left of the image.
    // Based on the Radiance file format, this would be "+Y blah +X blah". However, no software (including
    // HDRShop v1!) seems to support this; they all expect -Y blah +X blah, and then flip the image. This
    // is unfortunate, and is what provokes the default behaviour of OSG having to flip the image prior to
    // write.
    stream << "-Y "<<img->s()<<" +X "<<img->t()<<"\n";

    fout.write(stream.str().c_str(), stream.str().length());

    return true;
}

/* simple write routine that does not use run length encoding */
/* These routines can be made faster by allocating a larger buffer and
   fread-ing and fwrite-ing the data in larger chunks */
bool HDRWriter::writePixelsNoRLE( std::ostream& fout, float* data, int numpixels)
{
  unsigned char rgbe[4];

  while (numpixels-- > 0)
  {
    float2rgbe(
        rgbe,
        data[R],
        data[G],
        data[B]
        );
    data += RGBE_DATA_SIZE;
    fout.write(reinterpret_cast<const char*>(rgbe), sizeof(rgbe)); //img->getTotalSizeInBytesIncludingMipmaps()
  }
  return true;
}

bool HDRWriter::writePixelsRAW( std::ostream& fout, unsigned char* data, int numpixels)
{
  unsigned char rgbe[4];

  while (numpixels-- > 0)
  {
    rgbe[0] = (unsigned char) *(data+R);
    rgbe[1] = (unsigned char) *(data+G);
    rgbe[2] = (unsigned char) *(data+B);
    rgbe[3] = (unsigned char) *(data+E);
    data += RGBE_DATA_SIZE;
    fout.write(reinterpret_cast<const char*>(rgbe), sizeof(rgbe)); //img->getTotalSizeInBytesIncludingMipmaps()
  }
  return true;
}

/* The code below is only needed for the run-length encoded files. */
/* Run length encoding adds considerable complexity but does */
/* save some space.  For each scanline, each channel (r,g,b,e) is */
/* encoded separately for better compression. */
bool HDRWriter::writeBytesRLE(std::ostream& fout, unsigned char *data, int numbytes)
{
#define MINRUNLENGTH 4
    int cur, beg_run, run_count, old_run_count, nonrun_count;
    unsigned char buf[2];

    cur = 0;
    while(cur < numbytes)
    {
        beg_run = cur;
        /* find next run of length at least 4 if one exists */
        run_count = old_run_count = 0;
        while((run_count < MINRUNLENGTH) && (beg_run < numbytes))
        {
            beg_run += run_count;
            old_run_count = run_count;
            run_count = 1;
            while((data[beg_run] == data[beg_run + run_count]) 
                && (beg_run + run_count < numbytes)
                && (run_count < 127))
            {
                run_count++;
            }
        }
        /* if data before next big run is a short run then write it as such */
        if ((old_run_count > 1)&&(old_run_count == beg_run - cur))
        {
            buf[0] = 128 + old_run_count;   /*write short run*/
            buf[1] = data[cur];
            fout.write(reinterpret_cast<const char*>(buf), sizeof(buf[0])*2);
            //if (fwrite(buf,sizeof(buf[0])*2,1,fp) < 1) return false
            cur = beg_run;
        }
        /* write out bytes until we reach the start of the next run */
        while(cur < beg_run)
        {
            nonrun_count = beg_run - cur;
            if (nonrun_count > 128) nonrun_count = 128;
            buf[0] = nonrun_count;
            fout.write(reinterpret_cast<const char*>(buf),sizeof(buf[0]));
            //if (fwrite(buf,sizeof(buf[0]),1,fp) < 1) return false
            fout.write(reinterpret_cast<const char*>(&data[cur]),sizeof(data[0])*nonrun_count);
            // if (fwrite(&data[cur],sizeof(data[0])*nonrun_count,1,fp) < 1) return false;
            cur += nonrun_count;
        }
        /* write out next run if one was found */
        if (run_count >= MINRUNLENGTH)
        {
            buf[0] = 128 + run_count;
            buf[1] = data[beg_run];
            fout.write(reinterpret_cast<const char*>(buf),sizeof(buf[0])*2);
            //if (fwrite(buf,sizeof(buf[0])*2,1,fp) < 1) return false;
            cur += run_count;
        }
    }
    return true;
#undef MINRUNLENGTH
}

bool HDRWriter::writePixelsRLE( std::ostream& fout, float* data, int scanline_width, int num_scanlines )

{
    unsigned char rgbe[4];
    unsigned char *buffer;

    if ((scanline_width < MINELEN)||(scanline_width > MAXELEN))
        // run length encoding is not allowed so write flat
        return writePixelsNoRLE(fout,data,scanline_width*num_scanlines);

    buffer = (unsigned char *)malloc(sizeof(unsigned char)*4*scanline_width);
    if (buffer == NULL) 
        // no buffer space so write flat
        return writePixelsNoRLE(fout,data,scanline_width*num_scanlines);

    while(num_scanlines-- > 0)
    {
        rgbe[0] = 2;
        rgbe[1] = 2;
        rgbe[2] = scanline_width >> 8;
        rgbe[3] = scanline_width & 0xFF;

        fout.write(reinterpret_cast<const char*>(rgbe), sizeof(rgbe));
        /*
        if (fwrite(rgbe, sizeof(rgbe), 1, fp) < 1)
        {
            free(buffer);
            return rgbe_error(rgbe_write_error,NULL);
        }
        */
        for(int i=0;i<scanline_width;i++)
        {
            float2rgbe(rgbe,data[R], data[G],data[B]);
            buffer[i] = rgbe[0];
            buffer[i+scanline_width] = rgbe[1];
            buffer[i+2*scanline_width] = rgbe[2];
            buffer[i+3*scanline_width] = rgbe[3];
            data += RGBE_DATA_SIZE;
        }
        /* write out each of the four channels separately run length encoded */
        /* first red, then green, then blue, then exponent */
        for(int i=0;i<4;i++)
        {
            if (writeBytesRLE(fout,&buffer[i*scanline_width],scanline_width) != true)
            {
                free(buffer);
                return false;
            }
        }
    }

    free(buffer);
    return true;
}
