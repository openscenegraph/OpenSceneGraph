// Opcode.h


#ifndef __FLT_OPCODE_H
#define __FLT_OPCODE_H

#define OF_VERSION                      1580    //OpenFlight version

#define UNKNOWN_OP                       0

#define HEADER_OP                        1
#define GROUP_OP                         2
#define OBJECT_OP                        4
#define FACE_OP                          5
#define PUSH_LEVEL_OP                   10
#define POP_LEVEL_OP                    11
#define DOF_OP                          14
#define PUSH_SUBFACE_OP                 19
#define POP_SUBFACE_OP                  20
#define PUSH_EXTENSION_OP               21
#define POP_EXTENSION_OP                22
#define CONTINUATION_OP                 23 // ignored
#define COMMENT_OP                      31
#define COLOR_PALETTE_OP                32
#define LONG_ID_OP                      33
#define MATRIX_OP                       49
#define VECTOR_OP                       50
#define MULTI_TEXTURE_OP                52
#define UV_LIST_OP                      53
#define BSP_OP                          55 // ignored
#define REPLICATE_OP                    60 // ignored
#define INSTANCE_REFERENCE_OP           61
#define INSTANCE_DEFINITION_OP          62
#define EXTERNAL_REFERENCE_OP           63
#define TEXTURE_PALETTE_OP              64
#define VERTEX_PALETTE_OP               67
#define VERTEX_C_OP                     68
#define VERTEX_CN_OP                    69
#define VERTEX_CNT_OP                   70
#define VERTEX_CT_OP                    71
#define VERTEX_LIST_OP                  72
#define LOD_OP                          73
#define BOUNDING_BOX_OP                 74
#define ROTATE_ABOUT_EDGE_OP            76
#define TRANSLATE_OP                    78
#define SCALE_OP                        79
#define ROTATE_ABOUT_POINT_OP           80
#define ROTATE_SCALE_TO_POINT_OP        81
#define PUT_TRANSFORM_OP                82
#define EYEPOINT_TRACKPLANE_OP          83 // ignored
#define MESH_OP                         84
#define LOCAL_VERTEX_POOL_OP            85
#define MESH_PRIMITIVE_OP               86
#define ROAD_SEGMENT_OP                 87
#define ROAD_ZONE_OP                    88 // ignored
#define MORPH_VERTEX_LIST_OP            89
#define LINKAGE_PALETTE_OP              90 // ignored
#define SOUND_OP                        91 // ignored
#define ROAD_PATH_OP                    92
#define SOUND_PALETTE_OP                93 // ignored
#define GENERAL_MATRIX_OP               94
#define TEXT_OP                         95 // ignored
#define SWITCH_OP                       96
#define LINE_STYLE_PALETTE_OP           97 // ignored
#define CLIP_REGION_OP                  98 // ignored
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
#define COLOR_NAME_PALETTE_OP           114 // ignored
#define CAT_OP                          115 // ignored
#define CAT_DATA_OP                     116 // ignored
#define PUSH_ATTRIBUTE_OP               122 // ignored
#define POP_ATTRIBUTE_OP                123 // ignored
#define CURVE_OP                        126 // ignored
#define ROAD_CONSTRUCTION_OP            127
#define LIGHT_PT_APPEARANCE_PALETTE_OP  128
#define LIGHT_PT_ANIMATION_PALETTE_OP   129
#define INDEXED_LIGHT_PT_OP             130
#define LIGHT_PT_SYSTEM_OP              131
#define INDEXED_STRING_OP               132 // ignored


// Obsolete opcodes as of 15.8
#define OBS_VERTEX_WITH_ID_OP            6
#define OBS_TRANSLATE_OP_0              12
#define OBS_DOF_OP                      13
#define OBS_INSTANCE_REF_OP             16
#define OBS_INSTANCE_DEF_OP             17
#define OBS_TRANSLATE_OP_1              40
#define OBS_ROTATE_POINT_OP_0           41
#define OBS_ROTATE_EDGE_OP              42
#define OBS_SCALE_OP_0                  43
#define OBS_SCALE_OP_1                  45
#define OBS_ROTATE_POINT_OP_1           46
#define OBS_ROTATE_SCALE_POINT_OP       47
#define OBS_PUT_OP                      48
#define OBS_EYEPOINT_PALETTE_OP         65
#define OBS_SCALE_OP_2         77


// Obsolete, but still referenced (possibly supported) by this loader
#define OLD_LOD_OP                       3
#define OLD_VERTEX_OP                    7
#define OLD_VERTEX_COLOR_OP              8
#define OLD_VERTEX_COLOR_NORMAL_OP       9
#define OLD_TRANSLATE_OP                44
#define OLD_MATERIAL_PALETTE_OP         66


#endif // __FLT_OPCODE_H


