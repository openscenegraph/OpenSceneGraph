/*===========================================================================*\

NAME:			geoFormat.h

DESCRIPTION:	Native Format struct definitions, tokens & functionc

AUTHOR:			Andy Bushnell

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

//#include "geoCore.h"
#define GEO_DB_API



//
// Constants to define the Node disk records. Used in RecordToken.id
//
const unsigned int DB_DSK_HEADER						= 101;
const unsigned int DB_DSK_GROUP							= 102;

const unsigned int DB_DSK_SEQUENCE						= 104;
const unsigned int DB_DSK_LOD							= 105;
const unsigned int DB_DSK_GEODE							= 106;
const unsigned int DB_DSK_POLYGON						= 107;
const unsigned int DB_DSK_MESH							= 108;	// Unused - Possible Future expansion
const unsigned int DB_DSK_CUBE							= 109;	// Unused - Possible Future expansion
const unsigned int DB_DSK_SPHERE						= 110;	// Unused - Possible Future expansion
const unsigned int DB_DSK_CONE							= 111;	// Unused - Possible Future expansion
const unsigned int DB_DSK_CYLINDER						= 112;	// Unused - Possible Future expansion
const unsigned int DB_DSK_VERTEX						= 113;
const unsigned int DB_DSK_PUSH							= 114;
const unsigned int DB_DSK_POP							= 115;
const unsigned int DB_DSK_TEXTURE						= 116;
const unsigned int DB_DSK_MATERIAL						= 117;
const unsigned int DB_DSK_VIEW							= 118;
const unsigned int DB_DSK_EXTENSION_LIST				= 119;
const unsigned int DB_DSK_SWITCH						= 120;
const unsigned int DB_DSK_TEXT							= 121;
const unsigned int DB_DSK_BASE_GROUP					= 122;
const unsigned int DB_DSK_BASE_SURFACE					= 123;

const unsigned int DB_DSK_BEHAVIOR						= 124;

const unsigned int DB_DSK_CLAMP_ACTION					= 125;	
const unsigned int DB_DSK_RANGE_ACTION					= 126;		
const unsigned int DB_DSK_ROTATE_ACTION					= 127;		
const unsigned int DB_DSK_TRANSLATE_ACTION				= 128;	
const unsigned int DB_DSK_SCALE_ACTION					= 129;					
const unsigned int DB_DSK_ARITHMETIC_ACTION				= 130;			
const unsigned int DB_DSK_LOGIC_ACTION					= 131;				
const unsigned int DB_DSK_CONDITIONAL_ACTION			= 132;		
const unsigned int DB_DSK_LOOPING_ACTION				= 133;				
const unsigned int DB_DSK_COMPARE_ACTION				= 134;	   	
const unsigned int DB_DSK_VISIBILITY_ACTION				= 135;	   		
const unsigned int DB_DSK_STRING_CONTENT_ACTION			= 136;
		   		
const unsigned int DB_DSK_INTERNAL_VARS					= 137;  		   		
const unsigned int DB_DSK_LOCAL_VARS					= 138; 		   		
const unsigned int DB_DSK_EXTERNAL_VARS					= 139;
 	   		
const unsigned int DB_DSK_FLOAT_VAR						= 140; 	
const unsigned int DB_DSK_INT_VAR						= 141; 
const unsigned int DB_DSK_LONG_VAR						= 142; 
const unsigned int DB_DSK_DOUBLE_VAR					= 143; 	
const unsigned int DB_DSK_BOOL_VAR						= 144;
 
const unsigned int DB_DSK_CONDITIONAL_BEGIN_CHILDREN	= 145;	// complex nested action branch mechanism
const unsigned int DB_DSK_IF_CONDITION					= 146;
const unsigned int DB_DSK_ELSE_CONDITION				= 147;
const unsigned int DB_DSK_CONDITIONAL_END_CHILDREN		= 148;

const unsigned int DB_DSK_COLOR_PALETTE					= 149;
const unsigned int DB_DSK_COLOR_RAMP_ACTION				= 150;

const unsigned int DB_DSK_FLOAT2_VAR					= 151; 
const unsigned int DB_DSK_FLOAT3_VAR					= 152; 	
const unsigned int DB_DSK_FLOAT4_VAR					= 153; 
	
const unsigned int DB_DSK_LINEAR_ACTION					= 154; 	
const unsigned int DB_DSK_TASK_ACTION					= 155; 
	
const unsigned int DB_DSK_PERIODIC_ACTION				= 156; 
const unsigned int DB_DSK_PERIODIC2_ACTION				= 157;	// Redundant record - Periodic supports types 1 & 2
const unsigned int DB_DSK_TRIG_ACTION					= 158;  
const unsigned int DB_DSK_INVERSE_ACTION				= 159; 
const unsigned int DB_DSK_TRUNCATE_ACTION				= 160;
const unsigned int DB_DSK_ABS_ACTION					= 161; 
const unsigned int DB_DSK_IF_THEN_ELSE_ACTION			= 162; 	// simple variable value check

const unsigned int DB_DSK_DCS_ACTION					= 163; 	

const unsigned int DB_DSK_INSTANCE						= 164;

const unsigned int DB_DSK_COORD_POOL					= 165;

const unsigned int DB_DSK_LIGHTPT						= 166;
const unsigned int DB_DSK_EXTERNAL						= 167;

const unsigned int DB_DSK_NORMAL_POOL					= 168;

const unsigned int DB_DSK_DISCRETE_ACTION				= 169;

const unsigned int DB_DSK_STRING_VAR					= 170;    		
const unsigned int DB_DSK_STRING_COPY_ACTION			= 171;

const unsigned int DB_DSK_PAGE							= 172;

const unsigned int DB_DSK_SQRT_ACTION			        = 173; 	
const unsigned int DB_DSK_LOG_ACTION			        = 174; 	

const unsigned int DB_DSK_PLANE_TEXTURE_MAPPING_INFO	= 175;
const unsigned int DB_DSK_CYLINDER_TEXTURE_MAPPING_INFO	= 176;	// not implemented in 1.0
const unsigned int DB_DSK_SPHERE_TEXTURE_MAPPING_INFO	= 177;	// not implemented in 1.0
const unsigned int DB_DSK_GRID_TEXTURE_MAPPING_INFO		= 178;	// not implemented in 1.0

const unsigned int DB_DSK_PERSPECTIVE_GRID_INFO			= 179;
const unsigned int DB_DSK_XY_GRID_INFO					= 180;	// not implemented in 1.0
const unsigned int DB_DSK_XZ_GRID_INFO					= 181;	// not implemented in 1.0
const unsigned int DB_DSK_YZ_GRID_INFO					= 182;	// not implemented in 1.0
					
					




//
// Constants to define the data types supported in the format
//
const unsigned char DB_CHAR							    = 1;
const unsigned char DB_SHORT						    = 2;
const unsigned char DB_INT							    = 3;
const unsigned char DB_FLOAT						    = 4;
const unsigned char DB_LONG							    = 5;
const unsigned char DB_DOUBLE						    = 6;
const unsigned char DB_VEC2F						    = 7;
const unsigned char DB_VEC3F						    = 8;
const unsigned char DB_VEC4F						    = 9;
const unsigned char DB_VEC2I						    = 10;
const unsigned char DB_VEC3I						    = 11;
const unsigned char DB_VEC4I						    = 12;
const unsigned char DB_VEC16F						    = 13;
const unsigned char DB_VEC2D						    = 14;
const unsigned char DB_VEC3D						    = 15;
const unsigned char DB_VEC4D						    = 16;
const unsigned char DB_VEC16D						    = 17;
const unsigned char DB_VRTX_STRUCT					    = 18; // deprecated (obsolete) after 0.982
const unsigned char DB_UINT							    = 19;
const unsigned char DB_USHORT						    = 20;
const unsigned char DB_UCHAR						    = 21;
const unsigned char DB_ULONG						    = 22;
const unsigned char DB_EXT_STRUCT					    = 23;
const unsigned char DB_SHORT_WITH_PADDING			    = 24;
const unsigned char DB_CHAR_WITH_PADDING			    = 25;
const unsigned char DB_USHORT_WITH_PADDING			    = 26;
const unsigned char DB_UCHAR_WITH_PADDING			    = 27;
const unsigned char DB_BOOL_WITH_PADDING			    = 28;
const unsigned char DB_EXTENDED_FIELD_STRUCT			= 31;
const unsigned char DB_VEC4UC							= 32; // array of 4 unsigned chars
const unsigned char DB_DISCRETE_MAPPING_STRUCT			= 33;
const unsigned char DB_BITFLAGS							= 34;

//
// Constants to define sizeof() values
//
const unsigned char SIZEOF_FIELD_STRUCT					= 4;
const unsigned char SIZEOF_EXTENDED_FIELD_STRUCT		= 8;
const unsigned char SIZEOF_CHAR				            = 1;
const unsigned char SIZEOF_SHORT			            = 2;
const unsigned char SIZEOF_INT				            = 4;
const unsigned char SIZEOF_FLOAT			            = 4;
const unsigned char SIZEOF_LONG				            = 4;
const unsigned char SIZEOF_ULONG				        = 4;
const unsigned char SIZEOF_DOUBLE			            = 8;
const unsigned char SIZEOF_VEC2F			            = (SIZEOF_FLOAT*2);
const unsigned char SIZEOF_VEC3F			            = (SIZEOF_FLOAT*3);
const unsigned char SIZEOF_VEC4F			            = (SIZEOF_FLOAT*4);
const unsigned char SIZEOF_VEC16F			            = (SIZEOF_FLOAT*16);
const unsigned char SIZEOF_VEC2I			            = (SIZEOF_INT*2);
const unsigned char SIZEOF_VEC3I			            = (SIZEOF_INT*3);
const unsigned char SIZEOF_VEC4I			            = (SIZEOF_INT*4);
const unsigned char SIZEOF_VEC2D			            = (SIZEOF_DOUBLE*2);
const unsigned char SIZEOF_VEC3D			            = (SIZEOF_DOUBLE*3);
const unsigned char SIZEOF_VEC4D			            = (SIZEOF_DOUBLE*4);
const unsigned char SIZEOF_VEC16D			            = (SIZEOF_DOUBLE*16);
const unsigned char SIZEOF_VRTX_STRUCT		            = 32;
const unsigned char SIZEOF_EXT_STRUCT		            = 32;
const unsigned char SIZEOF_UCHAR			            = (SIZEOF_CHAR);
const unsigned char SIZEOF_USHORT			            = (SIZEOF_SHORT);
const unsigned char SIZEOF_UINT				            = (SIZEOF_INT);
const unsigned char SIZEOF_VEC4UC			            = (SIZEOF_INT);
const unsigned char SIZEOF_SHORT_WITH_PADDING			= (SIZEOF_INT);
const unsigned char SIZEOF_CHAR_WITH_PADDING			= (SIZEOF_INT);
const unsigned char SIZEOF_USHORT_WITH_PADDING			= (SIZEOF_INT);
const unsigned char SIZEOF_UCHAR_WITH_PADDING			= (SIZEOF_INT);
const unsigned char SIZEOF_BOOL_WITH_PADDING			= (SIZEOF_INT);
const unsigned char SIZEOF_DISCRETE_MAPPING_STRUCT		= 12;
const unsigned char SIZEOF_BITFLAGS						= (SIZEOF_INT);



// Is this really meant to be unsigned????
const unsigned short MIN_CHAR_VAL						= (unsigned short)(-128);
const unsigned short MAX_CHAR_VAL			            = 127;
const unsigned short MAX_UCHAR_VAL			            = 255;
// Is this really meant to be unsigned????
const unsigned short MIN_SHORT_VAL			            = (unsigned short)(-32768);
const unsigned short MAX_SHORT_VAL			            = 32767;
const unsigned short MAX_USHORT_VAL			            = 65535;





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
	unsigned int		id; // e.g. DB_DSK_HEADER etc.
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
	unsigned char		id;		// field ID for record

	/** The data type of the field coming up */
	unsigned char		type;	// DB_INT, etc.

	/** How many of the data types (described above) must be read */
	unsigned short		num;	// How many of them follow

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
 *      DB_UCHAR	id		GEO_DB_VRTX_COORDS
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
 *      DB_USHORT	id		GEO_DB_VRTX_COORDS
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
 *      DB_UCHAR	id		GEO_DB_EXTENDED_FIELD
 *      DB_UCHAR    type    DB_EXTENDED_FIELD_STRUCT  
 *      DB_USHORT   num     1
 *  }
 *
 *  followed by...
 *
 *  geoExtendedFieldHeader
 *  {
 *      DB_USHORT	id		GEO_DB_SOME_FUTURE_USHORT_ID
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
	unsigned short		id;		// field ID for record

	/** The data type of the field coming up */
	unsigned short		type;	// DB_INT, etc.

	/** How many of the data types (described above) must be read */
	unsigned int		num;	// How many of them follow

};




///////////////////////////////////////////////////////////////////////////////
// Constant to define the last field types
//
const unsigned char GEO_DB_LAST_FIELD				= 0;

///////////////////////////////////////////////////////////////////////////////
// Common field types for all Nodes
//
const unsigned char GEO_DB_USER_EXT_VALUE_FIELD		= 1;
const unsigned char GEO_DB_PADDING					= 252;
const unsigned char GEO_DB_TRANSFORM_FIELD			= 253;
const unsigned char GEO_DB_EXTENDED_FIELD			= 254;
const unsigned char GEO_DB_COMMENT_FIELD			= 255;






///////////////////////////////////////////////////////////////////////////////
// GEO_DB_HEADER Record - Field Ids
//
const unsigned char GEO_DB_HDR_EXT					= 1;	// alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_HDR_UNITS				= 2;
const unsigned char GEO_DB_HDR_BBOX					= 3;	
const unsigned char GEO_DB_HDR_NAME					= 5;
const unsigned char GEO_DB_HDR_VERSION				= 8;
const unsigned char GEO_DB_HDR_EXT_TEMPLATE			= 9;
const unsigned char GEO_DB_HDR_UP_AXIS				= 12;
const unsigned char GEO_DB_HDR_EXTENDED				= 254;	// alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_HDR_COMMENT				= 255;	// alias for GEO_DB_COMMENT_FIELD


///////////////////////////////////////////////////////////////////////////////
// DB_DSK_COORD_POOL Record - Field Ids
//
const unsigned char GEO_DB_COORD_POOL_SIZE			= 1;
const unsigned char GEO_DB_COORD_POOL_VALUES		= 2;
const unsigned char GEO_DB_COORD_POOL_SCALE			= 3;
const unsigned char GEO_DB_COORD_POOL_OFFSET		= 4;

///////////////////////////////////////////////////////////////////////////////
// DB_DSK_NORMAL_POOL Record - Field Ids
//
const unsigned char GEO_DB_NORMAL_POOL_SIZE			= 1;
const unsigned char GEO_DB_NORMAL_POOL_VALUES		= 2;


///////////////////////////////////////////////////////////////////////////////
// DB_DSK_MATERIAL Record - Field Ids
//
const unsigned char GEO_DB_MAT_AMBIENT				= 1;
const unsigned char GEO_DB_MAT_DIFFUSE				= 2;
const unsigned char GEO_DB_MAT_SPECULAR				= 3;
const unsigned char GEO_DB_MAT_SHININESS			= 4;
const unsigned char GEO_DB_MAT_NAME					= 5;
const unsigned char GEO_DB_MAT_EMISSIVE				= 6;


///////////////////////////////////////////////////////////////////////////////
// DB_DSK_COLOR_PALETTE Record - Field Ids
//
const unsigned char GEO_DB_COLOR_PALETTE_HIGHEST_INTENSITIES = 1;


///////////////////////////////////////////////////////////////////////////////
// DB_DSK_TEXTURE Record - Field Ids
//
const unsigned char GEO_DB_TEX_WRAPS				= 1;
const unsigned char GEO_DB_TEX_WRAPT				= 2;
const unsigned char GEO_DB_TEX_MAGFILTER			= 3;
const unsigned char GEO_DB_TEX_MINFILTER			= 4;
const unsigned char GEO_DB_TEX_ENV					= 5;
const unsigned char GEO_DB_TEX_FILE_NAME			= 6;


///////////////////////////////////////////////////////////////////////////////
// DB_DSK_VIEW Record - Field Ids Ids
//
const unsigned char GEO_DB_VIEW_NEAR				= 1;
const unsigned char GEO_DB_VIEW_FAR					= 2;
const unsigned char GEO_DB_VIEW_POS					= 3;
const unsigned char GEO_DB_VIEW_CEN					= 4;
const unsigned char GEO_DB_VIEW_TRACKBALL			= 5;


///////////////////////////////////////////////////////////////////////////////
// DB_DSK_PERSPECTIVE_GRID_INFO Record 
// DB_DSK_XY_GRID_INFO Record 
// DB_DSK_XZ_GRID_INFO Record 
// DB_DSK_YZ_GRID_INFO Record - Field Ids Ids
//
const unsigned char GEO_DB_GRID_ON					= 1;
const unsigned char GEO_DB_GRID_ZBUFFER				= 2;
const unsigned char GEO_DB_GRID_SNAP				= 3;
const unsigned char GEO_DB_GRID_OVER				= 4;
const unsigned char GEO_DB_GRID_MAJOR				= 5;
const unsigned char GEO_DB_GRID_MINOR				= 6;
const unsigned char GEO_DB_GRID_NUM_CELLS			= 7;
const unsigned char GEO_DB_GRID_POS					= 8;
const unsigned char GEO_DB_GRID_MATRIX				= 9;



///////////////////////////////////////////////////////////////////////////////
// GEO_DB_GROUP Record - Field Ids IDs
//
const unsigned char GEO_DB_GRP_EXT					= 1;   // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_GRP_BBOX					= 2;
const unsigned char GEO_DB_GRP_NAME					= 3;
const unsigned char GEO_DB_GRP_INSTANCE_DEF			= 4;
const unsigned char GEO_DB_GRP_FLAG_SHOW_BBOX		= 5;
const unsigned char GEO_DB_GRP_TRANSFORM			= 253;
const unsigned char GEO_DB_GRP_EXTENDED				= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_GRP_COMMENT				= 255; // alias for GEO_DB_COMMENT_FIELD


///////////////////////////////////////////////////////////////////////////////
// GEO_DB_LOD Record - Field Ids IDs
//
const unsigned char GEO_DB_LOD_EXT					= 1; // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_LOD_IN					= 2;
const unsigned char GEO_DB_LOD_OUT					= 3;
const unsigned char GEO_DB_LOD_CENTER				= 4;
const unsigned char GEO_DB_LOD_CALC					= 5; // depricated Field only some pre 1.0 databases should have this field
const unsigned char GEO_DB_LOD_NAME					= 6;
const unsigned char GEO_DB_LOD_INSTANCE_DEF			= 7;
const unsigned char GEO_DB_LOD_FREEZE_CENTER		= 8;
const unsigned char GEO_DB_LOD_TRANSFORM			= 253;
const unsigned char GEO_DB_LOD_EXTENDED				= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_LOD_COMMENT				= 255; // alias for GEO_DB_COMMENT_FIELD


///////////////////////////////////////////////////////////////////////////////
// GEO_DB_SEQUENCE Record - Field Ids IDs
//
const unsigned char GEO_DB_SEQUENCE_EXT				= 1; // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_SEQUENCE_NAME			= 2;
const unsigned char GEO_DB_SEQUENCE_MODE			= 3;
const unsigned char GEO_DB_SEQUENCE_ACTIVE			= 4;
const unsigned char GEO_DB_SEQUENCE_INSTANCE_DEF	= 5;
const unsigned char GEO_DB_SEQUENCE_FRAME_TIME		= 6;
const unsigned char GEO_DB_SEQUENCE_USE_FRAME_TIME	= 7;
const unsigned char GEO_DB_SEQUENCE_TRANSFORM		= 253;
const unsigned char GEO_DB_SEQUENCE_EXTENDED		= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_SEQUENCE_COMMENT			= 255; // alias for GEO_DB_COMMENT_FIELD


///////////////////////////////////////////////////////////////////////////////
// GEO_DB_INSTANCE Record - Field Ids IDs
//
const unsigned char GEO_DB_INSTANCE_EXT				= 1; // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_INSTANCE_NAME			= 2;
const unsigned char GEO_DB_INSTANCE_DEF				= 3;
const unsigned char GEO_DB_INSTANCE_TRANSFORM		= 253;
const unsigned char GEO_DB_INSTANCE_EXTENDED		= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_INSTANCE_COMMENT			= 255; // alias for GEO_DB_COMMENT_FIELD


///////////////////////////////////////////////////////////////////////////////
// GEO_DB_SWITCH Record - Field Ids IDs
//
const unsigned char GEO_DB_SWITCH_EXT				= 1; // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_SWITCH_CURRENT_MASK		= 2;
const unsigned char GEO_DB_SWITCH_MASK_WIDTH		= 3;// Not used
const unsigned char GEO_DB_SWITCH_NUM_MASKS			= 4; 
const unsigned char GEO_DB_SWITCH_MASKS				= 5;
const unsigned char GEO_DB_SWITCH_NAME				= 6;
const unsigned char GEO_DB_SWITCH_INSTANCE_DEF		= 7;
const unsigned char GEO_DB_SWITCH_TRANSFORM			= 253;
const unsigned char GEO_DB_SWITCH_EXTENDED			= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_SWITCH_COMMENT			= 255; // alias for GEO_DB_COMMENT_FIELD


///////////////////////////////////////////////////////////////////////////////
// GEO_DB_PAGE Record - Field Ids IDs
//
const unsigned char GEO_DB_PAGE_EXT					= 1;   // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_PAGE_NAME				= 2;
const unsigned char GEO_DB_PAGE_ACTIVE_CHILD		= 3;
const unsigned char GEO_DB_PAGE_TRANSFORM			= 253;
const unsigned char GEO_DB_PAGE_EXTENDED			= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_PAGE_COMMENT				= 255; // alias for GEO_DB_COMMENT_FIELD



///////////////////////////////////////////////////////////////////////////////
// GEO_DB_BASE_GROUP Record - Field Ids IDs
//
// It is important to note that user extended Group fields begin with a field
// ID of 20 (GEO_DB_BASE_GROUP_START_EXTERNAL). This allows Geo to add 
// additional fields which will be common to all derived groups up to this
// number
const unsigned char GEO_DB_BASE_GROUP_EXT			= 1; // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_BASE_GROUP_CLASSNAME		= 2;
const unsigned char GEO_DB_BASE_GROUP_EXTENDED_TYPE	= 3;
const unsigned char GEO_DB_BASE_GROUP_NAME			= 4;
const unsigned char GEO_DB_BASE_GROUP_INSTANCE_DEF	= 5;
const unsigned char GEO_DB_BASE_GROUP_PLUGIN_REQUIRED=6;
const unsigned char GEO_DB_BASE_GROUP_START_EXTERNAL= 20;
const unsigned char GEO_DB_BASE_GROUP_TRANSFORM		= 253;
const unsigned char GEO_DB_BASE_GROUP_EXTENDED		= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_BASE_GROUP_COMMENT		= 255; // alias for GEO_DB_COMMENT_FIELD



///////////////////////////////////////////////////////////////////////////////
// GEO_DB_BASE_SURFACE Record - Field Ids IDs
//
// It is important to note that user extended Surface fields begin with a field
// ID of 20 (GEO_DB_BASE_SURFACE_START_EXTERNAL). This allows Geo to add 
// additional fields which will be common to all derived groups up to this
// number
const unsigned char GEO_DB_BASE_SURFACE_EXT			    = 1; // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_BASE_SURFACE_CLASSNAME		= 2;
const unsigned char GEO_DB_BASE_SURFACE_EXTENDED_TYPE	= 3;
const unsigned char GEO_DB_BASE_SURFACE_NAME			= 4;
const unsigned char GEO_DB_BASE_SURFACE_PLUGIN_REQUIRED	= 5;
const unsigned char GEO_DB_BASE_SURFACE_START_EXTERNAL  = 20;
const unsigned char GEO_DB_BASE_SURFACE_EXTENDED		= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_BASE_SURFACE_COMMENT		    = 255; // alias for GEO_DB_COMMENT_FIELD



///////////////////////////////////////////////////////////////////////////////
// GEO_DB_RENDERGROUP Record - Field Ids IDs
//
const unsigned char GEO_DB_RENDERGROUP_EXT			= 1; // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_RENDERGROUP_MAT			= 2;
const unsigned char GEO_DB_RENDERGROUP_NAME			= 3;
const unsigned char GEO_DB_RENDERGROUP_INSTANCE_DEF	= 4;
const unsigned char GEO_DB_RENDERGROUP_FLAG_SHOW_BBOX=5;   // deprecated in 0.9.9.10
const unsigned char GEO_DB_RENDERGROUP_IS_TERRAIN	 =6;   
const unsigned char GEO_DB_RENDERGROUP_IS_BILLBOARD  =7;   
const unsigned char GEO_DB_RENDERGROUP_TRANSFORM	= 253;
const unsigned char GEO_DB_RENDERGROUP_EXTENDED		= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_RENDERGROUP_COMMENT		= 255; // alias for GEO_DB_COMMENT_FIELD




///////////////////////////////////////////////////////////////////////////////
// GEO_DB_POLYGON Record - Field Ids IDs
//
const unsigned char GEO_DB_POLY_EXT					= 1; // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_POLY_NORMAL				= 2;
const unsigned char GEO_DB_POLY_CENTER				= 3;
const unsigned char GEO_DB_POLY_PACKED_COLOR		= 4;
const unsigned char GEO_DB_POLY_TEX					= 5;
const unsigned char GEO_DB_POLY_NAME				= 6;
const unsigned char GEO_DB_POLY_DSTYLE				= 7;
const unsigned char GEO_DB_POLY_SHADEMODEL			= 8;
const unsigned char GEO_DB_POLY_USE_MATERIAL_DIFFUSE= 9;
const unsigned char GEO_DB_POLY_USE_VERTEX_COLORS   = 10;
const unsigned char GEO_DB_POLY_COLOR_INDEX			= 11;
const unsigned char GEO_DB_POLY_PT_SIZE				= 12;
const unsigned char GEO_DB_POLY_LINE_WIDTH			= 13;
const unsigned char GEO_DB_POLY_EXTENDED			= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_POLY_COMMENT				= 255; // alias for GEO_DB_COMMENT_FIELD



///////////////////////////////////////////////////////////////////
// DB_DSK_PLANE_TEXTURE_MAPPING_INFO  Record Field Ids
///////////////////////////////////////////////////////////////////
const unsigned char GEO_DB_PLANE_TEXTURE_MAPPING_INFO_ORIGIN_PT		= 1;
const unsigned char GEO_DB_PLANE_TEXTURE_MAPPING_INFO_U_AXIS_PT		= 2;
const unsigned char GEO_DB_PLANE_TEXTURE_MAPPING_INFO_V_AXIS_PT		= 3;



///////////////////////////////////////////////////////////////////////////////
// GEO_DB_LIGHTPT Record - Field Ids IDs
//
const unsigned char GEO_DB_LIGHTPT_EXT				= 1; // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_LIGHTPT_TYPE				= 2;
const unsigned char GEO_DB_LIGHTPT_DIRECTION_VECTOR	= 3;
const unsigned char GEO_DB_LIGHTPT_PACKED_COLOR		= 4;
const unsigned char GEO_DB_LIGHTPT_BACK_PACKED_COLOR= 5;
const unsigned char GEO_DB_LIGHTPT_MIN_PIXEL_SIZE	= 6;
const unsigned char GEO_DB_LIGHTPT_MAX_PIXEL_SIZE	= 7;
const unsigned char GEO_DB_LIGHTPT_HORIZ_LOBE_ANGLE	= 8;
const unsigned char GEO_DB_LIGHTPT_VERT_LOBE_ANGLE	= 9;
const unsigned char GEO_DB_LIGHTPT_DAY_DISPLAY		= 10;
const unsigned char GEO_DB_LIGHTPT_DUSK_DISPLAY		= 11;
const unsigned char GEO_DB_LIGHTPT_NIGHT_DISPLAY	= 12;
const unsigned char GEO_DB_LIGHTPT_NAME				= 13;
const unsigned char GEO_DB_LIGHTPT_COLOR_INDEX		= 14;
const unsigned char GEO_DB_LIGHTPT_BACK_COLOR_INDEX	= 15;
const unsigned char GEO_DB_LIGHTPT_EXTENDED			= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_LIGHTPT_COMMENT			= 255; // alias for GEO_DB_COMMENT_FIELD



///////////////////////////////////////////////////////////////////////////////
// GEO_DB_TEXT Record - Field Ids IDs
//
const unsigned char GEO_DB_TEXT_EXT					= 1;  // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_TEXT_NAME				= 2;
const unsigned char GEO_DB_TEXT_TYPE				= 3;
const unsigned char GEO_DB_TEXT_STRING				= 4;
const unsigned char GEO_DB_TEXT_JUSTIFICATION		= 5;
const unsigned char GEO_DB_TEXT_DIRECTION			= 6;
const unsigned char GEO_DB_TEXT_LINEWIDTH			= 7;
const unsigned char GEO_DB_TEXT_PACKED_COLOR		= 8;
const unsigned char GEO_DB_TEXT_SCALE_X				= 9;
const unsigned char GEO_DB_TEXT_SCALE_Y				= 10;
const unsigned char GEO_DB_TEXT_MATRIX				= 11;
const unsigned char GEO_DB_TEXT_EXPANSION			= 12;
const unsigned char GEO_DB_TEXT_COLOR_INDEX			= 13;
const unsigned char GEO_DB_TEXT_FONT				= 14;
const unsigned char GEO_DB_TEXT_EXTENDED			= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_TEXT_COMMENT				= 255; // alias for GEO_DB_COMMENT_FIELD

///////////////////////////////////////////////////////////////////////////////
// GEO_DB_IMAGE Record - Field Ids IDs			- Not Yet Implemented
//
const unsigned char GEO_DB_IMAGE_EXT				= 1;  // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_IMAGE_EXTENDED			= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_IMAGE_COMMENT			= 255; // alias for GEO_DB_COMMENT_FIELD


///////////////////////////////////////////////////////////////////////////////
// GEO_DB_MESH Record - Field Ids IDs			- Not Yet Implemented
//
const unsigned char GEO_DB_MESH_EXT					= 1; // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_MESH_EXTENDED			= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_MESH_COMMENT				= 255; // alias for GEO_DB_COMMENT_FIELD


///////////////////////////////////////////////////////////////////////////////
// GEO_DB_VERTEX Record - Field Ids IDs
//
const unsigned char GEO_DB_VRTX_EXT					= 1; // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_VRTX_COORD				= 2;
const unsigned char GEO_DB_VRTX_UV_SET_1			= 3;

const unsigned char GEO_DB_VRTX_UV_SET_2			= 4;	// Unused - Possible Future expansion
const unsigned char GEO_DB_VRTX_UV_SET_3			= 5;	// Unused - Possible Future expansion
const unsigned char GEO_DB_VRTX_UV_SET_4			= 6;	// Unused - Possible Future expansion
const unsigned char GEO_DB_VRTX_UV_SET_5			= 7;	// Unused - Possible Future expansion
const unsigned char GEO_DB_VRTX_UV_SET_6			= 8;	// Unused - Possible Future expansion
const unsigned char GEO_DB_VRTX_UV_SET_7			= 9;	// Unused - Possible Future expansion
const unsigned char GEO_DB_VRTX_UV_SET_8			= 10;	// Unused - Possible Future expansion

const unsigned char GEO_DB_VRTX_NORMAL				= 11;
const unsigned char GEO_DB_VRTX_PACKED_COLOR		= 12;
const unsigned char GEO_DB_VRTX_COLOR_INDEX			= 13;

const unsigned char GEO_DB_VRTX_EXTENDED			= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_VRTX_COMMENT				= 255; // alias for GEO_DB_COMMENT_FIELD


///////////////////////////////////////////////////////////////////////////////
// GEO_DB_EXTERNAL Record - Field Ids IDs
//
const unsigned char GEO_DB_EXTERNAL_EXT				= 1;   // alias for GEO_DB_USER_EXT_VALUE_FIELD
const unsigned char GEO_DB_EXTERNAL_NAME			= 2;
const unsigned char GEO_DB_EXTERNAL_INSTANCE_DEF	= 3;
const unsigned char GEO_DB_EXTERNAL_FILENAME		= 4;
const unsigned char GEO_DB_EXTERNAL_TRANSFORM		= 253;
const unsigned char GEO_DB_EXTERNAL_EXTENDED		= 254; // alias for GEO_DB_EXTENDED_FIELD
const unsigned char GEO_DB_EXTERNAL_COMMENT			= 255; // alias for GEO_DB_COMMENT_FIELD



///////////////////////////////////////////////////////////////////
// DB_DSK_INTERNAL_VARS  Record Field Ids
//
const unsigned char GEO_DB_INTERNAL_VAR_FRAMECOUNT	= 1;
const unsigned char GEO_DB_INTERNAL_VAR_CURRENT_TIME= 2;
const unsigned char GEO_DB_INTERNAL_VAR_ELAPSED_TIME= 3;
const unsigned char GEO_DB_INTERNAL_VAR_SINE		= 4;
const unsigned char GEO_DB_INTERNAL_VAR_COSINE		= 5;
const unsigned char GEO_DB_INTERNAL_VAR_TANGENT		= 6;
const unsigned char GEO_DB_INTERNAL_VAR_MOUSE_X		= 7;
const unsigned char GEO_DB_INTERNAL_VAR_MOUSE_Y		= 8;
const unsigned char GEO_DB_INTERNAL_VAR_LEFT_MOUSE	= 9;
const unsigned char GEO_DB_INTERNAL_VAR_MIDDLE_MOUSE= 10;
const unsigned char GEO_DB_INTERNAL_VAR_RIGHT_MOUSE	= 11;
const unsigned char GEO_DB_INTERNAL_VAR_KEYBOARD	= 12;
const unsigned char GEO_DB_INTERNAL_VAR_TEMP_FLOAT	= 13;
const unsigned char GEO_DB_INTERNAL_VAR_TEMP_INT	= 14;
const unsigned char GEO_DB_INTERNAL_VAR_TEMP_BOOL	= 15;
const unsigned char GEO_DB_INTERNAL_VAR_TEMP_STRING	= 16;




///////////////////////////////////////////////////////////////////
// DB_DSK_FLOAT_VAR  Record Field Ids
//
const unsigned char GEO_DB_FLOAT_VAR_NAME			= 1;
const unsigned char GEO_DB_FLOAT_VAR_VALUE			= 2;
const unsigned char GEO_DB_FLOAT_VAR_DEFAULT		= 3;
const unsigned char GEO_DB_FLOAT_VAR_FID			= 4;
const unsigned char GEO_DB_FLOAT_VAR_CONSTRAINED	= 5;
const unsigned char GEO_DB_FLOAT_VAR_MIN			= 6;
const unsigned char GEO_DB_FLOAT_VAR_MAX			= 7;
const unsigned char GEO_DB_FLOAT_VAR_STEP			= 8;



///////////////////////////////////////////////////////////////////
// DB_DSK_FLOAT3_VAR  Record Field Ids
//
const unsigned char GEO_DB_FLOAT3_VAR_NAME			= 1;
const unsigned char GEO_DB_FLOAT3_VAR_VALUE			= 2;
const unsigned char GEO_DB_FLOAT3_VAR_DEFAULT		= 3;
const unsigned char GEO_DB_FLOAT3_VAR_FID			= 4;
const unsigned char GEO_DB_FLOAT3_VAR_CONSTRAINED	= 5;
const unsigned char GEO_DB_FLOAT3_VAR_MIN			= 6;
const unsigned char GEO_DB_FLOAT3_VAR_MAX			= 7;
const unsigned char GEO_DB_FLOAT3_VAR_STEP			= 8;


///////////////////////////////////////////////////////////////////
// DB_DSK_INT_VAR  Record Field Ids
//
const unsigned char GEO_DB_INT_VAR_NAME				= 1;
const unsigned char GEO_DB_INT_VAR_VALUE			= 2;
const unsigned char GEO_DB_INT_VAR_DEFAULT			= 3;
const unsigned char GEO_DB_INT_VAR_FID				= 4;
const unsigned char GEO_DB_INT_VAR_CONSTRAINED		= 5;
const unsigned char GEO_DB_INT_VAR_MIN				= 6;
const unsigned char GEO_DB_INT_VAR_MAX				= 7;
const unsigned char GEO_DB_INT_VAR_STEP				= 8;



///////////////////////////////////////////////////////////////////
// DB_DSK_STRING_VAR  Record Field Ids
//
const unsigned char GEO_DB_STRING_VAR_NAME			= 1;
const unsigned char GEO_DB_STRING_VAR_VALUE			= 2;
const unsigned char GEO_DB_STRING_VAR_DEFAULT		= 3;
const unsigned char GEO_DB_STRING_VAR_FID			= 4;


///////////////////////////////////////////////////////////////////
// DB_DSK_BOOL_VAR  Record Field Ids
//
const unsigned char GEO_DB_BOOL_VAR_NAME			= 1;
const unsigned char GEO_DB_BOOL_VAR_VALUE			= 2;
const unsigned char GEO_DB_BOOL_VAR_DEFAULT			= 3;
const unsigned char GEO_DB_BOOL_VAR_FID				= 4;


///////////////////////////////////////////////////////////////////
// DB_DSK_LONG_VAR  Record Field Ids
//
const unsigned char GEO_DB_LONG_VAR_NAME			= 1;
const unsigned char GEO_DB_LONG_VAR_VALUE			= 2;
const unsigned char GEO_DB_LONG_VAR_DEFAULT			= 3;
const unsigned char GEO_DB_LONG_VAR_FID				= 4;
const unsigned char GEO_DB_LONG_VAR_CONSTRAINED		= 5;
const unsigned char GEO_DB_LONG_VAR_MIN				= 6;
const unsigned char GEO_DB_LONG_VAR_MAX				= 7;
const unsigned char GEO_DB_LONG_VAR_STEP			= 8;


///////////////////////////////////////////////////////////////////
// DB_DSK_DOUBLE_VAR  Record Field Ids
//
const unsigned char GEO_DB_DOUBLE_VAR_NAME			= 1;
const unsigned char GEO_DB_DOUBLE_VAR_VALUE			= 2;
const unsigned char GEO_DB_DOUBLE_VAR_DEFAULT		= 3;
const unsigned char GEO_DB_DOUBLE_VAR_FID			= 4;
const unsigned char GEO_DB_DOUBLE_VAR_CONSTRAINED	= 5;
const unsigned char GEO_DB_DOUBLE_VAR_MIN			= 6;
const unsigned char GEO_DB_DOUBLE_VAR_MAX			= 7;
const unsigned char GEO_DB_DOUBLE_VAR_STEP			= 8;



///////////////////////////////////////////////////////////////////
// GEO_DSK_BEHAVIOR  Record Field Ids
//
const unsigned char GEO_DB_BEHAVIOR_NAME			= 1;



///////////////////////////////////////////////////////////////////
// DB_DSK_ROTATE_ACTION  Record Field Ids
//
const unsigned char GEO_DB_ROTATE_ACTION_INPUT_VAR		= 1;
const unsigned char GEO_DB_ROTATE_ACTION_OUTPUT_VAR		= 2; // not used
const unsigned char GEO_DB_ROTATE_ACTION_ORIGIN			= 3;
const unsigned char GEO_DB_ROTATE_ACTION_VECTOR			= 4;
const unsigned char GEO_DB_ROTATE_ACTION_DIR			= 5;



///////////////////////////////////////////////////////////////////
// DB_DSK_CLAMP_ACTION  Record Field Ids
//
const unsigned char GEO_DB_CLAMP_ACTION_INPUT_VAR		= 1;
const unsigned char GEO_DB_CLAMP_ACTION_OUTPUT_VAR		= 2; 
const unsigned char GEO_DB_CLAMP_ACTION_MIN_VAL			= 3;
const unsigned char GEO_DB_CLAMP_ACTION_MAX_VAL			= 4;



///////////////////////////////////////////////////////////////////
// DB_DSK_RANGE_ACTION  Record Field Ids
//
const unsigned char GEO_DB_RANGE_ACTION_INPUT_VAR		= 1;
const unsigned char GEO_DB_RANGE_ACTION_OUTPUT_VAR		= 2; 
const unsigned char GEO_DB_RANGE_ACTION_IN_MIN_VAL		= 3;
const unsigned char GEO_DB_RANGE_ACTION_IN_MAX_VAL		= 4;
const unsigned char GEO_DB_RANGE_ACTION_OUT_MIN_VAL		= 5;
const unsigned char GEO_DB_RANGE_ACTION_OUT_MAX_VAL		= 6;



///////////////////////////////////////////////////////////////////
// DB_DSK_ARITHMETIC_ACTION  Record Field Ids
//
const unsigned char GEO_DB_ARITHMETIC_ACTION_INPUT_VAR	= 1;
const unsigned char GEO_DB_ARITHMETIC_ACTION_OUTPUT_VAR	= 2; 
const unsigned char GEO_DB_ARITHMETIC_ACTION_OP_TYPE	= 3;
const unsigned char GEO_DB_ARITHMETIC_ACTION_OPERAND_VALUE	= 4;
const unsigned char GEO_DB_ARITHMETIC_ACTION_OPERAND_VAR	= 5;



///////////////////////////////////////////////////////////////////
// DB_DSK_EQUATION_ACTION  Record Field Ids
//
const unsigned char GEO_DB_EQUATION_ACTION_INPUT_VAR	= 1;
const unsigned char GEO_DB_EQUATION_ACTION_OUTPUT_VAR	= 2; 
const unsigned char GEO_DB_EQUATION_ACTION_A_VAL		= 3;
const unsigned char GEO_DB_EQUATION_ACTION_C_VAL		= 4;
const unsigned char GEO_DB_EQUATION_ACTION_A_VAR		= 5;
const unsigned char GEO_DB_EQUATION_ACTION_C_VAR		= 6;



///////////////////////////////////////////////////////////////////
// DB_DSK_PERIODIC_ACTION  Record Field Ids
//
const unsigned char GEO_DB_PERIODIC_ACTION_TYPE			= 7;



///////////////////////////////////////////////////////////////////
// DB_DSK_TRIG_ACTION  Record Field Ids
//
const unsigned char GEO_DB_TRIG_ACTION_OP				= 7;





///////////////////////////////////////////////////////////////////
// DB_DSK_TASK_ACTION  Record Field Ids
//
const unsigned char GEO_DB_TASK_ACTION_INPUT_VAR		= 1;
const unsigned char GEO_DB_TASK_ACTION_OUTPUT_VAR		= 2; 


///////////////////////////////////////////////////////////////////
// DB_DSK_VISIBILITY_ACTION  Record Field Ids
//
const unsigned char GEO_DB_VISIBILITY_ACTION_INPUT_VAR	= 1;
const unsigned char GEO_DB_VISIBILITY_ACTION_OUTPUT_VAR	= 2; // Not used



///////////////////////////////////////////////////////////////////
// DB_DSK_COLOR_RAMP_ACTION  Record Field Ids
//
const unsigned char GEO_DB_COLOR_RAMP_ACTION_INPUT_VAR			= 1;
const unsigned char GEO_DB_COLOR_RAMP_ACTION_OUTPUT_VAR			= 2; // Not used
const unsigned char GEO_DB_COLOR_RAMP_ACTION_COLOR_FROM_PALETTE	= 3;
const unsigned char GEO_DB_COLOR_RAMP_ACTION_TOP_COLOR_INDEX	= 4;
const unsigned char GEO_DB_COLOR_RAMP_ACTION_BOTTOM_COLOR_INDEX	= 5;
const unsigned char GEO_DB_COLOR_RAMP_ACTION_NUM_RAMPS			= 6;
const unsigned char GEO_DB_COLOR_RAMP_ACTION_TOP_COLOR			= 7;
const unsigned char GEO_DB_COLOR_RAMP_ACTION_BOTTOM_COLOR		= 8;
const unsigned char GEO_DB_COLOR_RAMP_ACTION_MATCH_COLUMNS		= 9;



///////////////////////////////////////////////////////////////////
// DB_DSK_COMPARE_ACTION  Record Field Ids
//
const unsigned char GEO_DB_COMPARE_ACTION_INPUT_VAR		= 1;
const unsigned char GEO_DB_COMPARE_ACTION_OUTPUT_VAR	= 2; 
const unsigned char GEO_DB_COMPARE_ACTION_OP_TYPE		= 3;
const unsigned char GEO_DB_COMPARE_ACTION_OPERAND_VALUE = 4;
const unsigned char GEO_DB_COMPARE_ACTION_OPERAND_VAR   = 5;


///////////////////////////////////////////////////////////////////
// DB_DSK_TRANSLATE_ACTION  Record Field Ids
//
const unsigned char GEO_DB_TRANSLATE_ACTION_INPUT_VAR	= 1;
const unsigned char GEO_DB_TRANSLATE_ACTION_OUTPUT_VAR	= 2; // not used
const unsigned char GEO_DB_TRANSLATE_ACTION_ORIGIN		= 3;
const unsigned char GEO_DB_TRANSLATE_ACTION_VECTOR		= 4;
const unsigned char GEO_DB_TRANSLATE_ACTION_DIR			= 5;
const unsigned char GEO_DB_TRANSLATE_ACTION_SCALAR		= 6;



///////////////////////////////////////////////////////////////////
// DB_DSK_SCALE_ACTION  Record Field Ids
//
const unsigned char GEO_DB_SCALE_ACTION_INPUT_VAR	= 1;
const unsigned char GEO_DB_SCALE_ACTION_OUTPUT_VAR	= 2; // not used
const unsigned char GEO_DB_SCALE_ACTION_ORIGIN		= 3;
const unsigned char GEO_DB_SCALE_ACTION_VECTOR		= 4;
const unsigned char GEO_DB_SCALE_ACTION_DIR			= 5;
const unsigned char GEO_DB_SCALE_ACTION_SCALAR		= 6;
const unsigned char GEO_DB_SCALE_ACTION_SCALE_ALL	= 7;



///////////////////////////////////////////////////////////////////
// DB_DSK_STRING_CONTENT_ACTION  Record Field Ids
//
const unsigned char GEO_DB_STRING_CONTENT_ACTION_INPUT_VAR		= 1;
const unsigned char GEO_DB_STRING_CONTENT_ACTION_OUTPUT_VAR		= 2; // not used
const unsigned char GEO_DB_STRING_CONTENT_ACTION_PADDING_TYPE	= 3;
const unsigned char GEO_DB_STRING_CONTENT_ACTION_PAD_FOR_SIGN	= 4;
const unsigned char GEO_DB_STRING_CONTENT_ACTION_FORMAT			= 5;


///////////////////////////////////////////////////////////////////
// DB_DSK_STRING_COPY_ACTION  Record Field Ids
//
const unsigned char GEO_DB_STRING_COPY_ACTION_INPUT_VAR			= 1;
const unsigned char GEO_DB_STRING_COPY_ACTION_OUTPUT_VAR		= 2; // not used


///////////////////////////////////////////////////////////////////
// DB_DSK_CONDITIONAL_ACTION  Record Field Ids
//
const unsigned char GEO_DB_CONDITIONAL_ACTION_INPUT_VAR	= 1;
const unsigned char GEO_DB_CONDITIONAL_ACTION_OUTPUT_VAR= 2; // not used


///////////////////////////////////////////////////////////////////
// DB_DSK_DCS_ACTION  Record Field Ids
//
const unsigned char GEO_DB_DCS_ACTION_INPUT_VAR					= 1; // not used
const unsigned char GEO_DB_DCS_ACTION_OUTPUT_VAR				= 2; // not used
const unsigned char GEO_DB_DCS_ACTION_ORIGIN					= 3;
const unsigned char GEO_DB_DCS_ACTION_XPOS						= 4;
const unsigned char GEO_DB_DCS_ACTION_ZPOS						= 5;
const unsigned char GEO_DB_DCS_ACTION_VECTOR					= 6;
const unsigned char GEO_DB_DCS_ACTION_TRANSLATE_X_VAR			= 7;
const unsigned char GEO_DB_DCS_ACTION_TRANSLATE_Y_VAR			= 8;
const unsigned char GEO_DB_DCS_ACTION_TRANSLATE_Z_VAR			= 9;
const unsigned char GEO_DB_DCS_ACTION_ROTATE_X_VAR				= 10;
const unsigned char GEO_DB_DCS_ACTION_ROTATE_Y_VAR				= 11;
const unsigned char GEO_DB_DCS_ACTION_ROTATE_Z_VAR				= 12;
const unsigned char GEO_DB_DCS_ACTION_SCALE_X_VAR				= 13;
const unsigned char GEO_DB_DCS_ACTION_SCALE_Y_VAR				= 14;
const unsigned char GEO_DB_DCS_ACTION_SCALE_Z_VAR				= 15;







///////////////////////////////////////////////////////////////////
// DB_DSK_DISCRETE_ACTION  Record Field Ids
//
const unsigned char GEO_DB_DISCRETE_ACTION_INPUT_VAR			= 1;
const unsigned char GEO_DB_DISCRETE_ACTION_OUTPUT_VAR			= 2; 
const unsigned char GEO_DB_DISCRETE_ACTION_NUM_ITEMS			= 3;
const unsigned char GEO_DB_DISCRETE_ACTION_OUTPUT_VAR_TYPE		= 4;
const unsigned char GEO_DB_DISCRETE_ACTION_MIN_VALS				= 5;
const unsigned char GEO_DB_DISCRETE_ACTION_MAX_VALS				= 6;
const unsigned char GEO_DB_DISCRETE_ACTION_MAP_VALS				= 7;





/** Record identifiers can be read as ints or this structure. All subsequent
 *  fields are considered part of this Node until an special EOF(ield) record
 *  is found. The only exception to this rule id DB_DSK_PUSH & DB_DSK_POP
 *  which have no fields. User parse code should expect another REcord header
 *  immediately after reading the Push/Pop record.
 */
struct GEO_DB_API geoExtensionDefRec
{

	/** The Node type for which this extension exists */
	unsigned int			nodetype;					//  4 bytes 

	/** The data type of the extension - defined in terms of GEO_DB_EXT_INT
	 *  GEO_DB_EXT_FLOAT, GEO_DB_EXT_BOOL etc.
	 */
	unsigned char			datatype;					//  1 byte

	/** The extension can have a special "sub type" value. This could be
	 *  values like GEO_DB_EXT_MENU_ITEM which (when associated with a datatype
	 *  of GEO_DB_EXT_BOOL means that this extension will be accessed as one
	 *  of many in an option menu
	 */
	unsigned char			subdatatype;				//  1 bytes 

	/** The User ID (uid) is the optional value provided (in code) by the user 
	 *  to identify this particular extension. Users can search & retrieve 
	 *  extension values based on this user ID number.
	 */
	unsigned short			uid;						//  2 bytes

	/** The name of the extension.
	 *
	 *  Note that the "name" field is sized for the Geo 1.0 maximum property
	 *  label length that can be accomodated. The name field is also used to
	 *  encode the name/label of the option menu when the extension is flagged
	 *  as one of those. The following rules should be taken into consideration:
	 *  1. When the extension is an option menu (datatype=GEO_DB_EXT_BOOL
	 *     and subdatatype=GEO_DB_EXT_MENU_ITEM) then the name field is
	 *     divided up as 15 chars for the option menu title, 8 chars for
	 *     this particular option menu's label and 1 char for the terminator
	 * 2.  When the extension is a text field or boolean toggle value - it is
	 *     recommended that only the 15 chars for the field label be used - 
	 *     setting a 23 char-length  label for a text input field will be a 
	 *     waste of time, as it will get truncated on display anyway.
	 */
	char					name[24];					// 24 bytes
											//-----------------------------
};											// total:	   32 bytes





///////////////////////////////////////////////////////////////////
// Utility Parse/Read/Write Functions
//
GEO_DB_API int		geoIgnoreField(FILE* fp,geoFieldHeader field);
GEO_DB_API int 		geoIgnoreFields(FILE* fp);
GEO_DB_API int 		geoWriteString(FILE*,int, char*);
GEO_DB_API int 		geoWriteString(FILE* fp,char* string);
GEO_DB_API char* 	geoGetWritableString(char* string,int* num_chars);
GEO_DB_API char* 	geoReadString(FILE*,geoFieldHeader);
GEO_DB_API char* 	geoReadString(FILE*,int);
GEO_DB_API int 		geoIgnoreDataItems(FILE* fp, int type,int num );
GEO_DB_API int		geoGetSizeOfFormatDataType(int type);


#endif // __GEO_FORMAT_H__

