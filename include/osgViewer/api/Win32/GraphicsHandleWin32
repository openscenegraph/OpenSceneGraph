/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield 
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

#ifndef OSGVIEWER_GRAPHICSHANDLEWIN32
#define OSGVIEWER_GRAPHICSHANDLEWIN32 1

#include <osgViewer/Export>

// Fallback if not correctly detected in CMake macro
#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0A00 // Windows 10
//#define _WIN32_WINNT    0x0603 // Windows 8.1
//#define _WIN32_WINNT    0x0602 // Windows 8
//#define _WIN32_WINNT    0x0601 // Windows 7
//#define _WIN32_WINNT    0x0600 // Windows Server 2008
//#define _WIN32_WINNT    0x0600 // Windows Vista
//#define _WIN32_WINNT    0x0502 // Windows Server 2003 with SP1, Windows XP with SP2
//#define _WIN32_WINNT    0x0501 // Windows Server 2003, Windows XP
#define _WIN32_WINNT    0x0500 // Windows NT
#endif
#include <windows.h>

namespace osgViewer
{

/** Class to encapsulate platform-specific OpenGL context handle variables.
  * Derived osg::GraphicsContext classes can inherit from this class to
  * share OpenGL resources.*/

class OSGVIEWER_EXPORT GraphicsHandleWin32
{
    public:
    
        GraphicsHandleWin32():
            _hwnd(0),
            _hdc(0),
            _hglrc(0) {}

        /** Set native window.*/        
        inline void setHWND(HWND hwnd) { _hwnd = hwnd; }

        /** Get native window.*/        
        inline HWND getHWND() const { return _hwnd; }

        /** Set device context.*/        
        inline void setHDC(HDC hdc) { _hdc = hdc; }

        /** Get device context.*/        
        inline HDC getHDC() const { return _hdc; }

        /** Set native OpenGL graphics context.*/        
        inline void setWGLContext(HGLRC hglrc) { _hglrc = hglrc; }

        /** Get native OpenGL graphics context.*/        
        inline HGLRC getWGLContext() const { return _hglrc; }
     
    protected:
        
        HWND            _hwnd;
        HDC             _hdc;
        HGLRC           _hglrc;
    
};

}


// Definitions required to create an OpenGL pixel format, from the WGL_ARB_pixel_format specification document.
// See http://www.opengl.org/registry/specs/ARB/wgl_pixel_format.txt

#ifndef WGL_ARB_pixel_format
#define WGL_ARB_pixel_format 1
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
#define WGL_SAMPLE_BUFFERS_ARB              0x2041
#define WGL_SAMPLES_ARB                     0x2042
#endif

#endif
