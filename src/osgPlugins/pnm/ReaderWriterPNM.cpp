// PNM Reader -- Written by Eric Sokolowsky
// Reads Ascii and Binary files in the PPM, PGM, and PBM formats.

#include <osg/Image>
#include <osg/Notify>

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

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            FILE *fp = NULL;
            char line[300];
            int ppmtype = 0; /* P1, P2, etc. */
            int width = 0;
            int height = 0;
            int max_value = 0;

            bool binary_flag = false;
            int shift_value = 0; // if greater than 8 bits


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
            if (width <= 0 || height <= 0 || max_value <= 0 || ppmtype < 1 ||
                ppmtype > 6)
            {
                fclose(fp);
                return ReadResult::FILE_NOT_HANDLED;
            }

            // Check for binary file.
            if (ppmtype >= 4 && ppmtype <= 6)
                binary_flag = true;

            // Warn the user if the full image cannot be used.
            if (max_value > 255)
            {
                osg::notify(osg::NOTICE) << "PNM file " << fileName <<
                    " has channels larger than "
                    " 8 bits.  Color resolution will be lost." << std::endl;

                while (max_value > 255)
                {
                    max_value >>= 1;
                    shift_value++;
                }
            }

            // We always create a RGB image, no matter what type of
            // source it was.
            unsigned char *data = new unsigned char [width * height * 3];



            // For the ascii files
            if (!binary_flag)
            {
                unsigned char *end = data + width * height * 3;
                unsigned char *dst = data;

                char s_num[300];
                int s_num_count;
                int value = fgetc(fp);

                while (dst < end)
                {
                    if (feof(fp) || ferror(fp))
                    {
                        fclose(fp);
                        delete[] data;
                        return ReadResult::FILE_NOT_HANDLED;
                    }

                    // Read any extra whitespace
                    //while (isspace(value)) 
                    while (!isdigit(value))
                    {
                        value = fgetc(fp);
                    }

                    // Read any numeric digits
                    s_num_count = 0;
                    while (isdigit(value))
                    {
                        s_num[s_num_count++] = value;
                        value = fgetc(fp);
                    }
                    // Don't forget to terminate the string!
                    s_num[s_num_count] = 0;

                    if (s_num_count == 0)
                    {
                        fclose(fp);
                        delete[] data;
                        return ReadResult::FILE_NOT_HANDLED;
                    }

                    unsigned int data_value = atoi(s_num) >> shift_value;

                    // Now we have our value.  Put it into the array
                    // in the appropriate place.
                    if (ppmtype == 1)
                    {
                        if (data_value == 1)
                            data_value = 0;
                        else
                            data_value = 255;

                        *(dst++) = data_value;
                        *(dst++) = data_value;
                        *(dst++) = data_value;
                    }
                    else if (ppmtype == 2)
                    {
                        *(dst++) = data_value;
                        *(dst++) = data_value;
                        *(dst++) = data_value;
                    }
                    else if (ppmtype == 3)
                    {
                        *(dst++) = data_value;
                    }
                }
            }

            // If we have a binary bitmap
            else if (ppmtype == 4)
            {
                unsigned char *end = data + width * height * 3;
                unsigned char *dst = data;

                while (dst < end)
                {
                    unsigned char b = (unsigned char) fgetc(fp);
                    if (feof(fp) || ferror(fp))
                    {
                        fclose(fp);
                        delete[] data;
                        return ReadResult::FILE_NOT_HANDLED;
                    }

                    int i;
                    for (i = 7; i >= 0 && dst < end; i--)
                    {
                        // 1 means black, 0 means white
                        int data_value = (b & (1<<i)) ? 0 : 255;
                        *(dst++) = data_value;
                        *(dst++) = data_value;
                        *(dst++) = data_value;
                    }
                }
            }

            // If we have a binary pgm
            else if (ppmtype == 5)
            {
                int result = fread(data, width * height, 1, fp);
                if (result != 1)
                {
                    fclose(fp);
                    delete[] data;
                    return ReadResult::FILE_NOT_HANDLED;
                }

                unsigned char *src = data + width * height;
                unsigned char *dst = data + width * height * 3;
                while (src >= data)
                {
                    *(--dst) = *(--src);
                    *(--dst) = *src;
                    *(--dst) = *src;
                }
            }

            // If we have a binary ppm, reading is very easy.
            else if (ppmtype == 6)
            {
                int result = fread(data, width * height * 3, 1, fp);
                if (result != 1)
                {
                    fclose(fp);
                    delete[] data;
                    return ReadResult::FILE_NOT_HANDLED;
                }
            }

            if (fp)
                fclose(fp);

            osg::Image* pOsgImage = new osg::Image();

            pOsgImage->setFileName(fileName.c_str());
            pOsgImage->setImage(width, height, 1,
                    3,// int internalFormat,
                    GL_RGB,      // unsigned int pixelFormat
                    GL_UNSIGNED_BYTE,// unsigned int dataType
                    data,
                    osg::Image::USE_NEW_DELETE);
            pOsgImage->flipVertical();

            return pOsgImage;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterPNM> g_readerWriter_PNM_Proxy;
