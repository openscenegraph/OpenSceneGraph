#ifndef PBUFFERS_H
#define PBUFFERS_H
/*
#include <windows.h>
#include "GL/gl.h"
#include <GL/glut.h>
#include "GL/wglext.h"
#include <GL/glext.h>
*/

#include <osg/GL>

#if defined(WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif


#ifndef WGL_ARB_pbuffer
#define WGL_NUMBER_PIXEL_FORMATS_ARB        0x2000
#define WGL_DRAW_TO_WINDOW_ARB              0x2001
#define WGL_DRAW_TO_BITMAP_ARB              0x2002
#define WGL_ACCELERATION_ARB                0x2003
#define WGL_NEED_PALETTE_ARB                0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB         0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB          0x2006
#define WGL_SWAP_METHOD_ARB                 0x2007
#define WGL_NUMBER_OVERLAYS_ARB             0x2008
#define WGL_NUMBER_UNDERLAYS_ARB            0x2009
#define WGL_TRANSPARENT_ARB                 0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB       0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB     0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB      0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB     0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB     0x203B
#define WGL_SHARE_DEPTH_ARB                 0x200C
#define WGL_SHARE_STENCIL_ARB               0x200D
#define WGL_SHARE_ACCUM_ARB                 0x200E
#define WGL_SUPPORT_GDI_ARB                 0x200F
#define WGL_SUPPORT_OPENGL_ARB              0x2010
#define WGL_DOUBLE_BUFFER_ARB               0x2011
#define WGL_STEREO_ARB                      0x2012
#define WGL_PIXEL_TYPE_ARB                  0x2013
#define WGL_COLOR_BITS_ARB                  0x2014
#define WGL_RED_BITS_ARB                    0x2015
#define WGL_RED_SHIFT_ARB                   0x2016
#define WGL_GREEN_BITS_ARB                  0x2017
#define WGL_GREEN_SHIFT_ARB                 0x2018
#define WGL_BLUE_BITS_ARB                   0x2019
#define WGL_BLUE_SHIFT_ARB                  0x201A
#define WGL_ALPHA_BITS_ARB                  0x201B
#define WGL_ALPHA_SHIFT_ARB                 0x201C
#define WGL_ACCUM_BITS_ARB                  0x201D
#define WGL_ACCUM_RED_BITS_ARB              0x201E
#define WGL_ACCUM_GREEN_BITS_ARB            0x201F
#define WGL_ACCUM_BLUE_BITS_ARB             0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB            0x2021
#define WGL_DEPTH_BITS_ARB                  0x2022
#define WGL_STENCIL_BITS_ARB                0x2023
#define WGL_AUX_BUFFERS_ARB                 0x2024
#define WGL_NO_ACCELERATION_ARB             0x2025
#define WGL_GENERIC_ACCELERATION_ARB        0x2026
#define WGL_FULL_ACCELERATION_ARB           0x2027
#define WGL_SWAP_EXCHANGE_ARB               0x2028
#define WGL_SWAP_COPY_ARB                   0x2029
#define WGL_SWAP_UNDEFINED_ARB              0x202A
#define WGL_TYPE_RGBA_ARB                   0x202B
#define WGL_TYPE_COLORINDEX_ARB             0x202C
#define WGL_DRAW_TO_PBUFFER_ARB             0x202D
#define WGL_MAX_PBUFFER_PIXELS_ARB          0x202E
#define WGL_MAX_PBUFFER_WIDTH_ARB           0x202F
#define WGL_MAX_PBUFFER_HEIGHT_ARB          0x2030
#define WGL_PBUFFER_LARGEST_ARB             0x2033
#define WGL_PBUFFER_WIDTH_ARB               0x2034
#define WGL_PBUFFER_HEIGHT_ARB              0x2035
#define WGL_PBUFFER_LOST_ARB				0x2036
#endif


#ifndef WGL_ARB_pbuffer
#define WGL_ARB_pbuffer 1

DECLARE_HANDLE(HPBUFFERARB);

typedef HPBUFFERARB (WINAPI * WGLCreatePBufferProc) (
                                        HDC hDC,
                                        int iPixelFormat,
                                        int iWidth,
                                        int iHeight,
                                        const int *piAttribList);
typedef HDC (WINAPI * WGLGetPBufferDCProc) (HPBUFFERARB hPbuffer);
typedef int (WINAPI * WGLReleasePBufferDCProc) (
                                        HPBUFFERARB hPbuffer,
                                        HDC hDC);
typedef BOOL (WINAPI * WGLDestroyPBufferProc) (HPBUFFERARB hPbuffer);
typedef BOOL (WINAPI * WGLQueryPBufferProc) (
                                        HPBUFFERARB hPbuffer,
                                        int iAttribute,
                                        int *piValue);
#endif


#ifndef WGL_ARB_pixel_format
#define WGL_ARB_pixel_format 1
typedef BOOL (WINAPI * WGLGetPixelFormatAttribivProc) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues);
typedef BOOL (WINAPI * WGLGetPixelFormatAttribfvProc) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues);
typedef BOOL (WINAPI * WGLChoosePixelFormatProc) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
#endif



#ifndef WGL_ARB_render_texture
#define WGL_BIND_TO_TEXTURE_RGB_ARB             0x2070
#define WGL_BIND_TO_TEXTURE_RGBA_ARB            0x2071
#define WGL_TEXTURE_FORMAT_ARB                  0x2072
#define WGL_TEXTURE_TARGET_ARB                  0x2073
#define WGL_MIPMAP_TEXTURE_ARB                  0x2074
#define WGL_TEXTURE_RGB_ARB                     0x2075
#define WGL_TEXTURE_RGBA_ARB                    0x2076
#define WGL_NO_TEXTURE_ARB                      0x2077
#define WGL_TEXTURE_CUBE_MAP_ARB                0x2078
#define WGL_TEXTURE_1D_ARB                      0x2079
#define WGL_TEXTURE_2D_ARB                      0x207A
#define WGL_NO_TEXTURE_ARB                      0x2077
#define WGL_MIPMAP_LEVEL_ARB                    0x207B
#define WGL_CUBE_MAP_FACE_ARB                   0x207C
#define WGL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB     0x207D
#define WGL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB     0x207E
#define WGL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB     0x207F
#define WGL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB     0x2080
#define WGL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB     0x2081 
#define WGL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB     0x2082
#define WGL_FRONT_LEFT_ARB                      0x2083
#define WGL_FRONT_RIGHT_ARB                     0x2084
#define WGL_BACK_LEFT_ARB                       0x2085
#define WGL_BACK_RIGHT_ARB                      0x2086
#define WGL_AUX0_ARB                            0x2087 
#define WGL_AUX1_ARB                            0x2088 
#define WGL_AUX2_ARB                            0x2089 
#define WGL_AUX3_ARB                            0x208A 
#define WGL_AUX4_ARB                            0x208B 
#define WGL_AUX5_ARB                            0x208C 
#define WGL_AUX6_ARB                            0x208D
#define WGL_AUX7_ARB                            0x208E 
#define WGL_AUX8_ARB                            0x208F 
#define WGL_AUX9_ARB                            0x2090
#endif

#ifndef WGL_ARB_render_texture
#define WGL_ARB_render_texture 1
typedef BOOL (WINAPI * WGLBindTexImageProc) (HPBUFFERARB hPbuffer, int iBuffer);
typedef BOOL (WINAPI * WGLReleaseTexImageProc) (HPBUFFERARB hPbuffer, int iBuffer);
typedef BOOL (WINAPI * WGLSetPbufferAttribProc) (HPBUFFERARB hPbuffer, const int * piAttribList);
#endif


#define MAX_PFORMATS 256
#define MAX_ATTRIBS  32



class PBuffer
{
    private:

        HDC             _hDC;           // Handle to a device context.
        HGLRC           _hGLcontext;    // Handle to a GL context.
        HPBUFFERARB     _hPBuffer;      // Handle to a pbuffer.
        int             _width;
        int             _height;

        bool            _doubleBuffer;
        bool            _RGB;
        bool            _shareLists;
        unsigned int    _minimumNumberDepthBits;
        unsigned int    _minimumNumberAlphaBits;
        unsigned int    _minimumNumberStencilBits;
        unsigned int    _minimumNumberAccumulationBits;

        bool            _isPBufferSupported;
        bool            _isPixelFormatSupported;
        bool            _isRenderTextureSupported;

    public:

        PBuffer(const int width, const int height );
        ~PBuffer();
        void handleModeSwitch();
        void makeCurrent();
        void initialize();
        void bindTexImage(GLuint textureID);
        void releaseTexImage();


    protected:

        void setupGLExtenions();

};

#endif