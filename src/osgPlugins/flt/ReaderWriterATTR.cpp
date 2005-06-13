
//
// OpenFlight® texture attribute loader for Open Scene Graph
//
//  Copyright (C) 2001  Brede Johansen
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.
//
//  The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for 
//  real-time rendering of large 3D photo-realistic models. 
//  The OSG homepage is http://www.openscenegraph.org/
//
//  MultiGen, OpenFlight, and Flight Format are registered trademarks of MultiGen Inc.
//

#include <stdio.h>
#include <string.h>

#include <osg/Notify>
#include <osg/TexEnv>
#include <osg/Texture2D>
#include <osg/StateSet>
#include <osg/GL>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include <iostream>
#include <fstream>
#include <sstream>

#include "AttrData.h"

typedef signed char     int8;
typedef unsigned char   uint8;
typedef signed short    int16;
typedef unsigned short  uint16;
typedef signed int      int32;
typedef unsigned int    uint32;
typedef float           float32;
typedef double          float64;

#define READ(DST) readField(file, (void*)&(DST), sizeof(DST))


static int isLittleEndianMachine()
{
    int a = 1;
    return (int)(*(char*)&a);
}


static void endian2(void* pSrc, int nSrc, void* pDst)
{
    if (nSrc == 2)
    {
        short tmp1;
        tmp1 = *(short *)pSrc;
        tmp1 = (tmp1 << 8) | ((tmp1 >> 8) & 0xff);
        *(short *)pDst = tmp1;
    }
    else if (nSrc == 4)
    {
        uint32 tmp1;
        tmp1 = *(uint32 *)pSrc;
        tmp1 = (tmp1 << 24) | ((tmp1 << 8) & 0xff0000) | ((tmp1 >> 8) & 0xff00) | ((tmp1 >> 24) & 0xff);
        *(uint32 *)pDst = tmp1;
    }
    else if (nSrc == 8)
    {
        uint32 tmp1, tmp2;
        tmp1 = *(uint32 *)pSrc;
        tmp2 = *(1 + (uint32 *)pSrc);
        tmp1 = (tmp1 << 24) | ((tmp1 << 8) & 0xff0000) | ((tmp1 >> 8) & 0xff00) | ((tmp1 >> 24) & 0xff);
        tmp2 = (tmp2 << 24) | ((tmp2 << 8) & 0xff0000) | ((tmp2 >> 8) & 0xff00) | ((tmp2 >> 24) & 0xff);
        *(uint32 *)pDst = tmp2;
        *(1 + (uint32 *)pDst) = tmp1;
    }
}


using namespace osg;

class Attr
{
    public :

        enum MinFilterMode {
            MIN_FILTER_POINT = 0,
            MIN_FILTER_BILINEAR = 1,
            MIN_FILTER_MIPMAP = 2,                      // (Obsolete)
            MIN_FILTER_MIPMAP_POINT = 3,
            MIN_FILTER_MIPMAP_LINEAR = 4,
            MIN_FILTER_MIPMAP_BILINEAR = 5,
            MIN_FILTER_MIPMAP_TRILINEAR = 6,
            MIN_FILTER_NONE = 7,
            MIN_FILTER_BICUBIC = 8,
            MIN_FILTER_BILINEAR_GEQUAL = 9,
            MIN_FILTER_BILINEAR_LEQUAL = 10,
            MIN_FILTER_BICUBIC_GEQUAL = 11,
            MIN_FILTER_BICUBIC_LEQUAL = 12
        };

        enum MagFilterMode {
            MAG_FILTER_POINT = 0,
            MAG_FILTER_BILINEAR = 1,
            MAG_FILTER_NONE = 2,
            MAG_FILTER_BICUBIC = 3,
            MAG_FILTER_SHARPEN = 4,
            MAG_FILTER_ADD_DETAIL = 5,
            MAG_FILTER_MODULATE_DETAIL = 6,
            MAG_FILTER_BILINEAR_GEQUAL = 7,
            MAG_FILTER_BILINEAR_LEQUAL = 8,
            MAG_FILTER_BICUBIC_GEQUAL = 9,
            MAG_FILTER_BICUBIC_LEQUAL = 10
        };

        enum WrapMode {
            WRAP_REPEAT = 0,
            WRAP_CLAMP = 1
        };

        enum TexEnvMode {
            TEXENV_MODULATE = 0,
            TEXENV_BLEND = 1,
            TEXENV_DECAL = 2,
            TEXENV_COLOR = 3
        };

        enum Projection {
            PROJECTION_FLAT = 0,
            PROJECTION_LAMBERT_CONIC = 3,
            PROJECTION_UTM = 4,
            PROJECTION_UNDEFINED = 7
        };

        enum Datum {
            DATUM_WGS84 = 0,
            DATUM_WGS72 = 1,
            DATUM_BESSEL = 2,
            DATUM_CLARK_1866 = 3,
            DATUM_NAD27 = 4
        };


        Attr(int version) : _flt_version(version) { init(); }
        void    init();
        void    readField(std::ifstream& file, void* buf, size_t size);
        bool    readAttrFile(const char* szName);
        flt::AttrData* createOsgStateSet();


    
        int32   texels_u;               // Number of texels in u direction
        int32   textel_v;               // Number of texels in v direction
        int32   direction_u;            // Real world size u direction
        int32   direction_v;            // Real world size v direction
        int32   x_up;                   // x component of up vector
        int32   y_up;                   // y component of up vector
        int32   fileFormat;             // File format type
                                        //   -1 Not used
                                        //    0 AT&T image 8 pattern
                                        //    1 AT&T image 8 template
                                        //    2 SGI intensity modulation
                                        //    3 SGI intensity w/ alpha
                                        //    4 SGI RGB
                                        //    5 SGI RGB w/ alpha
        int32   minFilterMode;          // Minification filter type
                                        //    0 - TX_POINT
                                        //    1 - TX_BILINEAR
                                        //    2 - TX_MIPMAP (Obsolete)
                                        //    3 - TX_MIPMAP_POINT
                                        //    4 - TX_MIPMAP_LINEAR
                                        //    5 - TX_MIPMAP_BILINEAR
                                        //    6 - TX_MIPMAP_TRILINEAR
                                        //    7 - None
                                        //    8 - TX_BICUBIC
                                        //    9 - TX_BILINEAR_GEQUAL
                                        //   10 - TX_BILINEAR_LEQUAL
                                        //   11 - TX_BICUBIC_GEQUAL
                                        //   12 - TX_BICUBIC_LEQUAL
        int32   magFilterMode;          // Magnification filter type
                                        //    0 - TX_POINT
                                        //    1 - TX_BILINEAR
                                        //    2 - None
                                        //    3 - TX_BICUBIC
                                        //    4 - TX_SHARPEN
                                        //    5 - TX_ADD_DETAIL
                                        //    6 - TX_MODULATE_DETAIL
                                        //    7 - TX_BILINEAR_GEQUAL
                                        //    8 - TX_BILINEAR_LEQUAL
                                        //    9 - TX_BICUBIC_GEQUAL
                                        //   10 - TX_BICUBIC_LEQUAL
        int32   wrapMode;               // Repetition type
                                        //    0 - TX_REPEAT
                                        //    1 - TX_CLAMP
                                        //    2 - (Obsolete)
        int32   wrapMode_u;             // Repetition type in u direction (see above)
        int32   wrapMode_v;             // Repetition type in v direction (see above)
        int32   modifyFlag;             // Modify flag (for internal use)
        int32   pivot_x;                // x pivot point for rotating textures
        int32   pivot_y;                // y pivot point for rotating textures

        // --------------
        // v11 ends here
        // --------------

        int32   texEnvMode;             // Environment type
                                        //    0 - TV_MODULATE
                                        //    1 - TV_BLEND
                                        //    2 - TV_DECAL
                                        //    3 - TV_COLOR
        int32   intensityAsAlpha;       // TRUE if intensity pattern to be loaded in alpha with white in color
        int32   spare1[8];              // 8 words of spare
        float64 size_u;                 // Real world size u for floating point databases
        float64 size_v;                 // Real world size v for floating point databases
        int32   originCode;             // Code for origin of imported texture
        int32   kernelVersion;          // Kernel version number
        int32   intFormat;              // Internal format type
                                        //    0 - Default
                                        //    1 - TX_I_12A_4
                                        //    2 - TX_IA_8
                                        //    3 - TX_RGB_5
                                        //    4 - TX_RGBA_4
                                        //    5 - TX_IA_12
                                        //    6 - TX_RGBA_8
                                        //    7 - TX_RGBA_12
                                        //    8 - TX_I_16 (shadow mode only)
                                        //    9 - TX_RGB_12
        int32   extFormat;              // External format type
                                        //    0 - Default
                                        //    1 - TX_PACK_8
                                        //    2 - TX_PACK_16
        int32   useMips;                // TRUE if using following 8 floats for MIPMAP kernel
        float32 _mipMapKernel[8];                // 8 floats for kernel of separable symmetric filter
        int32   useLodScale;            // Boolean if TRUE send:
        float32 lod0;                   // LOD0 for TX_CONTROL_POINT
        float32 scale0;                 // SCALE0 for TX_CONTROL_POINT
        float32 lod1;                   // LOD1 for TX_CONTROL_POINT
        float32 scale1;                 // SCALE1 for TX_CONTROL_POINT
        float32 lod2;                   // LOD2 for TX_CONTROL_POINT
        float32 scale2;                 // SCALE2 for TX_CONTROL_POINT
        float32 lod3;                   // LOD3 for TX_CONTROL_POINT
        float32 scale3;                 // SCALE3 for TX_CONTROL_POINT
        float32 lod4;                   // LOD4 for TX_CONTROL_POINT
        float32 scale4;                 // SCALE4 for TX_CONTROL_POINT
        float32 lod5;                   // LOD5 for TX_CONTROL_POINT
        float32 scale5;                 // SCALE5 for TX_CONTROL_POINT
        float32 lod6;                   // LOD6 for TX_CONTROL_POINT
        float32 scale6;                 // SCALE6 for TX_CONTROL_POINT
        float32 lod7;                   // LOD7 for TX_CONTROL_POINT
        float32 scale7;                 // SCALE7 for TX_CONTROL_POINT

        float32 clamp;                  // Clamp
        int32   magFilterAlpha;         // magfilteralpha:
                                        //    0 = TX_POINT
                                        //    1 = TX_BILINEAR
                                        //    2 = None
                                        //    3 = TX_BICUBIC
                                        //    4 = TX_SHARPEN
                                        //    5 = TX_ADD_DETAIL
                                        //    6 = TX_MODULATE_DETAIL
                                        //    7 = TX_BILINEAR_GEQUAL
                                        //    8 = TX_BILINEAR_LEQUAL
                                        //    9 = TX_BICUBIC_GEQUAL
                                        //    10 = TX_BIBICUBIC_LEQUAL
        int32   magFilterColor;         // magfiltercolor:
                                        //    0 = TX_POINT
                                        //    1 = TX_BILINEAR
                                        //    2 = None
                                        //    3 = TX_BICUBIC
                                        //    4 = TX_SHARPEN
                                        //    5 = TX_ADD_DETAIL
                                        //    6 = TX_MODULATE_DETAIL
                                        //    7 = TX_BILINEAR_GEQUAL
                                        //    8 = TX_BILINEAR_LEQUAL
                                        //    9 = TX_BICUBIC_GEQUAL
                                        //   10 = TX_BIBICUBIC_LEQUAL
        float32 reserved1;              // Reserved
        float32 reserved2[8];           // Reserved
        float64 lambertMeridian;        // Lambert conic projection central meridian
        float64 lambertUpperLat;        // Lambert conic projection upper latitude
        float64 lambertlowerLat;        // Lambert conic projection lower latitude
        float64 reserved3;              // Reserved
        float32 spare2[5];              // Spare
        int32   useDetail;              // TRUE if using next 5 integers for detail texture
        int32   txDetail_j;             // J argument for TX_DETAIL
        int32   txDetail_k;             // K argument for TX_DETAIL
        int32   txDetail_m;             // M argument for TX_DETAIL
        int32   txDetail_n;             // N argument for TX_DETAIL
        int32   txDetail_s;             // Scramble argument for TX_DETAIL
        int32   useTile;                // TRUE if using next for floats for TX_TILE
        float32 txTile_ll_u;            // Lower-left u value for TX_TILE
        float32 txTile_ll_v;            // Lower-left v value for TX_TILE
        float32 txTile_ur_u;            // Upper-right u value for TX_TILE
        float32 txTile_ur_v;            // Upper-right v value for TX_TILE
        int32   projection;             // Projection
                                        //    0 = Flat earth
                                        //    3 = Lambert conic
                                        //    4 = UTM
                                        //    7 = Undefined projection
        int32   earthModel;             // Earth model
                                        //    0 = WGS84
                                        //    1 = WGS72
                                        //    2 = Bessel
                                        //    3 = Clark 1866
                                        //    4 = NAD27
        int32   reserved4;              // Reserved
        int32   utmZone;                // UTM zone
        int32   imageOrigin;            // Image origin
                                        //    0 = Lower-left
                                        //    1 = Upper-left
        int32   geoUnits;               // Geospecific points units
                                        //    0 = Degrees
                                        //    1 = Meters
                                        //    2 = Pixels
        int32   reserved5;              // Reserved
        int32   reserved6;              // Reserved
        int32   hemisphere;             // Hemisphere for geospecific points units
                                        //    0 = Southern
                                        //    1 = Northern
        int32   reserved7;              // Reserved
        int32   reserved8;              // Reserved
        int32   spare3[149];            // Spare
        char    comments[512];          // Comments

        // --------------
        // v12 ends here
        // --------------

        int32   reserved9[13];          // Reserved
        int32   attrVersion;            // Attribute file version number

        int32   controlPoints;          // Number of geospecific control points
        // If the number of geospecific control points is > 0,
        // the following fields are also in the attribute file:
        int32   reserved10;             // Reserved
    #if 0
        // For each geospecific control point:
        {
            float64 texel_u;            // Texel u of geospecific control point
            float64 texel_v;            // Texel v of geospecific control point
            float64 geoPoint[2];        // Real earth coordinate of geospecific control point
                                        // (this value depends on the projection, earth model,
                                        // and geospecific points units)
        }                               

        // ----------------
        // v15.6 ends here
        // ----------------

        // After all geospecific control points are listed, the following subtexture
        // information appears:
        int32   subtextures;            // Number of subtexture definitions contained in the
                                        // texture attribute file
        // If the number of subtexture definitions is >0,
        // the following fields are repeated for each subtexture definition:
        {
            char    name[32];           // name of subtexture definition
            int32   left;               // Coordinate of left edge of subtexture
                                        // definition measured in texels.
            int32   bottom;             // Coordinate of bottom edge of subtexture
                                        // definition measured in texels.
            int32   right;              // Coordinate of right edge of subtexture
                                        // definition measured in texels.
            int32   top;                // Coordinate of top edge of subtexture
                                        // definition measured in texels.
        }
    #endif

        void read();

    private :

        int _flt_version;
};



void Attr::init()
{
    texels_u = 0;
    textel_v = 0;
    direction_u = 0;
    direction_v = 0;
    x_up = 0;
    y_up = 0;
    fileFormat = -1;                //   -1 Not used
    minFilterMode = MIN_FILTER_NONE;
    magFilterMode = MAG_FILTER_POINT;
    wrapMode = WRAP_REPEAT;
    wrapMode_u = WRAP_REPEAT;
    wrapMode_v = WRAP_REPEAT;
    modifyFlag = 0;
    pivot_x = 0;
    pivot_y = 0;
    texEnvMode = TEXENV_MODULATE;
    intensityAsAlpha = 0;
    size_u = 0;
    size_v = 0;
    originCode = 0;
    kernelVersion = 0;
    intFormat = 0;                  //    0 - Default
    extFormat = 0;                  //    0 - Default
    useMips = 0;
    // float32 _mipMapKernel[8];
    useLodScale = 0;
    // float32 lod0;
    // float32 scale0;
    // ...
    // float32 lod7;
    // float32 scale7;
    clamp = 0;
    magFilterAlpha = 2;             //    2 = None
    magFilterColor = 2;             //    2 = None
    lambertMeridian = 0;
    lambertUpperLat = 0;
    lambertlowerLat = 0;
    useDetail = 0;
    txDetail_j = 0;
    txDetail_k = 0;
    txDetail_m = 0;
    txDetail_n = 0;
    txDetail_s = 0;
    useTile = 0;
    txTile_ll_u = 0;
    txTile_ll_v = 0;
    txTile_ur_u = 0;
    txTile_ur_v = 0;
    projection = PROJECTION_UNDEFINED;
    earthModel = DATUM_WGS84;
    utmZone = 0;
    imageOrigin = 0;
    geoUnits = 0;
    hemisphere = 1;
    comments[0] = '\0';
    attrVersion = 0;
    controlPoints = 0;
    // TODO:
}                               


void Attr::readField(std::ifstream& file, void* buf, size_t size)
{
    if (file.eof()) return;
    file.read((char*)buf, size);
    if(::isLittleEndianMachine())
        ::endian2(buf, size, buf);
}


bool Attr::readAttrFile(const char* szName)
{
    int n;
    std::ifstream file;

    file.open (szName, std::ios::in | std::ios::binary);    

    READ( texels_u );
    READ( textel_v );
    READ( direction_u );
    READ( direction_v );
    READ( x_up );
    READ( y_up );
    READ( fileFormat );
    READ( minFilterMode );
    READ( magFilterMode );
    READ( wrapMode );
    READ( wrapMode_u );
    READ( wrapMode_v );
    READ( modifyFlag );
    READ( pivot_x );
    READ( pivot_y );

    // v11 ends here
    if (_flt_version <= 11) return true;

    READ( texEnvMode );
    READ( intensityAsAlpha );
    for (n=0; n<8; n++) {
        READ(spare1[n]); }
    READ( size_u );
    READ( size_v );
    READ( originCode );
    READ( kernelVersion );
    READ( intFormat );
    READ( extFormat );
    READ( useMips );
    for (n=0; n<8; n++) {
        READ(_mipMapKernel[n]); }
    READ( useLodScale );
    READ( lod0 );
    READ( scale0 );
    READ( lod1 );
    READ( scale1 );
    READ( lod2 );
    READ( scale2 );
    READ( lod3 );
    READ( scale3 );
    READ( lod4 );
    READ( scale4 );
    READ( lod5 );
    READ( scale5 );
    READ( lod6 );
    READ( scale6 );
    READ( lod7 );
    READ( scale7 );
    READ( clamp );
    READ( magFilterAlpha );
    READ( magFilterColor );
    READ( reserved1 );
    for (n=0; n<8; n++) {
        READ(reserved2[n]); }
    READ( lambertMeridian );
    READ( lambertUpperLat );
    READ( lambertlowerLat );
    READ( reserved3 );
    
    for (n=0; n<5; n++) {
        READ(spare2[n]); }

    // I don't know why I get a 4 bytes displacement before reading Detail Texture parameters
    // My ATTR files have been created with Creator 2.5.1
    // Julian Ortiz, June 18th 2003.    
    int32 dummyAjust;
    READ( dummyAjust);    

    READ( useDetail );    
    READ( txDetail_j );
    READ( txDetail_k );
    READ( txDetail_m );
    READ( txDetail_n );
    READ( txDetail_s );
    READ( useTile );
    READ( txTile_ll_u );
    READ( txTile_ll_v );
    READ( txTile_ur_u );
    READ( txTile_ur_v );
    READ( projection );
    READ( earthModel );
    READ( reserved4 );
    READ( utmZone );
    READ( imageOrigin);
    READ( geoUnits );
    READ( reserved5 );
    READ( reserved6 );
    READ( hemisphere );
    READ( reserved7 );
    READ( reserved8 );
    for (n=0; n<149; n++) {
        READ(spare3[n]); }
    file.read(comments, sizeof(comments));

    // v12 ends here
    if (_flt_version <= 12) return true;

    for (n=0; n<13; n++) {
        READ(reserved9[n]); }
    READ( attrVersion );
    READ( controlPoints);
    READ( reserved10 );

    file.close();
    return true;
}


flt::AttrData* Attr::createOsgStateSet()
{    
    TexEnv* osgTexEnv = new TexEnv;
    Texture2D* osgTexture = new Texture2D;
    flt::AttrData* attrdata = new flt::AttrData;    

    if ((wrapMode_u != WRAP_CLAMP) && ((wrapMode_u != WRAP_REPEAT)))
        wrapMode_u = wrapMode;
    if ((wrapMode_v != WRAP_CLAMP) && ((wrapMode_v != WRAP_REPEAT)))
        wrapMode_v = wrapMode;

    if (wrapMode_u == WRAP_CLAMP) osgTexture->setWrap(Texture2D::WRAP_S,Texture2D::CLAMP);
    else osgTexture->setWrap(Texture2D::WRAP_S,Texture2D::REPEAT);

    if (wrapMode_v == WRAP_CLAMP) osgTexture->setWrap(Texture2D::WRAP_T,Texture2D::CLAMP);
    else osgTexture->setWrap(Texture2D::WRAP_T,Texture2D::REPEAT);


    switch (texEnvMode)
    {
    case TEXENV_MODULATE:
        osgTexEnv->setMode(TexEnv::MODULATE);
        break;
    case TEXENV_BLEND:
        osgTexEnv->setMode(TexEnv::BLEND);
        break;
    case TEXENV_DECAL:
        osgTexEnv->setMode(TexEnv::DECAL);
        break;
    case TEXENV_COLOR:
        osgTexEnv->setMode(TexEnv::REPLACE);
        break;
    }

/*  An email on ATTR mappings to OpenGL : "[osg-news] OpenFlight Texture Filter Modes"
    from Joseph Steel:
    

    I posted a query on a forum on the Multigen web site re. the OpenFlight
    texture filer modes. This is the reply:

    Here are the mappings used by Creator under OpenGL: 
    For Minification:
    Point (0): GL_NEAREST
    Bilinear (1): GL_LINEAR
    Mipmap Point (3): GL_NEAREST_MIPMAP_NEAREST
    Mipmap Linear (4): GL_NEAREST_MIPMAP_LINEAR
    Mipmap Bilinear (5): GL_LINEAR_MIPMAP_NEAREST
    Mipmap Trilinear (6): GL_LINEAR_MIPMAP_LINEAR
    Bicubic (8): GL_LINEAR_MIPMAP_NEAREST
    Bilinear Greater/Equal (9): GL_LINEAR_MIPMAP_NEAREST
    Bilinear Less/Equal (10): GL_LINEAR_MIPMAP_NEAREST
    Bicubic Greater/Equal (11): GL_LINEAR_MIPMAP_NEAREST
    Bicubic Less/Equal (12): GL_LINEAR_MIPMAP_NEAREST 
    For Magnification:
    Point (0): GL_NEAREST
    Bilinear (1): GL_LINEAR
    Bicubic (3): GL_LINEAR
    Sharpen (4): GL_LINEAR
    Add Detail (5): GL_LINEAR
    Modulate Detail (6): GL_LINEAR
    Bilinear Greater/Equal (7): GL_LINEAR
    Bilinear Less/Equal (8): GL_LINEAR
    Bicubic Greater/Equal (9): GL_LINEAR
    Bicubic Less/Equal (10): GL_LINEAR

    Note that mode 2 for both minification & magnification are no longer
    supported.
*/

    // -----------
    // Min filter
    // -----------

    switch (minFilterMode)
    {
    case MIN_FILTER_POINT:
        osgTexture->setFilter(osg::Texture2D::MIN_FILTER, Texture2D::NEAREST);
        break;
    case MIN_FILTER_BILINEAR:
        osgTexture->setFilter(osg::Texture2D::MIN_FILTER, Texture2D::LINEAR);
        break;
    case MIN_FILTER_MIPMAP_POINT:
        osgTexture->setFilter(osg::Texture2D::MIN_FILTER, Texture2D::NEAREST_MIPMAP_NEAREST);
        break;
    case MIN_FILTER_MIPMAP_LINEAR:
        osgTexture->setFilter(osg::Texture2D::MIN_FILTER, Texture2D::NEAREST_MIPMAP_LINEAR);
        break;
    case MIN_FILTER_MIPMAP_BILINEAR:
        osgTexture->setFilter(osg::Texture2D::MIN_FILTER, Texture2D::LINEAR_MIPMAP_NEAREST);
        break;
    case MIN_FILTER_MIPMAP_TRILINEAR:
        osgTexture->setFilter(osg::Texture2D::MIN_FILTER, Texture2D::LINEAR_MIPMAP_LINEAR);
        break;
    case MIN_FILTER_BICUBIC:
    case MIN_FILTER_BILINEAR_GEQUAL:
    case MIN_FILTER_BILINEAR_LEQUAL:
    case MIN_FILTER_BICUBIC_GEQUAL:
    case MIN_FILTER_BICUBIC_LEQUAL:
        osgTexture->setFilter(osg::Texture2D::MIN_FILTER, Texture2D::LINEAR_MIPMAP_NEAREST);
        break;
    default:
        osgTexture->setFilter(osg::Texture2D::MIN_FILTER, Texture2D::LINEAR_MIPMAP_LINEAR);
        break;
          break;
    }


    // -----------
    // Mag filter
    // -----------

    switch (magFilterMode)
    {
    case MAG_FILTER_POINT:
        osgTexture->setFilter(osg::Texture2D::MAG_FILTER, Texture2D::NEAREST);
        break;
    case MAG_FILTER_BILINEAR:
    case MAG_FILTER_BILINEAR_GEQUAL:
    case MAG_FILTER_BILINEAR_LEQUAL:
    case MAG_FILTER_SHARPEN:
    case MAG_FILTER_BICUBIC:
    case MAG_FILTER_BICUBIC_GEQUAL:
    case MAG_FILTER_BICUBIC_LEQUAL:
    case MAG_FILTER_ADD_DETAIL:
    case MAG_FILTER_MODULATE_DETAIL:
        osgTexture->setFilter(osg::Texture2D::MAG_FILTER, Texture2D::LINEAR);
        break;
    }

    // I have just ported the two below set*Attribute lines to use the new
    // texture attribute methods, however this tieing to the texture unit 0
    // is probably inappropriate.  Perhaps it would be better to create a 
    // StateSet to store the texture an modes, it is probably best
    // to use an intermediate data structure for the flt loader to use to
    // encapsulate ATTR files. Need to speak to Brede about this issue.
    // Robert Osfield, July 9th 2002.
    
    // This is now a bit diferrent. We create a new AttrData object, and StateSet object
    // is instead with attribute information needed to setup detail texutre
    //
    // Julian Ortiz, June 18th 2003.    
    attrdata->stateset = new StateSet;
    attrdata->stateset->setTextureAttribute( 0, osgTexEnv );
    attrdata->stateset->setTextureAttributeAndModes( 0, osgTexture, StateAttribute::ON );
    attrdata->useDetail    = useDetail;
    attrdata->txDetail_j  = txDetail_j;
    attrdata->txDetail_k  = txDetail_k;
    attrdata->txDetail_m  = txDetail_m;
    attrdata->txDetail_n  = txDetail_n;
    attrdata->txDetail_s  = txDetail_s;
    if (magFilterMode == MAG_FILTER_MODULATE_DETAIL)
     attrdata->modulateDetail = true;
    else
     attrdata->modulateDetail = false;
    
    return attrdata;
}


class ReaderWriterATTR : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() const { return "ATTR Image Attribute Reader/Writer"; }
        
        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension,"attr");
        }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            int version = 0;

            // Check the options string for the OpenFlight file version
            if (options)
            {
                // Get the character index of the FLT_VER option in the option
                // string
                std::string::size_type optionIndex = options->getOptionString().find("FLT_VER");

                // Default to zero for the version if it's not found
                if (optionIndex == std::string::npos)
                    version = 0;
                else
                {
                    // Copy the option string, starting with the F in FLT_VER
                    std::string fltVersionStr(options->getOptionString(), 
                        optionIndex);
                    std::string optionText;

                    // Read the version from the string
                    std::istringstream ins(fltVersionStr);
                    ins >> optionText >> version;
                }
            }

            Attr attr(version);

            if (!attr.readAttrFile(fileName.c_str()))
            {
                return "Unable to open \""+fileName+"\"";
            }

            //StateSet* stateset = attr.createOsgStateSet();
            // Julian Ortiz, June 18th 2003.        
            flt::AttrData* attrdata = attr.createOsgStateSet();

            notify(INFO) << "texture attribute read ok" << std::endl;
            return attrdata;
        }

};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterATTR> g_readerWriter_ATTR_Proxy;

