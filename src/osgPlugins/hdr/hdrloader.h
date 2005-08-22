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
* Description:
* Class to load a HDR file
* An HDR file is an image file where each pixel is stored as float values in opposite
* of traditional other formats (BMP, TGA, etc.) where integerer values are used.
* see http://www.flipcode.com/cgi-bin/msg.cgi?showThread=COTD-HDRImageReader&forum=cotd&id=-1 for more informations
*/

#ifndef HDRLOADER_H
#define HDRLOADER_H

class HDRLoaderResult {
public:
    int width, height;
    // each pixel takes 3 float32, each component can be of any value...
    float *cols; // this is to be freed using free() and not delete[]
};

class HDRLoader {
public:
    static bool isHDRFile(const char *fileName);
    static bool load(const char *fileName, const bool rawRGBE, HDRLoaderResult &res);
};

#endif
