/*
*
* Copyright (C) 2004 Mekensleep
*
*    Mekensleep
*    24 rue vieille du temple
*    75004 Paris
*       licensing@mekensleep.com
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*
* Author:
*  Igor Kravtchenko <igor@obraz.net>
*
*/

#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>

#include "hdrloader.h"
#include "hdrwriter.h"

#ifndef GL_RGBA8
    #define GL_RGBA8 0x8058
#endif

#ifndef GL_RGB8
    #define GL_RGB8 0x8051
#endif


class ReaderWriterHDR : public osgDB::ReaderWriter
{
public:
    ReaderWriterHDR()
    {
        supportsExtension("hdr","High Dynamic Range image format");
        supportsOption("RGBMUL","");
        supportsOption("RGB8","");
        supportsOption("RAW","");
        supportsOption("YFLIP","");
        supportsOption("NO_YFLIP","");
    }
    virtual const char* className() { return "HDR Image Reader"; }

    virtual ReadResult readImage(const std::string &_file, const osgDB::ReaderWriter::Options *_opts) const
    {
        std::string filepath = osgDB::findDataFile(_file, _opts);
        if (filepath.empty())
            return ReadResult::FILE_NOT_FOUND;

        if (!HDRLoader::isHDRFile(filepath.c_str()))
            return ReadResult::FILE_NOT_HANDLED;

        float mul = 1.0f;
        bool bYFlip = false;
        bool convertToRGB8 = false;
        bool rawRGBE = false;
        if(_opts)
        {
            std::istringstream iss(_opts->getOptionString());
            std::string opt;
            while (iss >> opt)
            {
                if(opt == "RGBMUL")
                {
                    iss >> mul;
                }
                else if(opt == "RGB8")
                {
                    convertToRGB8 = true;
                }
                /* RAW: store the RGBE values into a Image, to use this option you
                 * need to decode the RGBE value in the fragment shader. Follow
                 * the cube map glsl decoder:
                 *
                 * vec4 textureCubeRGBE( uniform samplerCube sampler, vec3 coords )
                 * {
                 *     ivec4 rgbe = textureCube( sampler, coords ) * 255. + 0.5;
                 *     float e = rgbe.a - ( 128 + 8 );
                 *     return vec4( rgbe.rgb * exp2( e ), 1.0 );
                 * }
                 *
                 */
                else if(opt == "RAW")
                {
                    rawRGBE = true;
                }
                else if(opt == "YFLIP")
                {
                    bYFlip = true; // Image is flipped later if required
                }
            }
        }

        HDRLoaderResult res;
        bool ret = HDRLoader::load(filepath.c_str(), rawRGBE, res);
        if (!ret)
            return ReadResult::ERROR_IN_READING_FILE;

        // create the osg::Image to fill in.
        osg::Image *img = new osg::Image;

        // copy across the raw data into the osg::Image
        if (convertToRGB8)
        {
            int nbPixs = res.width * res.height;
            int nbElements = nbPixs * 3;
            unsigned char *rgb = new unsigned char[ nbElements ];
            unsigned char *tt = rgb;
            float *cols = res.cols;

            for (int i = 0; i < nbElements; i++) {
                float element = *cols++;
                element *= mul;
                if (element < 0) element = 0;
                else if (element > 1) element = 1;
                int intElement = (int) (element * 255.0f);
                *tt++ = intElement;
            }

            delete [] res.cols;

            int pixelFormat = GL_RGB;
            int dataType = GL_UNSIGNED_BYTE;

            img->setFileName(filepath.c_str());
            img->setImage(  res.width, res.height, 1,
                            3, // Why this value are here?
                            pixelFormat,
                            dataType,
                            (unsigned char*) rgb,
                            osg::Image::USE_NEW_DELETE);
        }
        else
        {
            int internalFormat;
            int pixelFormat;
            int dataType = GL_FLOAT;

            if (rawRGBE)
            {
                internalFormat = GL_RGBA8;
                pixelFormat = GL_RGBA;
            } else {
                internalFormat = GL_RGB32F_ARB;
                pixelFormat = GL_RGB;
            }

            img->setFileName(filepath.c_str());
            img->setImage(  res.width, res.height, 1,
                            internalFormat,
                            pixelFormat,
                            dataType,
                            (unsigned char*) res.cols,
                            osg::Image::USE_NEW_DELETE);
        }

        // Y flip
        if(bYFlip==true) img->flipVertical();

        return img;
    }


    // Additional write methods
    virtual WriteResult writeImage(const osg::Image &image,const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getFileExtension(file);
        if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

        osgDB::ofstream fout(file.c_str(), std::ios::out | std::ios::binary);
        if(!fout) return WriteResult::ERROR_IN_WRITING_FILE;

        return writeImage(image,fout,options);
    }

    virtual WriteResult writeImage(const osg::Image& image,std::ostream& fout,const Options* options) const
    {

        bool bYFlip = true;        // Whether to flip the vertical
        bool rawRGBE = false;    // Whether to write as raw RGBE

        if(options)
        {
            std::istringstream iss(options->getOptionString());
            std::string opt;
            while (iss >> opt)
            {
                if (opt=="NO_YFLIP")
                {
                    // We want to YFLIP because although the file format specification
                    // dictates that +Y M +X N is a valid resolution line, no software (including
                    // HDRShop!) actually recognises it; hence everything tends to be written upside down
                    // So we flip the image first...
                    bYFlip = false;
                }
                else if(opt=="RAW")
                {
                    rawRGBE = true;
                }
                /* The following are left out for the moment as
                   we don't really do anything with them in OSG
                else if(opt=="GAMMA")
                {
                   iss >> gamma;
                }
                else if(opt=="EXPOSURE")
                {
                    iss >> exposure;
                }
                */

            }
        }

        // Reject unhandled image formats
        if(rawRGBE==false)
        {
            if(image.getInternalTextureFormat()!=GL_RGB32F_ARB)    // We only handle RGB (no alpha) with 32F formats
            {
                return WriteResult::FILE_NOT_HANDLED;
            }
        }
        else    // Outputting raw RGBE (as interpreted by a shader, for example)
        {
            if(image.getInternalTextureFormat()!=GL_RGBA8) // We need 8 bit bytes including alpha (E)
            {
                return WriteResult::FILE_NOT_HANDLED;
            }
        }

        // Get a temporary copy to flip if we need to
        osg::ref_ptr<osg::Image> source = new osg::Image(image,osg::CopyOp::DEEP_COPY_ALL);

        if(bYFlip==true) source->flipVertical();

        bool success;
        success = HDRWriter::writeHeader(source.get(),fout);
        if(!success)
        {
            source = 0;    // delete the temporary image
            return WriteResult::ERROR_IN_WRITING_FILE;    // early out
        }

        success = HDRWriter::writeRLE(source.get(), fout);

        source = 0; // delete the temporary image
        return (success)? WriteResult::FILE_SAVED : WriteResult::ERROR_IN_WRITING_FILE;
    }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(hdr, ReaderWriterHDR)
