#include <osg/Image>
#include "osg/Notify"
#include <osg/Geode>
#include "osg/GL"

#include "osgDB/Registry"

using namespace osg;

extern "C"
{
    #include <png.h>
}


/* Transparency parameters */
#define PNG_ALPHA     -2         /* Use alpha channel in PNG file, if there is one */
#define PNG_SOLID     -1         /* No transparency                                */
#define PNG_STENCIL    0         /* Sets alpha to 0 for r=g=b=0, 1 otherwise       */

typedef struct
{
    unsigned int Width;
    unsigned int Height;
    unsigned int Depth;
    unsigned int Alpha;
} pngInfo;

class ReaderWriterPNG : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() { return "PNG Image Reader/Writer"; }
        virtual bool acceptsExtension(const std::string& extension) { return extension=="png"; }

        virtual Image* readImage(const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {

            int trans = PNG_ALPHA;
            FILE *fp = NULL;
            pngInfo pInfo;
            pngInfo *pinfo = &pInfo;

            unsigned char header[8];
            png_structp png;
            png_infop   info;
            png_infop   endinfo;
            png_bytep   data;    //, data2;
            png_bytep  *row_p;
            double  fileGamma;

            png_uint_32 width, height;
            int depth, color;

            png_uint_32 i;
            png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            info = png_create_info_struct(png);
            endinfo = png_create_info_struct(png);

            fp = fopen(fileName.c_str(), "rb");
            if (fp && fread(header, 1, 8, fp) && png_check_sig(header, 8))
                png_init_io(png, fp);
            else
            {
                png_destroy_read_struct(&png, &info, &endinfo);
                return NULL;
            }
            png_set_sig_bytes(png, 8);

            png_read_info(png, info);
            png_get_IHDR(png, info, &width, &height, &depth, &color, NULL, NULL, NULL);

            if (pinfo != NULL)
            {
                pinfo->Width  = width;
                pinfo->Height = height;
                pinfo->Depth  = depth;
            }

            if (color == PNG_COLOR_TYPE_GRAY || color == PNG_COLOR_TYPE_GRAY_ALPHA)
                png_set_gray_to_rgb(png);

            if (color&PNG_COLOR_MASK_ALPHA && trans != PNG_ALPHA)
            {
                png_set_strip_alpha(png);
                color &= ~PNG_COLOR_MASK_ALPHA;
            }

            //	if (!(PalettedTextures && mipmap >= 0 && trans == PNG_SOLID))
            if (color == PNG_COLOR_TYPE_PALETTE)
                png_set_expand(png);

            /*--GAMMA--*/
            //	checkForGammaEnv();
            double screenGamma = 2.2 / 1.0;
            if (png_get_gAMA(png, info, &fileGamma))
                png_set_gamma(png, screenGamma, fileGamma);
            else
                png_set_gamma(png, screenGamma, 1.0/2.2);

            png_read_update_info(png, info);

            data = (png_bytep) malloc(png_get_rowbytes(png, info)*height);
            row_p = (png_bytep *) malloc(sizeof(png_bytep)*height);

            bool StandardOrientation = false;
            for (i = 0; i < height; i++)
            {
                if (StandardOrientation)
                    row_p[height - 1 - i] = &data[png_get_rowbytes(png, info)*i];
                else
                    row_p[i] = &data[png_get_rowbytes(png, info)*i];
            }

            png_read_image(png, row_p);
            free(row_p);

            int iBitCount;

            if (trans == PNG_SOLID || trans == PNG_ALPHA || color == PNG_COLOR_TYPE_RGB_ALPHA || color == PNG_COLOR_TYPE_GRAY_ALPHA)
            {
                switch (color)
                {
                    case PNG_COLOR_TYPE_GRAY:
                    case PNG_COLOR_TYPE_RGB:
                    case PNG_COLOR_TYPE_PALETTE:
                        iBitCount = 24;
                        if (pinfo != NULL) pinfo->Alpha = 0;
                        break;

                    case PNG_COLOR_TYPE_GRAY_ALPHA:
                    case PNG_COLOR_TYPE_RGB_ALPHA:
                        iBitCount = 32;
                        if (pinfo != NULL) pinfo->Alpha = 8;
                        break;

                    default:
                        return NULL;
                }
            }

            png_read_end(png, endinfo);
            png_destroy_read_struct(&png, &info, &endinfo);

            //	free(data);

            if (fp)
                fclose(fp);

            osg::Image* pOsgImage = new osg::Image();

            pOsgImage->setFileName(fileName.c_str());
            if (iBitCount == 24)
                pOsgImage->setImage(width, height, 1,
                    iBitCount / 8,// int internalFormat,
                    GL_RGB,      // unsigned int pixelFormat
                    GL_UNSIGNED_BYTE,// unsigned int dataType
                    data);
            else
                pOsgImage->setImage(width, height, 1,
                    iBitCount / 8,// int internalFormat,
                    GL_RGBA,     // unsigned int pixelFormat
                    GL_UNSIGNED_BYTE,// unsigned int dataType
                    data);
            return pOsgImage;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterPNG> g_readerWriter_PNG_Proxy;
