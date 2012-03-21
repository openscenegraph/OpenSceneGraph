
#include <osg/Image>

#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <string>
#include <sstream>
#include <limits>
#include <gta/gta.hpp>

/* Copyright (C) 2011 Martin Lambers
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
 */

class ReaderWriterGTA : public osgDB::ReaderWriter
{
    public:

        ReaderWriterGTA()
        {
            supportsExtension("gta","GTA (Generic Tagged Arrays) file format");
            supportsOption("COMPRESSION","Set compression method: NONE, ZLIB (default), ZLIB1,...,ZLIB9, BZIP2, or XZ");
        }

        virtual const char* className() const { return "GTA Image Reader"; }

        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension,"gta");
        }

        ReadResult local_readImage(std::istream& fin,const osgDB::ReaderWriter::Options* /* options */) const
        {
            int s,t,r;
            int internalFormat;
            unsigned int pixelFormat;
            unsigned int dataType;
            unsigned char* imageData = NULL;

            std::string my_errmsg;
            try
            {
                gta::header hdr;
                hdr.read_from(fin);
                if (hdr.data_size() > static_cast<uintmax_t>(std::numeric_limits<int>::max()))
                {
                    my_errmsg = "GTA too large";
                    throw std::exception();
                }
                if (hdr.dimensions() < 1 || hdr.dimensions() > 3)
                {
                    my_errmsg = "GTA has less than 1 or more than 3 dimensions";
                    throw std::exception();
                }
                s = t = r = 1;
                for (uintmax_t i = 0; i < hdr.dimensions(); i++)
                {
                    if (hdr.dimension_size(i) > static_cast<uintmax_t>(std::numeric_limits<int>::max()))
                    {
                        my_errmsg = "GTA dimensions too large";
                        throw std::exception();
                    }
                    if (i == 0)
                        s = hdr.dimension_size(i);
                    else if (i == 1)
                        t = hdr.dimension_size(i);
                    else
                        r = hdr.dimension_size(i);
                }
                if (hdr.components() < 1 || hdr.components() > 4)
                {
                    my_errmsg = "GTA has less than 1 or more than 4 element components";
                    throw std::exception();
                }
                pixelFormat =
                    hdr.components() == 1 ? GL_LUMINANCE :
                    hdr.components() == 2 ? GL_LUMINANCE_ALPHA :
                    hdr.components() == 3 ? GL_RGB :
                    GL_RGBA;
                switch (hdr.component_type(0))
                {
                case gta::int8:
                    dataType = GL_BYTE;
                    break;
                case gta::uint8:
                    dataType = GL_UNSIGNED_BYTE;
                    break;
                case gta::int16:
                    dataType = GL_SHORT;
                    break;
                case gta::uint16:
                    dataType = GL_UNSIGNED_SHORT;
                    break;
                case gta::int32:
                    dataType = GL_INT;
                    break;
                case gta::uint32:
                    dataType = GL_UNSIGNED_INT;
                    break;
                case gta::float32:
                    dataType = GL_FLOAT;
                    break;
                default:
                    my_errmsg = "GTA component type(s) not supported";
                    throw std::exception();
                }
                for (uintmax_t i = 1; i < hdr.components(); i++)
                {
                    if (hdr.component_type(i) != hdr.component_type(0))
                    {
                        my_errmsg = "GTA component types differ";
                        throw std::exception();
                    }
                }
                if (dataType == GL_BYTE || dataType == GL_UNSIGNED_BYTE)
                {
                    internalFormat = hdr.components();
                }
                else
                {
                    internalFormat =
                        hdr.components() == 1 ? GL_LUMINANCE32F_ARB :
                        hdr.components() == 2 ? GL_LUMINANCE_ALPHA32F_ARB :
                        hdr.components() == 3 ? GL_RGB32F_ARB :
                        GL_RGBA32F_ARB;
                }
                imageData = new unsigned char[hdr.data_size()];
                hdr.read_data(fin, imageData);
            }
            catch (std::exception& e)
            {
                delete[] imageData;
                if (!(my_errmsg.empty()))
                {
                    OSG_WARN << my_errmsg << std::endl;
                }
                else
                {
                    OSG_WARN << e.what() << std::endl;
                }
                return ReadResult::ERROR_IN_READING_FILE;
            }

            osg::Image* pOsgImage = new osg::Image;
            pOsgImage->setImage(s,t,r,
                internalFormat,
                pixelFormat,
                dataType,
                imageData,
                osg::Image::USE_NEW_DELETE);
            pOsgImage->setOrigin(osg::Image::TOP_LEFT);

            return pOsgImage;
        }

        WriteResult local_writeImage(std::ostream& fout,const osg::Image& img,const osgDB::ReaderWriter::Options* options) const
        {
            std::string my_errmsg;
            try
            {
                gta::header hdr;
                gta::compression compression = gta::zlib;
                if (options)
                {
                    std::istringstream iss(options->getOptionString());
                    std::string opt;
                    std::string compressionMethod;
                    while (iss >> opt)
                    {
                        if (opt == "COMPRESSION")
                        {
                            iss >> compressionMethod;
                        }
                    };
                    if (compressionMethod == "NONE")
                        compression = gta::none;
                    else if (compressionMethod == "ZLIB")
                        compression = gta::zlib;
                    else if (compressionMethod == "ZLIB1")
                        compression = gta::zlib1;
                    else if (compressionMethod == "ZLIB2")
                        compression = gta::zlib2;
                    else if (compressionMethod == "ZLIB3")
                        compression = gta::zlib3;
                    else if (compressionMethod == "ZLIB4")
                        compression = gta::zlib4;
                    else if (compressionMethod == "ZLIB5")
                        compression = gta::zlib5;
                    else if (compressionMethod == "ZLIB6")
                        compression = gta::zlib6;
                    else if (compressionMethod == "ZLIB7")
                        compression = gta::zlib7;
                    else if (compressionMethod == "ZLIB8")
                        compression = gta::zlib8;
                    else if (compressionMethod == "ZLIB9")
                        compression = gta::zlib9;
                    else if (compressionMethod == "BZIP2")
                        compression = gta::bzip2;
                    else if (compressionMethod == "XZ")
                        compression = gta::xz;
                }
                hdr.set_compression(compression);
                if (img.s() > 0 && img.t() <= 1 && img.r() <= 1)
                {
                    hdr.set_dimensions(img.s());
                }
                else if (img.s() > 0 && img.t() > 1 && img.r() <= 1)
                {
                    hdr.set_dimensions(img.s(), img.t());
                }
                else if (img.s() > 0 && img.t() > 1 && img.r() > 1)
                {
                    hdr.set_dimensions(img.s(), img.t(), img.r());
                }
                else
                {
                    my_errmsg = "Image has unsupported dimensions";
                    throw std::exception();
                }
                gta::type type;
                switch (img.getDataType())
                {
                case GL_BYTE:
                    type = gta::int8;
                    break;
                case GL_UNSIGNED_BYTE:
                    type = gta::uint8;
                    break;
                case GL_SHORT:
                    type = gta::int16;
                    break;
                case GL_UNSIGNED_SHORT:
                    type = gta::uint16;
                    break;
                case GL_INT:
                    type = gta::int32;
                    break;
                case GL_UNSIGNED_INT:
                    type = gta::uint32;
                    break;
                case GL_FLOAT:
                    type = gta::float32;
                    break;
                default:
                    my_errmsg = "Image has unsupported data type";
                    throw std::exception();
                }
                switch (img.getPixelFormat())
                {
                case 1:
                case GL_DEPTH_COMPONENT:
                case GL_LUMINANCE:
                case GL_ALPHA:
                    hdr.set_components(type);
                    break;
                case 2:
                case GL_LUMINANCE_ALPHA:
                    hdr.set_components(type, type);
                    break;
                case 3:
                case GL_RGB:
                    hdr.set_components(type, type, type);
                    break;
                case 4:
                case GL_RGBA:
                    hdr.set_components(type, type, type, type);
                    break;
                default:
                    my_errmsg = "Image has unsupported pixel format";
                    throw std::exception();
                }
                if (img.getPacking() != 1)
                {
                    my_errmsg = "Image has unsupported packing";
                    throw std::exception();
                }
                hdr.write_to(fout);
#if 0   /* Does not seem to be necessary */
                if (img.t() > 1 && img.getOrigin() == osg::Image::BOTTOM_LEFT)
                {
                    int depth = (img.r() >= 1 ? img.r() : 1);
                    const unsigned char* data = static_cast<const unsigned char*>(img.getDataPointer());
                    size_t row_size = hdr.element_size() * img.s();
                    gta::io_state io_state;
                    for (int k = 0; k < depth; k++)
                    {
                        const unsigned char* slice = data + k * (row_size * img.t());
                        for (int j = 0; j < img.t(); j++)
                        {
                            const unsigned char* p = slice + (img.t() - 1 - j) * row_size;
                            hdr.write_elements(io_state, fout, img.s(), p);
                        }
                    }
                }
                else
                {
                    hdr.write_data(fout, img.getDataPointer());
                }
#endif
                hdr.write_data(fout, img.getDataPointer());
            }
            catch (std::exception& e)
            {
                if (!(my_errmsg.empty()))
                {
                    OSG_WARN << my_errmsg << std::endl;
                }
                else
                {
                    OSG_WARN << e.what() << std::endl;
                }
                return WriteResult::ERROR_IN_WRITING_FILE;
            }

            return WriteResult::FILE_SAVED;
        }

        virtual ReadResult readObject(std::istream& fin,const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return readImage(fin, options);
        }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return readImage(file, options);
        }

        virtual ReadResult readImage(std::istream& fin,const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return local_readImage(fin, options);
        }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osgDB::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
            if(!istream) return ReadResult::FILE_NOT_HANDLED;
            ReadResult rr = local_readImage(istream, options);
            if(rr.validImage()) rr.getImage()->setFileName(file);
            return rr;
        }

        virtual WriteResult writeImage(const osg::Image& img,std::ostream& fout,const osgDB::ReaderWriter::Options* options) const
        {
            return local_writeImage(fout,img,options);
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
REGISTER_OSGPLUGIN(gta, ReaderWriterGTA)
