#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <sstream>

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

#include <stdio.h>

extern "C"
{
    #include <jpeglib.h>
};

#include <setjmp.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define ERR_NO_ERROR 0
#define ERR_OPEN     1
#define ERR_MEM      2
#define ERR_JPEGLIB  3

static int jpegerror = ERR_NO_ERROR;

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


unsigned char *
simage_jpeg_load(const char *filename,
int *width_ret,
int *height_ret,
int *numComponents_ret)
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
    FILE * infile;               /* source file */
    JSAMPARRAY rowbuffer;        /* Output row buffer */
    int row_stride;              /* physical row width in output buffer */

    jpegerror = ERR_NO_ERROR;

    /* In this example we want to open the input file before doing anything else,
     * so that the setjmp() error recovery below can assume the file is open.
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to read binary files.
     */

    if ((infile = fopen(filename, "rb")) == NULL)
    {
        jpegerror = ERR_OPEN;
        return NULL;
    }

    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer))
    {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object, close the input file, and return.
         */
        jpegerror = ERR_JPEGLIB;
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        //if (buffer) delete [] buffer;
        return NULL;
    }

    // used to be before setjump above, but have moved to after to avoid compile warnings.
    unsigned char *buffer = NULL;

    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */

    jpeg_stdio_src(&cinfo, infile);

    /* Step 3: read file parameters with jpeg_read_header() */

    (void) jpeg_read_header(&cinfo, TRUE);
    /* We can ignore the return value from jpeg_read_header since
     *   (a) suspension is not possible with the stdio data source, and
     *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
     * See libjpeg.doc for more info.
     */

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
    fclose(infile);

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


class ReaderWriterJPEG : public osgDB::ReaderWriter
{
        WriteResult::WriteStatus write_JPEG_file (const char* filename,int image_width,int image_height,JSAMPLE* image_buffer,int quality = 100)
        {
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
            FILE * outfile;        /* target file */
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
            if (!(outfile = fopen(filename, "wb")))
            {
                return WriteResult::ERROR_IN_WRITING_FILE;
            }
            jpeg_stdio_dest(&cinfo, outfile);

            /* Step 3: set parameters for compression */

            /* First we supply a description of the input image.
            * Four fields of the cinfo struct must be filled in:
            */
            cinfo.image_width = image_width;     /* image width and height, in pixels */
            cinfo.image_height = image_height;
            cinfo.input_components = 3;        /* # of color components per pixel */
            cinfo.in_color_space = JCS_RGB;     /* colorspace of input image */
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
            row_stride = image_width * 3;    /* JSAMPLEs per row in image_buffer */

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
            fclose(outfile);

            /* Step 7: release JPEG compression object */

            /* This is an important step since it will release a good deal of memory. */
            jpeg_destroy_compress(&cinfo);

            /* And we're done! */
            return WriteResult::FILE_SAVED;
        }
        int getQuality(const osgDB::ReaderWriter::Options *options) {
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
        virtual const char* className() const { return "JPEG Image Reader/Writer"; }
        virtual bool acceptsExtension(const std::string& extension)
        {
            return osgDB::equalCaseInsensitive(extension,"jpeg") || osgDB::equalCaseInsensitive(extension,"jpg");
        }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options*)
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            unsigned char *imageData = NULL;
            int width_ret;
            int height_ret;
            int numComponents_ret;

            imageData = simage_jpeg_load(fileName.c_str(),&width_ret,&height_ret,&numComponents_ret);

            if (imageData==NULL) return ReadResult::FILE_NOT_HANDLED;

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
        virtual WriteResult writeImage(const osg::Image &img,const std::string& fileName, const osgDB::ReaderWriter::Options *options)
        {
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;
            osg::ref_ptr<osg::Image> tmp_img = new osg::Image(img);
            tmp_img->flipVertical();
            WriteResult::WriteStatus ws = write_JPEG_file(fileName.c_str(),img.s(),img.t(),(JSAMPLE*)(tmp_img->data()),getQuality(options));
            return ws;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterJPEG> g_readerWriter_JPEG_Proxy;
