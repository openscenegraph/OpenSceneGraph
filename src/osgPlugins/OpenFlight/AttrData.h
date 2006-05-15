//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#ifndef FLT_ATTRDATA_H
#define FLT_ATTRDATA_H

#include <string>
#include <osg/Object>
#include "types.h"

namespace flt {


class AttrData : public osg::Object
{
    public :

        AttrData();
        
        AttrData(const AttrData& attr, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(flt,AttrData);


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
            WRAP_CLAMP = 1,
            WRAP_MIRRORED_REPEAT = 2
        };

        enum TexEnvMode {
            TEXENV_MODULATE = 0,
            TEXENV_BLEND = 1,
            TEXENV_DECAL = 2,
            TEXENV_COLOR = 3,
            TEXENV_ADD = 4
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
                                        //    4 - TV_ADD
        int32   intensityAsAlpha;       // TRUE if intensity pattern to be loaded in alpha with white in color
//      int32   spare1[8];              // 8 words of spare
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
        float32 _mips[8];                // 8 floats for kernel of separable symmetric filter
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
//      float32 reserved1;              // Reserved
//      float32 reserved2[8];           // Reserved
        float64 lambertMeridian;        // Lambert conic projection central meridian
        float64 lambertUpperLat;        // Lambert conic projection upper latitude
        float64 lambertlowerLat;        // Lambert conic projection lower latitude
//      float64 reserved3;              // Reserved
//      float32 spare2[5];              // Spare
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
//      int32   reserved4;              // Reserved
        int32   utmZone;                // UTM zone
        int32   imageOrigin;            // Image origin
                                        //    0 = Lower-left
                                        //    1 = Upper-left
        int32   geoUnits;               // Geospecific points units
                                        //    0 = Degrees
                                        //    1 = Meters
                                        //    2 = Pixels
//      int32   reserved5;              // Reserved
//      int32   reserved6;              // Reserved
        int32   hemisphere;             // Hemisphere for geospecific points units
                                        //    0 = Southern
                                        //    1 = Northern
//      int32   reserved7;              // Reserved
//      int32   reserved8;              // Reserved
//      int32   spare3[149];            // Spare
//      char    comments[512];          // Comments
        std::string comments;
        // --------------
        // v12 ends here
        // --------------

//      int32   reserved9[13];          // Reserved
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

};


} // end namespace

#endif


