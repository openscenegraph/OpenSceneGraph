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

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>

#include "hdrloader.h"

class ReaderWriterHDR : public osgDB::ReaderWriter
{
public:
    virtual const char* className() { return "HDR Image Reader"; }
    virtual bool acceptsExtension(const std::string &extension) { return osgDB::equalCaseInsensitive(extension, "hdr"); }

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
                    bYFlip = true; // TODO
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
                // In a perfect world it should be GL_RGB32F_ARB,
                // but by default it works fine as GL_RGB8. You
                // can set it as GL_RGB32F_ARB in the method
                // setInternalFormat of osg::Texture{xD, Rectangle
                // and CubeMap} if your GPU support it.
                internalFormat = GL_RGB8;
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

        return img;
    }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterHDR> g_readerWriter_HDR_Proxy;
