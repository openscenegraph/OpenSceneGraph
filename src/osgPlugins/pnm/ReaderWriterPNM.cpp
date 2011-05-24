// PNM Reader -- Written by Eric Sokolowsky
// Reads Ascii and Binary files in the PPM, PGM, and PBM formats.

#include <osg/Image>
#include <osg/Notify>
#include <osg/Endian>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>

using namespace osg;

template <class T>
    unsigned char* read_bitmap_ascii(std::istream& fin, int width, int height)
{
    T* data = new T[width*height];
    T* end = data + width*height;

    int x = 0;
    T* dst = end - width;
    int value = 0;

    while(dst >= data)
    {
        fin >> value;
        if (!fin.good())
        {
            delete [] data;
            return NULL;
        }

        // place value in the image. 0 is white and anything else is black.
        *(dst++) = value ? 0 : 255;

        // At end of each row, jump back two rows
        ++x;
        if (x == width)
        {
            x = 0;
            dst -= 2*width;
        }
    }

    return reinterpret_cast<unsigned char*>(data);
}

template <class T>
    unsigned char* read_grayscale_ascii(std::istream& fin, int width, int height, int max_value)
{
    T* data = new T[width*height];
    T* end = data + width*height;

    int x = 0;
    T* dst = end - width;
    int value = 0;
    float max = (sizeof(T) == 1) ? 255.0 : 65535.0;

    while(dst >= data)
    {
        fin >> value;
        if (!fin.good())
        {
            delete [] data;
            return NULL;
        }

        // place value in the image
        *(dst++) = T(float(value)/float(max_value)*max);

        // At end of each row, jump back two rows
        ++x;
        if (x == width)
        {
            x = 0;
            dst -= 2*width;
        }
    }

    return reinterpret_cast<unsigned char*>(data);
}

template <class T>
    unsigned char* read_color_ascii(std::istream& fin, int width, int height, int max_value)
{
    T* data = new T[3*width*height];
    T* end = data + 3*width*height;

    int x = 0;
    T* dst = end - 3*width;
    int value = 0;
    float max = (sizeof(T) == 1) ? 255.0 : 65535.0;

    while(dst >= data)
    {
        fin >> value;
        if (!fin.good())
        {
            delete [] data;
            return NULL;
        }

        // place value in the image
        *(dst++) = T(float(value)/float(max_value)*max);

        // At end of the row, jump back two rows
        ++x;
        if (x == width*3)
        {
            x = 0;
            dst -= 6*width;
        }
    }

    return reinterpret_cast<unsigned char*>(data);
}

template <class T>
    unsigned char* read_bitmap_binary(std::istream& fin, int width, int height)
{
    T* data = new T[width*height];

    for(int y = height-1; y >= 0; --y)
    {
        T* dst = data + (y+0)*width;
        T* end = data + (y+1)*width;

        while(dst < end)
        {
            unsigned char b = (unsigned char) fin.get();
            if (!fin.good())
            {
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
    unsigned char* read_grayscale_binary(std::istream& fin, int width, int height)
{
    T* data = new T[width*height];

    for(int y = height-1; y >= 0; --y)
    {
        T* dst = data + y*width;
        fin.read((char*)dst, sizeof(T)*width);
        if (!fin.good())
        {
            delete [] data;
            return NULL;
        }
    }

    // if the machine is little endian swap the bytes around
    if (sizeof(T) == 2 && getCpuByteOrder() == osg::LittleEndian)
    {
        unsigned char *bs = reinterpret_cast<unsigned char *>(data);
        unsigned char *end = bs + sizeof(T)*width*height;
        for (; bs < end; bs += 2)
        {
            std::swap(bs[0], bs[1]);
        }
    }

    return reinterpret_cast<unsigned char *>(data);
}

template <class T>
    unsigned char* read_color_binary(std::istream& fin, int width, int height)
{
    T* data = new T[3*width*height];

    for(int y = height-1; y >= 0; --y)
    {
        T* dst = data + y*3*width;
        fin.read((char*)dst, sizeof(T)*3*width);
        if (!fin.good())
        {
            delete [] data;
            return NULL;
        }
    }

    // if the machine is little endian swap the bytes around
    if (sizeof(T) == 2 && getCpuByteOrder() == osg::LittleEndian)
    {
        unsigned char *bs = reinterpret_cast<unsigned char *>(data);
        unsigned char *end = bs + sizeof(T)*3*width*height;
        for (; bs < end; bs+=2)
        {
            std::swap(bs[0], bs[1]);
        }
    }

    return reinterpret_cast<unsigned char*>(data);
}

class ReaderWriterPNM : public osgDB::ReaderWriter
{
    public:
        ReaderWriterPNM()
        {
            supportsExtension("pnm","PNM Image format");
            supportsExtension("ppm","PNM Image format");
            supportsExtension("pgm","PNM Image format");
            supportsExtension("pbm","PNM Image format");
        }
        
        virtual const char* className() const { return "PNM Image Reader/Writer"; }

        virtual ReadResult readImage(std::istream& fin, const osgDB::ReaderWriter::Options* options=NULL) const
        {
            int ppmtype = 0;    /* P1, P2, etc. */
            int width = 0;
            int height = 0;
            int max_value = 0;

            // Read header items.
            std::string line;
            int row;
            for (row = 1; row <= 3; row++)
            {
                getline(fin, line);
                if (!fin.good())
                    return ReadResult::ERROR_IN_READING_FILE;

                const char *cp = line.c_str();
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
                    std::istringstream istr(line);

                    istr >> width;
                    istr >> height;

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
                    std::istringstream istr(line);
                    istr >> max_value;
                }
            }

            // Check for valid values.
            if (width <= 0 || height <= 0 ||
                max_value <= 0 || max_value > 65535 ||
                ppmtype < 1 || ppmtype > 6)
            {
                return ReadResult::ERROR_IN_READING_FILE;
            }

            int pixelFormat = 0;
            int dataType = 0;
            unsigned char* data = NULL;

            if (max_value > 255)
            {
                dataType = GL_UNSIGNED_SHORT;
                switch(ppmtype)
                {
                    case 2:    // grayscale ascii
                        pixelFormat = GL_LUMINANCE;
                        data = read_grayscale_ascii<unsigned short>(fin, width, height, max_value);
                        break;
                    case 3:    // color ascii
                        pixelFormat = GL_RGB;
                        data = read_color_ascii<unsigned short>(fin, width, height, max_value);
                        break;
                    case 5:    // grayscale binary
                        pixelFormat = GL_LUMINANCE;
                        data = read_grayscale_binary<unsigned short>(fin, width, height);
                        break;
                    case 6:    // color binary
                        pixelFormat = GL_RGB;
                        data = read_color_binary<unsigned short>(fin, width, height);
                        break;
                    default:
                        return ReadResult::ERROR_IN_READING_FILE;
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
                        data = read_bitmap_ascii<unsigned char>(fin, width, height);
                        break;
                    case 2:    // grayscale ascii
                        pixelFormat = GL_LUMINANCE;
                        data = read_grayscale_ascii<unsigned char>(fin, width, height, max_value);
                        break;
                    case 3:    // color ascii
                        pixelFormat = GL_RGB;
                        data = read_color_ascii<unsigned char>(fin, width, height, max_value);
                        break;
                    case 4:    // bitmap binary
                        pixelFormat = GL_LUMINANCE;
                        data = read_bitmap_binary<unsigned char>(fin, width, height);
                        break;
                    case 5:    // grayscale binary
                        pixelFormat = GL_LUMINANCE;
                        data = read_grayscale_binary<unsigned char>(fin, width, height);
                        break;
                    case 6:    // color binary
                        pixelFormat = GL_RGB;
                        data = read_color_binary<unsigned char>(fin, width, height);
                        break;
                    default:
                        return ReadResult::ERROR_IN_READING_FILE;
                        break;
                }
            }

            if (data == NULL)
            {
                return ReadResult::FILE_NOT_HANDLED;
            }

            osg::Image* pOsgImage = new osg::Image();

            pOsgImage->setImage(width, height, 1,
                pixelFormat,
                pixelFormat,
                dataType,
                data,
                osg::Image::USE_NEW_DELETE);

            if (options && options->getOptionString().find("flip")!=std::string::npos)
            {
                pOsgImage->flipVertical();
            }

            return pOsgImage;
        }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            std::ifstream fin(fileName.c_str());
            if (!fin.good())
                return ReadResult::ERROR_IN_READING_FILE;

            ReadResult rr = readImage(fin, options);
            fin.close();
            if (rr.validImage()) rr.getImage()->setFileName(file);
            return rr;
        }

        virtual WriteResult writeImage(const osg::Image& image,std::ostream& fout,const osgDB::ReaderWriter::Options* options) const
        {
            bool ascii = (options && options->getOptionString().find("ascii")!=std::string::npos);

            if (ascii)
            {
                // ascii ppm format.
                fout<<"P3"<<std::endl;
                fout<<image.s()<<" "<<image.t()<<std::endl;
                fout<<"255"<<std::endl;
                for(int row = image.t()-1; row >= 0; --row)
                {
                    const unsigned char* ptr = image.data(0,row);
                    for(int col = 0; col < image.s(); ++col)
                    {
                        fout<<static_cast<int>(*(ptr++));
                        fout<<" "<<static_cast<int>(*(ptr++));
                        fout<<" "<<static_cast<int>(*(ptr++))<<"  ";
                    }
                    fout<<std::endl;
                }
            }
            else
            {
                // binary ppm format        
                fout<<"P6"<<std::endl;
                fout<<image.s()<<" "<<image.t()<<std::endl;
                fout<<"255"<<std::endl;
                for(int row = image.t()-1; row >= 0; --row)
                {
                    const unsigned char* ptr = image.data(0,row);
                    for(int col = 0; col < image.s(); ++col)
                    {
                        fout.put(*(ptr++));
                        fout.put(*(ptr++));
                        fout.put(*(ptr++));
                    }
                }
            }
            return WriteResult::FILE_SAVED;
        }

        virtual WriteResult writeImage(const osg::Image& image,const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            // Only ppm format output supported
            std::string ext = osgDB::getFileExtension(fileName);
            if ( !osgDB::equalCaseInsensitive(ext, "ppm") ) return WriteResult::FILE_NOT_HANDLED;
            
            // only support rgb images right now.
            if (image.getPixelFormat()!=GL_RGB || image.getDataType()!=GL_UNSIGNED_BYTE) return WriteResult("Error image pixel format not supported by pnm writer.");

            osgDB::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
            if(!fout) return WriteResult::ERROR_IN_WRITING_FILE;

            return writeImage(image,fout,options);
        }


};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(pnm, ReaderWriterPNM)
