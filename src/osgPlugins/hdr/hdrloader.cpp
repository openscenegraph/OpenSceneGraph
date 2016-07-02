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

#include "hdrloader.h"

#include <math.h>
#include <memory.h>
#include <stdio.h>

#include <osgDB/FileUtils>

typedef unsigned char RGBE[4];
#define R            0
#define G            1
#define B            2
#define E            3

#define  MINELEN    8                // minimum scanline length for encoding
#define  MAXELEN    0x7fff            // maximum scanline length for encoding

static void rawRGBEData(RGBE *scan, int len, float *cols);
static void workOnRGBE(RGBE *scan, int len, float *cols);
static bool decrunch(RGBE *scanline, int len, FILE *file);
static bool oldDecrunch(RGBE *scanline, int len, FILE *file);


inline char read_char(FILE* stream, int& error)
{
    if (error) return 0;

    int value = fgetc(stream);
    if (value>=0 && value<128)
    {
        return static_cast<char>(value);
    }
    else
    {
        error = value;
        return 0;
    }
}

inline unsigned char read_unsigned_char(FILE* stream, int& error)
{
    if (error) return 0;

    int value = fgetc(stream);
    if (value>=0 && value<256)
    {
        return static_cast<unsigned char>(value);
    }
    else
    {
        error = value;
        return 0;
    }
}

bool HDRLoader::isHDRFile(const char *_fileName)
{
    FILE *file;
    file = osgDB::fopen(_fileName, "rb");
    if (!file)
        return false;

    char str[10];
    size_t numRead = fread(str, 10, 1, file);

    fclose(file);

    if (numRead<1) return false;

    if (memcmp(str, "#?RADIANCE", 10) && memcmp(str, "#?RGBE", 6))
        return false;

    return true;
}

bool HDRLoader::load(const char *_fileName, const bool _rawRGBE, HDRLoaderResult &_res)
{
    int i;
    char str[200];
    FILE *file;

    file = osgDB::fopen(_fileName, "rb");
    if (!file)
        return false;

    size_t numRead = fread(str, 10, 1, file);

    if (numRead<1)
    {
        fclose(file);
        return false;
    }

    if (memcmp(str, "#?RADIANCE", 10))
    {
        if (fseek(file, 0, SEEK_SET)!=0)
        {
            fclose(file);
            return false;
        }

        numRead = fread(str, 6, 1, file);
        if (numRead<1 || memcmp(str, "#?RGBE", 6))
        {
            fclose(file);
            return false;
        }
    }
    if (fseek(file, 1, SEEK_CUR)!=0)
    {
        fclose(file);
        return false;
    }

    //char cmd[2000];
    i = 0;
    char c = 0, oldc;
    int error = 0;
    while(!error) {
        oldc = c;
        c = read_char(file, error);
        if (c == 0xa && oldc == 0xa)
            break;
        //cmd[i++] = c;
    }

    char reso[2000];
    i = 0;
    while(!error) {
        c = read_char(file, error);
        reso[i++] = c;
        if (c == 0xa)
            break;
    }

    int w, h;
    if (!sscanf(reso, "-Y %d +X %d", &h, &w)) {
        fclose(file);
        return false;
    }

    _res.width = w;
    _res.height = h;

    int components = _rawRGBE ? 4 : 3;
    float *cols = new float[w * h * components];
    _res.cols = cols;

    RGBE *scanline = new RGBE[w];
    if (!scanline) {
        fclose(file);
        return false;
    }

    // convert image
    cols += (h-1) * w * components;
    for (int y = h - 1; y >= 0; y--) {
        if (decrunch(scanline, w, file) == false)
            break;
        if (_rawRGBE)
            rawRGBEData(scanline, w, cols);
        else
            workOnRGBE(scanline, w, cols);
        cols -= w * components;
    }

    delete [] scanline;
    fclose(file);

    return true;
}

void rawRGBEData(RGBE *_scan, int _len, float *_cols)
{
    int ii = 0;
    while (_len-- > 0) {
        _cols[0] = _scan[0][R] / 255.0f;
        _cols[1] = _scan[0][G] / 255.0f;
        _cols[2] = _scan[0][B] / 255.0f;
        _cols[3] = _scan[0][E] / 255.0f;
        _cols += 4;
        _scan++;
        ii++;
    }
}

inline float convertComponent(int _expo, int _val)
{
    return static_cast<float>(ldexp( static_cast<float>(_val), _expo-8));
}

void workOnRGBE(RGBE *_scan, int _len, float *_cols)
{
    int ii = 0;
    while (_len-- > 0) {
        int expo = _scan[0][E] - 128;
        _cols[0] = convertComponent(expo, _scan[0][R]);
        _cols[1] = convertComponent(expo, _scan[0][G]);
        _cols[2] = convertComponent(expo, _scan[0][B]);
        _cols += 3;
        _scan++;
        ii++;
    }
}

bool decrunch(RGBE *_scanline, int _len, FILE *_file)
{
    int  i, j;

    if (_len < MINELEN || _len > MAXELEN)
        return oldDecrunch(_scanline, _len, _file);

    i = fgetc(_file);
    if (i != 2) {
        if (fseek(_file, -1, SEEK_CUR)!=0)
        {
            return false;
        }
        return oldDecrunch(_scanline, _len, _file);
    }

    int error = 0;

    _scanline[0][G] = read_unsigned_char(_file, error);
    _scanline[0][B] = read_unsigned_char(_file, error);
    i = fgetc(_file);

    if (_scanline[0][G] != 2 || _scanline[0][B] & 128) {
        _scanline[0][R] = 2;
        _scanline[0][E] = i;
        return oldDecrunch(_scanline + 1, _len - 1, _file);
    }

    // read each component
    for (i = 0; i < 4; i++) {
        for (j = 0; j < _len; ) {
            unsigned char code = read_unsigned_char(_file, error);
            if (code > 128) { // run
                code &= 127;
                unsigned char val = read_unsigned_char(_file, error);
                while (code--)
                    _scanline[j++][i] = val;
            }
            else  {    // non-run
                while(code--)
                    _scanline[j++][i] = read_unsigned_char(_file, error);
            }
        }
    }

    return feof(_file) ? false : true;
}

bool oldDecrunch(RGBE *_scanline, int _len, FILE *_file)
{
    int i;
    int rshift = 0;

    int error = 0;

    while (_len > 0 && !error) {
        _scanline[0][R] = read_unsigned_char(_file, error);
        _scanline[0][G] = read_unsigned_char(_file, error);
        _scanline[0][B] = read_unsigned_char(_file, error);
        _scanline[0][E] = read_unsigned_char(_file, error);
        if (feof(_file))
            return false;

        if (_scanline[0][R] == 1 &&
            _scanline[0][G] == 1 &&
            _scanline[0][B] == 1) {
                for (i = _scanline[0][E] << rshift; i > 0; i--) {
                    memcpy(&_scanline[0][0], &_scanline[-1][0], 4);
                    _scanline++;
                    _len--;
                }
                rshift += 8;
            }
        else {
            _scanline++;
            _len--;
            rshift = 0;
        }
    }
    return true;
}
