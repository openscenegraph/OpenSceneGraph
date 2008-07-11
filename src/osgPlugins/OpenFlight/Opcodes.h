/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
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

//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#ifndef FLT_OPCODES_H
#define FLT_OPCODES_H

namespace flt {


// Note that INVALID_OP = -1 is not an actual opcode defined in the OpenFlight format.
// The purpose of INVALID_OP is to mark an opcode variable as invalid or uninitialized.
enum Opcodes
{
    INVALID_OP                          = -1,
    UNKNOWN_OP                          = 0,
    HEADER_OP                           = 1,
    GROUP_OP                            = 2,
    OLD_LOD_OP                          = 3,
    OBJECT_OP                           = 4,
    FACE_OP                             = 5,
    OLD_ABSOLUTE_VERTEX_OP              = 7,
    OLD_SHADED_VERTEX_OP                = 8,
    OLD_NORMAL_VERTEX_OP                = 9,
    PUSH_LEVEL_OP                       = 10,
    POP_LEVEL_OP                        = 11,
    DOF_OP                              = 14,
    PUSH_SUBFACE_OP                     = 19,
    POP_SUBFACE_OP                      = 20,
    PUSH_EXTENSION_OP                   = 21,
    POP_EXTENSION_OP                    = 22,
    CONTINUATION_OP                     = 23,
    COMMENT_OP                          = 31,
    COLOR_PALETTE_OP                    = 32,
    LONG_ID_OP                          = 33,
    OLD_TRANSLATE_OP                    = 40,
    OLD_ROTATE_ABOUT_POINT_OP           = 41,
    OLD_ROTATE_ABOUT_EDGE_OP            = 42,
    OLD_SCALE_OP                        = 43,
    OLD_TRANSLATE2_OP                   = 44,
    OLD_NONUNIFORM_SCALE_OP             = 45,
    OLD_ROTATE_ABOUT_POINT2_OP          = 46,
    OLD_ROTATE_SCALE_TO_POINT_OP        = 47,
    OLD_PUT_TRANSFORM_OP                = 48,
    MATRIX_OP                           = 49,
    VECTOR_OP                           = 50,
    OLD_BOUNDING_BOX_OP                 = 51,
    MULTITEXTURE_OP                     = 52,
    UV_LIST_OP                          = 53,
    BINARY_SEPARATING_PLANE_OP          = 55,
    REPLICATE_OP                        = 60,
    INSTANCE_REFERENCE_OP               = 61,
    INSTANCE_DEFINITION_OP              = 62,
    EXTERNAL_REFERENCE_OP               = 63,
    TEXTURE_PALETTE_OP                  = 64,
    OLD_EYEPOINT_PALETTE_OP             = 65,
    OLD_MATERIAL_PALETTE_OP             = 66,
    VERTEX_PALETTE_OP                   = 67,
    VERTEX_C_OP                         = 68,
    VERTEX_CN_OP                        = 69,
    VERTEX_CNT_OP                       = 70,
    VERTEX_CT_OP                        = 71,
    VERTEX_LIST_OP                      = 72,
    LOD_OP                              = 73,
    BOUNDING_BOX_OP                     = 74,
    ROTATE_ABOUT_EDGE_OP                = 76,
    SCALE_OP                            = 77,
    TRANSLATE_OP                        = 78,
    NONUNIFORM_SCALE_OP                 = 79,
    ROTATE_ABOUT_POINT_OP               = 80,
    ROTATE_SCALE_TO_POINT_OP            = 81,
    PUT_TRANSFORM_OP                    = 82,
    EYEPOINT_AND_TRACKPLANE_PALETTE_OP  = 83,
    MESH_OP                             = 84,
    LOCAL_VERTEX_POOL_OP                = 85,
    MESH_PRIMITIVE_OP                   = 86,
    ROAD_SEGMENT_OP                     = 87,
    ROAD_ZONE_OP                        = 88,
    MORPH_VERTEX_LIST_OP                = 89,
    LINKAGE_PALETTE_OP                  = 90,
    SOUND_OP                            = 91,
    ROAD_PATH_OP                        = 92,
    SOUND_PALETTE_OP                    = 93,
    GENERAL_MATRIX_OP                   = 94,
    TEXT_OP                             = 95,
    SWITCH_OP                           = 96,
    LINE_STYLE_PALETTE_OP               = 97,
    CLIP_REGION_OP                      = 98,
    EXTENSION_OP                        = 100,
    LIGHT_SOURCE_OP                     = 101,
    LIGHT_SOURCE_PALETTE_OP             = 102,
    BOUNDING_SPHERE_OP                  = 105,
    BOUNDING_CYLINDER_OP                = 106,
    BOUNDING_CONVEX_HULL_OP             = 107,
    BOUNDING_VOLUME_CENTER_OP           = 108,
    BOUNDING_VOLUME_ORIENTATION_OP      = 109,
    HISTOGRAM_BOUNDING_VOLUME_OP        = 110,
    LIGHT_POINT_OP                      = 111,
    TEXTURE_MAPPING_PALETTE_OP          = 112,
    MATERIAL_PALETTE_OP                 = 113,
    NAME_TABLE_OP                       = 114,
    CAT_OP                              = 115,
    CAT_DATA_OP                         = 116,
    BOUNDING_HISTOGRAM                  = 119,
    PUSH_ATTRIBUTE_OP                   = 122,
    POP_ATTRIBUTE_OP                    = 123,
    ADAPTIVE_ATTRIBUTE_OP               = 125,
    CURVE_NODE_OP                       = 126,
    ROAD_CONSTRUCTION_OP                = 127,
    LIGHT_POINT_APPEARANCE_PALETTE_OP   = 128,
    LIGHT_POINT_ANIMATION_PALETTE_OP    = 129,
    INDEXED_LIGHT_POINT_OP              = 130,
    LIGHT_POINT_SYSTEM_OP               = 131,
    INDEXED_STRING_OP                   = 132,
    SHADER_PALETTE_OP                   = 133
};


} // end namespace

#endif




