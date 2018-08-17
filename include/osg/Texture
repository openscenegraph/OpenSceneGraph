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

#ifndef OSG_TEXTURE
#define OSG_TEXTURE 1

#include <osg/GL>
#include <osg/Image>
#include <osg/StateAttribute>
#include <osg/GraphicsContext>
#include <osg/ref_ptr>
#include <osg/Vec4>
#include <osg/Vec4d>
#include <osg/Vec4i>
#include <osg/buffered_value>
#include <osg/GLExtensions>

#include <list>
#include <map>

// If not defined by gl.h use the definition found in:
// http://oss.sgi.com/projects/ogl-sample/registry/EXT/texture_filter_anisotropic.txt
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif

// If not defined, use the definition found in:
// http://www.opengl.org/registry/specs/ARB/texture_swizzle.txt
#ifndef GL_TEXTURE_SWIZZLE_RGBA
    #define GL_TEXTURE_SWIZZLE_RGBA 0x8E46
#endif

#ifndef GL_ARB_texture_compression
    #define GL_COMPRESSED_ALPHA_ARB                 0x84E9
    #define GL_COMPRESSED_LUMINANCE_ARB             0x84EA
    #define GL_COMPRESSED_LUMINANCE_ALPHA_ARB       0x84EB
    #define GL_COMPRESSED_INTENSITY_ARB             0x84EC
    #define GL_COMPRESSED_RGB_ARB                   0x84ED
    #define GL_COMPRESSED_RGBA_ARB                  0x84EE
    #define GL_TEXTURE_COMPRESSION_HINT_ARB         0x84EF
    #define GL_TEXTURE_COMPRESSED_ARB               0x86A1
    #define GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB   0x86A2
    #define GL_COMPRESSED_TEXTURE_FORMATS_ARB       0x86A3
#endif

#ifndef GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB
    #define GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB    0x86A0
#endif

#ifndef GL_EXT_texture_compression_s3tc
    #define GL_COMPRESSED_RGB_S3TC_DXT1_EXT         0x83F0
    #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT        0x83F1
    #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT        0x83F2
    #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT        0x83F3
#endif

#ifndef GL_EXT_texture_compression_rgtc
  #define GL_COMPRESSED_RED_RGTC1_EXT                0x8DBB
  #define GL_COMPRESSED_SIGNED_RED_RGTC1_EXT         0x8DBC
  #define GL_COMPRESSED_RED_GREEN_RGTC2_EXT          0x8DBD
  #define GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT   0x8DBE
#endif

#ifndef GL_IMG_texture_compression_pvrtc
    #define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG      0x8C00
    #define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG      0x8C01
    #define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG     0x8C02
    #define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG     0x8C03
#endif

#ifndef GL_OES_compressed_ETC1_RGB8_texture
    #define GL_ETC1_RGB8_OES                            0x8D64
#endif

#ifndef GL_ARB_INTERNAL_TEXTURE_FORMAT
    #define GL_RGBA32F_ARB                           0x8814
    #define GL_RGB32F_ARB                            0x8815
    #define GL_ALPHA32F_ARB                          0x8816
    #define GL_INTENSITY32F_ARB                      0x8817
    #define GL_LUMINANCE32F_ARB                      0x8818
    #define GL_LUMINANCE_ALPHA32F_ARB                0x8819
    #define GL_RGBA16F_ARB                           0x881A
    #define GL_RGB16F_ARB                            0x881B
    #define GL_ALPHA16F_ARB                          0x881C
    #define GL_INTENSITY16F_ARB                      0x881D
    #define GL_LUMINANCE16F_ARB                      0x881E
    #define GL_LUMINANCE_ALPHA16F_ARB                0x881F
#endif

#ifndef GL_HALF_FLOAT
    #define GL_HALF_FLOAT                            0x140B
#endif

#ifndef GL_NV_texture_shader
    #define GL_HILO_NV                              0x86F4
    #define GL_DSDT_NV                              0x86F5
    #define GL_DSDT_MAG_NV                          0x86F6
    #define GL_DSDT_MAG_VIB_NV                      0x86F7
    #define GL_HILO16_NV                            0x86F8
    #define GL_SIGNED_HILO_NV                       0x86F9
    #define GL_SIGNED_HILO16_NV                     0x86FA
    #define GL_SIGNED_RGBA_NV                       0x86FB
    #define GL_SIGNED_RGBA8_NV                      0x86FC
    #define GL_SIGNED_RGB_NV                        0x86FE
    #define GL_SIGNED_RGB8_NV                       0x86FF
    #define GL_SIGNED_LUMINANCE_NV                  0x8701
    #define GL_SIGNED_LUMINANCE8_NV                 0x8702
    #define GL_SIGNED_LUMINANCE_ALPHA_NV            0x8703
    #define GL_SIGNED_LUMINANCE8_ALPHA8_NV          0x8704
    #define GL_SIGNED_ALPHA_NV                      0x8705
    #define GL_SIGNED_ALPHA8_NV                     0x8706
    #define GL_SIGNED_INTENSITY_NV                  0x8707
    #define GL_SIGNED_INTENSITY8_NV                 0x8708
    #define GL_DSDT8_NV                             0x8709
    #define GL_DSDT8_MAG8_NV                        0x870A
    #define GL_DSDT8_MAG8_INTENSITY8_NV             0x870B
    #define GL_SIGNED_RGB_UNSIGNED_ALPHA_NV         0x870C
    #define GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV       0x870D
#endif

#ifndef GL_NV_float_buffer
    #define GL_FLOAT_R_NV                           0x8880
    #define GL_FLOAT_RG_NV                          0x8881
    #define GL_FLOAT_RGB_NV                         0x8882
    #define GL_FLOAT_RGBA_NV                        0x8883
    #define GL_FLOAT_R16_NV                         0x8884
    #define GL_FLOAT_R32_NV                         0x8885
    #define GL_FLOAT_RG16_NV                        0x8886
    #define GL_FLOAT_RG32_NV                        0x8887
    #define GL_FLOAT_RGB16_NV                       0x8888
    #define GL_FLOAT_RGB32_NV                       0x8889
    #define GL_FLOAT_RGBA16_NV                      0x888A
    #define GL_FLOAT_RGBA32_NV                      0x888B
#endif

#ifndef GL_ATI_texture_float
    #define GL_RGBA_FLOAT32_ATI                     0x8814
    #define GL_RGB_FLOAT32_ATI                      0x8815
    #define GL_ALPHA_FLOAT32_ATI                    0x8816
    #define GL_INTENSITY_FLOAT32_ATI                0x8817
    #define GL_LUMINANCE_FLOAT32_ATI                0x8818
    #define GL_LUMINANCE_ALPHA_FLOAT32_ATI          0x8819
    #define GL_RGBA_FLOAT16_ATI                     0x881A
    #define GL_RGB_FLOAT16_ATI                      0x881B
    #define GL_ALPHA_FLOAT16_ATI                    0x881C
    #define GL_INTENSITY_FLOAT16_ATI                0x881D
    #define GL_LUMINANCE_FLOAT16_ATI                0x881E
    #define GL_LUMINANCE_ALPHA_FLOAT16_ATI          0x881F
#endif

#ifndef GL_MIRRORED_REPEAT_IBM
    #define GL_MIRRORED_REPEAT_IBM            0x8370
#endif

#ifndef GL_CLAMP_TO_EDGE
    #define GL_CLAMP_TO_EDGE                  0x812F
#endif

#ifndef GL_CLAMP
    #define GL_CLAMP                          0x2900
#endif

#ifndef GL_CLAMP_TO_BORDER_ARB
    #define GL_CLAMP_TO_BORDER_ARB            0x812D
#endif

#ifndef GL_INTENSITY
    // OpenGL ES1 and ES2 doesn't provide GL_INTENSITY
    #define GL_INTENSITY 0x8049
#endif

#ifndef GL_GENERATE_MIPMAP_SGIS
    #define GL_GENERATE_MIPMAP_SGIS           0x8191
    #define GL_GENERATE_MIPMAP_HINT_SGIS      0x8192
#endif

#ifndef GL_TEXTURE_3D
    #define GL_TEXTURE_3D                     0x806F
#endif


#ifndef GL_TEXTURE_CUBE_MAP
    #define GL_TEXTURE_CUBE_MAP             0x8513
    #define GL_TEXTURE_BINDING_CUBE_MAP     0x8514
    #define GL_TEXTURE_CUBE_MAP_POSITIVE_X  0x8515
    #define GL_TEXTURE_CUBE_MAP_NEGATIVE_X  0x8516
    #define GL_TEXTURE_CUBE_MAP_POSITIVE_Y  0x8517
    #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y  0x8518
    #define GL_TEXTURE_CUBE_MAP_POSITIVE_Z  0x8519
    #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z  0x851A
    #define GL_PROXY_TEXTURE_CUBE_MAP       0x851B
    #define GL_MAX_CUBE_MAP_TEXTURE_SIZE    0x851C
#endif

#ifndef GL_TEXTURE_BINDING_3D
    #define GL_TEXTURE_BINDING_3D             0x806A
#endif

#ifndef GL_DEPTH_TEXTURE_MODE_ARB
    #define GL_DEPTH_TEXTURE_MODE_ARB         0x884B
#endif

#ifndef GL_TEXTURE_COMPARE_MODE_ARB
    #define GL_TEXTURE_COMPARE_MODE_ARB       0x884C
#endif
#ifndef GL_TEXTURE_COMPARE_FUNC_ARB
    #define GL_TEXTURE_COMPARE_FUNC_ARB       0x884D
#endif
#ifndef GL_COMPARE_R_TO_TEXTURE_ARB
    #define GL_COMPARE_R_TO_TEXTURE_ARB       0x884E
#endif

#ifndef TEXTURE_COMPARE_FAIL_VALUE_ARB
    #define TEXTURE_COMPARE_FAIL_VALUE_ARB    0x80BF
#endif

#if !defined( GL_MAX_TEXTURE_UNITS )
    #define GL_MAX_TEXTURE_UNITS              0x84E2
#endif

#ifndef  GL_TEXTURE_DEPTH
    #define GL_TEXTURE_DEPTH                  0x8071
#endif

#ifndef GL_TEXTURE_2D_MULTISAMPLE
    #define GL_TEXTURE_2D_MULTISAMPLE         0x9100
#endif

// Integer texture extension as in http://www.opengl.org/registry/specs/EXT/texture_integer.txt
#ifndef GL_EXT_texture_integer
    #define GL_RGBA32UI_EXT                                    0x8D70
    #define GL_RGB32UI_EXT                                     0x8D71
    #define GL_ALPHA32UI_EXT                                   0x8D72
    #define GL_INTENSITY32UI_EXT                               0x8D73
    #define GL_LUMINANCE32UI_EXT                               0x8D74
    #define GL_LUMINANCE_ALPHA32UI_EXT                         0x8D75

    #define GL_RGBA16UI_EXT                                    0x8D76
    #define GL_RGB16UI_EXT                                     0x8D77
    #define GL_ALPHA16UI_EXT                                   0x8D78
    #define GL_INTENSITY16UI_EXT                               0x8D79
    #define GL_LUMINANCE16UI_EXT                               0x8D7A
    #define GL_LUMINANCE_ALPHA16UI_EXT                         0x8D7B

    #define GL_RGBA8UI_EXT                                     0x8D7C
    #define GL_RGB8UI_EXT                                      0x8D7D
    #define GL_ALPHA8UI_EXT                                    0x8D7E
    #define GL_INTENSITY8UI_EXT                                0x8D7F
    #define GL_LUMINANCE8UI_EXT                                0x8D80
    #define GL_LUMINANCE_ALPHA8UI_EXT                          0x8D81

    #define GL_RGBA32I_EXT                                     0x8D82
    #define GL_RGB32I_EXT                                      0x8D83
    #define GL_ALPHA32I_EXT                                    0x8D84
    #define GL_INTENSITY32I_EXT                                0x8D85
    #define GL_LUMINANCE32I_EXT                                0x8D86
    #define GL_LUMINANCE_ALPHA32I_EXT                          0x8D87

    #define GL_RGBA16I_EXT                                     0x8D88
    #define GL_RGB16I_EXT                                      0x8D89
    #define GL_ALPHA16I_EXT                                    0x8D8A
    #define GL_INTENSITY16I_EXT                                0x8D8B
    #define GL_LUMINANCE16I_EXT                                0x8D8C
    #define GL_LUMINANCE_ALPHA16I_EXT                          0x8D8D

    #define GL_RGBA8I_EXT                                      0x8D8E
    #define GL_RGB8I_EXT                                       0x8D8F
    #define GL_ALPHA8I_EXT                                     0x8D90
    #define GL_INTENSITY8I_EXT                                 0x8D91
    #define GL_LUMINANCE8I_EXT                                 0x8D92
    #define GL_LUMINANCE_ALPHA8I_EXT                           0x8D93

    #define GL_RED_INTEGER_EXT                                 0x8D94
    #define GL_GREEN_INTEGER_EXT                               0x8D95
    #define GL_BLUE_INTEGER_EXT                                0x8D96
    #define GL_ALPHA_INTEGER_EXT                               0x8D97
    #define GL_RGB_INTEGER_EXT                                 0x8D98
    #define GL_RGBA_INTEGER_EXT                                0x8D99
    #define GL_BGR_INTEGER_EXT                                 0x8D9A
    #define GL_BGRA_INTEGER_EXT                                0x8D9B
    #define GL_LUMINANCE_INTEGER_EXT                           0x8D9C
    #define GL_LUMINANCE_ALPHA_INTEGER_EXT                     0x8D9D

    #define GL_RGBA_INTEGER_MODE_EXT                           0x8D9E
#endif

#ifndef GL_VERSION_1_1
#define GL_R3_G3_B2                       0x2A10
#define GL_RGB4                           0x804F
#define GL_RGB5                           0x8050
#define GL_RGB8                           0x8051
#define GL_RGB10                          0x8052
#define GL_RGB12                          0x8053
#define GL_RGB16                          0x8054
#define GL_RGBA2                          0x8055
#define GL_RGBA4                          0x8056
#define GL_RGB5_A1                        0x8057
#define GL_RGBA8                          0x8058
#define GL_RGB10_A2                       0x8059
#define GL_RGBA12                         0x805A
#define GL_RGBA16                         0x805B
#define GL_BGR_EXT                        0x80E0
#define GL_BGRA_EXT                       0x80E1
#endif

#ifndef GL_ARB_texture_rg
    #define GL_RG                             0x8227
    #define GL_RG_INTEGER                     0x8228
    #define GL_R8                             0x8229
    #define GL_R16                            0x822A
    #define GL_RG8                            0x822B
    #define GL_RG16                           0x822C
    #define GL_R16F                           0x822D
    #define GL_R32F                           0x822E
    #define GL_RG16F                          0x822F
    #define GL_RG32F                          0x8230
    #define GL_R8I                            0x8231
    #define GL_R8UI                           0x8232
    #define GL_R16I                           0x8233
    #define GL_R16UI                          0x8234
    #define GL_R32I                           0x8235
    #define GL_R32UI                          0x8236
    #define GL_RG8I                           0x8237
    #define GL_RG8UI                          0x8238
    #define GL_RG16I                          0x8239
    #define GL_RG16UI                         0x823A
    #define GL_RG32I                          0x823B
    #define GL_RG32UI                         0x823C
#endif

#ifndef GL_ARB_shader_image_load_store
    #define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 0x00000001
    #define GL_ELEMENT_ARRAY_BARRIER_BIT      0x00000002
    #define GL_UNIFORM_BARRIER_BIT            0x00000004
    #define GL_TEXTURE_FETCH_BARRIER_BIT      0x00000008
    #define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
    #define GL_COMMAND_BARRIER_BIT            0x00000040
    #define GL_PIXEL_BUFFER_BARRIER_BIT       0x00000080
    #define GL_TEXTURE_UPDATE_BARRIER_BIT     0x00000100
    #define GL_BUFFER_UPDATE_BARRIER_BIT      0x00000200
    #define GL_FRAMEBUFFER_BARRIER_BIT        0x00000400
    #define GL_TRANSFORM_FEEDBACK_BARRIER_BIT 0x00000800
    #define GL_ATOMIC_COUNTER_BARRIER_BIT     0x00001000
    #define GL_ALL_BARRIER_BITS               0xFFFFFFFF
    #define GL_MAX_IMAGE_UNITS                0x8F38
    #define GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS 0x8F39
    #define GL_IMAGE_BINDING_NAME             0x8F3A
    #define GL_IMAGE_BINDING_LEVEL            0x8F3B
    #define GL_IMAGE_BINDING_LAYERED          0x8F3C
    #define GL_IMAGE_BINDING_LAYER            0x8F3D
    #define GL_IMAGE_BINDING_ACCESS           0x8F3E
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
    #define GL_MAX_IMAGE_SAMPLES              0x906D
    #define GL_IMAGE_BINDING_FORMAT           0x906E
    #define GL_IMAGE_FORMAT_COMPATIBILITY_TYPE 0x90C7
    #define GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE 0x90C8
    #define GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS 0x90C9
    #define GL_MAX_VERTEX_IMAGE_UNIFORMS      0x90CA
    #define GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS 0x90CB
    #define GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS 0x90CC
    #define GL_MAX_GEOMETRY_IMAGE_UNIFORMS    0x90CD
    #define GL_MAX_FRAGMENT_IMAGE_UNIFORMS    0x90CE
    #define GL_MAX_COMBINED_IMAGE_UNIFORMS    0x90CF
#endif

//#define OSG_COLLECT_TEXTURE_APPLIED_STATS 1

namespace osg {


// forward declare
class TextureObjectSet;
class TextureObjectManager;


/** Texture pure virtual base class that encapsulates OpenGL texture
  * functionality common to the various types of OSG textures.
*/
class OSG_EXPORT Texture : public osg::StateAttribute
{

    public :

        Texture();

        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
        Texture(const Texture& text,const CopyOp& copyop=CopyOp::SHALLOW_COPY);

        virtual osg::Object* cloneType() const = 0;
        virtual osg::Object* clone(const CopyOp& copyop) const = 0;
        virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const Texture *>(obj)!=NULL; }
        virtual const char* libraryName() const { return "osg"; }
        virtual const char* className() const { return "Texture"; }

        /** Fast alternative to dynamic_cast<> for determining if state attribute is a Texture.*/
        virtual Texture* asTexture() { return this; }

        /** Fast alternative to dynamic_cast<> for determining if state attribute is a Texture.*/
        virtual const Texture* asTexture() const { return this; }

        virtual Type getType() const { return TEXTURE; }

        virtual bool isTextureAttribute() const { return true; }

        virtual GLenum getTextureTarget() const = 0;

        #ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
        virtual bool getModeUsage(StateAttribute::ModeUsage& usage) const
        {
                usage.usesTextureMode(getTextureTarget());
            return true;
        }
        #endif

        virtual int getTextureWidth() const { return 0; }
        virtual int getTextureHeight() const { return 0; }
        virtual int getTextureDepth() const { return 0; }

        enum WrapParameter {
            WRAP_S,
            WRAP_T,
            WRAP_R
        };

        enum WrapMode {
            CLAMP  = GL_CLAMP,
            CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
            CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER_ARB,
            REPEAT = GL_REPEAT,
            MIRROR = GL_MIRRORED_REPEAT_IBM
        };

        /** Sets the texture wrap mode. */
        void setWrap(WrapParameter which, WrapMode wrap);
        /** Gets the texture wrap mode. */
        WrapMode getWrap(WrapParameter which) const;


        /** Sets the border color. Only used when wrap mode is CLAMP_TO_BORDER.
         * The border color will be casted to the appropriate type to match the
         * internal pixel format of the texture. */
        void setBorderColor(const Vec4d& color) { _borderColor = color; dirtyTextureParameters(); }

        /** Gets the border color. */
        const Vec4d& getBorderColor() const { return _borderColor; }

        /** Sets the border width. */
        void setBorderWidth(GLint width) { _borderWidth = width; dirtyTextureParameters(); }

        GLint getBorderWidth() const { return _borderWidth; }

        enum FilterParameter {
            MIN_FILTER,
            MAG_FILTER
        };

        enum FilterMode {
            LINEAR                    = GL_LINEAR,
            LINEAR_MIPMAP_LINEAR      = GL_LINEAR_MIPMAP_LINEAR,
            LINEAR_MIPMAP_NEAREST     = GL_LINEAR_MIPMAP_NEAREST,
            NEAREST                   = GL_NEAREST,
            NEAREST_MIPMAP_LINEAR     = GL_NEAREST_MIPMAP_LINEAR,
            NEAREST_MIPMAP_NEAREST    = GL_NEAREST_MIPMAP_NEAREST
        };

        /** Sets the texture filter mode. */
        void setFilter(FilterParameter which, FilterMode filter);

        /** Gets the texture filter mode. */
        FilterMode getFilter(FilterParameter which) const;

        /** Sets the maximum anisotropy value, default value is 1.0 for no
          * anisotropic filtering. If hardware does not support anisotropic
          * filtering, use normal filtering (equivalent to a max anisotropy
          * value of 1.0. Valid range is 1.0f upwards.  The maximum value
          * depends on the graphics system. */
        void setMaxAnisotropy(float anis);

        /** Gets the maximum anisotropy value. */
        inline float getMaxAnisotropy() const { return _maxAnisotropy; }

        /** Sets the minimum level of detail value. */
        void setMinLOD(float minlod);

        /** Gets the minimum level of detail value. */
        inline float getMinLOD() const { return _minlod; }

        /** Sets the maximum level of detail value. */
        void setMaxLOD(float maxlod);

        /** Gets the maximum level of detail value. */
        inline float getMaxLOD() const { return _maxlod; }

        /** Gets the level of detail bias value. */
        void setLODBias(float lodbias);

        /** Sets the level of detail bias value. */
        inline float getLODBias() const { return _lodbias; }

        /** Configure the source of texture swizzling for all channels */
        inline void setSwizzle(const Vec4i& swizzle) { _swizzle = swizzle; dirtyTextureParameters(); };

        /** Gets the source of texture swizzling for all channels */
        inline const Vec4i& getSwizzle() const { return _swizzle; }

        /** Sets the hardware mipmap generation hint. If enabled, it will
          * only be used if supported in the graphics system. */
        inline void setUseHardwareMipMapGeneration(bool useHardwareMipMapGeneration) { _useHardwareMipMapGeneration = useHardwareMipMapGeneration; }

        /** Gets the hardware mipmap generation hint. */
        inline bool getUseHardwareMipMapGeneration() const { return _useHardwareMipMapGeneration; }

        /** Sets whether or not the apply() function will unreference the image
          * data. If enabled, and the image data is only referenced by this
          * Texture, apply() will delete the image data. */
        inline void setUnRefImageDataAfterApply(bool flag) { _unrefImageDataAfterApply = flag; }

        /** Gets whether or not apply() unreferences image data. */
        inline bool getUnRefImageDataAfterApply() const { return _unrefImageDataAfterApply; }

        /** Sets whether to use client storage for the texture, if supported
          * by the graphics system. Note: If enabled, and the graphics system
          * supports it, the osg::Image(s) associated with this texture cannot
          * be deleted, so the UnRefImageDataAfterApply flag would be ignored. */
        inline void setClientStorageHint(bool flag) { _clientStorageHint = flag; }

        /** Gets whether to use client storage for the texture. */
        inline bool getClientStorageHint() const { return _clientStorageHint; }

        /** Sets whether to force the texture to resize images that have dimensions
          * that are not a power of two. If enabled, NPOT images will be resized,
          * whether or not NPOT textures are supported by the hardware. If disabled,
          * NPOT images will not be resized if supported by hardware. */
        inline void setResizeNonPowerOfTwoHint(bool flag) { _resizeNonPowerOfTwoHint = flag; }

        /** Gets whether texture will force non power to two images to be resized. */
        inline bool getResizeNonPowerOfTwoHint() const { return _resizeNonPowerOfTwoHint; }

        enum InternalFormatMode {
            USE_IMAGE_DATA_FORMAT,
            USE_USER_DEFINED_FORMAT,
            USE_ARB_COMPRESSION,
            USE_S3TC_DXT1_COMPRESSION,
            USE_S3TC_DXT3_COMPRESSION,
            USE_S3TC_DXT5_COMPRESSION,
            USE_PVRTC_2BPP_COMPRESSION,
            USE_PVRTC_4BPP_COMPRESSION,
            USE_ETC_COMPRESSION,
            USE_ETC2_COMPRESSION,
            USE_RGTC1_COMPRESSION,
            USE_RGTC2_COMPRESSION,
            USE_S3TC_DXT1c_COMPRESSION,
            USE_S3TC_DXT1a_COMPRESSION
        };

        /** Sets the internal texture format mode. Note: If the texture format is
          * USE_IMAGE_DATA_FORMAT, USE_ARB_COMPRESSION, or USE_S3TC_COMPRESSION,
          * the internal format mode is set automatically and will overwrite the
          * previous _internalFormat. */
        inline void setInternalFormatMode(InternalFormatMode mode) { _internalFormatMode = mode; }

        /** Gets the internal texture format mode. */
        inline InternalFormatMode getInternalFormatMode() const { return _internalFormatMode; }

        /** Sets the internal texture format. Implicitly sets the
          * internalFormatMode to USE_USER_DEFINED_FORMAT.
          * The corresponding internal format type will be computed. */
        inline void setInternalFormat(GLint internalFormat)
        {
            _internalFormatMode = USE_USER_DEFINED_FORMAT;
            _internalFormat = internalFormat;
            computeInternalFormatType();
        }


        /** Gets the internal texture format. */
        inline GLint getInternalFormat() const { if (_internalFormat==0) computeInternalFormat(); return _internalFormat; }

        /** Return true if the internal format is one of the compressed formats.*/
        bool isCompressedInternalFormat() const;

        /** Sets the external source image format, used as a fallback when no osg::Image is attached to provide the source image format. */
        inline void setSourceFormat(GLenum sourceFormat) { _sourceFormat = sourceFormat; }

        /** Gets the external source image format. */
        inline GLenum getSourceFormat() const { return _sourceFormat; }

        /** Sets the external source data type, used as a fallback when no osg::Image is attached to provide the source image format.*/
        inline void setSourceType(GLenum sourceType) { _sourceType = sourceType; }

        /** Gets the external source data type.*/
        inline GLenum getSourceType() const { return _sourceType; }

        /** Texture type determined by the internal texture format */
        enum InternalFormatType{

            //! default OpenGL format (clamped values to [0,1) or [0,255])
            NORMALIZED = 0x0,

            //! float values, Shader Model 3.0 (see ARB_texture_float)
            FLOAT = 0x1,

            //! Signed integer values (see EXT_texture_integer)
            SIGNED_INTEGER = 0x2,

            //! Unsigned integer value (see EXT_texture_integer)
            UNSIGNED_INTEGER = 0x4
        };

        /** Get the internal texture format type. */
        inline InternalFormatType getInternalFormatType() const { return _internalFormatType; }

        /* select the size internal format to use based on Image when available or Texture format settings.*/
        GLenum selectSizedInternalFormat(const osg::Image* image=0) const;

        class TextureObject;

        /** return true if the texture image data has been modified and the associated GL texture object needs to be updated.*/
        virtual bool isDirty(unsigned int /*contextID*/) const { return false; }


        /** Returns a pointer to the TextureObject for the current context. */
        inline TextureObject* getTextureObject(unsigned int contextID) const
        {
            return _textureObjectBuffer[contextID].get();
        }

        inline void setTextureObject(unsigned int contextID, TextureObject* to)
        {
            _textureObjectBuffer[contextID] = to;
        }

        /** Forces a recompile on next apply() of associated OpenGL texture
          * objects. */
        void dirtyTextureObject();

        /** Returns true if the texture objects for all the required graphics
          * contexts are loaded. */
        bool areAllTextureObjectsLoaded() const;


        /** Gets the dirty flag for the current contextID. */
        inline unsigned int& getTextureParameterDirty(unsigned int contextID) const
        {
            return _texParametersDirtyList[contextID];
        }


        /** Force a reset on next apply() of associated OpenGL texture
          * parameters. */
        void dirtyTextureParameters();

        /** Force a manual allocation of the mipmap levels on the next apply() call.
          * User is responsible for filling the mipmap levels with valid data.
          * The OpenGL's glGenerateMipmapEXT function is used to generate the mipmap levels.
          * If glGenerateMipmapEXT is not supported or texture's internal format is not supported
          * by the glGenerateMipmapEXT, then empty mipmap levels will
          * be allocated manually. The mipmap levels are also allocated if a non-mipmapped
          * min filter is used. */
        void allocateMipmapLevels();

        /** Sets GL_TEXTURE_COMPARE_MODE_ARB to GL_COMPARE_R_TO_TEXTURE_ARB
          * See http://oss.sgi.com/projects/ogl-sample/registry/ARB/shadow.txt. */
        void setShadowComparison(bool flag) { _use_shadow_comparison = flag; }
        bool getShadowComparison() const { return _use_shadow_comparison; }

        enum ShadowCompareFunc {
            NEVER = GL_NEVER,
            LESS = GL_LESS,
            EQUAL = GL_EQUAL,
            LEQUAL = GL_LEQUAL,
            GREATER = GL_GREATER,
            NOTEQUAL = GL_NOTEQUAL,
            GEQUAL = GL_GEQUAL,
            ALWAYS = GL_ALWAYS
        };

        /** Sets shadow texture comparison function. */
        void setShadowCompareFunc(ShadowCompareFunc func) { _shadow_compare_func = func; }
        ShadowCompareFunc getShadowCompareFunc() const { return _shadow_compare_func; }

        enum ShadowTextureMode {
            LUMINANCE = GL_LUMINANCE,
            INTENSITY = GL_INTENSITY,
            ALPHA = GL_ALPHA,
            NONE = GL_NONE
        };

        /** Sets shadow texture mode after comparison. */
        void setShadowTextureMode(ShadowTextureMode mode) { _shadow_texture_mode = mode; }
        ShadowTextureMode getShadowTextureMode() const { return _shadow_texture_mode; }

        /** Sets the TEXTURE_COMPARE_FAIL_VALUE_ARB texture parameter. See
          * http://oss.sgi.com/projects/ogl-sample/registry/ARB/shadow_ambient.txt. */
        void setShadowAmbient(float shadow_ambient) { _shadow_ambient = shadow_ambient; }
        float getShadowAmbient() const { return _shadow_ambient; }


        /** Sets the texture image for the specified face. */
        virtual void setImage(unsigned int face, Image* image) = 0;

        template<class T> void setImage(unsigned int face, const ref_ptr<T>& image) { setImage(face, image.get()); }

        /** Gets the texture image for the specified face. */
        virtual Image* getImage(unsigned int face) = 0;

        /** Gets the const texture image for specified face. */
        virtual const Image* getImage(unsigned int face) const = 0;

        /** Gets the number of images that can be assigned to this Texture. */
        virtual unsigned int getNumImages() const = 0;


        /** Set the PBuffer graphics context to read from when using PBuffers for RenderToTexture.*/
        void setReadPBuffer(GraphicsContext* context) { _readPBuffer = context; }

        template<class T> void setReadPBuffer(const ref_ptr<T>& context) { setReadPBuffer(context.get()); }

        /** Get the PBuffer graphics context to read from when using PBuffers for RenderToTexture.*/
        GraphicsContext* getReadPBuffer() { return _readPBuffer.get(); }

        /** Get the const PBuffer graphics context to read from when using PBuffers for RenderToTexture.*/
        const GraphicsContext* getReadPBuffer() const { return _readPBuffer.get(); }

        /** Texture is a pure virtual base class, apply must be overridden. */
        virtual void apply(State& state) const = 0;

        /** Calls apply(state) to compile the texture. */
        virtual void compileGLObjects(State& state) const;

        /** Resize any per context GLObject buffers to specified size. */
        virtual void resizeGLObjectBuffers(unsigned int maxSize);

        /** If State is non-zero, this function releases OpenGL objects for
          * the specified graphics context. Otherwise, releases OpenGL objects
          * for all graphics contexts. */
        virtual void releaseGLObjects(State* state=0) const;

        /** Determine whether the given internalFormat is a compressed
          * image format. */
        static bool isCompressedInternalFormat(GLint internalFormat);

        /** Determine the size of a compressed image, given the internalFormat,
          * the width, the height, and the depth of the image. The block size
          * and the size are output parameters. */
        static void getCompressedSize(GLenum internalFormat, GLint width, GLint height, GLint depth, GLint& blockSize, GLint& size);


        /** Helper method. Creates the texture, but doesn't set or use a
          * texture binding. Note: Don't call this method directly unless
          * you're implementing a subload callback. */
        void applyTexImage2D_load(State& state, GLenum target, const Image* image, GLsizei width, GLsizei height,GLsizei numMipmapLevels) const;

        /** Helper method. Subloads images into the texture, but doesn't set
          * or use a texture binding. Note: Don't call this method directly
          * unless you're implementing a subload callback. */
        void applyTexImage2D_subload(State& state, GLenum target, const Image* image, GLsizei width, GLsizei height, GLint inInternalFormat, GLsizei numMipmapLevels) const;


        /** Returned by mipmapBeforeTexImage() to indicate what
          * mipmapAfterTexImage() should do */
        enum GenerateMipmapMode
        {
            GENERATE_MIPMAP_NONE,
            GENERATE_MIPMAP,
            GENERATE_MIPMAP_TEX_PARAMETER
        };

    protected :

        virtual ~Texture();

        virtual void computeInternalFormat() const = 0;

        /** Computes the internal format from Image parameters. */
        void computeInternalFormatWithImage(const osg::Image& image) const;

        /** Computes the texture dimension for the given Image. */
        void computeRequiredTextureDimensions(State& state, const osg::Image& image,GLsizei& width, GLsizei& height,GLsizei& numMipmapLevels) const;

        /** Computes the internal format type. */
        void computeInternalFormatType() const;

        /** Helper method. Sets texture parameters. */
        void applyTexParameters(GLenum target, State& state) const;

        /** Returns true if _useHardwareMipMapGeneration is true and either
          * glGenerateMipmapEXT() or GL_GENERATE_MIPMAP_SGIS are supported. */
        bool isHardwareMipmapGenerationEnabled(const State& state) const;

        /** Returns true if the associated Image should be released and it's safe to do so. */
        bool isSafeToUnrefImageData(const State& state) const {
            return (_unrefImageDataAfterApply && state.getMaxTexturePoolSize()==0 && areAllTextureObjectsLoaded());
        }

        /** Helper methods to be called before and after calling
          * gl[Compressed][Copy]Tex[Sub]Image2D to handle generating mipmaps. */
        GenerateMipmapMode mipmapBeforeTexImage(const State& state, bool hardwareMipmapOn) const;
        void mipmapAfterTexImage(State& state, GenerateMipmapMode beforeResult) const;

        /** Helper method to generate mipmap levels by calling of glGenerateMipmapEXT.
          * If it is not supported, then call the virtual allocateMipmap() method */
        void generateMipmap(State& state) const;

        /** Allocate mipmap levels of the texture by subsequent calling of glTexImage* function. */
        virtual void allocateMipmap(State& state) const = 0;

        /** Returns -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
        int compareTexture(const Texture& rhs) const;

        /** Returns -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
        int compareTextureObjects(const Texture& rhs) const;

        typedef buffered_value<unsigned int> TexParameterDirtyList;
        mutable TexParameterDirtyList _texParametersDirtyList;
        mutable TexParameterDirtyList _texMipmapGenerationDirtyList;

        WrapMode _wrap_s;
        WrapMode _wrap_t;
        WrapMode _wrap_r;

        FilterMode      _min_filter;
        FilterMode      _mag_filter;
        float           _maxAnisotropy;
        float           _minlod;
        float           _maxlod;
        float           _lodbias;
        Vec4i           _swizzle;
        bool            _useHardwareMipMapGeneration;
        bool            _unrefImageDataAfterApply;
        bool            _clientStorageHint;
        bool            _resizeNonPowerOfTwoHint;

        Vec4d           _borderColor;
        GLint           _borderWidth;

        InternalFormatMode          _internalFormatMode;
        mutable InternalFormatType  _internalFormatType;
        mutable GLint       _internalFormat;
        mutable GLenum      _sourceFormat;
        mutable GLenum      _sourceType;

        bool                _use_shadow_comparison;
        ShadowCompareFunc   _shadow_compare_func;
        ShadowTextureMode   _shadow_texture_mode;
        float               _shadow_ambient;

    public:

        struct OSG_EXPORT TextureProfile
        {
            inline TextureProfile(GLenum target):
                _target(target),
                _numMipmapLevels(0),
                _internalFormat(0),
                _width(0),
                _height(0),
                _depth(0),
                _border(0),
                _size(0) {}

            inline TextureProfile(GLenum    target,
                                 GLint     numMipmapLevels,
                                 GLenum    internalFormat,
                                 GLsizei   width,
                                 GLsizei   height,
                                 GLsizei   depth,
                                 GLint     border):
                _target(target),
                _numMipmapLevels(numMipmapLevels),
                _internalFormat(internalFormat),
                _width(width),
                _height(height),
                _depth(depth),
                _border(border),
                _size(0) { computeSize(); }


            #define LESSTHAN(A,B) if (A<B) return true; if (B<A) return false;
            #define FINALLESSTHAN(A,B) return (A<B);

            bool operator < (const TextureProfile& rhs) const
            {
                LESSTHAN(_size,rhs._size);
                LESSTHAN(_target,rhs._target);
                LESSTHAN(_numMipmapLevels,rhs._numMipmapLevels);
                LESSTHAN(_internalFormat,rhs._internalFormat);
                LESSTHAN(_width,rhs._width);
                LESSTHAN(_height,rhs._height);
                LESSTHAN(_depth,rhs._depth);
                FINALLESSTHAN(_border, rhs._border);
            }

            bool operator == (const TextureProfile& rhs) const
            {
                return _target == rhs._target &&
                       _numMipmapLevels == rhs._numMipmapLevels &&
                       _internalFormat == rhs._internalFormat &&
                       _width == rhs._width &&
                       _height == rhs._height &&
                       _depth == rhs._depth &&
                       _border == rhs._border;
            }

            inline void set(GLint numMipmapLevels,
                            GLenum    internalFormat,
                            GLsizei   width,
                            GLsizei   height,
                            GLsizei   depth,
                            GLint     border)
            {
                _numMipmapLevels = numMipmapLevels;
                _internalFormat = internalFormat;
                _width = width;
                _height = height;
                _depth = depth;
                _border = border;
                computeSize();
            }

            inline bool match(GLenum    target,
                       GLint     numMipmapLevels,
                       GLenum    internalFormat,
                       GLsizei   width,
                       GLsizei   height,
                       GLsizei   depth,
                       GLint     border)
            {
                return (_target == target) &&
                       (_numMipmapLevels == numMipmapLevels) &&
                       (_internalFormat == internalFormat) &&
                       (_width == width) &&
                       (_height == height) &&
                       (_depth == depth) &&
                       (_border == border);
            }

            void computeSize();

            GLenum       _target;
            GLint        _numMipmapLevels;
            GLenum       _internalFormat;
            GLsizei      _width;
            GLsizei      _height;
            GLsizei      _depth;
            GLint        _border;
            unsigned int _size;
        };

        class OSG_EXPORT TextureObject : public GraphicsObject
        {
        public:

            inline TextureObject(Texture* texture, GLuint id, GLenum target):
                _id(id),
                _profile(target),
                _set(0),
                _previous(0),
                _next(0),
                _texture(texture),
                _allocated(false),
                _frameLastUsed(0),
                _timeStamp(0) {}

            inline TextureObject(Texture* texture, GLuint id, const TextureProfile& profile):
                _id(id),
                _profile(profile),
                _set(0),
                _previous(0),
                _next(0),
                _texture(texture),
                _allocated(false),
                _frameLastUsed(0),
                _timeStamp(0) {}

            inline TextureObject(Texture* texture,
                          GLuint    id,
                          GLenum    target,
                          GLint     numMipmapLevels,
                          GLenum    internalFormat,
                          GLsizei   width,
                          GLsizei   height,
                          GLsizei   depth,
                          GLint     border):
                _id(id),
                _profile(target,numMipmapLevels,internalFormat,width,height,depth,border),
                _set(0),
                _previous(0),
                _next(0),
                _texture(texture),
                _allocated(false),
                _frameLastUsed(0),
                _timeStamp(0) {}

            inline bool match(GLenum    target,
                       GLint     numMipmapLevels,
                       GLenum    internalFormat,
                       GLsizei   width,
                       GLsizei   height,
                       GLsizei   depth,
                       GLint     border)
            {
                return isReusable() &&
                       _profile.match(target,numMipmapLevels,internalFormat,width,height,depth,border);
            }


            void bind();

            inline GLenum id() const { return _id; }
            inline GLenum target() const { return _profile._target; }

            inline unsigned int size() const { return _profile._size; }

            inline void setTexture(Texture* texture) { _texture = texture; }
            inline Texture* getTexture() const { return _texture; }

            inline void setTimeStamp(double timestamp) { _timeStamp = timestamp; }
            inline double getTimeStamp() const { return _timeStamp; }

            inline void setAllocated(bool allocated=true) { _allocated = allocated; }

            void setAllocated(GLint     numMipmapLevels,
                              GLenum    internalFormat,
                              GLsizei   width,
                              GLsizei   height,
                              GLsizei   depth,
                              GLint     border);

            inline bool isAllocated() const { return _allocated; }

            inline bool isReusable() const { return _allocated && _profile._width!=0; }

            /** release TextureObject to the orphan list to be reused or deleted.*/
            void release();

            GLuint              _id;
            TextureProfile      _profile;
            TextureObjectSet*   _set;
            TextureObject*      _previous;
            TextureObject*      _next;
            Texture*            _texture;
            bool                _allocated;
            unsigned int        _frameLastUsed;
            double              _timeStamp;

        protected:
            virtual ~TextureObject();

        };

        typedef std::list< ref_ptr<TextureObject> > TextureObjectList;


        static osg::ref_ptr<TextureObject> generateTextureObject(const Texture* texture, unsigned int contextID,GLenum target);

        static osg::ref_ptr<TextureObject> generateTextureObject(const Texture* texture,
                                                     unsigned int contextID,
                                                     GLenum    target,
                                                     GLint     numMipmapLevels,
                                                     GLenum    internalFormat,
                                                     GLsizei   width,
                                                     GLsizei   height,
                                                     GLsizei   depth,
                                                     GLint     border);

        TextureObject* generateAndAssignTextureObject(unsigned int contextID, GLenum target) const;

        TextureObject* generateAndAssignTextureObject(unsigned int contextID,
                                                     GLenum    target,
                                                     GLint     numMipmapLevels,
                                                     GLenum    internalFormat,
                                                     GLsizei   width,
                                                     GLsizei   height,
                                                     GLsizei   depth,
                                                     GLint     border) const;

    protected:

        typedef buffered_object< ref_ptr<TextureObject> >  TextureObjectBuffer;
        mutable TextureObjectBuffer         _textureObjectBuffer;
        mutable ref_ptr<GraphicsContext>    _readPBuffer;

};

class OSG_EXPORT TextureObjectSet : public Referenced
{
public:
    TextureObjectSet(TextureObjectManager* parent, const Texture::TextureProfile& profile);

    const Texture::TextureProfile& getProfile() const { return _profile; }

    void handlePendingOrphandedTextureObjects();

    void deleteAllTextureObjects();
    void discardAllTextureObjects();
    void flushAllDeletedTextureObjects();
    void discardAllDeletedTextureObjects();
    void flushDeletedTextureObjects(double currentTime, double& availableTime);

    osg::ref_ptr<Texture::TextureObject> takeFromOrphans(Texture* texture);
    osg::ref_ptr<Texture::TextureObject> takeOrGenerate(Texture* texture);
    void moveToBack(Texture::TextureObject* to);
    void addToBack(Texture::TextureObject* to);
    void orphan(Texture::TextureObject* to);
    void remove(Texture::TextureObject* to);
    void moveToSet(Texture::TextureObject* to, TextureObjectSet* set);

    unsigned int size() const { return _profile._size * _numOfTextureObjects; }

    bool makeSpace(unsigned int& size);

    bool checkConsistency() const;

    TextureObjectManager* getParent() { return _parent; }

    unsigned int computeNumTextureObjectsInList() const;
    unsigned int getNumOfTextureObjects() const { return _numOfTextureObjects; }
    unsigned int getNumOrphans() const { return static_cast<unsigned int>(_orphanedTextureObjects.size()); }
    unsigned int getNumPendingOrphans() const { return static_cast<unsigned int>(_pendingOrphanedTextureObjects.size()); }

protected:

    virtual ~TextureObjectSet();

    OpenThreads::Mutex  _mutex;

    TextureObjectManager*       _parent;
    unsigned int                _contextID;
    Texture::TextureProfile     _profile;
    unsigned int                _numOfTextureObjects;
    Texture::TextureObjectList  _orphanedTextureObjects;
    Texture::TextureObjectList  _pendingOrphanedTextureObjects;

    Texture::TextureObject*     _head;
    Texture::TextureObject*     _tail;

};

class OSG_EXPORT TextureObjectManager : public GraphicsObjectManager
{
public:
    TextureObjectManager(unsigned int contextID);


    void setNumberActiveTextureObjects(unsigned int size) { _numActiveTextureObjects = size; }
    unsigned int& getNumberActiveTextureObjects() { return _numActiveTextureObjects; }
    unsigned int getNumberActiveTextureObjects() const { return _numActiveTextureObjects; }

    void setNumberOrphanedTextureObjects(unsigned int size) { _numOrphanedTextureObjects = size; }
    unsigned int& getNumberOrphanedTextureObjects() { return _numOrphanedTextureObjects; }
    unsigned int getNumberOrphanedTextureObjects() const { return _numOrphanedTextureObjects; }

    void setCurrTexturePoolSize(unsigned int size) { _currTexturePoolSize = size; }
    unsigned int& getCurrTexturePoolSize() { return _currTexturePoolSize; }
    unsigned int getCurrTexturePoolSize() const { return _currTexturePoolSize; }

    void setMaxTexturePoolSize(unsigned int size);
    unsigned int getMaxTexturePoolSize() const { return _maxTexturePoolSize; }

    bool hasSpace(unsigned int size) const { return (_currTexturePoolSize+size)<=_maxTexturePoolSize; }
    bool makeSpace(unsigned int size);

    osg::ref_ptr<Texture::TextureObject> generateTextureObject(const Texture* texture, GLenum target);
    osg::ref_ptr<Texture::TextureObject> generateTextureObject(const Texture* texture,
                                                GLenum    target,
                                                GLint     numMipmapLevels,
                                                GLenum    internalFormat,
                                                GLsizei   width,
                                                GLsizei   height,
                                                GLsizei   depth,
                                                GLint     border);
    void handlePendingOrphandedTextureObjects();

    void deleteAllGLObjects();
    void discardAllGLObjects();
    void flushAllDeletedGLObjects();
    void discardAllDeletedGLObjects();
    void flushDeletedGLObjects(double currentTime, double& availableTime);

    TextureObjectSet* getTextureObjectSet(const Texture::TextureProfile& profile);

    void newFrame(osg::FrameStamp* fs);
    void resetStats();
    void reportStats(std::ostream& out);
    void recomputeStats(std::ostream& out) const;
    bool checkConsistency() const;

    unsigned int& getFrameNumber() { return _frameNumber; }
    unsigned int& getNumberFrames() { return _numFrames; }

    unsigned int& getNumberDeleted() { return _numDeleted; }
    double& getDeleteTime() { return _deleteTime; }

    unsigned int& getNumberGenerated() { return _numGenerated; }
    double& getGenerateTime() { return _generateTime; }

protected:

    ~TextureObjectManager();

    typedef std::map< Texture::TextureProfile, osg::ref_ptr<TextureObjectSet> > TextureSetMap;

    unsigned int        _numActiveTextureObjects;
    unsigned int        _numOrphanedTextureObjects;
    unsigned int        _currTexturePoolSize;
    unsigned int        _maxTexturePoolSize;
    TextureSetMap       _textureSetMap;

    unsigned int        _frameNumber;

    unsigned int        _numFrames;
    unsigned int        _numDeleted;
    double              _deleteTime;

    unsigned int        _numGenerated;
    double              _generateTime;
};
}

#endif
