// Opcode.h


#ifndef __FLT_OPCODE_H
#define __FLT_OPCODE_H

#define OF_VERSION                      1520    //OpenFlight version

#define UNKNOWN_OP                       0

#define HEADER_OP                        1
#define GROUP_OP                         2
#define OLD_LOD_OP                       3
#define OBJECT_OP                        4
#define FACE_OP                          5
#define OLD_VERTEX_OP                    7
#define OLD_VERTEX_COLOR_OP              8
#define OLD_VERTEX_COLOR_NORMAL_OP       9
#define PUSH_LEVEL_OP                   10
#define POP_LEVEL_OP                    11
#define DOF_OP                          14
#define PUSH_SUBFACE_OP                 19
#define POP_SUBFACE_OP                  20
#define PUSH_EXTENSION_OP               21
#define POP_EXTENSION_OP                22
#define COMMENT_OP                      31
#define COLOR_PALETTE_OP                32
#define LONG_ID_OP                      33
/*
Ignore 40-48
#define OLD_TRANSLATE_OP                44
*/
#define MATRIX_OP                       49
#define VECTOR_OP                       50
#define MULTI_TEXTURE_OP                52
#define UV_LIST_OP                      53
#define BSP_OP                          55
#define REPLICATE_OP                    60
#define INSTANCE_REFERENCE_OP           61
#define INSTANCE_DEFINITION_OP          62
#define EXTERNAL_REFERENCE_OP           63
#define TEXTURE_PALETTE_OP              64
#define OLD_MATERIAL_PALETTE_OP         66
#define VERTEX_PALETTE_OP               67
#define VERTEX_C_OP                     68
#define VERTEX_CN_OP                    69
#define VERTEX_CNT_OP                   70
#define VERTEX_CT_OP                    71
#define VERTEX_LIST_OP                  72
#define LOD_OP                          73
#define BOUNDING_BOX_OP                 74
/*
Ignore 76-82
#define ROTATE_ABOUT_EDGE_OP            76
#define TRANSLATE_OP                    78
#define SCALE_OP                        79
#define ROTATE_ABOUT_POINT_OP           80
#define ROTATE_SCALE_TO_POINT_OP        81
#define PUT_TRANSFORM_OP                82
*/
#define EYEPOINT_TRACKPLANE_OP          83
#define MESH_OP                         84
#define LOCAL_VERTEX_POOL_OP            85
#define MESH_PRIMITIVE_OP               86
#define ROAD_SEGMENT_OP                 87
#define ROAD_ZONE_OP                    88
#define MORPH_VERTEX_LIST_OP            89
#define LINKAGE_PALETTE_OP              90
#define ROAD_PATH_OP                    92
#define SOUND_PALETTE_OP                93
#define GENERAL_MATRIX_OP               94
#define SWITCH_OP                       96
#define EXTENSION_OP                    100
#define LIGHT_SOURCE_OP                 101
#define LIGHT_SOURCE_PALETTE_OP         102
#define BOUNDING_SPHERE_OP              105
#define BOUNDING_CYLINDER_OP            106
#define BOUNDING_VOLUME_CENTER_OP       108
#define BOUNDING_VOLUME_ORIENTATION_OP  109
#define LIGHT_POINT_OP                  111
#define TEXTURE_MAPPING_PALETTE_OP      112
#define MATERIAL_PALETTE_OP             113
#define COLOR_NAME_PALETTE_OP           114
#define CAT_OP                          115
#define CAT_DATA_OP                     116
#define RESERVED_OP                     124
#define ROAD_CONSTRUCTION_OP            127

#endif // __FLT_OPCODE_H


