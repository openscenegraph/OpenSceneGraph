#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/ImageUtils>
#include <osg/GL>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <sstream>

#if defined(_MSC_VER) && defined(OSG_DISABLE_MSVC_WARNINGS)
    // disable "structure was padded due to __declspec(align())
    #pragma warning( disable : 4324 )
#endif

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
 * Ported into the OSG as a plugin, Robert Osfield December 2000.
 * Note, reference above to license of simage_rgb is not relevant to the OSG
 * as the OSG does not use it.  Also for patches, bugs and new features
 * please send them direct to the OSG dev team rather than address above.
 *
 **********************************************************************/

/*
 * Based on example code found in the libjpeg archive
 *
 */

#include "EXIF_Orientation.h"

#include <setjmp.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#if defined(_MSC_VER) && defined(OSG_DISABLE_MSVC_WARNINGS)
    #pragma warning( disable : 4611 )
#endif

namespace osgDBJPEG
{

#define ERR_NO_ERROR 0
#define ERR_OPEN     1
#define ERR_MEM      2
#define ERR_JPEGLIB  3

static int jpegerror = ERR_NO_ERROR;

/* Some versions of jmorecfg.h define boolean, some don't...
   Those that do also define HAVE_BOOLEAN, so we can guard using that. */
#ifndef HAVE_BOOLEAN
  typedef int boolean;
  #define FALSE 0
  #define TRUE 1
#endif

/* CODE FOR READING/WRITING JPEG FROM STREAMS
 *  This code was taken directly from jdatasrc.c and jdatadst.c (libjpeg source)
 *  and modified to use a std::istream/ostream* instead of a FILE*
 */

/* Expanded data source object for stdio input */

typedef struct {
    struct jpeg_source_mgr pub;    /* public fields */
    std::istream * infile;        /* source stream */
    JOCTET * buffer;              /* start of buffer */
    boolean start_of_file;        /* have we gotten any data yet? */
} stream_source_mgr;

typedef stream_source_mgr * stream_src_ptr;

#define INPUT_BUF_SIZE  4096    /* choose an efficiently fread'able size */

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

void init_source (j_decompress_ptr cinfo)
{
  stream_src_ptr src = (stream_src_ptr) cinfo->src;

  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  src->start_of_file = TRUE;
}


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

boolean fill_input_buffer (j_decompress_ptr cinfo)
{
  stream_src_ptr src = (stream_src_ptr) cinfo->src;
  size_t nbytes;

  src->infile->read((char*)src->buffer,INPUT_BUF_SIZE);
  nbytes = src->infile->gcount();

  if (nbytes <= 0) {
    if (src->start_of_file)    /* Treat empty input file as fatal error */
      ERREXIT(cinfo, JERR_INPUT_EMPTY);
    WARNMS(cinfo, JWRN_JPEG_EOF);
    /* Insert a fake EOI marker */
    src->buffer[0] = (JOCTET) 0xFF;
    src->buffer[1] = (JOCTET) JPEG_EOI;
    nbytes = 2;
  }

  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;
  src->start_of_file = FALSE;

  return TRUE;
}


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  stream_src_ptr src = (stream_src_ptr) cinfo->src;

  /* Just a dumb implementation for now.  Could use fseek() except
   * it doesn't work on pipes.  Not clear that being smart is worth
   * any trouble anyway --- large skips are infrequent.
   */
  if (num_bytes > 0) {
    while (num_bytes > (long) src->pub.bytes_in_buffer) {
      num_bytes -= (long) src->pub.bytes_in_buffer;
      (void) fill_input_buffer(cinfo);
      /* note we assume that fill_input_buffer will never return FALSE,
       * so suspension need not be handled.
       */
    }
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}


/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */
void term_source (j_decompress_ptr /*cinfo*/)
{
  /* no work necessary here */
}

void jpeg_istream_src(j_decompress_ptr cinfo, std::istream *infile)
{
    stream_src_ptr src;

    /* The source object and input buffer are made permanent so that a series
     * of JPEG images can be read from the same file by calling jpeg_stdio_src
     * only before the first one.  (If we discarded the buffer at the end of
     * one image, we'd likely lose the start of the next one.)
     * This makes it unsafe to use this manager and a different source
     * manager serially with the same JPEG object.  Caveat programmer.
     */
    if (cinfo->src == NULL) {    /* first time for this JPEG object? */
        cinfo->src = (struct jpeg_source_mgr *)
            (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,sizeof(stream_source_mgr));
        src = (stream_src_ptr) cinfo->src;
        src->buffer = (JOCTET *)
            (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,INPUT_BUF_SIZE * sizeof(JOCTET));
    }

    src = (stream_src_ptr) cinfo->src;
    src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_input_buffer;
    src->pub.skip_input_data = skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
    src->pub.term_source = term_source;
    src->infile = infile;
    src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
    src->pub.next_input_byte = NULL; /* until buffer loaded */
}

/* Expanded data destination object for stdio output */

typedef struct {
  struct jpeg_destination_mgr pub; /* public fields */

  std::ostream * outfile;    /* target stream */
  JOCTET * buffer;          /* start of buffer */
} stream_destination_mgr;

typedef stream_destination_mgr * stream_dest_ptr;

#define OUTPUT_BUF_SIZE  4096    /* choose an efficiently fwrite'able size */


/*
 * Initialize destination --- called by jpeg_start_compress
 * before any data is actually written.
 */

void init_destination (j_compress_ptr cinfo)
{
  stream_dest_ptr dest = (stream_dest_ptr) cinfo->dest;

  /* Allocate the output buffer --- it will be released when done with image */
  dest->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE, OUTPUT_BUF_SIZE * sizeof(JOCTET));

  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}


/*
 * Empty the output buffer --- called whenever buffer fills up.
 *
 * In typical applications, this should write the entire output buffer
 * (ignoring the current state of next_output_byte & free_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been dumped.
 *
 * In applications that need to be able to suspend compression due to output
 * overrun, a FALSE return indicates that the buffer cannot be emptied now.
 * In this situation, the compressor will return to its caller (possibly with
 * an indication that it has not accepted all the supplied scanlines).  The
 * application should resume compression after it has made more room in the
 * output buffer.  Note that there are substantial restrictions on the use of
 * suspension --- see the documentation.
 *
 * When suspending, the compressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_output_byte & free_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point will be regenerated after resumption, so do not
 * write it out when emptying the buffer externally.
 */

boolean empty_output_buffer (j_compress_ptr cinfo)
{
  stream_dest_ptr dest = (stream_dest_ptr) cinfo->dest;

  dest->outfile->write((const char*)dest->buffer,OUTPUT_BUF_SIZE);
  if (dest->outfile->bad())
    ERREXIT(cinfo, JERR_FILE_WRITE);

  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

  return TRUE;
}


/*
 * Terminate destination --- called by jpeg_finish_compress
 * after all data has been written.  Usually needs to flush buffer.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

void term_destination (j_compress_ptr cinfo)
{
  stream_dest_ptr dest = (stream_dest_ptr) cinfo->dest;
  size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;

  /* Write any data remaining in the buffer */
  if (datacount > 0) {
    dest->outfile->write((const char*)dest->buffer,datacount);
    if (dest->outfile->bad())
      ERREXIT(cinfo, JERR_FILE_WRITE);
  }
  dest->outfile->flush();
  /* Make sure we wrote the output file OK */
  if (dest->outfile->bad())
    ERREXIT(cinfo, JERR_FILE_WRITE);
}


/*
 * Prepare for output to a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing compression.
 */

void jpeg_stream_dest (j_compress_ptr cinfo, std::ostream * outfile)
{
    stream_dest_ptr dest;

    /* The destination object is made permanent so that multiple JPEG images
     * can be written to the same file without re-executing jpeg_stdio_dest.
     * This makes it dangerous to use this manager and a different destination
     * manager serially with the same JPEG object, because their private object
     * sizes may be different.  Caveat programmer.
     */
    if (cinfo->dest == NULL) {    /* first time for this JPEG object? */
        cinfo->dest = (struct jpeg_destination_mgr *)
            (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(stream_destination_mgr));
    }

    dest = (stream_dest_ptr) cinfo->dest;
    dest->pub.init_destination = init_destination;
    dest->pub.empty_output_buffer = empty_output_buffer;
    dest->pub.term_destination = term_destination;
    dest->outfile = outfile;
}

/* END OF READ/WRITE STREAM CODE */

int
simage_jpeg_error(char * buffer, int buflen)
{
    switch (jpegerror)
    {
        case ERR_OPEN:
            strncpy(buffer, "JPEG loader: Error opening file", buflen);
            break;
        case ERR_MEM:
            strncpy(buffer, "JPEG loader: Out of memory error", buflen);
            break;
        case ERR_JPEGLIB:
            strncpy(buffer, "JPEG loader: Illegal jpeg file", buflen);
            break;
    }
    return jpegerror;
}


struct my_error_mgr
{
    struct jpeg_error_mgr pub;   /* "public" fields */

    jmp_buf setjmp_buffer;       /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

static void
my_error_exit (j_common_ptr cinfo)
{
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    my_error_ptr myerr = (my_error_ptr) cinfo->err;

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    /*(*cinfo->err->output_message) (cinfo);*/

    /* FIXME: get error messahe from jpeglib */

    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
}

static void
my_output_message (j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];

  /* Create the message */
  (*cinfo->err->format_message) (cinfo, buffer);

  OSG_WARN<<buffer<<std::endl;
}


int
simage_jpeg_identify(const char *,
const unsigned char *header,
int headerlen)
{
    static unsigned char jpgcmp[] = {'J', 'F', 'I', 'F' };
    if (headerlen < 4) return 0;
    if (memcmp((const void*)&header[6],
        (const void*)jpgcmp, 4) == 0) return 1;
    return 0;
}


static unsigned char*
copyScanline(unsigned char *currPtr, unsigned char *from, int cnt)
{
    memcpy((void*)currPtr, (void*)from, cnt);
    currPtr -= cnt;
    return currPtr;
}

unsigned char* simage_jpeg_load(std::istream& fin,
                                int *width_ret,
                                int *height_ret,
                                int *numComponents_ret,
                                unsigned int* exif_orientation)
{
    int width;
    int height;
    unsigned char *currPtr;
    int format;
    /* This struct contains the JPEG decompression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     */
    struct jpeg_decompress_struct cinfo;
    /* We use our private extension JPEG error handler.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */
    struct my_error_mgr jerr;
    /* More stuff */
    //FILE * infile;               /* source file */
    JSAMPARRAY rowbuffer;        /* Output row buffer */
    int row_stride;              /* physical row width in output buffer */

    jpegerror = ERR_NO_ERROR;

    /* In this example we want to open the input file before doing anything else,
     * so that the setjmp() error recovery below can assume the file is open.
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to read binary files.
     */

    /*if ((infile = fopen(filename, "rb")) == NULL)
    {
        jpegerror = ERR_OPEN;
        return NULL;
    }*/

    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    jerr.pub.output_message = my_output_message;
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer))
    {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object, close the input file, and return.
         */
        jpegerror = ERR_JPEGLIB;
        jpeg_destroy_decompress(&cinfo);
        //fclose(infile);
        //if (buffer) delete [] buffer;
        return NULL;
    }

    // used to be before setjump above, but have moved to after to avoid compile warnings.
    unsigned char *buffer = NULL;

    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */

    //jpeg_stdio_src(&cinfo, infile);
    jpeg_istream_src(&cinfo,&fin);



    /* Step 3: read file parameters with jpeg_read_header() */

    jpeg_save_markers (&cinfo, EXIF_JPEG_MARKER, 0xffff);

    (void) jpeg_read_header(&cinfo, TRUE);
    /* We can ignore the return value from jpeg_read_header since
     *   (a) suspension is not possible with the stdio data source, and
     *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
     * See libjpeg.doc for more info.
     */

    /* check for orientation tag */
    *exif_orientation = EXIF_Orientation (&cinfo);
    if (*exif_orientation!=0)
    {
        OSG_INFO<<"We have an EXIF_Orientation "<<exif_orientation<<std::endl;
    }


    /* Step 4: set parameters for decompression */
    /* In this example, we don't need to change any of the defaults set by
     * jpeg_read_header(), so we do nothing here.
     */

    /* Step 5: Start decompressor */
    if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
    {
        format = 1;
        cinfo.out_color_space = JCS_GRAYSCALE;
    }
    else                         /* use rgb */
    {
        format = 3;
        cinfo.out_color_space = JCS_RGB;
    }

    (void) jpeg_start_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* We may need to do some setup of our own at this point before reading
     * the data.  After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     * In this example, we need to make an output work buffer of the right size.
     */
    /* JSAMPLEs per row in output buffer */
    row_stride = cinfo.output_width * cinfo.output_components;
    /* Make a one-row-high sample array that will go away when done with image */
    rowbuffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
    width = cinfo.output_width;
    height = cinfo.output_height;
    buffer = currPtr = new unsigned char [width*height*cinfo.output_components];

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    /* Here we use the library's state variable cinfo.output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     */

    /* flip image upside down */
    if (buffer)
    {
        currPtr = buffer + row_stride * (cinfo.output_height-1);

        while (cinfo.output_scanline < cinfo.output_height)
        {
            /* jpeg_read_scanlines expects an array of pointers to scanlines.
             * Here the array is only one element long, but you could ask for
             * more than one scanline at a time if that's more convenient.
             */
            (void) jpeg_read_scanlines(&cinfo, rowbuffer, 1);
            /* Assume put_scanline_someplace wants a pointer and sample count. */
            currPtr = copyScanline(currPtr, rowbuffer[0], row_stride);
        }
    }
    /* Step 7: Finish decompression */

    (void) jpeg_finish_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* Step 8: Release JPEG decompression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);

    /* After finish_decompress, we can close the input file.
     * Here we postpone it until after no more JPEG errors are possible,
     * so as to simplify the setjmp error logic above.  (Actually, I don't
     * think that jpeg_destroy can do an error exit, but why assume anything...)
     */
    //fclose(infile);

    /* At this point you may want to check to see whether any corrupt-data
     * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
     */

    /* And we're done! */
    if (buffer)
    {
        *width_ret = width;
        *height_ret = height;
        *numComponents_ret = format;
    }
    else
    {
        jpegerror = ERR_MEM;
    }
    return buffer;
}
} // namespace osgDBJPEG

class ReaderWriterJPEG : public osgDB::ReaderWriter
{

        WriteResult::WriteStatus write_JPEG_file (std::ostream &fout, const osg::Image &img, int quality = 100) const
        {
            if (!img.isDataContiguous())
            {
                OSG_WARN<<"Warning: Writing of image data, that is non contiguous, is not supported by JPEG plugin."<<std::endl;
                return WriteResult::ERROR_IN_WRITING_FILE;
            }

            int image_width = img.s();
            int image_height = img.t();
            if ( (image_width == 0) || (image_height == 0) )
            {
                OSG_DEBUG << "ReaderWriterJPEG::write_JPEG_file - Error no size" << std::endl;
                return WriteResult::ERROR_IN_WRITING_FILE;
            }

            J_COLOR_SPACE image_color_space = JCS_RGB;
            int image_components = 3;
            // Only cater for gray, alpha and RGB for now
            switch(img.getPixelFormat()) {
              case(GL_DEPTH_COMPONENT):
              case(GL_LUMINANCE):
              case(GL_ALPHA): {
                  image_color_space = JCS_GRAYSCALE;
                  image_components = 1;
                  break;
              }
              case(GL_RGB): {
                  image_color_space = JCS_RGB;
                  image_components = 3;
                  break;
              }
              default:
              {
                  OSG_DEBUG << "ReaderWriterJPEG::write_JPEG_file - Error pixel format non supported" << std::endl;
                return WriteResult::ERROR_IN_WRITING_FILE; break;
              }
            }

            JSAMPLE* image_buffer = (JSAMPLE*)(img.data());

            /* This struct contains the JPEG compression parameters and pointers to
            * working space (which is allocated as needed by the JPEG library).
            * It is possible to have several such structures, representing multiple
            * compression/decompression processes, in existence at once.  We refer
            * to any one struct (and its associated working data) as a "JPEG object".
            */
            struct jpeg_compress_struct cinfo;
            /* This struct represents a JPEG error handler.  It is declared separately
            * because applications often want to supply a specialized error handler
            * (see the second half of this file for an example).  But here we just
            * take the easy way out and use the standard error handler, which will
            * print a message on stderr and call exit() if compression fails.
            * Note that this struct must live as long as the main JPEG parameter
            * struct, to avoid dangling-pointer problems.
            */
            struct jpeg_error_mgr jerr;
            /* More stuff */
            //FILE * outfile;        /* target file */
            JSAMPROW row_pointer[1];    /* pointer to JSAMPLE row[s] */
            int row_stride;        /* physical row width in image buffer */

            /* Step 1: allocate and initialize JPEG compression object */

            /* We have to set up the error handler first, in case the initialization
            * step fails.  (Unlikely, but it could happen if you are out of memory.)
            * This routine fills in the contents of struct jerr, and returns jerr's
            * address which we place into the link field in cinfo.
            */
            cinfo.err = jpeg_std_error(&jerr);
            /* Now we can initialize the JPEG compression object. */
            jpeg_create_compress(&cinfo);

            /* Step 2: specify data destination (eg, a file) */
            /* Note: steps 2 and 3 can be done in either order. */

            /* Here we use the library-supplied code to send compressed data to a
            * stdio stream.  You can also write your own code to do something else.
            * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
            * requires it in order to write binary files.
            */
            /*if (!(outfile = fopen(filename, "wb")))
            {
                return WriteResult::ERROR_IN_WRITING_FILE;
            }*/

            //jpeg_stdio_dest(&cinfo, outfile);
            osgDBJPEG::jpeg_stream_dest(&cinfo, &fout);

            /* Step 3: set parameters for compression */

            /* First we supply a description of the input image.
            * Four fields of the cinfo struct must be filled in:
            */
            cinfo.image_width = image_width;     /* image width and height, in pixels */
            cinfo.image_height = image_height;
            cinfo.input_components = image_components;        /* # of color components per pixel */
            cinfo.in_color_space = image_color_space;     /* colorspace of input image */
            /* Now use the library's routine to set default compression parameters.
            * (You must set at least cinfo.in_color_space before calling this,
            * since the defaults depend on the source color space.)
            */
            jpeg_set_defaults(&cinfo);
            /* Now you can set any non-default parameters you wish to.
            * Here we just illustrate the use of quality (quantization table) scaling:
            */
            jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

            /* Step 4: Start compressor */

            /* TRUE ensures that we will write a complete interchange-JPEG file.
            * Pass TRUE unless you are very sure of what you're doing.
            */
            jpeg_start_compress(&cinfo, TRUE);

            /* Step 5: while (scan lines remain to be written) */
            /*           jpeg_write_scanlines(...); */

            /* Here we use the library's state variable cinfo.next_scanline as the
            * loop counter, so that we don't have to keep track ourselves.
            * To keep things simple, we pass one scanline per call; you can pass
            * more if you wish, though.
            */
            row_stride = image_width * image_components;    /* JSAMPLEs per row in image_buffer */

            while (cinfo.next_scanline < cinfo.image_height)
            {
                /* jpeg_write_scanlines expects an array of pointers to scanlines.
                * Here the array is only one element long, but you could pass
                * more than one scanline at a time if that's more convenient.
                */
                row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
                (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
            }

            /* Step 6: Finish compression */

            jpeg_finish_compress(&cinfo);
            /* After finish_compress, we can close the output file. */
            //fclose(outfile);

            /* Step 7: release JPEG compression object */

            /* This is an important step since it will release a good deal of memory. */
            jpeg_destroy_compress(&cinfo);

            /* And we're done! */
            return WriteResult::FILE_SAVED;
        }
        int getQuality(const osgDB::ReaderWriter::Options *options) const {
            if(options) {
                std::istringstream iss(options->getOptionString());
                std::string opt;
                while (iss >> opt) {
                    if(opt=="JPEG_QUALITY") {
                        int quality;
                        iss >> quality;
                        return quality;
                    }
                }
            }

            return 100;
        }
    public:

        ReaderWriterJPEG()
        {
            supportsExtension("jpeg","JPEG image format");
            supportsExtension("jpg","JPEG image format");
        }

        virtual const char* className() const { return "JPEG Image Reader/Writer"; }

        ReadResult readJPGStream(std::istream& fin) const
        {
            unsigned char *imageData = NULL;
            int width_ret;
            int height_ret;
            int numComponents_ret;
            unsigned int exif_orientation=0;

            imageData = osgDBJPEG::simage_jpeg_load(fin, &width_ret, &height_ret, &numComponents_ret, &exif_orientation);

            if (imageData==NULL) return ReadResult::ERROR_IN_READING_FILE;

            int s = width_ret;
            int t = height_ret;
            int r = 1;

            //int internalFormat = numComponents_ret;
            int internalFormat =
                numComponents_ret == 1 ? GL_LUMINANCE :
                numComponents_ret == 2 ? GL_LUMINANCE_ALPHA :
                numComponents_ret == 3 ? GL_RGB :
                numComponents_ret == 4 ? GL_RGBA : (GLenum)-1;

            unsigned int pixelFormat =
                numComponents_ret == 1 ? GL_LUMINANCE :
                numComponents_ret == 2 ? GL_LUMINANCE_ALPHA :
                numComponents_ret == 3 ? GL_RGB :
                numComponents_ret == 4 ? GL_RGBA : (GLenum)-1;

            unsigned int dataType = GL_UNSIGNED_BYTE;

            osg::ref_ptr<osg::Image> pOsgImage = new osg::Image;
            pOsgImage->setImage(s,t,r,
                internalFormat,
                pixelFormat,
                dataType,
                imageData,
                osg::Image::USE_NEW_DELETE);

            if (exif_orientation>0)
            {
                // guide for meaning of exif_orientation provided by webpage: http://sylvana.net/jpegcrop/exif_orientation.html
                switch(exif_orientation)
                {
                    case(1):
                        OSG_INFO<<"EXIF_Orientation 1 (top, left side), No need to rotate image. "<<std::endl;
                        break;
                    case(2):
                        OSG_INFO<<"EXIF_Orientation 2 (top, right side), flip x."<<std::endl;
                        pOsgImage = osg::createImageWithOrientationConversion(pOsgImage.get(),
                                                                    osg::Vec3i(pOsgImage->s()-1, 0, 0),
                                                                    osg::Vec3i(-pOsgImage->s(), 0, 0),
                                                                    osg::Vec3i(0, pOsgImage->t(), 0),
                                                                    osg::Vec3i(0, 0, 1));
                        break;
                    case(3):
                        OSG_INFO<<"EXIF_Orientation 3 (bottom, right side), rotate 180."<<std::endl;
                        pOsgImage = osg::createImageWithOrientationConversion(pOsgImage.get(),
                                                                    osg::Vec3i(pOsgImage->s()-1, pOsgImage->t()-1, 0),
                                                                    osg::Vec3i(-pOsgImage->s(), 0, 0),
                                                                    osg::Vec3i(0, -pOsgImage->t(), 0),
                                                                    osg::Vec3i(0, 0, 1));
                        break;
                    case(4):
                        OSG_INFO<<"EXIF_Orientation 4 (bottom, left side). flip y, rotate 180."<<std::endl;
                        pOsgImage = osg::createImageWithOrientationConversion(pOsgImage.get(),
                                                                    osg::Vec3i(0, pOsgImage->t()-1, 0),
                                                                    osg::Vec3i(pOsgImage->s(), 0, 0),
                                                                    osg::Vec3i(0, -pOsgImage->t(), 0),
                                                                    osg::Vec3i(0, 0, 1));
                        break;
                    case(5):
                        OSG_INFO<<"EXIF_Orientation 5 (left side, top). flip y, rotate 90."<<std::endl;
                        pOsgImage = osg::createImageWithOrientationConversion(pOsgImage.get(),
                                                                    osg::Vec3i(pOsgImage->s()-1, pOsgImage->t()-1, 0),
                                                                    osg::Vec3i(0, -pOsgImage->t(), 0),
                                                                    osg::Vec3i(-pOsgImage->s(), 0, 0),
                                                                    osg::Vec3i(0, 0, 1));
                        break;
                    case(6):
                        OSG_INFO<<"EXIF_Orientation 6 (right side, top). rotate 90."<<std::endl;
                        pOsgImage = osg::createImageWithOrientationConversion(pOsgImage.get(),
                                                                    osg::Vec3i(pOsgImage->s()-1, 0, 0),
                                                                    osg::Vec3i(0, pOsgImage->t(), 0),
                                                                    osg::Vec3i(-pOsgImage->s(), 0, 0),
                                                                    osg::Vec3i(0, 0, 1));
                        break;
                    case(7):
                        OSG_INFO<<"EXIF_Orientation 7 (right side, bottom), flip Y, rotate 270."<<std::endl;
                        pOsgImage = osg::createImageWithOrientationConversion(pOsgImage.get(),
                                                                    osg::Vec3i(0, 0, 0),
                                                                    osg::Vec3i(0, pOsgImage->t(), 0),
                                                                    osg::Vec3i(pOsgImage->s(), 0, 0),
                                                                    osg::Vec3i(0, 0, 1));
                    case(8):
                        OSG_INFO<<"EXIF_Orientation 8 (left side, bottom). rotate 270."<<std::endl;
                        pOsgImage = osg::createImageWithOrientationConversion(pOsgImage.get(),
                                                                    osg::Vec3i(0, pOsgImage->t()-1, 0),
                                                                    osg::Vec3i(0, -pOsgImage->t(), 0),
                                                                    osg::Vec3i(pOsgImage->s(), 0, 0),
                                                                    osg::Vec3i(0, 0, 1));
                        break;
                }

            }

            return pOsgImage.release();
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
            return readJPGStream(fin);
        }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osgDB::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
            if(!istream) return ReadResult::ERROR_IN_READING_FILE;
            ReadResult rr = readJPGStream(istream);
            if(rr.validImage()) rr.getImage()->setFileName(file);
            return rr;
        }

        virtual WriteResult writeImage(const osg::Image& img,std::ostream& fout,const osgDB::ReaderWriter::Options *options) const
        {
            osg::ref_ptr<osg::Image> tmp_img = new osg::Image(img);
            tmp_img->flipVertical();
            WriteResult::WriteStatus ws = write_JPEG_file(fout, *(tmp_img.get()), getQuality(options));
            return ws;
        }

        virtual WriteResult writeImage(const osg::Image &img,const std::string& fileName, const osgDB::ReaderWriter::Options *options) const
        {
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

            osgDB::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
            if(!fout) return WriteResult::ERROR_IN_WRITING_FILE;

            return writeImage(img,fout,options);
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(jpeg, ReaderWriterJPEG)
