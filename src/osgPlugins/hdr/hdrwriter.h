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

 modified for OSG September 2007 david.spilling@gmail.com
*/

#ifndef HDRWRITER_H
#define HDRWRITER_H

#include <osg/Image>

#include <iosfwd>
#include <math.h>

class HDRWriter {
public:
    // all return "true" for success, "false" for failure.
    static bool writeRLE(const osg::Image *img, std::ostream& fout);
    static bool writeRAW(const osg::Image *img, std::ostream& fout);
    static bool writeHeader(const osg::Image *img, std::ostream& fout);

protected:

// can read or write pixels in chunks of any size including single pixels
    static bool writePixelsNoRLE( std::ostream& fout, float* data, int numpixels);
    static bool writePixelsRAW(  std::ostream& fout, unsigned char* data, int numpixels);

// read or write run length encoded files 
// must be called to read or write whole scanlines
    static bool writeBytesRLE(std::ostream& fout, unsigned char *data, int numbytes);
    static bool writePixelsRLE( std::ostream& fout, float* data, int scanline_width, int num_scanlines );

// inline conversions
    inline static void float2rgbe(unsigned char rgbe[4], float red, float green, float blue);
    inline static void rgbe2float(float *red, float *green, float *blue, unsigned char rgbe[4]);
};


/* standard conversion from float pixels to rgbe pixels */
/* note: you can remove the "inline"s if your compiler complains about it */
inline void HDRWriter::float2rgbe(unsigned char rgbe[4], float red, float green, float blue)
{
  float v;
  int e;

  v = red;
  if (green > v) v = green;
  if (blue > v) v = blue;
  if (v < 1e-32) {
    rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
  }
  else {
    v = frexp(v,&e) * 256.0/v;
    rgbe[0] = (unsigned char) (red * v);
    rgbe[1] = (unsigned char) (green * v);
    rgbe[2] = (unsigned char) (blue * v);
    rgbe[3] = (unsigned char) (e + 128);
  }
}

/* standard conversion from rgbe to float pixels */
/* note: Ward uses ldexp(col+0.5,exp-(128+8)).  However we wanted pixels */
/*       in the range [0,1] to map back into the range [0,1].            */
inline void HDRWriter::rgbe2float(float *red, float *green, float *blue, unsigned char rgbe[4])
{
  float f;

  if (rgbe[3]) {   /*nonzero pixel*/
    f = ldexp(1.0,rgbe[3]-(int)(128+8));
    *red = rgbe[0] * f;
    *green = rgbe[1] * f;
    *blue = rgbe[2] * f;
  }
  else
    *red = *green = *blue = 0.0;
}

#endif
