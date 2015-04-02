#ifndef EXIF_Orientation_H
#define EXIF_Orientation_H

#include <stdio.h>

extern "C"
{
    #include <jpeglib.h>
    #include "jerror.h"
}

#define EXIF_JPEG_MARKER   JPEG_APP0+1

extern int EXIF_Orientation (j_decompress_ptr cinfo);

#endif
