// PNM Reader -- Written by Eric Sokolowsky
// Reads Ascii and Binary files in the PPM, PGM, and PBM formats.

#include <osg/Image>
#include <osg/Notify>
#include <osg/Endian>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <stdio.h>

using namespace osg;

class ReaderWriterPNM : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() const { return "PNM Image Reader/Writer"; }
        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension, "pnm") ||
                osgDB::equalCaseInsensitive(extension, "ppm") ||
                osgDB::equalCaseInsensitive(extension, "pgm") ||
                osgDB::equalCaseInsensitive(extension, "pbm");
        }

        template <class T>
            unsigned char* read_bitmap_ascii(FILE* fp, int width, int height) const
        {
            T* data = new T[width*height];

            T* dst = data;
            T* end = data + width*height;

            while(dst < end)
            {
                T value = 0;

                // read in characters looking for '0's and '1's, these
                // values map to 255 and 0. Any other characters
                // are silently ignored.
                while(1)
                {
                    int ch = fgetc(fp);
                    if (feof(fp) || ferror(fp))
                    {
                        fclose(fp);
                        delete [] data;
                        return NULL;
                    }

                    if (ch == '0')
                    {
                        value = 255;
                        break;
                    }
                    else if (ch == '1')
                    {
                        value = 0;
                        break;
                    }
                }

                // place value in the image
                *(dst++) = value;
            }

            return reinterpret_cast<unsigned char*>(data);
        }

        template <class T>
            unsigned char* read_grayscale_ascii(FILE* fp, int width, int height) const
        {
            T* data = new T[width*height];

            T* dst = data;
            T* end = data + width*height;

            while(dst < end)
            {
                int ch;
                T value = 0;

                // read and discard any whitespace
                // until a digit is reached
                do
                {
                    ch = fgetc(fp);
                    if (feof(fp) || ferror(fp))
                    {
                        fclose(fp);
                        delete [] data;
                        return NULL;
                    }
                }
                while(!isdigit(ch));

                // continue reading digits and incrementally
                // construct the integer value
                do
                {
                    value = 10*value + (ch - '0');
                    ch = fgetc(fp);
                    if (feof(fp) || ferror(fp))
                    {
                        fclose(fp);
                        delete [] data;
                        return NULL;
                    }
                }
                while(isdigit(ch));

                // place value in the image
                *(dst++) = value;
            }

            return reinterpret_cast<unsigned char*>(data);
        }

        template <class T>
            unsigned char* read_color_ascii(FILE* fp, int width, int height) const
        {
            T* data = new T[3*width*height];

            T* dst = data;
            T* end = data + 3*width*height;

            while(dst < end)
            {
                int ch;
                T value = 0;

                // read and discard any whitespace
                // until a digit is reached
                do
                {
                    ch = fgetc(fp);
                    if (feof(fp) || ferror(fp))
                    {
                        fclose(fp);
                        delete [] data;
                        return NULL;
                    }
                }
                while(!isdigit(ch));

                // continue reading digits and incrementally
                // construct the integer value
                do
                {
                    value = 10*value + (ch - '0');
                    ch = fgetc(fp);
                    if (feof(fp) || ferror(fp))
                    {
                        fclose(fp);
                        delete [] data;
                        return NULL;
                    }
                }
                while(isdigit(ch));

                // place value in the image
                *(dst++) = value;
            }

            return reinterpret_cast<unsigned char*>(data);
        }

        template <class T>
            unsigned char* read_bitmap_binary(FILE* fp, int width, int height) const
        {
            T* data = new T[width*height];

            for(int y = 0; y < height; y++)
            {
                T* dst = data + (y+0)*width;
                T* end = data + (y+1)*width;

                while(dst < end)
                {
                    unsigned char b = fgetc(fp);
                    if (feof(fp) || ferror(fp))
                    {
                        fclose(fp);
                        delete [] data;
                        return NULL;
                    }

                    for(int i = 7; i >= 0 && dst < end; i--)
                    {
                        // 1 means black, 0 means white
                        T data_value = (b & (1<<i)) ? 0 : 255;
                        *(dst++) = data_value;
                    }
                }
            }

            return reinterpret_cast<unsigned char*>(data);
        }

        template <class T>
            unsigned char* read_grayscale_binary(FILE* fp, int width, int height) const
        {
            T* data = new T[width*height];

            if (fread(data, sizeof(T)*width*height, 1, fp) != 1)
            {
                fclose(fp);
                delete [] data;
                return NULL;
            }

            // if the machine is little endian swap the bytes around
            if (sizeof(T) > 1 && getCpuByteOrder() == osg::LittleEndian)
            {
                for(int i = 0; i < width*height; i++)
                {
                    unsigned char* bs = (unsigned char*)(&data[i]);
                    std::swap(bs[0], bs[1]);
                }
            }

            return reinterpret_cast<unsigned char*>(data);
        }

        template <class T>
            unsigned char* read_color_binary(FILE* fp, int width, int height) const
        {
            T* data = new T[3*width*height];

            if (fread(data, 3*sizeof(T)*width*height, 1, fp) != 1)
            {
                fclose(fp);
                delete [] data;
                return NULL;
            }

            // if the machine is little endian swap the bytes around
            if (sizeof(T) > 1 && getCpuByteOrder() == osg::LittleEndian)
            {
                for(int i = 0; i < 3*width*height; i++)
                {
                    unsigned char* bs = (unsigned char*)(&data[i]);
                    std::swap(bs[0], bs[1]);
                }
            }

            return reinterpret_cast<unsigned char*>(data);
        }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            FILE *fp = NULL;
            char line[300];
            int ppmtype = 0;    /* P1, P2, etc. */
            int width = 0;
            int height = 0;
            int max_value = 0;

            // Open file.
            fp = fopen(fileName.c_str(), "rb");

            // Read header items.
            int row;
            for (row = 1; row <= 3; row++)
            {
                fgets(line, 300, fp);

                char *cp = line;
                while (*cp && isspace(*cp))
                    cp++;
                if (! *cp || *cp == '#')
                {
                    // Skip comment lines.
                    row--;
                }
                else if (row == 1)
                {
                    // Get the image type.
                    if (line[0] == 'p' || line[0] == 'P')
                    {
                        ppmtype = line[1] - '0';
                    }
                }
                else if (row == 2)
                {
                    // Get the image size.
                    width = atoi(line);
                    char *cp = line + strspn(line, "\t \n\r");
                    cp += strspn(cp, "0123456789");
                    cp += strspn(line, "\t \n\r");
                    height = atoi(cp);

                    // pbm files don't have row 3
                    if (ppmtype == 1 || ppmtype == 4)
                    {
                        max_value = 1;
                        break;
                    }
                }
                else if (row == 3)
                {
                    // Get the maximum value
                    max_value = atoi(line);
                }
            }

            // Check for valid values.
            if (width <= 0 || height <= 0 ||
                max_value <= 0 || max_value > 65535 ||
                ppmtype < 1 || ppmtype > 6)
            {
                fclose(fp);
                return ReadResult::FILE_NOT_HANDLED;
            }

            int pixelFormat = 0;
            int dataType = 0;
            unsigned char* data = NULL;

            if (max_value > 255)
            {
                dataType = GL_UNSIGNED_SHORT;
                switch(ppmtype)
                {
                    case 1:    // bitmap ascii
                        pixelFormat = GL_LUMINANCE;
                        data = read_bitmap_ascii<unsigned short>(fp, width, height);
                        break;
                    case 2:    // grayscale ascii
                        pixelFormat = GL_LUMINANCE;
                        data = read_grayscale_ascii<unsigned short>(fp, width, height);
                        break;
                    case 3:    // color ascii
                        pixelFormat = GL_RGB;
                        data = read_color_ascii<unsigned short>(fp, width, height);
                        break;
                    case 4:    // bitmap binary
                        pixelFormat = GL_LUMINANCE;
                        data = read_bitmap_binary<unsigned short>(fp, width, height);
                        break;
                    case 5:    // grayscale binary
                        pixelFormat = GL_LUMINANCE;
                        data = read_grayscale_binary<unsigned short>(fp, width, height);
                        break;
                    case 6:    // color binary
                        pixelFormat = GL_RGB;
                        data = read_color_binary<unsigned short>(fp, width, height);
                        break;
                }
            }
            else
            {
                dataType = GL_UNSIGNED_BYTE;
                switch(ppmtype)
                {
                    case 1:    // bitmap ascii
                        pixelFormat = GL_LUMINANCE;
                        data = read_bitmap_ascii<unsigned char>(fp, width, height);
                        break;
                    case 2:    // grayscale ascii
                        pixelFormat = GL_LUMINANCE;
                        data = read_grayscale_ascii<unsigned char>(fp, width, height);
                        break;
                    case 3:    // color ascii
                        pixelFormat = GL_RGB;
                        data = read_color_ascii<unsigned char>(fp, width, height);
                        break;
                    case 4:    // bitmap binary
                        pixelFormat = GL_LUMINANCE;
                        data = read_bitmap_binary<unsigned char>(fp, width, height);
                        break;
                    case 5:    // grayscale binary
                        pixelFormat = GL_LUMINANCE;
                        data = read_grayscale_binary<unsigned char>(fp, width, height);
                        break;
                    case 6:    // color binary
                        pixelFormat = GL_RGB;
                        data = read_color_binary<unsigned char>(fp, width, height);
                        break;
                }
            }

            if (data == NULL)
            {
                fclose(fp);
                return ReadResult::FILE_NOT_HANDLED;
            }

            if (fp)
                fclose(fp);

            osg::Image* pOsgImage = new osg::Image();

            pOsgImage->setFileName(fileName.c_str());
            pOsgImage->setImage(width, height, 1,
                pixelFormat,
                pixelFormat,
                dataType,
                data,
                osg::Image::USE_NEW_DELETE);

            return pOsgImage;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterPNM> g_readerWriter_PNM_Proxy;
