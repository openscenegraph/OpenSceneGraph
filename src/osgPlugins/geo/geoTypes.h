/*===========================================================================*\

NAME:			geoTypes.h

DESCRIPTION:	Constants fro Node types etc.

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



#ifndef _GEO_TYPES_H_
#define _GEO_TYPES_H_



#ifndef uint
#define uint		unsigned int
#endif

#ifndef ushort
#define ushort		unsigned short
#endif

#ifndef ubyte
#define ubyte		unsigned char
#endif



	
/** 
 * constants to identify the plugin type 
 */
const uint GEO_PLUGIN_TYPE_UNDEFINED			= 1;
const uint GEO_PLUGIN_TYPE_GEOMETRY_IMPORTER	= 2;
const uint GEO_PLUGIN_TYPE_GEOMETRY_EXPORTER	= 3;
const uint GEO_PLUGIN_TYPE_IMAGE_IMPORTER		= 4;
const uint GEO_PLUGIN_TYPE_TOOL					= 5;
const uint GEO_PLUGIN_TYPE_BEHAVIOR				= 6;
const uint GEO_PLUGIN_TYPE_GROUP_NODE_DEF		= 7;
const uint GEO_PLUGIN_TYPE_SURFACE_NODE_DEF		= 8;
const uint GEO_PLUGIN_TYPE_TASK					= 9;
const uint GEO_PLUGIN_TYPE_LAST					= GEO_PLUGIN_TYPE_TASK;




/** put nowhere */
const uint GEO_TOOL_TYPE_NONE					= 0;

/** user tool constant - put in favorites menu & toolbar */
const uint GEO_TOOL_TYPE_USER					= 1;

/** create tool constant - put in create menu & toolbar */
const uint GEO_TOOL_TYPE_CREATE					= 2;

/** modify tool constant - put in modify menu & toolbar */
const uint GEO_TOOL_TYPE_MODIFY					= 3;

/** helper point tool constant - put in helpers menu & toolbar */
const uint GEO_TOOL_TYPE_HELPER_PT				= 4;

/** appearance tool constant - put in plugins menu & toolbar */
const uint GEO_TOOL_TYPE_APPEARANCE				= 5;

/** behavior tool constant - put in plugins menu & toolbar */
const uint GEO_TOOL_TYPE_BEHAVIOR				= 6;

/** optimize tool constant - put in plugins menu & toolbar */
const uint GEO_TOOL_TYPE_OPTIMIZE				= 7;

/** scenegraph tool constant - put in scenegraph menu & toolbar */
const uint GEO_TOOL_TYPE_SCENEGRAPH				= 8;

const uint GEO_TOOL_TYPE_FILE					= 9;

const uint GEO_TOOL_TYPE_EDIT					= 10;

const uint GEO_TOOL_TYPE_VIEW					= 11;

const uint GEO_TOOL_TYPE_LOD					= 12;

const uint GEO_TOOL_TYPE_SELECT					= 13;

const uint GEO_TOOL_TYPE_GRID					= 14;

/** convenience constant */
const uint GEO_TOOL_TYPE_LAST					= GEO_TOOL_TYPE_GRID;



/**
 *  Node Type identifiers. These tokens encode the Node's inheritance
 *  information within the type
 *
 *  The GEO Node Type Class Hierarchy is as follows...
 *
 *	GEO_DB_BASE
 *			GEO_DB_GROUP 
 *					GEO_DB_SEQUENCE
 *					GEO_DB_LOD
 *					GEO_DB_SWITCH
 *					GEO_DB_BASE_GROUP
 *					GEO_DB_RENDERGROUP
 *							GEO_DB_MULTI_TEX_SHADER	
 *							GEO_DB_BASE_RENDERGROUP (*)
 *					GEO_DB_EXTERNAL
 *					GEO_DB_INSTANCE
 *					GEO_DB_PAGE
 *					GEO_DB_CULL_GROUP
 *					GEO_DB_Z_OFFSET_GROUP
 *					GEO_DB_MULTI_SAMPLE_AA_GROUP
 *					GEO_DB_LINE_AA_GROUP
 *					GEO_DB_FADE_GROUP
 *					GEO_DB_TERRAIN
 *					GEO_DB_BSP
 *					GEO_DB_DECAL_GROUP
 *					GEO_DB_LIGHT_GROUP
 *					GEO_DB_DCS
 *			GEO_DB_GEOMETRY
 *					GEO_DB_SURFACE
 *							GEO_DB_POLYGON
 *									GEO_DB_LIGHTPT
 *									GEO_DB_MESH
 *							GEO_DB_BASE_SURFACE (*)
 *					GEO_DB_TEXT
 *					GEO_DB_VERTEX
 *						GEO_DB_FAT_VERTEX	
 *						GEO_DB_SLIM_VERTEX	
 *			GEO_DB_HEADER
 *
 * (*) Not available in Geo Version 1.0
 */


//--------------------------------------------------------------------
// Geo Node type Identifiers
//--------------------------------------------------------------------
//
const uint GEO_DB_BASE					=  0x00000003;
const uint GEO_DB_GROUP					= (0x00000004 | GEO_DB_BASE);
const uint GEO_DB_TERRAIN				= (0x00000008 | GEO_DB_GROUP);
//------------
const uint GEO_DB_SEQUENCE				= (0x00000010 | GEO_DB_GROUP);
const uint GEO_DB_LOD					= (0x00000020 | GEO_DB_GROUP);
const uint GEO_DB_SWITCH				= (0x00000040 | GEO_DB_GROUP);
const uint GEO_DB_RENDERGROUP			= (0x00000080 | GEO_DB_GROUP);
//------------
const uint GEO_DB_GEOMETRY				= (0x00000100 | GEO_DB_BASE);
const uint GEO_DB_SURFACE				= (0x00000200 | GEO_DB_GEOMETRY);
const uint GEO_DB_BSP					= (0x00000400 | GEO_DB_GROUP);
const uint GEO_DB_POLYGON				= (0x00000800 | GEO_DB_SURFACE);
//------------
const uint GEO_DB_MESH					= (0x00001000 | GEO_DB_POLYGON);
const uint GEO_DB_CULL_GROUP			= (0x00002000 | GEO_DB_GROUP);	
const uint GEO_DB_MULTI_TEX_SHADER		= (0x00004000 | GEO_DB_RENDERGROUP);	
const uint GEO_DB_PAGE					= (0x00008000 | GEO_DB_GROUP);
//------------
const uint GEO_DB_Z_OFFSET_GROUP		= (0x00010000 | GEO_DB_GROUP);	
const uint GEO_DB_MULTI_SAMPLE_AA_GROUP	= (0x00020000 | GEO_DB_GROUP);	
const uint GEO_DB_TEXT					= (0x00040000 | GEO_DB_GEOMETRY);
const uint GEO_DB_VERTEX				= (0x00080000 | GEO_DB_GEOMETRY);
//------------
const uint GEO_DB_HEADER				= (0x00100000 | GEO_DB_BASE);
const uint GEO_DB_LINE_AA_GROUP			= (0x00200000 | GEO_DB_GROUP);	
const uint GEO_DB_BASE_GROUP			= (0x00400000 | GEO_DB_GROUP);
const uint GEO_DB_BASE_SURFACE			= (0x00800000 | GEO_DB_SURFACE);
//------------
const uint GEO_DB_EXTERNAL 				= (0x01000000 | GEO_DB_GROUP);					
const uint GEO_DB_BASE_RENDERGROUP		= (0x02000000 | GEO_DB_RENDERGROUP);
const uint GEO_DB_INSTANCE				= (0x04000000 | GEO_DB_GROUP);
const uint GEO_DB_LIGHTPT				= (0x08000000 | GEO_DB_POLYGON);
//------------
const uint GEO_DB_FADE_GROUP			= (0x10000000 | GEO_DB_GROUP);	
const uint GEO_DB_DECAL_GROUP			= (0x20000000 | GEO_DB_GROUP);	
const uint GEO_DB_LIGHT_GROUP			= (0x40000000 | GEO_DB_GROUP);	
const uint GEO_DB_FAT_VERTEX			= (0x80000000 | GEO_DB_VERTEX);	
//------------

//--------------------------------------------------------------------
// Geo Extended Node type Identifiers
//--------------------------------------------------------------------
const uint GEO_DB_SLIM_VERTEX			= (0x00000010 | GEO_DB_VERTEX);
const uint GEO_DB_DCS					= (0x00001000 | GEO_DB_GROUP);




// older version types for Compatability & convenience
//
const uint GEO_DB_ALL					= GEO_DB_BASE;	
const uint GEO_DB_ALL_GROUP_TYPES		= GEO_DB_GROUP;		
const uint GEO_DB_ALL_GEOMETRY_TYPES	= GEO_DB_GEOMETRY;
const uint GEO_DB_ALL_SURFACE_TYPES		= GEO_DB_SURFACE;	



	
///////////////////////////////////////////////////////////////////////////////	
/** constants to identify the type of picking to be done */
const uint GEO_PICK_GROUP				= 0x00000001; 
const uint GEO_PICK_PRIM				= 0x00000002;
const uint GEO_PICK_VERTEX				= 0x00000004;
const uint GEO_PICK_GRID				= 0x00000010;
const uint GEO_PICK_NON_NODE			= 0x00000020;	// manipulators, user geometry etc.
const uint GEO_PICK_EXTERNAL			= 0x00000040;
const uint GEO_PICK_TEXT				= 0x00000080;




///////////////////////////////////////////////////////////////////////////////		
/** constants to identify mouse button usage */
const uint GEO_NO_MOUSE					= 0x00000000;
const uint GEO_LEFT_MOUSE				= 0x00000001;
const uint GEO_MIDDLE_MOUSE				= 0x00000002;
const uint GEO_RIGHT_MOUSE				= 0x00000004;
const uint GEO_LEFT_AND_RIGHT_MOUSE		= 0x00000008;
const uint GEO_MIDDLE_AND_RIGHT_MOUSE	= 0x00000010;


///////////////////////////////////////////////////////////////////
// PROPERTY TYPES
///////////////////////////////////////////////////////////////////

// Identifiers for Geo data types - Used in geoProperty & geoPropertyExtension Classes 
const unsigned char GEO_DB_DATATYPE_STRING					= 1;
const unsigned char GEO_DB_DATATYPE_SHORT					= 2;
const unsigned char GEO_DB_DATATYPE_INT						= 3;
const unsigned char GEO_DB_DATATYPE_FLOAT					= 4;
const unsigned char GEO_DB_DATATYPE_LONG					= 5;
const unsigned char GEO_DB_DATATYPE_DOUBLE					= 6;
const unsigned char GEO_DB_DATATYPE_VEC3F					= 8;
const unsigned char GEO_DB_DATATYPE_VEC4F					= 9;
const unsigned char GEO_DB_DATATYPE_BOOL					= 28;

///////////////////////////////////////////////////////////////////
// VARIABLE TYPES
///////////////////////////////////////////////////////////////////

const uint GEO_VAR_TYPE_FLOAT								= 1;
const uint GEO_VAR_TYPE_INT									= 2;
const uint GEO_VAR_TYPE_LONG								= 3;
const uint GEO_VAR_TYPE_DOUBLE								= 4;
const uint GEO_VAR_TYPE_BOOL								= 5;
const uint GEO_VAR_TYPE_2FV									= 6;
const uint GEO_VAR_TYPE_3FV									= 7;
const uint GEO_VAR_TYPE_4FV									= 8;
const uint GEO_VAR_TYPE_STRING								= 9;
const uint GEO_VAR_TYPE_2IV									= 10;
const uint GEO_VAR_TYPE_3IV									= 11;
const uint GEO_VAR_TYPE_4IV									= 12;
const uint GEO_VAR_TYPE_16FV								= 13;
const uint GEO_VAR_TYPE_2BV									= 14;
const uint GEO_VAR_TYPE_3BV									= 15;
const uint GEO_VAR_TYPE_4BV									= 16;
const uint GEO_VAR_TYPE_SAMPLER_1D							= 17;
const uint GEO_VAR_TYPE_SAMPLER_2D							= 18;
const uint GEO_VAR_TYPE_SAMPLER_3D							= 19;
const uint GEO_VAR_TYPE_SAMPLER_CUBE						= 20;
const uint GEO_VAR_TYPE_SAMPLER_1D_SHADOW					= 21;
const uint GEO_VAR_TYPE_SAMPLER_2D_SHADOW					= 22;



///////////////////////////////////////////////////////////////////
// TRANSFORM TYPES
///////////////////////////////////////////////////////////////////

const uint GEO_TRANSFORM_TYPE_TRANSLATE	= 1;
const uint GEO_TRANSFORM_TYPE_ROTATE	= 2;
const uint GEO_TRANSFORM_TYPE_SCALE		= 3;
const uint GEO_TRANSFORM_TYPE_MATRIX	= 4;



///////////////////////////////////////////////////////////////////////////////		
/** Predefined model unit identifier. database model units can be modified 
 *  via set/getUnits
 */
const uint GEO_DB_INCHES				= 1;
const uint GEO_DB_FEET					= 2;
const uint GEO_DB_YARDS					= 3;
const uint GEO_DB_MILES					= 4;
const uint GEO_DB_CENTIMETERS			= 5;
const uint GEO_DB_METERS				= 6;
const uint GEO_DB_KILOMETERS			= 7;



		
///////////////////////////////////////////////////////////////////////////////	
/** Constants to define the modeler's intended "up" direction if that 
 *  makes any sense 
 */
	
const int GEO_DB_UP_AXIS_X				= 1;
const int GEO_DB_UP_AXIS_Y				= 2; // the default
const int GEO_DB_UP_AXIS_Z				= 3;


const short GEO_DB_PROJ_TYPE_FLAT_EARTH		= 0;
const short GEO_DB_PROJ_TYPE_TRAPEZOIDAL	= 1;
const short GEO_DB_PROJ_TYPE_ROUND_EARTH	= 2;
const short GEO_DB_PROJ_TYPE_LAMBERT		= 3;
const short GEO_DB_PROJ_TYPE_UTM			= 4;
const short GEO_DB_PROJ_TYPE_GEODETIC		= 5;
const short GEO_DB_PROJ_TYPE_GEOCENTRIC		= 6;
const short GEO_DB_PROJ_TYPE_LAST			= GEO_DB_PROJ_TYPE_GEOCENTRIC;




///////////////////////////////////////////////////////////////////////////////
// DB_HDR_ELLIPSOID - defines
// Constants to define the ellipsoid model used for the projection
//		
const short GEO_DB_ELLIPSOID_USER_DEFINED	= -1;
const short GEO_DB_ELLIPSOID_WGS_1984		= 0;
const short GEO_DB_ELLIPSOID_WGS_1972		= 1;
const short GEO_DB_ELLIPSOID_BESSEL			= 2;
const short GEO_DB_ELLIPSOID_CLARKE_1866	= 3;
const short GEO_DB_ELLIPSOID_NAD_1927		= 4;
const short GEO_DB_ELLIPSOID_LAST			= GEO_DB_ELLIPSOID_NAD_1927;




///////////////////////////////////////////////////////////////////////////////		
/** Constants to control the drawing effect
 * 
 *  Constants to control the drawing of geometry primitives - usefull if user 
 *  wants to call standard draw method in a tool postDraw callback
 */
const uint GEO_DB_SOLID					=  0x00000001;
const uint GEO_DB_WIRE					=  0x00000002;
const uint GEO_DB_OUTLINED				=  (GEO_DB_SOLID | GEO_DB_WIRE);
const uint GEO_DB_WIRE_ON_MOVE			=  0x00000004;
const uint GEO_DB_DETEXTURE_ON_MOVE		=  0x00000008;
const uint GEO_DB_PROXY_ON_MOVE			=  0x00000010;

const uint GEO_DB_SHRINK				=  0x00000080;

const uint GEO_DB_ZBUFFER				=  0x00000100;
const uint GEO_DB_BBOX_HIGHLIGHT		=  0x00000200;
const uint GEO_DB_BACKFACE				=  0x00000400;
const uint GEO_DB_SELECTIVE_CULLFACE	=  0x00000800;


const uint GEO_DB_DRAW_FACE_NORMALS		=  0x00001000;
const uint GEO_DB_DRAW_VERTEX_NORMALS	=  0x00002000;
const uint GEO_DB_SELECTIVE_BLENDING	=  0x00008000;

const uint GEO_DB_TEXTURE				=  0x00010000;
const uint GEO_DB_HIGHLIGHT				=  0x00020000;
const uint GEO_DB_USE_VERTEX_ARRAYS		=  0x00040000;
const uint GEO_DB_REBUILD_VERTEX_ARRAYS =  0x00080000;

const uint GEO_DB_SELECTIVE_SHADING		=  0x00100000;
const uint GEO_DB_DRAW_SIMPLE			=  0x00200000;

const uint GEO_DB_ILLUMINATED			=  0x01000000;
const uint GEO_DB_NORMAL_PER_PRIM		=  0x04000000;
const uint GEO_DB_NORMAL_PER_VERTEX		=  0x08000000;

const uint GEO_DB_COLOR_PER_GEODE		=  0x10000000;
const uint GEO_DB_COLOR_PER_PRIM		=  0x20000000;
const uint GEO_DB_COLOR_PER_VERTEX		=  0x40000000;

const uint GEO_DB_SELECTIVE_ZBUFFER		=  0x80000000;




	
///////////////////////////////////////////////////////////////////////////////	
/** constants to identify the different Group types 
*/
const uint GEO_GROUP_TYPE_CONTAINER				= 1;
const uint GEO_GROUP_TYPE_CULL					= 2;
const uint GEO_GROUP_TYPE_Z_OFFSET				= 3;
const uint GEO_GROUP_TYPE_MULTI_SAMPLE_AA		= 4;
const uint GEO_GROUP_TYPE_LINE_AA				= 5;
const uint GEO_GROUP_TYPE_FADE					= 6;
const uint GEO_GROUP_TYPE_TERRAIN				= 7;
const uint GEO_GROUP_TYPE_DECAL					= 8;


///////////////////////////////////////////////////////////////////////////////	
/** Constants to control the display of a Group based on time-of-day
* 
*/
const uint GEO_DB_GROUP_TOD_DISPLAY_NIGHT		= 0x00000001;
const uint GEO_DB_GROUP_TOD_DISPLAY_DAWN		= 0x00000002;
const uint GEO_DB_GROUP_TOD_DISPLAY_DAY			= 0x00000004;
const uint GEO_DB_GROUP_TOD_DISPLAY_DUSK		= 0x00000008;




///////////////////////////////////////////////////////////////////////////////	
/** Constants to control the intersection testing of this Group at runtime
* 
*/
const uint GEO_DB_GROUP_ISECT_IG_DEFINED		= 0;
const uint GEO_DB_GROUP_ISECT_YES				= 1;
const uint GEO_DB_GROUP_ISECT_NO				= 2;




///////////////////////////////////////////////////////////////////////////////	
/** Constants to control the switch Node behavior 
 * 
 *  Switch Nodes can either be addative (in which case the
 *  accumulate drawable children) or selective (in which case
 *  the determine which of their children should be drawn).
 *
 *  Selctive control is not implemented.
 */
const uint GEO_SWITCH_TYPE_ADDATIVE		=  1;
const uint GEO_SWITCH_TYPE_SELECTIVE	=  2;





///////////////////////////////////////////////////////////////////////////////	
/** Constants to identify special behavior int ZOffset GRoups
 */
const uint GEO_DB_ZOFFSET_GROUP_TYPE_UNDEFINED		= 0;
const uint GEO_DB_ZOFFSET_GROUP_TYPE_RUNWAY			= 1;
const uint GEO_DB_ZOFFSET_GROUP_TYPE_MARKINGS		= 2;



///////////////////////////////////////////////////////////////////////////////	
/** Constants to control the Light Group behavior 
* 
*  Light Groups are Groups with the Light-Group flag set. Any Light pt children
*  are effected by these settings
*/
const uint GEO_LIGHT_GROUP_ANIM_OFF  	=  0;
const uint GEO_LIGHT_GROUP_ANIM_ON  	=  1;
const uint GEO_LIGHT_GROUP_ANIM_RANDOM  =  2;


///////////////////////////////////////////////////////////////////////////////	
/** Constants that specify the type of Light Group 
* 
*  FIXED is for airfields etc.
*  MOVING is for aircraft/ships etc.
*/
const uint GEO_LIGHT_GROUP_TYPE_FIXED	=  0;
const uint GEO_LIGHT_GROUP_TYPE_MOVING	=  1;


///////////////////////////////////////////////////////////////////////////////
/** Type Tokens for Node & Tool Gui Widgets 
*/
const int GUI_FLOAT		= 1;
const int GUI_INT		= 2;
const int GUI_STRING	= 3;


///////////////////////////////////////////////////////////////////////////////
/** geoWidget Typedef - Used by Node & Tool Gui Widgets 
*/
typedef void geoWidget;


///////////////////////////////////////////////////////////////////////////////
/** Animated String padding tokens */
const int GEO_TEXT_PAD_NONE				= 0;
const int GEO_TEXT_PAD_WITH_SPACES		= 1;
const int GEO_TEXT_PAD_WITH_ZEROES		= 2;


///////////////////////////////////////////////////////////////////////////////
// Polygon draw style types
//
const int GEO_POLY_DSTYLE_SOLID				= 0;
const int GEO_POLY_DSTYLE_OPEN_WIRE			= 1;
const int GEO_POLY_DSTYLE_CLOSED_WIRE		= 2;
const int GEO_POLY_DSTYLE_POINTS			= 3;
const int GEO_POLY_DSTYLE_SOLID_BOTH_SIDES	= 4;



///////////////////////////////////////////////////////////////////////////////
// Polygon shade style types
//

const int GEO_POLY_SHADEMODEL_FLAT			= 0;
const int GEO_POLY_SHADEMODEL_GOURAUD		= 1;
const int GEO_POLY_SHADEMODEL_LIT			= 2;
const int GEO_POLY_SHADEMODEL_LIT_GOURAUD	= 3;


///////////////////////////////////////////////////////////////////////////////
// Texture Mapping types
//

const int GEO_POLY_PLANAR_MAP				= 0;
const int GEO_POLY_CYLINDRICAL_MAP			= 1;
const int GEO_POLY_SPHERICAL_MAP			= 2;



///////////////////////////////////////////////////////////////////////////////
// Texture Unit Functions - used in Polys, meshes & multi-tex shaders
//

const int GEO_DB_TEXTURE_UNIT_FUNC_AS_DEFINED	= 0;
const int GEO_DB_TEXTURE_UNIT_FUNC_MODULATE		= 1;
const int GEO_DB_TEXTURE_UNIT_FUNC_DECAL		= 2;
const int GEO_DB_TEXTURE_UNIT_FUNC_BLEND		= 3;
const int GEO_DB_TEXTURE_UNIT_FUNC_REPLACE		= 4;
const int GEO_DB_TEXTURE_UNIT_FUNC_COMBINE		= 5;



///////////////////////////////////////////////////////////////////////////////
// STring type constants
//
const int GEO_TEXT_RASTER			= 0;
const int GEO_TEXT_STROKE			= 1;
const int GEO_TEXT_POLY				= 2;

///////////////////////////////////////////////////////////////////////////////
// Justification constants
//
const int GEO_TEXT_LEFT_JUSTIFY		= 0;
const int GEO_TEXT_CENTER_JUSTIFY	= 1;
const int GEO_TEXT_RIGHT_JUSTIFY	= 2;

///////////////////////////////////////////////////////////////////////////////
// Direction constants
//
const int GEO_TEXT_LEFT_TO_RIGHT	= 0;
const int GEO_TEXT_RIGHT_TO_LEFT	= 1;
const int GEO_TEXT_TOP_TO_BOTTOM	= 2;
const int GEO_TEXT_BOTTOM_TO_TOP	= 3;



///////////////////////////////////////////////////////////////////////////////
// LightPoint Type constants
//
const int GEO_DB_LIGHTPT_OMNI_DIRECTIONAL	= 0;
const int GEO_DB_LIGHTPT_UNI_DIRECTIONAL	= 1;
const int GEO_DB_LIGHTPT_BI_DIRECTIONAL		= 2;


///////////////////////////////////////////////////////////////////////////////
// Texture Record Wrap S & T Modes
const unsigned GEO_DB_TEX_CLAMP					=  0x00000001;
const unsigned GEO_DB_TEX_REPEAT				=  0x00000002;


///////////////////////////////////////////////////////////////////////////////
// Texture Record MagFilter
const unsigned GEO_DB_TEX_NEAREST				= 0x00000001;
const unsigned GEO_DB_TEX_LINEAR				= 0x00000002;


///////////////////////////////////////////////////////////////////////////////
// Texture Record MinFilter
const unsigned GEO_DB_TEX_NEAREST_MIPMAP_NEAREST = 0x00000004;
const unsigned GEO_DB_TEX_LINEAR_MIPMAP_NEAREST  = 0x00000008;
const unsigned GEO_DB_TEX_NEAREST_MIPMAP_LINEAR  = 0x00000010;
const unsigned GEO_DB_TEX_LINEAR_MIPMAP_LINEAR   = 0x00000020;


///////////////////////////////////////////////////////////////////////////////
// Texture Record TexEnv
const unsigned GEO_DB_TEX_MODULATE				= 0x00000001;
const unsigned GEO_DB_TEX_DECAL					= 0x00000002;
const unsigned GEO_DB_TEX_BLEND					= 0x00000004;
const unsigned GEO_DB_TEX_REPLACE				= 0x00000008;


///////////////////////////////////////////////////////////////////////////////
// Header Vertex Table Usage
const unsigned GEO_DB_USES_PRIVATE_DATA			= 0x00000000;
const unsigned GEO_DB_USES_SHARED_TABLE_DATA	= 0x00000001;
const unsigned GEO_DB_USES_UNSHARED_TABLE_DATA	= 0x00000002;


#endif //_GEO_TYPES_H_

