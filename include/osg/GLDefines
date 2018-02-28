/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2017 Robert Osfield
 * Copyright (C) 2003-2005 3Dlabs Inc. Ltd.
 * Copyright (C) 2004-2005 Nathan Cournia
 * Copyright (C) 2007 Art Tevs
 * Copyright (C) 2008 Zebra Imaging
 * Copyright (C) 2010 VIRES Simulationstechnologie GmbH
 * Copyright (C) 2012 David Callu
 * Copyright (C) 2008 Mike Weiblen
 * Copyright (C) 2012 Holger Helmich
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial
 * applications, as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/


#ifndef OSG_GLDEFINES
#define OSG_GLDEFINES 1

#include <osg/Referenced>
#include <osg/GL>

#include <string>


// identify GLES 1.1
#if (defined(GL_VERSION_ES_CM_1_0) && GL_VERSION_ES_CM_1_0 > 0) || \
    (defined(GL_VERSION_ES_CM_1_1) && GL_VERSION_ES_CM_1_1 > 0)

    #define OPENGLES_1_1_FOUND 1
#endif

#ifndef GL_SAMPLER_2D_ARRAY_EXT
    #define GL_SAMPLER_1D_ARRAY_EXT           0x8DC0
    #define GL_SAMPLER_2D_ARRAY_EXT           0x8DC1
    #define GL_SAMPLER_1D_ARRAY_SHADOW_EXT    0x8DC3
    #define GL_SAMPLER_2D_ARRAY_SHADOW_EXT    0x8DC4
#endif

#if !defined(GL_VERSION_2_0)
typedef char GLchar;
#endif

#if !defined(GL_VERTEX_PROGRAM_POINT_SIZE)
    #define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
#endif
#if !defined(GL_VERTEX_PROGRAM_TWO_SIDE)
    #define GL_VERTEX_PROGRAM_TWO_SIDE        0x8643
#endif

#if !defined(GL_VERSION_2_0) && !defined(GL_ES_VERSION_2_0)
#define GL_VERSION_2_0 1
#define GL_BLEND_EQUATION_RGB             GL_BLEND_EQUATION
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED    0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE       0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE     0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE       0x8625
#define GL_CURRENT_VERTEX_ATTRIB          0x8626
#define GL_VERTEX_ATTRIB_ARRAY_POINTER    0x8645
#define GL_STENCIL_BACK_FUNC              0x8800
#define GL_STENCIL_BACK_FAIL              0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL   0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS   0x8803
#define GL_MAX_DRAW_BUFFERS               0x8824
#define GL_DRAW_BUFFER0                   0x8825
#define GL_DRAW_BUFFER1                   0x8826
#define GL_DRAW_BUFFER2                   0x8827
#define GL_DRAW_BUFFER3                   0x8828
#define GL_DRAW_BUFFER4                   0x8829
#define GL_DRAW_BUFFER5                   0x882A
#define GL_DRAW_BUFFER6                   0x882B
#define GL_DRAW_BUFFER7                   0x882C
#define GL_DRAW_BUFFER8                   0x882D
#define GL_DRAW_BUFFER9                   0x882E
#define GL_DRAW_BUFFER10                  0x882F
#define GL_DRAW_BUFFER11                  0x8830
#define GL_DRAW_BUFFER12                  0x8831
#define GL_DRAW_BUFFER13                  0x8832
#define GL_DRAW_BUFFER14                  0x8833
#define GL_DRAW_BUFFER15                  0x8834
#define GL_BLEND_EQUATION_ALPHA           0x883D
#define GL_POINT_SPRITE                   0x8861
#define GL_COORD_REPLACE                  0x8862
#define GL_MAX_VERTEX_ATTRIBS             0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_MAX_TEXTURE_COORDS             0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS        0x8872
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS  0x8B4A
#define GL_MAX_VARYING_FLOATS             0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_SHADER_TYPE                    0x8B4F
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
#define GL_INT_VEC2                       0x8B53
#define GL_INT_VEC3                       0x8B54
#define GL_INT_VEC4                       0x8B55
#define GL_BOOL                           0x8B56
#define GL_BOOL_VEC2                      0x8B57
#define GL_BOOL_VEC3                      0x8B58
#define GL_BOOL_VEC4                      0x8B59
#define GL_FLOAT_MAT2                     0x8B5A
#define GL_FLOAT_MAT3                     0x8B5B
#define GL_FLOAT_MAT4                     0x8B5C
#define GL_DELETE_STATUS                  0x8B80
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_ATTACHED_SHADERS               0x8B85
#define GL_ACTIVE_UNIFORMS                0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH      0x8B87
#define GL_SHADER_SOURCE_LENGTH           0x8B88
#define GL_ACTIVE_ATTRIBUTES              0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH    0x8B8A
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_CURRENT_PROGRAM                0x8B8D
#define GL_POINT_SPRITE_COORD_ORIGIN      0x8CA0
#define GL_LOWER_LEFT                     0x8CA1
#define GL_UPPER_LEFT                     0x8CA2
#define GL_STENCIL_BACK_REF               0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK        0x8CA4
#define GL_STENCIL_BACK_WRITEMASK         0x8CA5

#define GL_SAMPLER_2D                     0x8B5E
#define GL_SAMPLER_CUBE                   0x8B60
#endif

#if !defined(GL_SAMPLER_1D)
#define GL_SAMPLER_1D                     0x8B5D
#endif
#if !defined(GL_SAMPLER_3D)
#define GL_SAMPLER_3D                     0x8B5F
#endif

#if !defined(GL_SAMPLER_1D_SHADOW)
#define GL_SAMPLER_1D_SHADOW              0x8B61
#define GL_SAMPLER_2D_SHADOW              0x8B62
#endif

#ifndef GL_VERSION_2_1
#define GL_VERSION_2_1 1
#define GL_CURRENT_RASTER_SECONDARY_COLOR 0x845F
#define GL_PIXEL_PACK_BUFFER              0x88EB
#define GL_PIXEL_UNPACK_BUFFER            0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING      0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING    0x88EF
#define GL_FLOAT_MAT2x3                   0x8B65
#define GL_FLOAT_MAT2x4                   0x8B66
#define GL_FLOAT_MAT3x2                   0x8B67
#define GL_FLOAT_MAT3x4                   0x8B68
#define GL_FLOAT_MAT4x2                   0x8B69
#define GL_FLOAT_MAT4x3                   0x8B6A
#define GL_SRGB                           0x8C40
#define GL_SRGB8                          0x8C41
#define GL_SRGB_ALPHA                     0x8C42
#define GL_SRGB8_ALPHA8                   0x8C43
#define GL_SLUMINANCE_ALPHA               0x8C44
#define GL_SLUMINANCE8_ALPHA8             0x8C45
#define GL_SLUMINANCE                     0x8C46
#define GL_SLUMINANCE8                    0x8C47
#define GL_COMPRESSED_SRGB                0x8C48
#define GL_COMPRESSED_SRGB_ALPHA          0x8C49
#define GL_COMPRESSED_SLUMINANCE          0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA    0x8C4B
#endif

#ifndef GL_ARB_framebuffer_sRGB
#define GL_FRAMEBUFFER_SRGB               0x8DB9
#endif

#ifndef GL_EXT_geometry_shader4
#define GL_GEOMETRY_VERTICES_OUT_EXT            0x8DDA
#define GL_GEOMETRY_INPUT_TYPE_EXT              0x8DDB
#define GL_GEOMETRY_OUTPUT_TYPE_EXT             0x8DDC
#define GL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT  0x8DDD
#define GL_MAX_VERTEX_VARYING_COMPONENTS_EXT    0x8DDE
#define GL_LINES_ADJACENCY_EXT                  0x000A
#define GL_LINE_STRIP_ADJACENCY_EXT             0x000B
#define GL_TRIANGLES_ADJACENCY_EXT              0x000C
#define GL_TRIANGLE_STRIP_ADJACENCY_EXT         0x000D
#endif

#ifndef GL_VERSION_3_0
#define GL_MAX_VARYING_COMPONENTS                0x8B4B
#endif

#ifndef GL_VERSION_3_2
#define GL_GEOMETRY_SHADER                       0x8DD9
#define GL_GEOMETRY_VERTICES_OUT                 0x8916
#define GL_GEOMETRY_INPUT_TYPE                   0x8917
#define GL_GEOMETRY_OUTPUT_TYPE                  0x8918
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS      0x8C29
#define GL_MAX_GEOMETRY_VARYING_COMPONENTS       0x8DDD
#define GL_MAX_VERTEX_VARYING_COMPONENTS         0x8DDE
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS       0x8DDF
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES          0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS  0x8DE1
#define GL_LINES_ADJACENCY                       0x000A
#define GL_LINE_STRIP_ADJACENCY                  0x000B
#define GL_TRIANGLES_ADJACENCY                   0x000C
#define GL_TRIANGLE_STRIP_ADJACENCY              0x000D
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS  0x8DA8
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT    0x8DA9
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED        0x8DA7
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER  0x8CD4
#define GL_PROGRAM_POINT_SIZE                    0x8642
#endif

// ARB_tessellation_shader
#ifndef GL_ARB_tessellation_shader
#define GL_PATCHES                                             0x000E
#define GL_PATCH_VERTICES                                      0x8E72
#define GL_PATCH_DEFAULT_INNER_LEVEL                           0x8E73
#define GL_PATCH_DEFAULT_OUTER_LEVEL                           0x8E74
#define GL_MAX_PATCH_VERTICES                                  0x8E7D
#define GL_MAX_TESS_GEN_LEVEL                                  0x8E7E
#define GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS                 0x8E7F
#define GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS              0x8E80
#define GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS                0x8E81
#define GL_MAX_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS         0x8E82
#define GL_MAX_MAX_TESS_CONTROL_OUTPUT_COMPONENTS              0x8E83
#define GL_MAX_MAX_TESS_PATCH_COMPONENTS                       0x8E84
#define GL_MAX_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS        0x8E85
#define GL_MAX_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS           0x8E86
#define GL_MAX_MAX_TESS_CONTROL_UNIFORM_BLOCKS                 0x8E89
#define GL_MAX_MAX_TESS_EVALUATION_UNIFORM_BLOCKS              0x8E8A
#define GL_MAX_MAX_TESS_CONTROL_INPUT_COMPONENTS               0x886C
#define GL_MAX_MAX_TESS_EVALUATION_INPUT_COMPONENTS            0x886D
#define GL_MAX_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS    0x8E1E
#define GL_MAX_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS 0x8E1F
#define GL_TESS_EVALUATION_SHADER                              0x8E87
#define GL_TESS_CONTROL_SHADER                                 0x8E88
#define GL_TESS_CONTROL_OUTPUT_VERTICES                        0x8E75
#define GL_TESS_GEN_MODE                                       0x8E76
#define GL_TESS_GEN_SPACING                                    0x8E77
#define GL_TESS_GEN_VERTEX_ORDER                               0x8E78
#define GL_TESS_GEN_POINT_MODE                                 0x8E79
#define GL_ISOLINES                                            0x8E7A
#define GL_FRACTIONAL_ODD                                      0x8E7B
#define GL_FRACTIONAL_EVEN                                     0x8E7C
#define GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER     0x84F0
#define GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER  0x84F1
#endif

// EXT_gpu_shader4
#ifndef GL_EXT_gpu_shader4
#define GL_SAMPLER_1D_ARRAY_EXT               0x8DC0
#define GL_SAMPLER_2D_ARRAY_EXT               0x8DC1
#define GL_SAMPLER_BUFFER_EXT                 0x8DC2
#define GL_SAMPLER_1D_ARRAY_SHADOW_EXT        0x8DC3
#define GL_SAMPLER_2D_ARRAY_SHADOW_EXT        0x8DC4
#define GL_SAMPLER_CUBE_SHADOW_EXT            0x8DC5
#define GL_UNSIGNED_INT_VEC2_EXT              0x8DC6
#define GL_UNSIGNED_INT_VEC3_EXT              0x8DC7
#define GL_UNSIGNED_INT_VEC4_EXT              0x8DC8
#define GL_INT_SAMPLER_1D_EXT                 0x8DC9
#define GL_INT_SAMPLER_2D_EXT                 0x8DCA
#define GL_INT_SAMPLER_3D_EXT                 0x8DCB
#define GL_INT_SAMPLER_CUBE_EXT               0x8DCC
#define GL_INT_SAMPLER_2D_RECT_EXT            0x8DCD
#define GL_INT_SAMPLER_1D_ARRAY_EXT           0x8DCE
#define GL_INT_SAMPLER_2D_ARRAY_EXT           0x8DCF
#define GL_INT_SAMPLER_BUFFER_EXT             0x8DD0
#define GL_UNSIGNED_INT_SAMPLER_1D_EXT        0x8DD1
#define GL_UNSIGNED_INT_SAMPLER_2D_EXT        0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D_EXT        0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE_EXT      0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_2D_RECT_EXT   0x8DD5
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY_EXT  0x8DD6
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY_EXT  0x8DD7
#define GL_UNSIGNED_INT_SAMPLER_BUFFER_EXT    0x8DD8
#define GL_MIN_PROGRAM_TEXEL_OFFSET_EXT       0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET_EXT       0x8905
#endif

// ARB_uniform_buffer_object
#ifndef GL_ARB_uniform_buffer_object
#define GL_UNIFORM_BUFFER                 0x8A11
#define GL_UNIFORM_BUFFER_BINDING         0x8A28
#define GL_UNIFORM_BUFFER_START           0x8A29
#define GL_UNIFORM_BUFFER_SIZE            0x8A2A
#define GL_MAX_VERTEX_UNIFORM_BLOCKS      0x8A2B
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS    0x8A2C
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS    0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS    0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS    0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE         0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS 0x8A31
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS 0x8A32
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS 0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT 0x8A34
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH 0x8A35
#define GL_ACTIVE_UNIFORM_BLOCKS          0x8A36
#define GL_UNIFORM_TYPE                   0x8A37
#define GL_UNIFORM_SIZE                   0x8A38
#define GL_UNIFORM_NAME_LENGTH            0x8A39
#define GL_UNIFORM_BLOCK_INDEX            0x8A3A
#define GL_UNIFORM_OFFSET                 0x8A3B
#define GL_UNIFORM_ARRAY_STRIDE           0x8A3C
#define GL_UNIFORM_MATRIX_STRIDE          0x8A3D
#define GL_UNIFORM_IS_ROW_MAJOR           0x8A3E
#define GL_UNIFORM_BLOCK_BINDING          0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE        0x8A40
#define GL_UNIFORM_BLOCK_NAME_LENGTH      0x8A41
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS  0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES 0x8A43
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER 0x8A44
#define GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER 0x8A45
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER 0x8A46
#define GL_INVALID_INDEX                  0xFFFFFFFFu
#endif

// ARB_get_program_binary
#ifndef GL_ARB_get_program_binary
#define GL_PROGRAM_BINARY_RETRIEVABLE_HINT 0x8257
#define GL_PROGRAM_BINARY_LENGTH           0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS      0x87FE
#define GL_PROGRAM_BINARY_FORMATS          0x87FF
#endif

// ARB_gpu_shader_fp64
#ifndef GL_ARB_gpu_shader_fp64
#define GL_DOUBLE_VEC2                    0x8FFC
#define GL_DOUBLE_VEC3                    0x8FFD
#define GL_DOUBLE_VEC4                    0x8FFE
#define GL_DOUBLE_MAT2                    0x8F46
#define GL_DOUBLE_MAT3                    0x8F47
#define GL_DOUBLE_MAT4                    0x8F48
#define GL_DOUBLE_MAT2x3                  0x8F49
#define GL_DOUBLE_MAT2x4                  0x8F4A
#define GL_DOUBLE_MAT3x2                  0x8F4B
#define GL_DOUBLE_MAT3x4                  0x8F4C
#define GL_DOUBLE_MAT4x2                  0x8F4D
#define GL_DOUBLE_MAT4x3                  0x8F4E
#endif

// ARB_texture_multisample
#ifndef GL_ARB_texture_multisample
#define GL_SAMPLER_2D_MULTISAMPLE                    0x9108
#define GL_INT_SAMPLER_2D_MULTISAMPLE                0x9109
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE       0x910A
#define GL_SAMPLER_2D_MULTISAMPLE_ARRAY              0x910B
#define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY          0x910C
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910D
#endif

// GL_ARB_shader_image_load_store
#ifndef GL_ARB_shader_image_load_store
#define GL_IMAGE_1D                       0x904C
#define GL_IMAGE_2D                       0x904D
#define GL_IMAGE_3D                       0x904E
#define GL_IMAGE_2D_RECT                  0x904F
#define GL_IMAGE_CUBE                     0x9050
#define GL_IMAGE_BUFFER                   0x9051
#define GL_IMAGE_1D_ARRAY                 0x9052
#define GL_IMAGE_2D_ARRAY                 0x9053
#define GL_IMAGE_CUBE_MAP_ARRAY           0x9054
#define GL_IMAGE_2D_MULTISAMPLE           0x9055
#define GL_IMAGE_2D_MULTISAMPLE_ARRAY     0x9056
#define GL_INT_IMAGE_1D                   0x9057
#define GL_INT_IMAGE_2D                   0x9058
#define GL_INT_IMAGE_3D                   0x9059
#define GL_INT_IMAGE_2D_RECT              0x905A
#define GL_INT_IMAGE_CUBE                 0x905B
#define GL_INT_IMAGE_BUFFER               0x905C
#define GL_INT_IMAGE_1D_ARRAY             0x905D
#define GL_INT_IMAGE_2D_ARRAY             0x905E
#define GL_INT_IMAGE_CUBE_MAP_ARRAY       0x905F
#define GL_INT_IMAGE_2D_MULTISAMPLE       0x9060
#define GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY 0x9061
#define GL_UNSIGNED_INT_IMAGE_1D          0x9062
#define GL_UNSIGNED_INT_IMAGE_2D          0x9063
#define GL_UNSIGNED_INT_IMAGE_3D          0x9064
#define GL_UNSIGNED_INT_IMAGE_2D_RECT     0x9065
#define GL_UNSIGNED_INT_IMAGE_CUBE        0x9066
#define GL_UNSIGNED_INT_IMAGE_BUFFER      0x9067
#define GL_UNSIGNED_INT_IMAGE_1D_ARRAY    0x9068
#define GL_UNSIGNED_INT_IMAGE_2D_ARRAY    0x9069
#define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY 0x906A
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE 0x906B
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY 0x906C
#endif

#ifndef GL_VERSION_3_1
#define GL_SAMPLER_2D_RECT                0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW         0x8B64
#define GL_SAMPLER_BUFFER                 0x8DC2
#define GL_INT_SAMPLER_2D_RECT            0x8DCD
#define GL_INT_SAMPLER_BUFFER             0x8DD0
#define GL_UNSIGNED_INT_SAMPLER_2D_RECT   0x8DD5
#define GL_UNSIGNED_INT_SAMPLER_BUFFER    0x8DD8
#define GL_TEXTURE_BUFFER                 0x8C2A
#define GL_MAX_TEXTURE_BUFFER_SIZE        0x8C2B
#define GL_TEXTURE_BINDING_BUFFER         0x8C2C
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING 0x8C2D
#define GL_TEXTURE_RECTANGLE              0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE      0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE        0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE     0x84F8
#define GL_RGB_SNORM                      0x8F92
#define GL_RGBA_SNORM                     0x8F93
#define GL_R8_SNORM                       0x8F94
#define GL_RG8_SNORM                      0x8F95
#define GL_RGB8_SNORM                     0x8F96
#define GL_RGBA8_SNORM                    0x8F97
#define GL_R16_SNORM                      0x8F98
#define GL_RG16_SNORM                     0x8F99
#define GL_RGB16_SNORM                    0x8F9A
#define GL_RGBA16_SNORM                   0x8F9B
#define GL_SIGNED_NORMALIZED              0x8F9C
#define GL_PRIMITIVE_RESTART              0x8F9D
#define GL_PRIMITIVE_RESTART_INDEX        0x8F9E
#endif

#ifndef GL_RED_SNORM
#define GL_RED_SNORM                      0x8F90
#endif
#ifndef GL_RG_SNORM
#define GL_RG_SNORM                       0x8F91
#endif


#ifndef GL_VERSION_4_0
#define GL_SAMPLER_CUBE_MAP_ARRAY         0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW  0x900D
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY     0x900E
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY 0x900F
#define GL_DRAW_INDIRECT_BUFFER           0x8F3F
#define GL_DRAW_INDIRECT_BUFFER_BINDING   0x8F43
#endif

// ARB_shader_atomic_counters
#ifndef GL_ARB_shader_atomic_counters
#define GL_ATOMIC_COUNTER_BUFFER          0x92C0
#define GL_ATOMIC_COUNTER_BUFFER_BINDING  0x92C1
#define GL_ATOMIC_COUNTER_BUFFER_START    0x92C2
#define GL_ATOMIC_COUNTER_BUFFER_SIZE     0x92C3
#define GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE 0x92C4
#define GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS 0x92C5
#define GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES 0x92C6
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER 0x92C7
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER 0x92C8
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER 0x92C9
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER 0x92CA
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER 0x92CB
#define GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS 0x92CC
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS 0x92CD
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS 0x92CE
#define GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS 0x92CF
#define GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS 0x92D0
#define GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS 0x92D1
#define GL_MAX_VERTEX_ATOMIC_COUNTERS     0x92D2
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS 0x92D3
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS 0x92D4
#define GL_MAX_GEOMETRY_ATOMIC_COUNTERS   0x92D5
#define GL_MAX_FRAGMENT_ATOMIC_COUNTERS   0x92D6
#define GL_MAX_COMBINED_ATOMIC_COUNTERS   0x92D7
#define GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE 0x92D8
#define GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS 0x92DC
#define GL_ACTIVE_ATOMIC_COUNTER_BUFFERS  0x92D9
#define GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX 0x92DA
#define GL_UNSIGNED_INT_ATOMIC_COUNTER    0x92DB
#endif

// ARB_compute_shader
#ifndef GL_ARB_compute_shader
#define GL_COMPUTE_SHADER                 0x91B9
#define GL_MAX_COMPUTE_UNIFORM_BLOCKS     0x91BB
#define GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS 0x91BC
#define GL_MAX_COMPUTE_IMAGE_UNIFORMS     0x91BD
#define GL_MAX_COMPUTE_SHARED_MEMORY_SIZE 0x8262
#define GL_MAX_COMPUTE_UNIFORM_COMPONENTS 0x8263
#define GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS 0x8264
#define GL_MAX_COMPUTE_ATOMIC_COUNTERS    0x8265
#define GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS 0x8266
#define GL_MAX_COMPUTE_LOCAL_INVOCATIONS  0x90EB
#define GL_MAX_COMPUTE_WORK_GROUP_COUNT   0x91BE
#define GL_MAX_COMPUTE_WORK_GROUP_SIZE    0x91BF
#define GL_COMPUTE_LOCAL_WORK_SIZE        0x8267
#define GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER 0x90EC
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER 0x90ED
#define GL_DISPATCH_INDIRECT_BUFFER       0x90EE
#define GL_DISPATCH_INDIRECT_BUFFER_BINDING 0x90EF
#define GL_COMPUTE_SHADER_BIT             0x00000020
#endif

#ifndef GL_ARB_clip_control
#define GL_LOWER_LEFT 0x8CA1
#define GL_UPPER_LEFT 0x8CA2
#define GL_CLIP_ORIGIN 0x935C
#define GL_CLIP_DEPTH_MODE 0x935D
#define GL_NEGATIVE_ONE_TO_ONE 0x935E
#define GL_ZERO_TO_ONE 0x935F
#endif

#ifndef GL_ARB_depth_clamp
#define GL_DEPTH_CLAMP                    0x864F
#endif

#ifndef GL_ARB_provoking_vertex
#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION 0x8E4C
#define GL_FIRST_VERTEX_CONVENTION        0x8E4D
#define GL_LAST_VERTEX_CONVENTION         0x8E4E
#define GL_PROVOKING_VERTEX               0x8E4F
#endif

#ifndef GL_ARB_seamless_cube_map
#define GL_TEXTURE_CUBE_MAP_SEAMLESS      0x884F
#endif

#ifndef GL_VERSION_4_3
#define GL_SHADER_STORAGE_BARRIER_BIT 0x00002000
#define GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES 0x8F39
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#define GL_SHADER_STORAGE_BUFFER_BINDING 0x90D3
#define GL_SHADER_STORAGE_BUFFER_START 0x90D4
#define GL_SHADER_STORAGE_BUFFER_SIZE 0x90D5
#define GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS 0x90D6
#define GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS 0x90D7
#define GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS 0x90D8
#define GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS 0x90D9
#define GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS 0x90DA
#define GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS 0x90DB
#define GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS 0x90DC
#define GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS 0x90DD
#define GL_MAX_SHADER_STORAGE_BLOCK_SIZE 0x90DE
#define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT 0x90DF


#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 0x00000001
#define GL_ELEMENT_ARRAY_BARRIER_BIT 0x00000002
#define GL_UNIFORM_BARRIER_BIT 0x00000004
#define GL_TEXTURE_FETCH_BARRIER_BIT 0x00000008
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#define GL_COMMAND_BARRIER_BIT 0x00000040
#define GL_PIXEL_BUFFER_BARRIER_BIT 0x00000080
#define GL_TEXTURE_UPDATE_BARRIER_BIT 0x00000100
#define GL_BUFFER_UPDATE_BARRIER_BIT 0x00000200
#define GL_FRAMEBUFFER_BARRIER_BIT 0x00000400
#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT 0x00000800
#define GL_ATOMIC_COUNTER_BARRIER_BIT 0x00001000

#define GL_MAP_READ_BIT 0x0001
#define GL_MAP_WRITE_BIT 0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT 0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT 0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT 0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT 0x0020
#endif

#define GL_INT64_ARB               0x140E
#define GL_UNSIGNED_INT64_ARB      0x140F
#define GL_INT64_VEC2_ARB          0x8FE9
#define GL_INT64_VEC3_ARB          0x8FEA
#define GL_INT64_VEC4_ARB          0x8FEB
#define GL_UNSIGNED_INT64_VEC2_ARB 0x8FF5
#define GL_UNSIGNED_INT64_VEC3_ARB 0x8FF6
#define GL_UNSIGNED_INT64_VEC4_ARB 0x8FF7
/* ------------------------------ GL_KHR_debug ----------------------------- */
#ifndef GL_KHR_debug
#define GL_KHR_debug 1

#define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#define GL_STACK_OVERFLOW 0x0503
#define GL_STACK_UNDERFLOW 0x0504
#define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH 0x8243
#define GL_DEBUG_CALLBACK_FUNCTION 0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM 0x8245
#define GL_DEBUG_SOURCE_API 0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM 0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER 0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY 0x8249
#define GL_DEBUG_SOURCE_APPLICATION 0x824A
#define GL_DEBUG_SOURCE_OTHER 0x824B
#define GL_DEBUG_TYPE_ERROR 0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 0x824E
#define GL_DEBUG_TYPE_PORTABILITY 0x824F
#define GL_DEBUG_TYPE_PERFORMANCE 0x8250
#define GL_DEBUG_TYPE_OTHER 0x8251
#define GL_DEBUG_TYPE_MARKER 0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP 0x8269
#define GL_DEBUG_TYPE_POP_GROUP 0x826A
#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B
#define GL_MAX_DEBUG_GROUP_STACK_DEPTH 0x826C
#define GL_DEBUG_GROUP_STACK_DEPTH 0x826D
#define GL_BUFFER 0x82E0
#define GL_SHADER 0x82E1
#define GL_PROGRAM 0x82E2
#define GL_QUERY 0x82E3
#define GL_PROGRAM_PIPELINE 0x82E4
#define GL_SAMPLER 0x82E6
#define GL_DISPLAY_LIST 0x82E7
#define GL_MAX_LABEL_LENGTH 0x82E8
#define GL_MAX_DEBUG_MESSAGE_LENGTH 0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES 0x9144
#define GL_DEBUG_LOGGED_MESSAGES 0x9145
#define GL_DEBUG_SEVERITY_HIGH 0x9146
#define GL_DEBUG_SEVERITY_MEDIUM 0x9147
#define GL_DEBUG_SEVERITY_LOW 0x9148
#define GL_DEBUG_OUTPUT 0x92E0

#endif /* GL_KHR_debug */
#ifndef GL_ARB_sync
#define GL_MAX_SERVER_WAIT_TIMEOUT        0x9111
#define GL_OBJECT_TYPE                    0x9112
#define GL_SYNC_CONDITION                 0x9113
#define GL_SYNC_STATUS                    0x9114
#define GL_SYNC_FLAGS                     0x9115
#define GL_SYNC_FENCE                     0x9116
#define GL_SYNC_GPU_COMMANDS_COMPLETE     0x9117
#define GL_UNSIGNALED                     0x9118
#define GL_SIGNALED                       0x9119
#define GL_ALREADY_SIGNALED               0x911A
#define GL_TIMEOUT_EXPIRED                0x911B
#define GL_CONDITION_SATISFIED            0x911C
#define GL_WAIT_FAILED                    0x911D
#define GL_SYNC_FLUSH_COMMANDS_BIT        0x00000001
#define GL_TIMEOUT_IGNORED                0xFFFFFFFFFFFFFFFFull
#endif

#ifndef GL_TEXTURE_2D_ARRAY
    #define GL_TEXTURE_2D_ARRAY                        0x8C1A
    #define GL_PROXY_TEXTURE_2D_ARRAY                  0x8C1B
    #define GL_TEXTURE_BINDING_2D_ARRAY                0x8C1D
    #define GL_MAX_ARRAY_TEXTURE_LAYERS                0x88FF
    #define GL_COMPARE_REF_DEPTH_TO_TEXTURE            0x884E
    #define GL_SAMPLER_2D_ARRAY                        0x8DC1
    #define GL_SAMPLER_2D_ARRAY_SHADOW                 0x8DC4
    #define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER    0x8CD4
#endif

#ifndef GL_MAX_3D_TEXTURE_SIZE
    #define GL_MAX_3D_TEXTURE_SIZE      0x8073
#endif

#ifndef GL_TEXTURE_MIN_LOD
    #define GL_TEXTURE_MIN_LOD          0x813A
#endif

#ifndef GL_TEXTURE_MAX_LOD
    #define GL_TEXTURE_MAX_LOD          0x813B
#endif

#ifndef GL_TEXTURE_LOD_BIAS
    #define GL_TEXTURE_LOD_BIAS         0x8501
#endif


#ifndef GL_INTERLEAVED_ATTRIBS
    #define GL_INTERLEAVED_ATTRIBS      0x8C8C
#endif

#ifndef GL_SEPARATE_ATTRIBS
    #define GL_SEPARATE_ATTRIBS         0x8C8D
#endif

#ifndef GL_RASTERIZER_DISCARD
#define GL_RASTERIZER_DISCARD           0x8C89
#endif

#ifndef GL_ALPHA_TEST
    #define GL_ALPHA_TEST 0x0BC0
#endif

namespace osg
{
    #ifndef GL_VERSION_3_2
    typedef struct __GLsync *GLsync;
    #endif

    // for compatibility with gl.h headers that don't support VBO,
    //GL_VERSION_1_5 and GL_ARB_vertex_buffer_object provide these types for OpenGL
    //all ES versions except GL_OES_VERSION_1_0 provide these types for OpenGL ES
    #if !defined(GL_VERSION_1_5) && !defined(GL_ARB_vertex_buffer_object) \
    && !defined(GL_ES_VERSION_2_0) && !defined(OPENGLES_1_1_FOUND)

        #if 1
            // experimental defination.
            typedef ptrdiff_t GLsizeiptr;
            typedef ptrdiff_t GLintptr;
        #else

            #if defined(_WIN64)
                typedef __int64 GLintptr;
                typedef __int64 GLsizeiptr;
            #elif defined(__ia64__) || defined(__x86_64__) || defined(__ANDROID__)
                typedef long int GLintptr;
                typedef long int GLsizeiptr;
            #else
                typedef int GLintptr;
                typedef int GLsizeiptr;
            #endif
        #endif
    #endif
}

#endif
