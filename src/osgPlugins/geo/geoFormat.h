/*===========================================================================*\

NAME:            geoFormat.h

DESCRIPTION:    Native Format struct definitions, tokens & functionc

AUTHOR:            Andy Bushnell

    -------------------------------------------------------------------------

PROPRIETARY RIGHTS NOTICE:   
    
  This software contains proprietary information and trade secrets of Carbon 
  Graphics LLC. No part or all of this software may be reproduced in any form, 
  without the written permission of Carbon Graphics LLC.

  Exception:
  This Software file can be used by third-party software developers (without
  using the Geo SDK libraries) for any purpose OTHER THAN loading Geo format 
  files into an application or executable (such as, though not limited to, 
  geometry Modelers & animation systems) which is primarily intended to allow for
  the CREATION or MODIFICATION of geometric or animation data. 
  
  Specifically,using this software (either all or part thereof) to aid in the 
  creation of a Geo format loader for a run-time system, game engine, toolkit 
  IG (Image Generation) System or any software where the PRIMARY purpose is
  real-time image playback and interactivity and not Model Creation and/or
  modification is permitted.

COPYRIGHT NOTICE: 
   
  Copyright © 1998-2001 Carbon Graphics Llc, ALL RIGHTS RESERVED

\*===========================================================================*/


#ifndef __GEO_FORMAT_H__
#define __GEO_FORMAT_H__

#include "geoCore.h"


#define GEO_1_0_RC2    1132
#define GEO_1_0_RC3    1133
#define GEO_1_0_RC4    1134    
#define GEO_1_0       1141

#define GEO_1_1_RC1 1231
#define GEO_1_1_RC2 1232
#define GEO_1_1        1240



                            // GEO_VERSION = ((GEO_LIB_MAJOR_VERSION*1000)+
                            //                (GEO_LIB_MINOR_VERSION*100) +
                            //                  (GEO_LIB_LEVEL_VERSION*10)  +
                            //                  (GEO_LIB_RELEASE_VERSION))




//
// Constants to define the Node disk records. Used in RecordToken.id
//


///////////////////////////////////////////////////////////////////////////////
//
// Record Ids
//
///////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------
// Geo 1.0 RC 2 & Below
//----------------------------------------------------------------

//const unsigned int DB_DSK_HEADER                        = 101;

//----------------------------------------------------------------
// Geo 1.0 RC 3 & Above
//----------------------------------------------------------------
const unsigned int DB_DSK_HEADER                        = 100;    // NOTE File MAGIC ID *CHANGED*

//----------------------------------------------------------------
// 
//----------------------------------------------------------------

const unsigned int DB_DSK_GROUP                            = 102;
const unsigned int DB_DSK_SEQUENCE                        = 104;
const unsigned int DB_DSK_LOD                            = 105;
const unsigned int DB_DSK_RENDERGROUP                    = 106;
const unsigned int DB_DSK_POLYGON                        = 107;
const unsigned int DB_DSK_MESH                            = 108;    // Unused - Possible Future expansion
const unsigned int DB_DSK_CUBE                            = 109;    // Unused - Possible Future expansion
const unsigned int DB_DSK_SPHERE                        = 110;    // Unused - Possible Future expansion
const unsigned int DB_DSK_CONE                            = 111;    // Unused - Possible Future expansion
const unsigned int DB_DSK_CYLINDER                        = 112;    // Unused - Possible Future expansion
const unsigned int DB_DSK_VERTEX                        = 113;
const unsigned int DB_DSK_PUSH                            = 114;
const unsigned int DB_DSK_POP                            = 115;
const unsigned int DB_DSK_TEXTURE                        = 116;
const unsigned int DB_DSK_MATERIAL                        = 117;
const unsigned int DB_DSK_VIEW                            = 118;
const unsigned int DB_DSK_EXTENSION_LIST                = 119;
const unsigned int DB_DSK_SWITCH                        = 120;
const unsigned int DB_DSK_TEXT                            = 121;
const unsigned int DB_DSK_BASE_GROUP                    = 122;
const unsigned int DB_DSK_BASE_SURFACE                    = 123;
const unsigned int DB_DSK_BEHAVIOR                        = 124;
const unsigned int DB_DSK_CLAMP_ACTION                    = 125;    
const unsigned int DB_DSK_RANGE_ACTION                    = 126;        
const unsigned int DB_DSK_ROTATE_ACTION                    = 127;        
const unsigned int DB_DSK_TRANSLATE_ACTION                = 128;    
const unsigned int DB_DSK_SCALE_ACTION                    = 129;                    
const unsigned int DB_DSK_ARITHMETIC_ACTION                = 130;            
const unsigned int DB_DSK_LOGIC_ACTION                    = 131;                
const unsigned int DB_DSK_CONDITIONAL_ACTION            = 132;        
const unsigned int DB_DSK_LOOPING_ACTION                = 133;                
const unsigned int DB_DSK_COMPARE_ACTION                = 134;           
const unsigned int DB_DSK_VISIBILITY_ACTION                = 135;               
const unsigned int DB_DSK_STRING_CONTENT_ACTION            = 136;               
const unsigned int DB_DSK_INTERNAL_VARS                    = 137;                     
const unsigned int DB_DSK_LOCAL_VARS                    = 138;                    
const unsigned int DB_DSK_EXTERNAL_VARS                    = 139;               
const unsigned int DB_DSK_FLOAT_VAR                        = 140;     
const unsigned int DB_DSK_INT_VAR                        = 141; 
const unsigned int DB_DSK_LONG_VAR                        = 142; 
const unsigned int DB_DSK_DOUBLE_VAR                    = 143;     
const unsigned int DB_DSK_BOOL_VAR                        = 144;
//    unsigned int DEPRICATED                            = 145;
const unsigned int DB_DSK_IF_CONDITION                    = 146;
const unsigned int DB_DSK_ELSE_CONDITION                = 147;
//    unsigned int DEPRICATED                            = 148;
const unsigned int DB_DSK_COLOR_PALETTE                    = 149;
const unsigned int DB_DSK_COLOR_RAMP_ACTION                = 150;
const unsigned int DB_DSK_FLOAT2_VAR                    = 151; 
const unsigned int DB_DSK_FLOAT3_VAR                    = 152;     
const unsigned int DB_DSK_FLOAT4_VAR                    = 153;     
const unsigned int DB_DSK_LINEAR_ACTION                    = 154;     
const unsigned int DB_DSK_TASK_ACTION                    = 155;     
const unsigned int DB_DSK_PERIODIC_ACTION                = 156; 
//    unsigned int DEPRICATED                            = 157;
const unsigned int DB_DSK_TRIG_ACTION                    = 158;  
const unsigned int DB_DSK_INVERSE_ACTION                = 159; 
const unsigned int DB_DSK_TRUNCATE_ACTION                = 160;
const unsigned int DB_DSK_ABS_ACTION                    = 161; 
const unsigned int DB_DSK_IF_THEN_ELSE_ACTION            = 162;     // simple variable value check
const unsigned int DB_DSK_DCS_ACTION                    = 163;     
const unsigned int DB_DSK_INSTANCE                        = 164;
const unsigned int DB_DSK_COORD_POOL                    = 165;
const unsigned int DB_DSK_LIGHTPT                        = 166;
const unsigned int DB_DSK_EXTERNAL                        = 167;
const unsigned int DB_DSK_NORMAL_POOL                    = 168;
const unsigned int DB_DSK_DISCRETE_ACTION                = 169;
const unsigned int DB_DSK_STRING_VAR                    = 170;            
const unsigned int DB_DSK_STRING_COPY_ACTION            = 171;
const unsigned int DB_DSK_PAGE                            = 172;
const unsigned int DB_DSK_SQRT_ACTION                    = 173;     
const unsigned int DB_DSK_LOG_ACTION                    = 174;     
const unsigned int DB_DSK_PLANE_TEXTURE_MAPPING_INFO    = 175;
const unsigned int DB_DSK_CYLINDER_TEXTURE_MAPPING_INFO    = 176;    // not implemented in 1.0
const unsigned int DB_DSK_SPHERE_TEXTURE_MAPPING_INFO    = 177;    // not implemented in 1.0
const unsigned int DB_DSK_GRID_TEXTURE_MAPPING_INFO        = 178;    // not implemented in 1.0
const unsigned int DB_DSK_PERSPECTIVE_GRID_INFO            = 179;
const unsigned int DB_DSK_XY_GRID_INFO                    = 180;    // not implemented in 1.0
const unsigned int DB_DSK_XZ_GRID_INFO                    = 181;    // not implemented in 1.0
const unsigned int DB_DSK_YZ_GRID_INFO                    = 182;    // not implemented in 1.0
const unsigned int DB_DSK_MULTI_TEX_SHADER                = 183;
const unsigned int DB_DSK_CULL_GROUP                    = 184;
const unsigned int DB_DSK_Z_OFFSET_GROUP                = 185;
const unsigned int DB_DSK_MULTI_SAMPLE_AA_GROUP            = 186;
const unsigned int DB_DSK_LINE_AA_GROUP                    = 187;
const unsigned int DB_DSK_FADE_GROUP                    = 188;
const unsigned int DB_DSK_TERRAIN                        = 189;
const unsigned int DB_DSK_BSP                            = 190;
const unsigned int DB_DSK_DECAL_GROUP                    = 191;
const unsigned int DB_DSK_STATE_MACHINE                    = 192;
const unsigned int DB_DSK_STATE                            = 193;
const unsigned int DB_DSK_TRANSITION                    = 194;
const unsigned int DB_DSK_STATE_MACHINE_ACTION            = 195;
const unsigned int DB_DSK_STATE_RULE_ACTION                = 196;
const unsigned int DB_DSK_TRANSITION_RULE_ACTION        = 197;
const unsigned int DB_DSK_PUSH_ACTION                    = 198; // Indicates a list of child actions coming
const unsigned int DB_DSK_POP_ACTION                    = 199; // End of child action list
const unsigned int DB_DSK_LIGHT_GROUP                    = 200;
const unsigned int DB_DSK_CONTINUOUS_ACTION                = 201;  
const unsigned int DB_DSK_MOMENTARY_ACTION                = 202;  
const unsigned int DB_DSK_NSTATE_ACTION                    = 203;  
const unsigned int DB_DSK_ROTARY_DRAG_ACTION            = 204;  
const unsigned int DB_DSK_DCS                            = 205;
const unsigned int DB_DSK_FAT_VERTEX                    = 206;
const unsigned int DB_DSK_SLIM_VERTEX                    = 207;
const unsigned int DB_DSK_CG_SHADER                        = 208;
const unsigned int DB_DSK_CGFX_SHADER                    = 209;
const unsigned int DB_DSK_GLSL_SHADER                    = 210;

        
                    




//
// Constants to define the data types supported in the format
//
const unsigned char DB_CHAR                                = 1;
const unsigned char DB_SHORT                            = 2;
const unsigned char DB_INT                                = 3;
const unsigned char DB_FLOAT                            = 4;
const unsigned char DB_LONG                                = 5;
const unsigned char DB_DOUBLE                            = 6;
const unsigned char DB_VEC2F                            = 7;
const unsigned char DB_VEC3F                            = 8;
const unsigned char DB_VEC4F                            = 9;
const unsigned char DB_VEC2I                            = 10;
const unsigned char DB_VEC3I                            = 11;
const unsigned char DB_VEC4I                            = 12;
const unsigned char DB_VEC16F                            = 13;
const unsigned char DB_VEC2D                            = 14;
const unsigned char DB_VEC3D                            = 15;
const unsigned char DB_VEC4D                            = 16;
const unsigned char DB_VEC16D                            = 17;
const unsigned char DB_VRTX_STRUCT                        = 18; // deprecated (obsolete) after 0.982
const unsigned char DB_UINT                                = 19;
const unsigned char DB_USHORT                            = 20;
const unsigned char DB_UCHAR                            = 21;
const unsigned char DB_ULONG                            = 22;
const unsigned char DB_EXT_STRUCT                        = 23;
const unsigned char DB_SHORT_WITH_PADDING                = 24;
const unsigned char DB_CHAR_WITH_PADDING                = 25;
const unsigned char DB_USHORT_WITH_PADDING                = 26;
const unsigned char DB_UCHAR_WITH_PADDING                = 27;
const unsigned char DB_BOOL_WITH_PADDING                = 28;
const unsigned char DB_EXTENDED_FIELD_STRUCT            = 31;
const unsigned char DB_VEC4UC                            = 32; // array of 4 unsigned chars
const unsigned char DB_DISCRETE_MAPPING_STRUCT            = 33;
const unsigned char DB_BITFLAGS                            = 34;
const unsigned char DB_VEC2S                            = 35; // 2 dim array of shorts

//
// Constants to define sizeof() values
//
const unsigned char SIZEOF_FIELD_STRUCT                    = 4;
const unsigned char SIZEOF_EXTENDED_FIELD_STRUCT        = 8;
const unsigned char SIZEOF_CHAR                            = 1;
const unsigned char SIZEOF_SHORT                        = 2;
const unsigned char SIZEOF_INT                            = 4;
const unsigned char SIZEOF_FLOAT                        = 4;
const unsigned char SIZEOF_LONG                            = 4;
const unsigned char SIZEOF_ULONG                        = 4;
const unsigned char SIZEOF_DOUBLE                        = 8;
const unsigned char SIZEOF_VEC2F                        = (SIZEOF_FLOAT*2);
const unsigned char SIZEOF_VEC3F                        = (SIZEOF_FLOAT*3);
const unsigned char SIZEOF_VEC4F                        = (SIZEOF_FLOAT*4);
const unsigned char SIZEOF_VEC16F                        = (SIZEOF_FLOAT*16);
const unsigned char SIZEOF_VEC2I                        = (SIZEOF_INT*2);
const unsigned char SIZEOF_VEC3I                        = (SIZEOF_INT*3);
const unsigned char SIZEOF_VEC4I                        = (SIZEOF_INT*4);
const unsigned char SIZEOF_VEC2D                        = (SIZEOF_DOUBLE*2);
const unsigned char SIZEOF_VEC3D                        = (SIZEOF_DOUBLE*3);
const unsigned char SIZEOF_VEC4D                        = (SIZEOF_DOUBLE*4);
const unsigned char SIZEOF_VEC16D                        = (SIZEOF_DOUBLE*16);
const unsigned char SIZEOF_VRTX_STRUCT                    = 32;
const unsigned char SIZEOF_EXT_STRUCT                    = 32;
const unsigned char SIZEOF_UCHAR                        = (SIZEOF_CHAR);
const unsigned char SIZEOF_USHORT                        = (SIZEOF_SHORT);
const unsigned char SIZEOF_UINT                            = (SIZEOF_INT);
const unsigned char SIZEOF_VEC4UC                        = (SIZEOF_INT);
const unsigned char SIZEOF_SHORT_WITH_PADDING            = (SIZEOF_INT);
const unsigned char SIZEOF_CHAR_WITH_PADDING            = (SIZEOF_INT);
const unsigned char SIZEOF_USHORT_WITH_PADDING            = (SIZEOF_INT);
const unsigned char SIZEOF_UCHAR_WITH_PADDING            = (SIZEOF_INT);
const unsigned char SIZEOF_BOOL_WITH_PADDING            = (SIZEOF_INT);
const unsigned char SIZEOF_DISCRETE_MAPPING_STRUCT        = 12;
const unsigned char SIZEOF_BITFLAGS                        = (SIZEOF_INT);



const char MIN_CHAR_VAL                    = -128;
const char MAX_CHAR_VAL                        = 127;
const unsigned char MAX_UCHAR_VAL                        = 255;
const short MIN_SHORT_VAL                        = -32768;
const short MAX_SHORT_VAL                        = 32767;
const unsigned short MAX_USHORT_VAL                        = 65535;





//
// Valid field size values are any value cleanly divisible by 4 & < 65536
//


/** Record identifiers can be read as ints or this structure. All subsequent
 *  fields are considered part of this Node until an special EOF(ield) record
 *  is found. The only exception to this rule id DB_DSK_PUSH & DB_DSK_POP
 *  which have no fields. User parse code should expect another REcord header
 *  immediately after reading the Push/Pop record.
 */
struct GEO_DB_API geoRecordHeader
{
    unsigned int        id; // e.g. DB_DSK_HEADER etc.
};




/** When you are reading a Node's fields you read into this structure & expect
 *  a 1 byte id. When you are expecting records - you expect 4 byte int id's
 */
struct GEO_DB_API geoFieldHeader
{
    /** The Field ID for the data about to be read from disk. This Field
     *  token record in effect describes the data which is going to 
     *  follow this geoFieldToken struct. This description is sufficient to
     *  allow parsing code to step over and ignore either fields or whole
     *  records if they are unknown or unwanted.
     */
    unsigned char        id;        // field ID for record

    /** The data type of the field coming up */
    unsigned char        type;    // DB_INT, etc.

    /** How many of the data types (described above) must be read */
    unsigned short        num;    // How many of them follow

};





/** The Field ID for the data about to be read from disk. This Field
 *  token record in effect describes the data which is going to 
 *  follow this geoExtendedFieldHeader struct. This description is sufficient to
 *  allow parsing code to step over and ignore either fields or whole
 *  records if they are unknown or unwanted.
 *
 *  This field header exists only when field data items exceed the maximum
 *  number addressable by the size of an unsigned short (i.e. 65535). Typically
 *  this record will be the "data item" of a standard geoFieldHeader. Take the
 *  example of a large vertex palette...
 *
 *  on disk:
 *    
 *  geoFieldHeader
 *  {
 *      DB_UCHAR    id        GEO_DB_VRTX_COORDS
 *      DB_UCHAR    type    DB_EXTENDED_FIELD_STRUCT  
 *      DB_USHORT   num     1
 *  }
 *
 *  parse code now knows there is 1 record of an extended field struct 
 *  following the header. It just so happens that the following "data item"
 *  is itself a header (this time of the extended variety)
 *
 *  next we find the geoExtendedFieldHeader. Id is repeated.
 *
 *  geoExtendedFieldHeader
 *  {
 *      DB_USHORT    id        GEO_DB_VRTX_COORDS
 *      DB_USHORT   type    DB_VEC3F
 *      DB_UINT     num     number-of-verts
 *  }
 *
 *  Read the data items "as normal". The id should be the same as the previous
 *  geoFieldHeader. Basically you will only ever find an geoExtendedFieldHeader
 *  after being informed of such by a preceeding geoFieldHeader. This means that
 *  parse code only needs to look for geoRecordHeader & geoFieldHeader records.
 *  It also means that there is enough info grouped together, so that an
 *  ignoreField function can work - given a geoFieldHeader to ignore.   
 *
 *  The id field is also an unsigned short in this header - meaning that records
 *  will be able to have more than 255 fields (the limit of a typical
 *  geoFieldHeader field - since its "id" field is only an unsigned char). If any
 *  records have more than 255 fields then their ids will be unsigned shorts and
 *  on disk they will be represented by...
 *    
 *  geoFieldHeader
 *  {
 *      DB_UCHAR    id        GEO_DB_NODE_EXTENDED
 *      DB_UCHAR    type    DB_EXTENDED_FIELD_STRUCT  
 *      DB_USHORT   num     1
 *  }
 *
 *  followed by...
 *
 *  geoExtendedFieldHeader
 *  {
 *      DB_USHORT    id        GEO_DB_SOME_FUTURE_USHORT_ID
 *      DB_USHORT   type    DB_VEC3F
 *      DB_UINT     num     number_of_data_items
 *  }
 *
 *  The GEO_DB_EXTENDED_FIELD is a special token which indicates that the "real"
 *  id will be a ushort and be found in the following geoExtendedFieldHeader
 *
 */
struct GEO_DB_API geoExtendedFieldHeader
{
    /** Id of the field. This should be the same ID as the previous geoFieldHeader
     *  which indicated the existence of this record.
     */
    unsigned short        id;        // field ID for record

    /** The data type of the field coming up */
    unsigned short        type;    // DB_INT, etc.

    /** How many of the data types (described above) must be read */
    unsigned int        num;    // How many of them follow

};




///////////////////////////////////////////////////////////////////////////////
// Constant to define the last field types
//
const unsigned char GEO_DB_LAST_FIELD                = 0;






///////////////////////////////////////////////////////////////////////////////
//
// Common NODE field types - can found for all SceneGraph Nodes
//
///////////////////////////////////////////////////////////////////////////////

const unsigned char GEO_DB_NODE_EXT                            = 1;
const unsigned char GEO_DB_NODE_PADDING                        = 2;
const unsigned char GEO_DB_NODE_EXTENDED                    = 4;
const unsigned char GEO_DB_NODE_COMMENT                        = 5;
const unsigned char GEO_DB_NODE_NAME                        = 6;
const unsigned char GEO_DB_NODE_ID                            = 7;    // internal use only


///////////////////////////////////////////////////////////////////////////////
//
// Field ID Address Ranges...
//
//NODE                                                         1   through 19            
//    DERIVED_A : public NODE                                    20  through 79        
//        DERIVED_B : public DERIVED_A                        80  through 139    
//            DERIVED_C : public DERIVED_B                    140 through 199
//                DERIVED_D : public DERIVED_C                200 through 255
//
//Examples:
//
//GEO_DSK_MULTI_TEX_SHADER:
//            Node Property IDs                                1   through 19    
//            Group Property IDs                                20  through 79
//            RenderGroup property IDs                        80  through 139
//            MultiTexShader Property IDs                        140 through 199
//
//
//DB_DSK_LIGHTPT:
//            Node Property IDs                                1   through 19    
//            Polygon Property IDs                            20  through 79
//            LightPt property IDs                            80  through 139
//
///////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_HEADER Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

const unsigned char GEO_DB_HDR_EXT                            = 1;    // From Node
const unsigned char GEO_DB_HDR_PADDING                        = 2;    // From Node
const unsigned char GEO_DB_HDR_EXTENDED                        = 4;    // From Node
const unsigned char GEO_DB_HDR_COMMENT                        = 5;    // From Node
const unsigned char GEO_DB_HDR_NAME                            = 6;    // From Node
const unsigned char GEO_DB_HDR_NODE_ID                        = 7;    // From Node
    
const unsigned char GEO_DB_HDR_UNITS                        = 20;    // Header Additions
const unsigned char GEO_DB_HDR_BBOX                            = 21;
const unsigned char GEO_DB_HDR_VERSION                        = 22;
const unsigned char GEO_DB_HDR_EXT_TEMPLATE                    = 23;
const unsigned char GEO_DB_HDR_UP_AXIS                        = 24;
const unsigned char GEO_DB_HDR_PROJ_TYPE                    = 25;
const unsigned char GEO_DB_HDR_LAMBERT1                        = 26;
const unsigned char GEO_DB_HDR_LAMBERT2                        = 27;
const unsigned char GEO_DB_HDR_UTM_ZONE                        = 28;
const unsigned char GEO_DB_HDR_SOUTHERN_HEMISPHERE            = 29;
const unsigned char GEO_DB_HDR_ELLIPSOID                    = 30;
const unsigned char GEO_DB_HDR_ORIGIN_LAT                    = 31;    
const unsigned char GEO_DB_HDR_ORIGIN_LON                    = 32;
const unsigned char GEO_DB_HDR_SW_CORNER_LAT                = 33;    
const unsigned char GEO_DB_HDR_SW_CORNER_LON                = 34;    
const unsigned char GEO_DB_HDR_NE_CORNER_LAT                = 35;    
const unsigned char GEO_DB_HDR_NE_CORNER_LON                = 36;
const unsigned char GEO_DB_HDR_SW_CORNER_X                    = 37;
const unsigned char GEO_DB_HDR_SW_CORNER_Y                    = 38;
const unsigned char GEO_DB_HDR_OFFSET_X                        = 39;
const unsigned char GEO_DB_HDR_OFFSET_Y                        = 40;    
const unsigned char GEO_DB_HDR_OFFSET_Z                        = 41;    
const unsigned char GEO_DB_HDR_MAJOR_AXIS                    = 42;    
const unsigned char GEO_DB_HDR_MINOR_AXIS                    = 43;                    




///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_COORD_POOL Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

const unsigned char GEO_DB_COORD_POOL_SIZE                    = 1;
const unsigned char GEO_DB_COORD_POOL_VALUES                = 2;
const unsigned char GEO_DB_COORD_POOL_SCALE                    = 3;
const unsigned char GEO_DB_COORD_POOL_OFFSET                = 4;





///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_NORMAL_POOL Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

const unsigned char GEO_DB_NORMAL_POOL_SIZE                    = 1;
const unsigned char GEO_DB_NORMAL_POOL_VALUES                = 2;






///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_MATERIAL Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

const unsigned char GEO_DB_MAT_AMBIENT                        = 1;
const unsigned char GEO_DB_MAT_DIFFUSE                        = 2;
const unsigned char GEO_DB_MAT_SPECULAR                        = 3;
const unsigned char GEO_DB_MAT_SHININESS                    = 4;
const unsigned char GEO_DB_MAT_NAME                            = 5;
const unsigned char GEO_DB_MAT_EMISSIVE                        = 6;






///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_COLOR_PALETTE Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

const unsigned char GEO_DB_COLOR_PALETTE_HIGHEST_INTENSITIES = 1;


///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_TEXTURE Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

const unsigned char GEO_DB_TEX_WRAPS                        = 1;
const unsigned char GEO_DB_TEX_WRAPT                        = 2;
const unsigned char GEO_DB_TEX_MAGFILTER                    = 3;
const unsigned char GEO_DB_TEX_MINFILTER                    = 4;
const unsigned char GEO_DB_TEX_ENV                            = 5;
const unsigned char GEO_DB_TEX_FILE_NAME                    = 6;
const unsigned char GEO_DB_TEX_NAME                            = 7;
const unsigned char GEO_DB_TEX_CUBE_XP_FILE_NAME            = 8;
const unsigned char GEO_DB_TEX_CUBE_XN_FILE_NAME            = 9;
const unsigned char GEO_DB_TEX_CUBE_YP_FILE_NAME            = 10;
const unsigned char GEO_DB_TEX_CUBE_YN_FILE_NAME            = 11;
const unsigned char GEO_DB_TEX_CUBE_ZP_FILE_NAME            = 12;
const unsigned char GEO_DB_TEX_CUBE_ZN_FILE_NAME            = 13;
const unsigned char GEO_DB_TEX_INTERNAL_FORMAT                = 14;
const unsigned char GEO_DB_TEX_ANISOTROPIC_FILTER            = 15;
const unsigned char GEO_DB_TEX_REAL_WORLD_WIDTH                = 16;
const unsigned char GEO_DB_TEX_REAL_WORLD_HEIGHT            = 17;
const unsigned char GEO_DB_TEX_SENSOR_FILE_NAME                = 18;





///////////////////////////////////////////////////////////////////////////////
//
// Common SHADER field types - can found for all Shader Types
//
///////////////////////////////////////////////////////////////////////////////

const unsigned char GEO_DB_SHADER_NAME                        = 1;
const unsigned char GEO_DB_SHADER_VERTEX_PROGRAM            = 2;
const unsigned char GEO_DB_SHADER_FRAGMENT_PROGRAM            = 3;







///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_CG_SHADER Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All shader Fields    +

const unsigned char GEO_DB_SHADER_CG_VERTEX_ENTRY            = 20;
const unsigned char GEO_DB_SHADER_CG_VERTEX_PROFILE            = 21;
const unsigned char GEO_DB_SHADER_CG_FRAGMENT_ENTRY            = 22;
const unsigned char GEO_DB_SHADER_CG_FRAGMENT_PROFILE        = 23;






///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_CGFX_SHADER Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All shader Fields    +

// currently No additional fields                            = 20 will be first addition





///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_GLSL_SHADER Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All shader Fields    +

// currently No additional fields                            = 20 will be first addition




///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_VIEW Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

const unsigned char GEO_DB_VIEW_NEAR                        = 1;
const unsigned char GEO_DB_VIEW_FAR                            = 2;
const unsigned char GEO_DB_VIEW_POS                            = 3;
const unsigned char GEO_DB_VIEW_CEN                            = 4;
const unsigned char GEO_DB_VIEW_TRACKBALL                    = 5;
const unsigned char GEO_DB_VIEW_BACKFACE                    = 6;
const unsigned char GEO_DB_VIEW_TEXTURE                        = 7;
const unsigned char GEO_DB_VIEW_ILLUMINATED                    = 8;
const unsigned char GEO_DB_VIEW_ZBUFFER                        = 9;
const unsigned char GEO_DB_VIEW_SELECTIVE_ZBUFFER            = 10;
const unsigned char GEO_DB_VIEW_DRAWSTYLE                    = 11;
const unsigned char GEO_DB_VIEW_SELECTIVE_CULLFACE            = 12;
const unsigned char GEO_DB_VIEW_SELECTIVE_BLENDING            = 13;
const unsigned char GEO_DB_VIEW_SELECTIVE_SHADING            = 14;





///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_PERSPECTIVE_GRID_INFO Record 
//
// DB_DSK_XY_GRID_INFO Record 
// DB_DSK_XZ_GRID_INFO Record 
// DB_DSK_YZ_GRID_INFO Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

const unsigned char GEO_DB_GRID_ON                            = 1;
const unsigned char GEO_DB_GRID_ZBUFFER                        = 2;
const unsigned char GEO_DB_GRID_SNAP                        = 3;
const unsigned char GEO_DB_GRID_OVER                        = 4;
const unsigned char GEO_DB_GRID_MAJOR                        = 5;
const unsigned char GEO_DB_GRID_MINOR                        = 6;
const unsigned char GEO_DB_GRID_NUM_CELLS                    = 7;
const unsigned char GEO_DB_GRID_POS                            = 8;
const unsigned char GEO_DB_GRID_MATRIX                        = 9;





///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_GROUP Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
 
const unsigned char GEO_DB_GRP_BBOX                            = 20;    // Group Additions
const unsigned char GEO_DB_GRP_INSTANCE_DEF                    = 21;
const unsigned char GEO_DB_GRP_FLAG_SHOW_BBOX                = 22;
const unsigned char GEO_DB_GRP_ZBUFFER                        = 23;
const unsigned char GEO_DB_GRP_MATRIX_TRANSFORM                = 24;    
const unsigned char GEO_DB_GRP_TRANSLATE_TRANSFORM            = 25;    
const unsigned char GEO_DB_GRP_ROTATE_TRANSFORM                = 26;    
const unsigned char GEO_DB_GRP_SCALE_TRANSFORM                = 27;    
const unsigned char GEO_DB_GRP_TOD_DISPLAY                    = 28;
const unsigned char GEO_DB_GRP_NOISECT                        = 29;




///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_LIGHT_GROUP Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +

const unsigned char GEO_DB_LIGHT_GROUP_ENABLED                = 80;
const unsigned char GEO_DB_LIGHT_GROUP_INTENSITY            = 81;
const unsigned char GEO_DB_LIGHT_GROUP_ANIMATION            = 82;
const unsigned char GEO_DB_LIGHT_GROUP_STABILIZED            = 83;
const unsigned char GEO_DB_LIGHT_GROUP_TYPE                    = 84;
const unsigned char GEO_DB_LIGHT_GROUP_IG_LIGHTGROUP_ID        = 85;




///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_DCS Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +

const unsigned char GEO_DB_DCS_ACTIVE                        = 80;

const unsigned char GEO_DB_DCS_TRANSLATE_X_DEFAULT            = 81;
const unsigned char GEO_DB_DCS_TRANSLATE_X_MIN                = 82;
const unsigned char GEO_DB_DCS_TRANSLATE_X_MAX                = 83;
const unsigned char GEO_DB_DCS_TRANSLATE_X_STEP                = 84;
const unsigned char GEO_DB_DCS_TRANSLATE_X_CLAMP            = 85;

const unsigned char GEO_DB_DCS_TRANSLATE_Y_DEFAULT            = 86;
const unsigned char GEO_DB_DCS_TRANSLATE_Y_MIN                = 87;
const unsigned char GEO_DB_DCS_TRANSLATE_Y_MAX                = 88;
const unsigned char GEO_DB_DCS_TRANSLATE_Y_STEP                = 89;
const unsigned char GEO_DB_DCS_TRANSLATE_Y_CLAMP            = 90;

const unsigned char GEO_DB_DCS_TRANSLATE_Z_DEFAULT            = 91;
const unsigned char GEO_DB_DCS_TRANSLATE_Z_MIN                = 92;
const unsigned char GEO_DB_DCS_TRANSLATE_Z_MAX                = 93;
const unsigned char GEO_DB_DCS_TRANSLATE_Z_STEP                = 94;
const unsigned char GEO_DB_DCS_TRANSLATE_Z_CLAMP            = 95;

const unsigned char GEO_DB_DCS_ROTATE_X_DEFAULT                = 96;
const unsigned char GEO_DB_DCS_ROTATE_X_MIN                    = 97;
const unsigned char GEO_DB_DCS_ROTATE_X_MAX                    = 98;
const unsigned char GEO_DB_DCS_ROTATE_X_STEP                = 99;
const unsigned char GEO_DB_DCS_ROTATE_X_CLAMP                = 100;

const unsigned char GEO_DB_DCS_ROTATE_Y_DEFAULT                = 101;
const unsigned char GEO_DB_DCS_ROTATE_Y_MIN                    = 102;
const unsigned char GEO_DB_DCS_ROTATE_Y_MAX                    = 103;
const unsigned char GEO_DB_DCS_ROTATE_Y_STEP                = 104;
const unsigned char GEO_DB_DCS_ROTATE_Y_CLAMP                = 105;

const unsigned char GEO_DB_DCS_ROTATE_Z_DEFAULT                = 106;
const unsigned char GEO_DB_DCS_ROTATE_Z_MIN                    = 107;
const unsigned char GEO_DB_DCS_ROTATE_Z_MAX                    = 108;
const unsigned char GEO_DB_DCS_ROTATE_Z_STEP                = 109;
const unsigned char GEO_DB_DCS_ROTATE_Z_CLAMP                = 110;

const unsigned char GEO_DB_DCS_SCALE_X_DEFAULT                = 111;
const unsigned char GEO_DB_DCS_SCALE_X_MIN                    = 112;
const unsigned char GEO_DB_DCS_SCALE_X_MAX                    = 113;
const unsigned char GEO_DB_DCS_SCALE_X_STEP                    = 114;
const unsigned char GEO_DB_DCS_SCALE_X_CLAMP                = 115;

const unsigned char GEO_DB_DCS_SCALE_Y_DEFAULT                = 116;
const unsigned char GEO_DB_DCS_SCALE_Y_MIN                    = 117;
const unsigned char GEO_DB_DCS_SCALE_Y_MAX                    = 118;
const unsigned char GEO_DB_DCS_SCALE_Y_STEP                    = 119;
const unsigned char GEO_DB_DCS_SCALE_Y_CLAMP                = 120;

const unsigned char GEO_DB_DCS_SCALE_Z_DEFAULT                = 121;
const unsigned char GEO_DB_DCS_SCALE_Z_MIN                    = 122;
const unsigned char GEO_DB_DCS_SCALE_Z_MAX                    = 123;
const unsigned char GEO_DB_DCS_SCALE_Z_STEP                    = 124;
const unsigned char GEO_DB_DCS_SCALE_Z_CLAMP                = 125;

const unsigned char GEO_DB_DCS_ORIGIN                        = 126;
const unsigned char GEO_DB_DCS_XPOS                            = 127;
const unsigned char GEO_DB_DCS_ZPOS                            = 128;
const unsigned char GEO_DB_DCS_VECTOR                        = 129;






///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_CULL_GROUP Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +
              
const unsigned char GEO_DB_CULL_GRP_RADIUS                    = 80;    // CullGroup Additions    





///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_Z_OFFSET_GROUP Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +
                                          
const unsigned char GEO_DB_Z_GRP_DEPTH_OFFSET                = 80;    // ZOffsetGroup Additions
const unsigned char GEO_DB_Z_GRP_DEPTH_OFFSET_CONSTANT        = 81;    
const unsigned char GEO_DB_Z_GRP_TYPE                        = 82;    





///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_MULTI_SAMPLE_AA_GROUP Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +
                                                  
const unsigned char GEO_DB_MULTI_SAMPLE_AA_GRP_NUM_SAMPLES    = 80;    // MultiSampleAAGroup Additions
const unsigned char GEO_DB_MULTI_SAMPLE_AA_GRP_JITTER_RADIUS= 81;
const unsigned char GEO_DB_MULTI_SAMPLE_AA_GRP_ALPHA_CLAMP    = 82;





///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_LINE_AA_GROUP Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////


// All Node Fields    +
// All Group Fields +
                                      
const unsigned char GEO_DB_LINE_AA_GRP_RANGE                = 80;    // LineAAGroup Additions 
const unsigned char GEO_DB_LINE_AA_GRP_ALPHA_FACTOR            = 81;







///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_FADE_GROUP Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +
                                          
const unsigned char GEO_DB_FADE_GRP_NEAR_RANGE                = 80;    // FadeGroup Additions
const unsigned char GEO_DB_FADE_GRP_NEAR_TRANSITION            = 81;
const unsigned char GEO_DB_FADE_GRP_FAR_RANGE                = 82;
const unsigned char GEO_DB_FADE_GRP_FAR_TRANSITION            = 83;






///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_TERRAIN Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +

                                                                    // No Terrain Additions






///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_DECAL_GROUP Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +

// No Decal Group Additions





///////////////////////////////////////////////////////////////////////////////
//
// DB_DSK_BSP Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +

const unsigned char GEO_DB_BSP_PLANE_EQUATION                = 80;    // BSP Additions






///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_LOD Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +

const unsigned char GEO_DB_LOD_IN                            = 80;    // LOD Additions
const unsigned char GEO_DB_LOD_OUT                            = 81;
const unsigned char GEO_DB_LOD_CENTER                        = 82;
const unsigned char GEO_DB_LOD_CALC                            = 83; 
const unsigned char GEO_DB_LOD_FREEZE_CENTER                = 84;






///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_SEQUENCE Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +

const unsigned char GEO_DB_SEQUENCE_MODE                    = 80;    // Sequence Additions
const unsigned char GEO_DB_SEQUENCE_ACTIVE                    = 81;
const unsigned char GEO_DB_SEQUENCE_FRAME_TIME                = 82;
const unsigned char GEO_DB_SEQUENCE_USE_FRAME_TIME            = 83;
const unsigned char GEO_DB_SEQUENCE_SWING                    = 84;






///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_INSTANCE Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +

const unsigned char GEO_DB_INSTANCE_DEF                        = 80;    // instance Additions







///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_SWITCH Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +
    
const unsigned char GEO_DB_SWITCH_CURRENT_MASK                = 80;    // switch Additions
const unsigned char GEO_DB_SWITCH_MASK_WIDTH                = 81;    
const unsigned char GEO_DB_SWITCH_NUM_MASKS                    = 82; 
const unsigned char GEO_DB_SWITCH_MASKS                        = 83;








///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_PAGE Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +
    
const unsigned char GEO_DB_PAGE_ACTIVE_CHILD                = 80;    // page Additions







///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_BASE_GROUP Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// It is important to note that user extended Group fields begin with a field
// ID of 140 (GEO_DB_BASE_GROUP_START_EXTERNAL). This allows Geo to add 
// additional fields to the BaseGroup class 

// All Node Fields    +
// All Group Fields +
    
const unsigned char GEO_DB_BASE_GROUP_CLASSNAME                = 80;    // BaseGroup Additions
const unsigned char GEO_DB_BASE_GROUP_EXTENDED_TYPE            = 81;
const unsigned char GEO_DB_BASE_GROUP_PLUGIN_REQUIRED        = 82;
    
const unsigned char GEO_DB_BASE_GROUP_START_EXTERNAL        = 140;    // User Derived Node Additions    








///////////////////////////////////////////////////////////////////////////////
// GEO_DB_BASE_SURFACE Record - Field Ids
//
// It is important to note that user extended Surface fields begin with a field
// ID of 80 (GEO_DB_BASE_SURFACE_START_EXTERNAL). This allows Geo to add 
// additional fields which will be common to all derived groups up to this
// number
//
// NOT IMPLEMENTED IN GEO 1.0
//
///////////////////////////////////////////////////////////////////////////////


// All Node Fields    +
    
const unsigned char GEO_DB_BASE_SURFACE_CLASSNAME            = 20;    // BaseSurface Additions
const unsigned char GEO_DB_BASE_SURFACE_EXTENDED_TYPE        = 21;
const unsigned char GEO_DB_BASE_SURFACE_PLUGIN_REQUIRED        = 22;
    
const unsigned char GEO_DB_BASE_SURFACE_START_EXTERNAL        = 80;    // User Derived Node Additions    






///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_RENDERGROUP Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +

const unsigned char GEO_DB_RENDERGROUP_MAT                    = 80;    // RenderGroup Additions
const unsigned char GEO_DB_RENDERGROUP_TERRAIN                = 81;   
const unsigned char GEO_DB_RENDERGROUP_BILLBOARD            = 82;   
const unsigned char GEO_DB_RENDERGROUP_LIGHTING                = 83;    
const unsigned char GEO_DB_RENDERGROUP_FOG                    = 84;    
const unsigned char GEO_DB_RENDERGROUP_GAIN                    = 85;   
const unsigned char GEO_DB_RENDERGROUP_TRANSPARENCY            = 86;  
const unsigned char GEO_DB_RENDERGROUP_CULLING                = 87;    
const unsigned char GEO_DB_RENDERGROUP_BLENDING                = 88;   
const unsigned char GEO_DB_RENDERGROUP_ALPHA_REF            = 89;  
const unsigned char GEO_DB_RENDERGROUP_LIGHTPTS                = 90;   







///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_MULTI_TEX_SHADER Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +
// All RenderGroup Fields +

const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX0            = 140;    // MultiTexShader Additions
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX1            = 141;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX2            = 142;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX3            = 143;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX4            = 144;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX5            = 145;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX6            = 146;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX7            = 147;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX0_FUNCTION    = 148;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX1_FUNCTION    = 149;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX2_FUNCTION    = 150;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX3_FUNCTION    = 151;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX4_FUNCTION    = 152;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX5_FUNCTION    = 153;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX6_FUNCTION    = 154;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX7_FUNCTION    = 155;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX0_BLEND_COLOR= 156;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX1_BLEND_COLOR= 157;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX2_BLEND_COLOR= 158;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX3_BLEND_COLOR= 159;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX4_BLEND_COLOR= 160;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX5_BLEND_COLOR= 161;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX6_BLEND_COLOR= 162;
const unsigned char GEO_DB_MULTI_TEX_SHADER_TEX7_BLEND_COLOR= 163;







///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_POLYGON Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +

const unsigned char GEO_DB_POLY_NORMAL                        = 20;    // Polygon Additions 
const unsigned char GEO_DB_POLY_CENTER                        = 21;
const unsigned char GEO_DB_POLY_PACKED_COLOR                = 22;
const unsigned char GEO_DB_POLY_DSTYLE                        = 23;
const unsigned char GEO_DB_POLY_SHADEMODEL                    = 24;
const unsigned char GEO_DB_POLY_USE_MATERIAL_DIFFUSE        = 25;
const unsigned char GEO_DB_POLY_USE_VERTEX_COLORS            = 26;
const unsigned char GEO_DB_POLY_COLOR_INDEX                    = 27;
const unsigned char GEO_DB_POLY_PT_SIZE                        = 28;
const unsigned char GEO_DB_POLY_LINE_WIDTH                    = 29;
const unsigned char GEO_DB_POLY_TEX0                        = 30;
const unsigned char GEO_DB_POLY_TEX1                        = 31;
const unsigned char GEO_DB_POLY_TEX2                        = 32;
const unsigned char GEO_DB_POLY_TEX3                        = 33;
const unsigned char GEO_DB_POLY_TEX4                        = 34;
const unsigned char GEO_DB_POLY_TEX5                        = 35;
const unsigned char GEO_DB_POLY_TEX6                        = 36;
const unsigned char GEO_DB_POLY_TEX7                        = 37;
const unsigned char GEO_DB_POLY_TEX0_FUNCTION                = 38;
const unsigned char GEO_DB_POLY_TEX1_FUNCTION                = 39;
const unsigned char GEO_DB_POLY_TEX2_FUNCTION                = 40;
const unsigned char GEO_DB_POLY_TEX3_FUNCTION                = 41;
const unsigned char GEO_DB_POLY_TEX4_FUNCTION                = 42;
const unsigned char GEO_DB_POLY_TEX5_FUNCTION                = 43;
const unsigned char GEO_DB_POLY_TEX6_FUNCTION                = 44;
const unsigned char GEO_DB_POLY_TEX7_FUNCTION                = 45;
const unsigned char GEO_DB_POLY_TEX0_BLEND_COLOR            = 46;
const unsigned char GEO_DB_POLY_TEX1_BLEND_COLOR            = 47;
const unsigned char GEO_DB_POLY_TEX2_BLEND_COLOR            = 48;
const unsigned char GEO_DB_POLY_TEX3_BLEND_COLOR            = 49;
const unsigned char GEO_DB_POLY_TEX4_BLEND_COLOR            = 50;
const unsigned char GEO_DB_POLY_TEX5_BLEND_COLOR            = 51;
const unsigned char GEO_DB_POLY_TEX6_BLEND_COLOR            = 52;
const unsigned char GEO_DB_POLY_TEX7_BLEND_COLOR            = 53;
const unsigned char GEO_DB_POLY_WHITE_IF_TEXTURED            = 54;
const unsigned char GEO_DB_POLY_BASE                        = 55;
const unsigned char GEO_DB_POLY_DECAL                        = 56;
const unsigned char GEO_DB_POLY_HIDDEN                        = 57;
const unsigned char GEO_DB_POLY_HELPER_TYPE                    = 58;
const unsigned char GEO_DB_POLY_BINORMAL                    = 59;
const unsigned char GEO_DB_POLY_TANGENT                        = 60;
const unsigned char GEO_DB_POLY_SHADER                        = 61;
const unsigned char GEO_DB_POLY_SMC                            = 62;
const unsigned char GEO_DB_POLY_FID                            = 63;
const unsigned char GEO_DB_POLY_FOOTPRINT                    = 64;




///////////////////////////////////////////////////////////////////
//
// DB_DSK_PLANE_TEXTURE_MAPPING_INFO  Record Field Ids
//
///////////////////////////////////////////////////////////////////

const unsigned char GEO_DB_PLANE_TEXTURE_MAPPING_INFO_ORIGIN_PT        = 1;
const unsigned char GEO_DB_PLANE_TEXTURE_MAPPING_INFO_U_AXIS_PT        = 2;
const unsigned char GEO_DB_PLANE_TEXTURE_MAPPING_INFO_V_AXIS_PT        = 3;
const unsigned char GEO_DB_PLANE_TEXTURE_MAPPING_INFO_TEXTURE_UNIT    = 4;







///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_LIGHTPT Record - Field Ids
//
// Many of the possible LightPt fields do not make sense in the context of a
// Light point - namely most of the Polygon rendering & texturing properties.
// These will likely not be present in a Geo file - if they are they can be 
// ignored.
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Poly Fields +

const unsigned char GEO_DB_LIGHTPT_TYPE                        = 80;    // LightPt Additions
const unsigned char GEO_DB_LIGHTPT_DIRECTION_VECTOR            = 81;
const unsigned char GEO_DB_LIGHTPT_BACK_PACKED_COLOR        = 82;
const unsigned char GEO_DB_LIGHTPT_MIN_PIXEL_SIZE            = 83;
const unsigned char GEO_DB_LIGHTPT_MAX_PIXEL_SIZE            = 84;
const unsigned char GEO_DB_LIGHTPT_HORIZ_LOBE_ANGLE            = 85;
const unsigned char GEO_DB_LIGHTPT_VERT_LOBE_ANGLE            = 86;
const unsigned char GEO_DB_LIGHTPT_DAY_DISPLAY                = 87;
const unsigned char GEO_DB_LIGHTPT_DUSK_DISPLAY                = 88;
const unsigned char GEO_DB_LIGHTPT_NIGHT_DISPLAY            = 89;
const unsigned char GEO_DB_LIGHTPT_BACK_COLOR_INDEX            = 90;
const unsigned char GEO_DB_LIGHTPT_SPECIAL_FX                = 91;
const unsigned char GEO_DB_LIGHTPT_ANIM_FRAME_COUNT            = 92;
const unsigned char GEO_DB_LIGHTPT_ANIM_ACTUAL_COUNT        = 93;
const unsigned char GEO_DB_LIGHTPT_IG_LIGHTGROUP_ID            = 94;
const unsigned char GEO_DB_LIGHTPT_ANIM_PHASE_DELAY            = 95;
const unsigned char GEO_DB_LIGHTPT_OPTIMIZATION                = 96;






///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_TEXT Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +

const unsigned char GEO_DB_TEXT_TYPE                        = 20;    // Text Additions
const unsigned char GEO_DB_TEXT_STRING                        = 21;
const unsigned char GEO_DB_TEXT_JUSTIFICATION                = 22;
const unsigned char GEO_DB_TEXT_DIRECTION                    = 23;
const unsigned char GEO_DB_TEXT_LINEWIDTH                    = 24;
const unsigned char GEO_DB_TEXT_PACKED_COLOR                = 25;
const unsigned char GEO_DB_TEXT_SCALE_X                        = 26;
const unsigned char GEO_DB_TEXT_SCALE_Y                        = 27;
const unsigned char GEO_DB_TEXT_MATRIX                        = 28;
const unsigned char GEO_DB_TEXT_EXPANSION                    = 29;
const unsigned char GEO_DB_TEXT_COLOR_INDEX                    = 30;
const unsigned char GEO_DB_TEXT_FONT                        = 31;






///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_MESH Record - Field Ids            
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Poly Fields  +

const unsigned char GEO_DB_MESH_TYPE                        = 80;    // Mesh Additions
















///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_VERTEX Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +

const unsigned char GEO_DB_VRTX_COORD_INDEX                    = 20;    // Vertex Additions
const unsigned char GEO_DB_VRTX_UV_SET_0                    = 21;
const unsigned char GEO_DB_VRTX_UV_SET_1                    = 22;    
const unsigned char GEO_DB_VRTX_UV_SET_2                    = 23;    
const unsigned char GEO_DB_VRTX_UV_SET_3                    = 24;    
const unsigned char GEO_DB_VRTX_UV_SET_4                    = 25;    
const unsigned char GEO_DB_VRTX_UV_SET_5                    = 26;    
const unsigned char GEO_DB_VRTX_UV_SET_6                    = 27;    
const unsigned char GEO_DB_VRTX_UV_SET_7                    = 28;    
const unsigned char GEO_DB_VRTX_NORMAL                        = 29;
const unsigned char GEO_DB_VRTX_PACKED_COLOR                = 30;
const unsigned char GEO_DB_VRTX_COLOR_INDEX                    = 31;
const unsigned char GEO_DB_VRTX_COORD                        = 32;
const unsigned char GEO_DB_VRTX_HARD_EDGE                    = 33;
const unsigned char GEO_DB_VRTX_FREEZE_NORMAL                = 34;
const unsigned char GEO_DB_VRTX_BINORMAL                    = 59;
const unsigned char GEO_DB_VRTX_TANGENT                        = 60;


///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_FAT_VERTEX Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////


// All Node Fields        +
// All VERTEX Fields    +






///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_SLIM_VERTEX Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////


// All Node Fields        +
// All VERTEX Fields    +

const unsigned char GEO_DB_VRTX_FLAGS                        = 80;
/* packed 32-bit flags with following format...
        bit 0 GEO_DB_VRTX_UV_SET_0 is set
        bit 1 GEO_DB_VRTX_UV_SET_1 is set
        bit 2 spare
        bit 3 spare
        bit 4 hard_edge
        bit 5 freeze_normal
        bits 6-31 spare */




///////////////////////////////////////////////////////////////////////////////
//
// GEO_DB_EXTERNAL Record - Field Ids
//
///////////////////////////////////////////////////////////////////////////////

// All Node Fields    +
// All Group Fields +

const unsigned char GEO_DB_EXTERNAL_FILENAME                = 80;    // External Additions






///////////////////////////////////////////////////////////////////////////////
//
// GEO BEHAVIOR & ARTICULATION SYSTEM RECORDS
//
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////
// DB_DSK_INTERNAL_VARS  Record Field Ids
//
const unsigned char GEO_DB_INTERNAL_VAR_FRAMECOUNT    = 1;
const unsigned char GEO_DB_INTERNAL_VAR_CURRENT_TIME= 2;
const unsigned char GEO_DB_INTERNAL_VAR_ELAPSED_TIME= 3;
const unsigned char GEO_DB_INTERNAL_VAR_SINE        = 4;
const unsigned char GEO_DB_INTERNAL_VAR_COSINE        = 5;
const unsigned char GEO_DB_INTERNAL_VAR_TANGENT        = 6;
const unsigned char GEO_DB_INTERNAL_VAR_MOUSE_X        = 7;
const unsigned char GEO_DB_INTERNAL_VAR_MOUSE_Y        = 8;
const unsigned char GEO_DB_INTERNAL_VAR_LEFT_MOUSE    = 9;
const unsigned char GEO_DB_INTERNAL_VAR_MIDDLE_MOUSE= 10;
const unsigned char GEO_DB_INTERNAL_VAR_RIGHT_MOUSE    = 11;
const unsigned char GEO_DB_INTERNAL_VAR_KEYBOARD    = 12;
const unsigned char GEO_DB_INTERNAL_VAR_TEMP_FLOAT    = 13;
const unsigned char GEO_DB_INTERNAL_VAR_TEMP_INT    = 14;
const unsigned char GEO_DB_INTERNAL_VAR_TEMP_BOOL    = 15;
const unsigned char GEO_DB_INTERNAL_VAR_TEMP_STRING    = 16;
const unsigned char GEO_DB_INTERNAL_VAR_TRIGGER1    = 17;
const unsigned char GEO_DB_INTERNAL_VAR_TRIGGER2    = 18;
const unsigned char GEO_DB_INTERNAL_VAR_TRIGGER3    = 19;
const unsigned char GEO_DB_INTERNAL_VAR_TRIGGER4    = 20;




///////////////////////////////////////////////////////////////////
// DB_DSK_FLOAT_VAR  Record Field Ids
//
const unsigned char GEO_DB_FLOAT_VAR_NAME            = 1;
const unsigned char GEO_DB_FLOAT_VAR_VALUE            = 2;
const unsigned char GEO_DB_FLOAT_VAR_DEFAULT        = 3;
const unsigned char GEO_DB_FLOAT_VAR_FID            = 4;
const unsigned char GEO_DB_FLOAT_VAR_CONSTRAINED    = 5;
const unsigned char GEO_DB_FLOAT_VAR_MIN            = 6;
const unsigned char GEO_DB_FLOAT_VAR_MAX            = 7;
const unsigned char GEO_DB_FLOAT_VAR_STEP            = 8;



///////////////////////////////////////////////////////////////////
// DB_DSK_FLOAT3_VAR  Record Field Ids
//
const unsigned char GEO_DB_FLOAT3_VAR_NAME            = 1;
const unsigned char GEO_DB_FLOAT3_VAR_VALUE            = 2;
const unsigned char GEO_DB_FLOAT3_VAR_DEFAULT        = 3;
const unsigned char GEO_DB_FLOAT3_VAR_FID            = 4;
const unsigned char GEO_DB_FLOAT3_VAR_CONSTRAINED    = 5;
const unsigned char GEO_DB_FLOAT3_VAR_MIN            = 6;
const unsigned char GEO_DB_FLOAT3_VAR_MAX            = 7;
const unsigned char GEO_DB_FLOAT3_VAR_STEP            = 8;


///////////////////////////////////////////////////////////////////
// DB_DSK_INT_VAR  Record Field Ids
//
const unsigned char GEO_DB_INT_VAR_NAME                = 1;
const unsigned char GEO_DB_INT_VAR_VALUE            = 2;
const unsigned char GEO_DB_INT_VAR_DEFAULT            = 3;
const unsigned char GEO_DB_INT_VAR_FID                = 4;
const unsigned char GEO_DB_INT_VAR_CONSTRAINED        = 5;
const unsigned char GEO_DB_INT_VAR_MIN                = 6;
const unsigned char GEO_DB_INT_VAR_MAX                = 7;
const unsigned char GEO_DB_INT_VAR_STEP                = 8;



///////////////////////////////////////////////////////////////////
// DB_DSK_STRING_VAR  Record Field Ids
//
const unsigned char GEO_DB_STRING_VAR_NAME            = 1;
const unsigned char GEO_DB_STRING_VAR_VALUE            = 2;
const unsigned char GEO_DB_STRING_VAR_DEFAULT        = 3;
const unsigned char GEO_DB_STRING_VAR_FID            = 4;


///////////////////////////////////////////////////////////////////
// DB_DSK_BOOL_VAR  Record Field Ids
//
const unsigned char GEO_DB_BOOL_VAR_NAME            = 1;
const unsigned char GEO_DB_BOOL_VAR_VALUE            = 2;
const unsigned char GEO_DB_BOOL_VAR_DEFAULT            = 3;
const unsigned char GEO_DB_BOOL_VAR_FID                = 4;


///////////////////////////////////////////////////////////////////
// DB_DSK_LONG_VAR  Record Field Ids
//
const unsigned char GEO_DB_LONG_VAR_NAME            = 1;
const unsigned char GEO_DB_LONG_VAR_VALUE            = 2;
const unsigned char GEO_DB_LONG_VAR_DEFAULT            = 3;
const unsigned char GEO_DB_LONG_VAR_FID                = 4;
const unsigned char GEO_DB_LONG_VAR_CONSTRAINED        = 5;
const unsigned char GEO_DB_LONG_VAR_MIN                = 6;
const unsigned char GEO_DB_LONG_VAR_MAX                = 7;
const unsigned char GEO_DB_LONG_VAR_STEP            = 8;


///////////////////////////////////////////////////////////////////
// DB_DSK_DOUBLE_VAR  Record Field Ids
//
const unsigned char GEO_DB_DOUBLE_VAR_NAME                = 1;
const unsigned char GEO_DB_DOUBLE_VAR_VALUE                = 2;
const unsigned char GEO_DB_DOUBLE_VAR_DEFAULT            = 3;
const unsigned char GEO_DB_DOUBLE_VAR_FID                = 4;
const unsigned char GEO_DB_DOUBLE_VAR_CONSTRAINED        = 5;
const unsigned char GEO_DB_DOUBLE_VAR_MIN                = 6;
const unsigned char GEO_DB_DOUBLE_VAR_MAX                = 7;
const unsigned char GEO_DB_DOUBLE_VAR_STEP                = 8;




///////////////////////////////////////////////////////////////////
// DB_DSK_STATE_MACHINE  Record Field Ids
//
const unsigned char GEO_DB_STATE_MACHINE_NAME            = 1;




///////////////////////////////////////////////////////////////////
// DB_DSK_STATE  Record Field Ids (states of the State machine)
//
const unsigned char GEO_DB_STATE_NAME                    = 1;
const unsigned char GEO_DB_STATE_DEFAULT                = 2;
const unsigned char GEO_DB_STATE_POSITION                = 3;
const unsigned char GEO_DB_STATE_DEFAULT_TRANSITION        = 4;



///////////////////////////////////////////////////////////////////
// DB_DSK_TRANSITION  Record Field Ids (transitions of the State machine)
//
const unsigned char GEO_DB_TRANSITION_NAME                = 1;
const unsigned char GEO_DB_TRANSITION_SOURCE            = 2;
const unsigned char GEO_DB_TRANSITION_DESTINATION        = 3;
const unsigned char GEO_DB_TRANSITION_DURATION            = 4;
const unsigned char GEO_DB_TRANSITION_TRIGGER            = 5;
const unsigned char GEO_DB_TRANSITION_CONNECTORS        = 6;
const unsigned char GEO_DB_TRANSITION_RATIO                = 7;



///////////////////////////////////////////////////////////////////
// DB_DSK_STATE_MACHINE_ACTION  Record Field Ids
//
// Record structure for a State Machine Action with 2 States & 2 Transitions
// (where the 1st transition rule used a rotation & the second a translation)
// would be:
//        DB_DSK_STATE_MACHINE_ACTION
//        DB_DSK_PUSH_ACTION
//            DB_DSK_STATE_RULE_ACTION
//            DB_DSK_STATE_RULE_ACTION
//            DB_DSK_TRANSITION_RULE_ACTION
//            DB_DSK_PUSH_ACTION
//                DB_DSK_ROTATE_ACTION
//            DB_DSK_POP_ACTION
//            DB_DSK_TRANSITION_RULE_ACTION
//            DB_DSK_PUSH_ACTION
//                DB_DSK_TRANSLATE_ACTION
//            DB_DSK_POP_ACTION
//        DB_DSK_POP_ACTION
//
// if the first state had an optional set of actions to perform when 
// active it would have the following construct...
//        DB_DSK_STATE_MACHINE_ACTION
//        DB_DSK_PUSH_ACTION
//            DB_DSK_STATE_RULE_ACTION
//                DB_DSK_PUSH_ACTION
//                    DB_DSK_ARITHMETIC
//                    DB_DSK_VISIBILITY
//                DB_DSK_POP_ACTION
//            DB_DSK_STATE_RULE_ACTION
//            DB_DSK_TRANSITION_RULE_ACTION
//            DB_DSK_PUSH_ACTION
//                DB_DSK_ROTATE_ACTION
//            DB_DSK_POP_ACTION
//            DB_DSK_TRANSITION_RULE_ACTION
//            DB_DSK_PUSH_ACTION
//                DB_DSK_TRANSLATE_ACTION
//            DB_DSK_POP_ACTION
//        DB_DSK_POP_ACTION
const unsigned char GEO_DB_STATE_MACHINE_ACTION_NAME                    = 1;


///////////////////////////////////////////////////////////////////
// DB_DSK_STATE_RULE_ACTION  Record Field Ids
//
const unsigned char GEO_DB_STATE_RULE_ACTION_STATE_MACHINE_NAME            = 1;
const unsigned char GEO_DB_STATE_RULE_ACTION_STATE_NAME                    = 2;
const unsigned char GEO_DB_STATE_RULE_ACTION_HIDDEN                        = 3;
const unsigned char GEO_DB_STATE_RULE_ACTION_IDENTITY                    = 4;    // deprecated


///////////////////////////////////////////////////////////////////
// DB_DSK_TRANSITION_RULE_ACTION  Record Field Ids
//
const unsigned char GEO_DB_TRANSITION_RULE_ACTION_STATE_MACHINE_NAME    = 1;
const unsigned char GEO_DB_TRANSITION_RULE_ACTION_TRANSITION_NAME        = 2;
const unsigned char GEO_DB_TRANSITION_RULE_ACTION_HIDDEN                = 3;




///////////////////////////////////////////////////////////////////
// GEO_DSK_BEHAVIOR  Record Field Ids
//
const unsigned char GEO_DB_BEHAVIOR_NAME                = 1;



///////////////////////////////////////////////////////////////////
// DB_DSK_ROTATE_ACTION  Record Field Ids
//
const unsigned char GEO_DB_ROTATE_ACTION_INPUT_VAR          = 1;
const unsigned char GEO_DB_ROTATE_ACTION_OUTPUT_VAR         = 2; // not used
const unsigned char GEO_DB_ROTATE_ACTION_ORIGIN             = 3;
const unsigned char GEO_DB_ROTATE_ACTION_VECTOR             = 4;
const unsigned char GEO_DB_ROTATE_ACTION_DIR                = 5;
const unsigned char GEO_DB_ROTATE_ACTION_MAX                = 6;



///////////////////////////////////////////////////////////////////
// DB_DSK_CLAMP_ACTION  Record Field Ids
//
const unsigned char GEO_DB_CLAMP_ACTION_INPUT_VAR           = 1;
const unsigned char GEO_DB_CLAMP_ACTION_OUTPUT_VAR          = 2; 
const unsigned char GEO_DB_CLAMP_ACTION_MIN_VAL             = 3;
const unsigned char GEO_DB_CLAMP_ACTION_MAX_VAL             = 4;



///////////////////////////////////////////////////////////////////
// DB_DSK_RANGE_ACTION  Record Field Ids
//
const unsigned char GEO_DB_RANGE_ACTION_INPUT_VAR           = 1;
const unsigned char GEO_DB_RANGE_ACTION_OUTPUT_VAR          = 2; 
const unsigned char GEO_DB_RANGE_ACTION_IN_MIN_VAL          = 3;
const unsigned char GEO_DB_RANGE_ACTION_IN_MAX_VAL          = 4;
const unsigned char GEO_DB_RANGE_ACTION_OUT_MIN_VAL         = 5;
const unsigned char GEO_DB_RANGE_ACTION_OUT_MAX_VAL         = 6;



///////////////////////////////////////////////////////////////////
// DB_DSK_ARITHMETIC_ACTION  Record Field Ids
//
const unsigned char GEO_DB_ARITHMETIC_ACTION_INPUT_VAR    = 1;
const unsigned char GEO_DB_ARITHMETIC_ACTION_OUTPUT_VAR    = 2; 
const unsigned char GEO_DB_ARITHMETIC_ACTION_OP_TYPE    = 3;
const unsigned char GEO_DB_ARITHMETIC_ACTION_OPERAND_VALUE    = 4;
const unsigned char GEO_DB_ARITHMETIC_ACTION_OPERAND_VAR    = 5;



///////////////////////////////////////////////////////////////////
// DB_DSK_EQUATION_ACTION  Record Field Ids
//
const unsigned char GEO_DB_EQUATION_ACTION_INPUT_VAR    = 1;
const unsigned char GEO_DB_EQUATION_ACTION_OUTPUT_VAR   = 2; 
const unsigned char GEO_DB_EQUATION_ACTION_A_VAL        = 3;
const unsigned char GEO_DB_EQUATION_ACTION_C_VAL        = 4;
const unsigned char GEO_DB_EQUATION_ACTION_A_VAR        = 5;
const unsigned char GEO_DB_EQUATION_ACTION_C_VAR        = 6;



///////////////////////////////////////////////////////////////////
// DB_DSK_PERIODIC_ACTION  Record Field Ids
//
const unsigned char GEO_DB_PERIODIC_ACTION_TYPE            = 7;



///////////////////////////////////////////////////////////////////
// DB_DSK_TRIG_ACTION  Record Field Ids
//
const unsigned char GEO_DB_TRIG_ACTION_OP                = 7;




///////////////////////////////////////////////////////////////////
// DB_DSK_CONTINUOUS_ACTION  Record Field Ids
//
const unsigned char GEO_DB_CONTINUOUS_ACTION_INPUT_VAR              = 1;    // Not used
const unsigned char GEO_DB_CONTINUOUS_ACTION_OUTPUT_VAR             = 2; 
const unsigned char GEO_DB_CONTINUOUS_ACTION_UPDATE_TYPE            = 4;
const unsigned char GEO_DB_CONTINUOUS_ACTION_UPDATE_VAL             = 5;
const unsigned char GEO_DB_CONTINUOUS_ACTION_DEFAULT_VAL            = 6;
const unsigned char GEO_DB_CONTINUOUS_ACTION_MIN_VAL                = 7;
const unsigned char GEO_DB_CONTINUOUS_ACTION_MAX_VAL                = 8;
const unsigned char GEO_DB_CONTINUOUS_ACTION_DIRECTION              = 9;
const unsigned char GEO_DB_CONTINUOUS_ACTION_MINMAX_BEHAVIOR        = 10;




///////////////////////////////////////////////////////////////////
// DB_DSK_MOMENTARY_ACTION  Record Field Ids
//
const unsigned char GEO_DB_MOMENTARY_ACTION_INPUT_VAR               = 1;    // Not used
const unsigned char GEO_DB_MOMENTARY_ACTION_OUTPUT_VAR              = 2; 
const unsigned char GEO_DB_MOMENTARY_ACTION_DEFAULT_VAL             = 3;
const unsigned char GEO_DB_MOMENTARY_ACTION_VAL                     = 4;


///////////////////////////////////////////////////////////////////
// DB_DSK_NSTATE_ACTION  Record Field Ids
//
const unsigned char GEO_DB_NSTATE_ACTION_INPUT_VAR                  = 1;    // Not used
const unsigned char GEO_DB_NSTATE_ACTION_OUTPUT_VAR                 = 2; 
const unsigned char GEO_DB_NSTATE_ACTION_DIRECTION                  = 3;
const unsigned char GEO_DB_NSTATE_ACTION_BEHAVIOR                   = 4;
const unsigned char GEO_DB_NSTATE_ACTION_NUM_ITEMS                  = 5;
const unsigned char GEO_DB_NSTATE_ACTION_OUTPUT_VAR_TYPE            = 6;
const unsigned char GEO_DB_NSTATE_ACTION_VALS                       = 7;


///////////////////////////////////////////////////////////////////
// DB_DSK_ROTARY_DRAG_ACTION  Record Field Ids
//
const unsigned char GEO_DB_ROTARY_DRAG_ACTION_INPUT_VAR             = 1;    // Not used
const unsigned char GEO_DB_ROTARY_DRAG_ACTION_OUTPUT_VAR            = 2; 
const unsigned char GEO_DB_ROTARY_DRAG_ACTION_BEHAVIOR              = 3;
const unsigned char GEO_DB_ROTARY_DRAG_ACTION_ORIGIN                = 4;


///////////////////////////////////////////////////////////////////
// DB_DSK_TASK_ACTION  Record Field Ids
//
const unsigned char GEO_DB_TASK_ACTION_INPUT_VAR                    = 1;
const unsigned char GEO_DB_TASK_ACTION_OUTPUT_VAR                   = 2; 


///////////////////////////////////////////////////////////////////
// DB_DSK_VISIBILITY_ACTION  Record Field Ids
//
const unsigned char GEO_DB_VISIBILITY_ACTION_INPUT_VAR              = 1;
const unsigned char GEO_DB_VISIBILITY_ACTION_OUTPUT_VAR             = 2; // Not used



///////////////////////////////////////////////////////////////////
// DB_DSK_COLOR_RAMP_ACTION  Record Field Ids
//
const unsigned char GEO_DB_COLOR_RAMP_ACTION_INPUT_VAR              = 1;
const unsigned char GEO_DB_COLOR_RAMP_ACTION_OUTPUT_VAR             = 2; // Not used
const unsigned char GEO_DB_COLOR_RAMP_ACTION_COLOR_FROM_PALETTE     = 3;
const unsigned char GEO_DB_COLOR_RAMP_ACTION_TOP_COLOR_INDEX        = 4;
const unsigned char GEO_DB_COLOR_RAMP_ACTION_BOTTOM_COLOR_INDEX     = 5;
const unsigned char GEO_DB_COLOR_RAMP_ACTION_NUM_RAMPS              = 6;
const unsigned char GEO_DB_COLOR_RAMP_ACTION_TOP_COLOR              = 7;
const unsigned char GEO_DB_COLOR_RAMP_ACTION_BOTTOM_COLOR           = 8;
const unsigned char GEO_DB_COLOR_RAMP_ACTION_MATCH_COLUMNS          = 9;



///////////////////////////////////////////////////////////////////
// DB_DSK_COMPARE_ACTION  Record Field Ids
//
const unsigned char GEO_DB_COMPARE_ACTION_INPUT_VAR     = 1;
const unsigned char GEO_DB_COMPARE_ACTION_OUTPUT_VAR    = 2; 
const unsigned char GEO_DB_COMPARE_ACTION_OP_TYPE       = 3;
const unsigned char GEO_DB_COMPARE_ACTION_OPERAND_VALUE = 4;
const unsigned char GEO_DB_COMPARE_ACTION_OPERAND_VAR   = 5;


///////////////////////////////////////////////////////////////////
// DB_DSK_TRANSLATE_ACTION  Record Field Ids
//
const unsigned char GEO_DB_TRANSLATE_ACTION_INPUT_VAR   = 1;
const unsigned char GEO_DB_TRANSLATE_ACTION_OUTPUT_VAR  = 2; // not used
const unsigned char GEO_DB_TRANSLATE_ACTION_ORIGIN      = 3;
const unsigned char GEO_DB_TRANSLATE_ACTION_VECTOR      = 4;
const unsigned char GEO_DB_TRANSLATE_ACTION_DIR         = 5;
const unsigned char GEO_DB_TRANSLATE_ACTION_SCALAR      = 6;
const unsigned char GEO_DB_TRANSLATE_ACTION_MAX         = 7;



///////////////////////////////////////////////////////////////////
// DB_DSK_SCALE_ACTION  Record Field Ids
//
const unsigned char GEO_DB_SCALE_ACTION_INPUT_VAR       = 1;
const unsigned char GEO_DB_SCALE_ACTION_OUTPUT_VAR      = 2; // not used
const unsigned char GEO_DB_SCALE_ACTION_ORIGIN          = 3;
const unsigned char GEO_DB_SCALE_ACTION_VECTOR          = 4;
const unsigned char GEO_DB_SCALE_ACTION_DIR             = 5;
const unsigned char GEO_DB_SCALE_ACTION_SCALAR          = 6;
const unsigned char GEO_DB_SCALE_ACTION_SCALE_ALL       = 7;
const unsigned char GEO_DB_SCALE_ACTION_MAX             = 8;



///////////////////////////////////////////////////////////////////
// DB_DSK_STRING_CONTENT_ACTION  Record Field Ids
//
const unsigned char GEO_DB_STRING_CONTENT_ACTION_INPUT_VAR      = 1;
const unsigned char GEO_DB_STRING_CONTENT_ACTION_OUTPUT_VAR     = 2; // not used
const unsigned char GEO_DB_STRING_CONTENT_ACTION_PADDING_TYPE   = 3;
const unsigned char GEO_DB_STRING_CONTENT_ACTION_PAD_FOR_SIGN   = 4;
const unsigned char GEO_DB_STRING_CONTENT_ACTION_FORMAT         = 5;


///////////////////////////////////////////////////////////////////
// DB_DSK_STRING_COPY_ACTION  Record Field Ids
//
const unsigned char GEO_DB_STRING_COPY_ACTION_INPUT_VAR            = 1;
const unsigned char GEO_DB_STRING_COPY_ACTION_OUTPUT_VAR        = 2; // not used


///////////////////////////////////////////////////////////////////
// DB_DSK_CONDITIONAL_ACTION  Record Field Ids
//
// Record structure for compound if-the-else block of Actions
// The conditional action has an optional list of actions if the
// input var passes the conditional test (Not equal zero) and an
// alternative list of actions if the input var does not pass
// the conditional test (the else selction). In a simple case
// where the conditional action had one rotate action on pass &
// a range & rotate on fail the disk rep would be...
// would be:
//        DB_DSK_CONDITIONAL_ACTION
//      DB_DSK_PUSH_ACTION              // indicates list of child actions
//            DB_DSK_IF_CONDITION       // all following are for If list
//            DB_DSK_ROTATE
//            DB_DSK_ELSE_CONDITION     // all following are for Else list
//            DB_DSK_RANGE
//            DB_DSK_ROTATE
//        DB_DSK_POP_ACTION
//
const unsigned char GEO_DB_CONDITIONAL_ACTION_INPUT_VAR    = 1;
const unsigned char GEO_DB_CONDITIONAL_ACTION_OUTPUT_VAR= 2; // not used


///////////////////////////////////////////////////////////////////
// DB_DSK_DCS_ACTION  Record Field Ids
//
const unsigned char GEO_DB_DCS_ACTION_INPUT_VAR                 = 1; // not used
const unsigned char GEO_DB_DCS_ACTION_OUTPUT_VAR                = 2; // not used
const unsigned char GEO_DB_DCS_ACTION_ORIGIN                    = 3;
const unsigned char GEO_DB_DCS_ACTION_XPOS                      = 4;
const unsigned char GEO_DB_DCS_ACTION_ZPOS                      = 5;
const unsigned char GEO_DB_DCS_ACTION_VECTOR                    = 6;
const unsigned char GEO_DB_DCS_ACTION_TRANSLATE_X_VAR           = 7;
const unsigned char GEO_DB_DCS_ACTION_TRANSLATE_Y_VAR           = 8;
const unsigned char GEO_DB_DCS_ACTION_TRANSLATE_Z_VAR           = 9;
const unsigned char GEO_DB_DCS_ACTION_ROTATE_X_VAR              = 10;
const unsigned char GEO_DB_DCS_ACTION_ROTATE_Y_VAR              = 11;
const unsigned char GEO_DB_DCS_ACTION_ROTATE_Z_VAR              = 12;
const unsigned char GEO_DB_DCS_ACTION_SCALE_X_VAR               = 13;
const unsigned char GEO_DB_DCS_ACTION_SCALE_Y_VAR               = 14;
const unsigned char GEO_DB_DCS_ACTION_SCALE_Z_VAR               = 15;







///////////////////////////////////////////////////////////////////
// DB_DSK_DISCRETE_ACTION  Record Field Ids
//
const unsigned char GEO_DB_DISCRETE_ACTION_INPUT_VAR            = 1;
const unsigned char GEO_DB_DISCRETE_ACTION_OUTPUT_VAR           = 2; 
const unsigned char GEO_DB_DISCRETE_ACTION_NUM_ITEMS            = 3;
const unsigned char GEO_DB_DISCRETE_ACTION_OUTPUT_VAR_TYPE      = 4;
const unsigned char GEO_DB_DISCRETE_ACTION_MIN_VALS             = 5;
const unsigned char GEO_DB_DISCRETE_ACTION_MAX_VALS             = 6;
const unsigned char GEO_DB_DISCRETE_ACTION_MAP_VALS             = 7;





/** Record identifiers can be read as ints or this structure. All subsequent
 *  fields are considered part of this Node until an special EOF(ield) record
 *  is found. The only exception to this rule id DB_DSK_PUSH & DB_DSK_POP
 *  which have no fields. User parse code should expect another REcord header
 *  immediately after reading the Push/Pop record.
 */
struct GEO_DB_API geoExtensionDefRec
{

    /** The Node type for which this extension exists */
    unsigned int            nodetype;                    //  4 bytes 

    /** The data type of the extension - defined in terms of GEO_DB_DATATYPE_INT
     *  GEO_DB_DATATYPE_FLOAT, GEO_DB_DATATYPE_BOOL etc.
     */
    unsigned char            datatype;                   //  1 byte

    /** The extension can have a special "sub type" value. This could be
     *  values like GEO_DB_EXT_MENU_ITEM which (when associated with a datatype
     *  of GEO_DB_DATATYPE_BOOL means that this extension will be accessed as one
     *  of many in an option menu
     */
    unsigned char            subdatatype;                //  1 bytes 

    /** The User ID (uid) is the optional value provided (in code) by the user 
     *  to identify this particular extension. Users can search & retrieve 
     *  extension values based on this user ID number.
     */
    unsigned short            uid;                        //  2 bytes

    /** The name of the extension.
     *
     *  Note that the "name" field is sized for the Geo 1.0 maximum property
     *  label length that can be accomodated. The name field is also used to
     *  encode the name/label of the option menu when the extension is flagged
     *  as one of those. The following rules should be taken into consideration:
     *  1. When the extension is an option menu (datatype=GEO_DB_DATATYPE_BOOL
     *     and subdatatype=GEO_DB_EXT_MENU_ITEM) then the name field is
     *     divided up as 15 chars for the option menu title, 8 chars for
     *     this particular option menu's label and 1 char for the terminator
     * 2.  When the extension is a text field or boolean toggle value - it is
     *     recommended that only the 15 chars for the field label be used - 
     *     setting a 23 char-length  label for a text input field will be a 
     *     waste of time, as it will get truncated on display anyway.
     */
    char                    name[24];       // 24 bytes
                                            //-----------------------------
};                                          // total:       32 bytes




#endif // __GEO_FORMAT_H__

