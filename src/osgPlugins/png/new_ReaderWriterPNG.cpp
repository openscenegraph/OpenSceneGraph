#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>

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

/*
 * Based on example code found in the libjpeg archive
 *
 */

using namespace osg;

extern "C"
{
    #include <png.h>
}

#define ERR_NO_ERROR 0
#define ERR_OPEN     1
#define ERR_MEM      2
#define ERR_PNGLIB   3

static int pngerror = ERR_NO_ERROR;

/* my setjmp buffer */
static jmp_buf setjmp_buffer;

/* called my libpng */
static void 
warn_callback(png_structp /*ps*/, png_const_charp pc)
{
  /*FIXME: notify? */
  osg::notify(osg::WARN)<<"Warning in .png reader: ";
  if (pc) osg::notify(osg::WARN)<< pc;
  osg::notify(osg::WARN)<<endl;
}

static void 
err_callback(png_structp /*ps*/, png_const_charp pc)
{
  /* FIXME: store error message? */
  longjmp(setjmp_buffer, 1);

  osg::notify(osg::WARN)<<"Error in .png reader: ";
  if (pc) osg::notify(osg::WARN)<< pc;
  osg::notify(osg::WARN)<<endl;
}

int 
simage_png_error(char * buffer, int buflen)
{
  switch (pngerror) {
  case ERR_OPEN:
    strncpy(buffer, "PNG loader: Error opening file", buflen);
    break;
  case ERR_MEM:
    strncpy(buffer, "PNG loader: Out of memory error", buflen);
    break;
  case ERR_PNGLIB:
    strncpy(buffer, "PNG loader: Illegal png file", buflen);
    break;
  }
  return pngerror;

}

int 
simage_png_identify(const char * /*ptr*/,
		    const unsigned char *header,
		    int headerlen)
{
  static unsigned char pngcmp[] = {0x89, 'P', 'N', 'G', 0xd, 0xa, 0x1a, 0xa};
  if (headerlen < 8) return 0;
  if (memcmp((const void*)header, 
	     (const void*)pngcmp, 8) == 0) return 1;
  return 0;
}

unsigned char *
simage_png_load(const char *filename,
		 int *width_ret,
		 int *height_ret,
		 int *numComponents_ret)
{
  png_structp png_ptr;
  png_infop info_ptr;
  png_uint_32 width, height;
  
  int bit_depth, color_type, interlace_type;
  FILE *fp;
  unsigned char *buffer;
  int bytes_per_row;
  int number_passes;
  int channels;
  int format;

  if ((fp = fopen(filename, "rb")) == NULL) {
    pngerror = ERR_OPEN;
    return NULL;
  }

  /* Create and initialize the png_struct with the desired error handler
   * functions.  If you want to use the default stderr and longjump method,
   * you can supply NULL for the last three parameters.  We also supply the
   * the compiler header file version, so that we know if the application
   * was compiled with a compatible version of the library.  REQUIRED
   */
  /*png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      (void *)user_error_ptr, user_error_fn, user_warning_fn);*/

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
				   NULL, err_callback, warn_callback);
  
  if (png_ptr == NULL) {
    pngerror = ERR_MEM;
    fclose(fp);
    return 0;
  }

  /* Allocate/initialize the memory for image information.  REQUIRED. */
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    pngerror = ERR_MEM;
    fclose(fp);
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    return 0;
  }
  
  /* Set error handling if you are using the setjmp/longjmp method (this is
   * the normal method of doing things with libpng).  REQUIRED unless you
   * set up your own error handlers in the png_create_read_struct() earlier.
   */

  buffer = NULL;

  if (setjmp(setjmp_buffer)) {
    pngerror = ERR_PNGLIB;
    /* Free all of the memory associated with the png_ptr and info_ptr */
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fclose(fp);
    /* If we get here, we had a problem reading the file */

    if (buffer) free(buffer);
    return NULL;
  }
  
  /* Set up the input control if you are using standard C streams */
  png_init_io(png_ptr, fp);

  /* The call to png_read_info() gives us all of the information from the
   * PNG file before the first IDAT (image data chunk).  REQUIRED
   */
  png_read_info(png_ptr, info_ptr);

  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
	       &interlace_type, NULL, NULL);

  /**** Set up the data transformations you want.  Note that these are all
  **** optional.  Only call them if you want/need them.  Many of the
  **** transformations only work on specific types of images, and many
  **** are mutually exclusive.
  ****/

  /* tell libpng to strip 16 bit/color files down to 8 bits/color */
  png_set_strip_16(png_ptr);

  /* strip alpha bytes from the input data without combining with th
   * background (not recommended) */
  /* png_set_strip_alpha(png_ptr); */
  
  /* extract multiple pixels with bit depths of 1, 2, and 4 from a single
   * byte into separate bytes (useful for paletted and grayscale images).
   */
  /* png_set_packing(png_ptr); */

  /* change the order of packed pixels to least significant bit first
   * (not useful if you are using png_set_packing). */
  /* png_set_packswap(png_ptr); */
  
  /* expand paletted colors into true RGB triplets */
  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_expand(png_ptr);

  /* expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand(png_ptr);
  
  /* expand paletted or RGB images with transparency to full alpha channels
   * so the data will be available as RGBA quartets */
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_expand(png_ptr);
  
  /* Add filler (or alpha) byte (before/after each RGB triplet) */
  /* png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER); */

  /* Turn on interlace handling.  REQUIRED if you are not using
   * png_read_image().  To see how to handle interlacing passes,
   * see the png_read_row() method below.
   */

  png_read_update_info(png_ptr, info_ptr);

  number_passes = png_set_interlace_handling(png_ptr);
  channels = png_get_channels(png_ptr, info_ptr);

  /* allocate the memory to hold the image using the fields of info_ptr. */
  
  bytes_per_row = png_get_rowbytes(png_ptr, info_ptr);
  
  buffer = (unsigned char*) malloc(bytes_per_row*height);

  format = channels; /* this is safer than the above */

  if (buffer) {
    int pass, y;
    unsigned char *dummytab[1];
    for (pass = 0; pass < number_passes; pass++) {
      for ( y = 0; (unsigned int) y < height; y++ ) {
	/* flips image upside down */
	dummytab[0] = &buffer[bytes_per_row*(height-1-y)];
	png_read_rows(png_ptr, dummytab, NULL, 1);
      }
    }
    
    /* read rest of file, and get additional chunks in info_ptr - REQUIRED */
    png_read_end(png_ptr, info_ptr);
  }
  
  /* clean up after the read, and free any memory allocated - REQUIRED */
  png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

  /* close the file */
  fclose(fp);

  /* that's it */
  if (buffer) {
    *width_ret = width;
    *height_ret = height;
    *numComponents_ret = format;
    pngerror = ERR_NO_ERROR;
  }
  else {
    pngerror = ERR_MEM;
  }
  return buffer;
}

class ReaderWriterPNG : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() { return "PNG Image Reader"; }
        virtual bool acceptsExtension(const std::string& extension)
        {
            return osgDB::equalCaseInsensitive(extension,"png");
        }

        virtual osg::Image* readImage(const std::string& fileName)
        {

            unsigned char *imageData = NULL;
            int width_ret;
            int height_ret;
            int numComponents_ret;

            imageData = simage_png_load(fileName.c_str(),&width_ret,&height_ret,&numComponents_ret);

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
osgDB::RegisterReaderWriterProxy<ReaderWriterPNG> g_readerWriter_PNG_Proxy;
