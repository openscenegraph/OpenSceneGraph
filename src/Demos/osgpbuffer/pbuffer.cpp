#include <cstdio>
#include <set>
#include <vector>
#include <string>
#include <cassert>

#include <osg/GLExtensions>
#include <osg/Notify>

#include "pbuffer.h"

namespace osg {
    bool isWGLExtensionSupported(const char *extension);
}


// WGL_ARB_pbuffer
static WGLCreatePBufferProc            wglCreatePBuffer;
static WGLGetPBufferDCProc             wglGetPBufferDC;
static WGLReleasePBufferDCProc         wglReleasePBufferDC;
static WGLDestroyPBufferProc           wglDestroyPBuffer;
static WGLQueryPBufferProc             wglQueryPBuffer;

// WGL_ARB_pixel_format
static WGLGetPixelFormatAttribivProc   wglGetPixelFormatAttribiv;
static WGLGetPixelFormatAttribfvProc   wglGetPixelFormatAttribfv;
static WGLChoosePixelFormatProc        wglChoosePixelFormat;

// WGL_ARB_render_texture
static WGLBindTexImageProc              wglBindTexImage;
static WGLReleaseTexImageProc           wglReleaseTexImage;
static WGLSetPbufferAttribProc          wglSetPbufferAttrib;


#ifdef _WIN32

#ifndef WGL_ARB_extensions_string
#define WGL_ARB_extensions_string 1
typedef const char * (WINAPI * WGLGetExtensionsStringProc) (HDC hDC);
#endif

#endif

#ifdef _WIN32
bool osg::isWGLExtensionSupported(const char *extension)
{

    typedef std::set<std::string>  ExtensionSet;
    static ExtensionSet s_extensionSet;
    static const char* s_extensions = NULL;
    static WGLGetExtensionsStringProc wglGetExtensionsString = (WGLGetExtensionsStringProc)osg::getGLExtensionFuncPtr("wglGetExtensionsStringARB");
    if (wglGetExtensionsString == NULL) return false;
    if (s_extensions==NULL)
    {
        // get the extension list from OpenGL.
        s_extensions = (const char*)wglGetExtensionsString(::wglGetCurrentDC());
        if (s_extensions==NULL) return false;

        // insert the ' ' delimiated extensions words into the extensionSet.
        const char *startOfWord = s_extensions;
        const char *endOfWord;
        while ((endOfWord = strchr(startOfWord,' '))!=NULL)
        {
            s_extensionSet.insert(std::string(startOfWord,endOfWord));
            startOfWord = endOfWord+1;
        }
        if (*startOfWord!=0) s_extensionSet.insert(std::string(startOfWord));
        
        osg::notify(osg::INFO)<<"OpenGL extensions supported by installed OpenGL drivers are:"<<std::endl;
        for(ExtensionSet::iterator itr=s_extensionSet.begin();
            itr!=s_extensionSet.end();
            ++itr)
        {
            osg::notify(osg::INFO)<<"    "<<*itr<<std::endl;
        }
            
    }

    // true if extension found in extensionSet.
    bool result = s_extensionSet.find(extension)!=s_extensionSet.end();

    if (result)
        osg::notify(osg::INFO)<<"OpenGL WGL extension '"<<extension<<"' is supported."<<std::endl;
    else
        osg::notify(osg::INFO)<<"OpenGL WGL extension '"<<extension<<"' is not supported."<<std::endl;

    return result;
}
#endif


PBuffer::PBuffer(const int width, const int height) :
    _width(width),
    _height(height),
    _hDC(NULL),
    _hGLcontext(NULL),
    _hPBuffer(NULL)
{
    _doubleBuffer = false;
    _RGB = true;
    _shareLists = false;
    _minimumNumberDepthBits = 16;
    _minimumNumberAlphaBits = 8;
    _minimumNumberStencilBits = 0;
    _minimumNumberAccumulationBits = 0;
}

PBuffer::~PBuffer()
{
    if (_hPBuffer)
    {
        wglDeleteContext( _hGLcontext );
        wglReleasePBufferDC( _hPBuffer, _hDC );
        wglDestroyPBuffer( _hPBuffer );
    }
}

// Check to see if the pbuffer was lost.
// If it was lost, destroy it and then recreate it.
void PBuffer::handleModeSwitch()
{
    int lost = 0;
    wglQueryPBuffer( _hPBuffer, WGL_PBUFFER_LOST_ARB, &lost );

    if ( lost )
    {
        this->~PBuffer();
        initialize();
    }
}


// This function actually does the creation of the p-buffer.
// It can only be called once a window has already been created.
void PBuffer::initialize()
{
    setupGLExtenions();

    HDC hdc = wglGetCurrentDC();
	HGLRC hglrc = wglGetCurrentContext();

    // Query for a suitable pixel format based on the specified mode.
    std::vector<int> iattributes;
    std::vector<float> fattributes;

    // P-buffer will be used with OpenGL
    iattributes.push_back(WGL_SUPPORT_OPENGL_ARB);
    iattributes.push_back(true);

    // Since we are trying to create a pbuffer, the pixel format we
    // request (and subsequently use) must be "p-buffer capable".
    iattributes.push_back(WGL_DRAW_TO_PBUFFER_ARB);
    iattributes.push_back(true);

    // Bind to texture
    iattributes.push_back(WGL_BIND_TO_TEXTURE_RGBA_ARB);
    iattributes.push_back(true);

//iattributes.push_back(WGL_ACCELERATION_ARB);
//iattributes.push_back(WGL_FULL_ACCELERATION_ARB);

    if (_RGB)
    {
        iattributes.push_back(WGL_PIXEL_TYPE_ARB);
        iattributes.push_back(WGL_TYPE_RGBA_ARB);

        // We require a minimum of 8-bits for each R, G, B, and A.
        iattributes.push_back(WGL_RED_BITS_ARB);
        iattributes.push_back(8);
        iattributes.push_back(WGL_GREEN_BITS_ARB);
        iattributes.push_back(8);
        iattributes.push_back(WGL_BLUE_BITS_ARB);
        iattributes.push_back(8);
        if (_minimumNumberAlphaBits > 0)
        {
            iattributes.push_back(WGL_ALPHA_BITS_ARB);
            iattributes.push_back(_minimumNumberAlphaBits);
        }
    }
    else
    {
        iattributes.push_back(WGL_PIXEL_TYPE_ARB);
        iattributes.push_back(WGL_TYPE_COLORINDEX_ARB);
    }

    iattributes.push_back(WGL_DOUBLE_BUFFER_ARB);
    iattributes.push_back(_doubleBuffer);

    if (_minimumNumberDepthBits > 0)
    {
        iattributes.push_back(WGL_DEPTH_BITS_ARB);
        iattributes.push_back(_minimumNumberDepthBits);
    }

    if (_minimumNumberStencilBits > 0)
    {
        iattributes.push_back(WGL_STENCIL_BITS_ARB);
        iattributes.push_back(_minimumNumberStencilBits);
    }

    if (_minimumNumberAccumulationBits > 0)
    {
        iattributes.push_back(WGL_ACCUM_BITS_ARB);
        iattributes.push_back(_minimumNumberAccumulationBits);
    }

    // Terminate array
    iattributes.push_back(0);


    // Now obtain a list of pixel formats that meet these minimum requirements.
    int format;
    int pformat[MAX_PFORMATS];
    unsigned int nformats=0;
    if ( !wglChoosePixelFormat( hdc, &iattributes.front(), &fattributes.front(), MAX_PFORMATS, pformat, &nformats ) )
    {
        osg::notify(osg::FATAL)<< "pbuffer creation error:  Couldn't find a suitable pixel format." <<std::endl;
        exit( -1 );
    }
	format = pformat[0];


    // Create the p-buffer.
    std::vector<int> pbattr;

    // Texture format
    pbattr.push_back(WGL_TEXTURE_FORMAT_ARB);
    pbattr.push_back(WGL_TEXTURE_RGBA_ARB);
#if 1
    // Texture target
    pbattr.push_back(WGL_TEXTURE_TARGET_ARB);
    pbattr.push_back(WGL_TEXTURE_2D_ARB);
#else
    // Cubemap
    pbattr.push_back(WGL_TEXTURE_TARGET_ARB);
    pbattr.push_back(WGL_TEXTURE_CUBE_MAP_ARB);

    // Cubemap face
    pbattr.push_back(WGL_CUBE_MAP_FACE_ARB);
    pbattr.push_back(WGL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB);
#endif
    // Terminate array
    pbattr.push_back(0);

    iattributes[0] = 0;
    _hPBuffer = wglCreatePBuffer( hdc, format, _width, _height, &pbattr.front() );
    if ( !_hPBuffer )
    {
        DWORD err = GetLastError();
        osg::notify(osg::FATAL)<< "pbuffer creation error:  wglCreatePBufferARB() failed\n" <<std::endl;
        switch (err)
        {
        case ERROR_INVALID_PIXEL_FORMAT:
            osg::notify(osg::FATAL)<< "error:  ERROR_INVALID_PIXEL_FORMAT\n" <<std::endl;
            break;
        case ERROR_NO_SYSTEM_RESOURCES:
            osg::notify(osg::FATAL)<< "error:  ERROR_NO_SYSTEM_RESOURCES\n" <<std::endl;
            break;
        case ERROR_INVALID_DATA:
            osg::notify(osg::FATAL)<< "error:  ERROR_INVALID_DATA\n" <<std::endl;
            break;
        }
        exit( -1 );
    }

    // Get the device context.
    _hDC = wglGetPBufferDC( _hPBuffer );
    if ( !_hDC )
    {
        osg::notify(osg::FATAL)<< "pbuffer creation error:  wglGetPBufferDC() failed\n" << std::endl;
        exit( -1 );
    }

    // Create a gl context for the p-buffer.
    _hGLcontext = wglCreateContext( _hDC );
    if ( !_hGLcontext )
    {
        osg::notify(osg::FATAL)<< "pbuffer creation error:  wglCreateContext() failed\n" << std::endl;
        exit( -1 );
    }

	if( _shareLists )
    {
        if( !wglShareLists(hglrc, _hGLcontext) )
        {
            osg::notify(osg::FATAL)<< "pbuffer: wglShareLists() failed\n" << std::endl;
            exit( -1 );
        }
    }

    // Determine the actual width and height we were able to create.
    wglQueryPBuffer( _hPBuffer, WGL_PBUFFER_WIDTH_ARB, &_width );
    wglQueryPBuffer( _hPBuffer, WGL_PBUFFER_HEIGHT_ARB, &_height );

    osg::notify(osg::INFO)<< "PBuffer created " << _width << " x " << _height << "pbuffer."<< std::endl;
}


void PBuffer::releaseTexImage()
{
    if(!wglReleaseTexImage( _hPBuffer, WGL_FRONT_LEFT_ARB ) )
    {
        assert(0);
    }
}


void PBuffer::makeCurrent()
{
    if ( !wglMakeCurrent( _hDC, _hGLcontext ) )
    {
        osg::notify(osg::FATAL)<< "PBuffer::makeCurrent() failed.\n"<< std::endl;
        exit( -1 );
    }
}


void PBuffer::bindTexImage(GLuint textureID)
{
	glBindTexture(GL_TEXTURE_2D, textureID);

    if(!::wglBindTexImage(_hPBuffer, WGL_FRONT_LEFT_ARB))
	{
        assert(0);
	}
}


void PBuffer::setupGLExtenions()
{
#ifdef _WIN32
    _isPBufferSupported = osg::isWGLExtensionSupported("WGL_ARB_pbuffer");
    if (_isPBufferSupported)
    {
        wglCreatePBuffer    = (WGLCreatePBufferProc)osg::getGLExtensionFuncPtr("wglCreatePbufferARB");
        wglGetPBufferDC     = (WGLGetPBufferDCProc)osg::getGLExtensionFuncPtr("wglGetPbufferDCARB");
        wglReleasePBufferDC = (WGLReleasePBufferDCProc)osg::getGLExtensionFuncPtr("wglReleasePbufferDCARB");
        wglDestroyPBuffer   = (WGLDestroyPBufferProc)osg::getGLExtensionFuncPtr("wglDestroyPbufferARB");
        wglQueryPBuffer     = (WGLQueryPBufferProc)osg::getGLExtensionFuncPtr("wglQueryPbufferARB");
    }

    _isPixelFormatSupported = osg::isWGLExtensionSupported("WGL_ARB_pixel_format");
    if (_isPixelFormatSupported)
    {
        wglGetPixelFormatAttribiv = (WGLGetPixelFormatAttribivProc)osg::getGLExtensionFuncPtr("wglGetPixelFormatAttribivARB");
        wglGetPixelFormatAttribfv = (WGLGetPixelFormatAttribfvProc)osg::getGLExtensionFuncPtr("wglGetPixelFormatAttribfvARB");
        wglChoosePixelFormat      = (WGLChoosePixelFormatProc)osg::getGLExtensionFuncPtr("wglChoosePixelFormatARB");
    }

    _isRenderTextureSupported = osg::isWGLExtensionSupported("WGL_ARB_render_texture");
    if (_isRenderTextureSupported)
    {
		wglBindTexImage     = (WGLBindTexImageProc)osg::getGLExtensionFuncPtr("wglBindTexImageARB");
		wglReleaseTexImage  = (WGLReleaseTexImageProc)osg::getGLExtensionFuncPtr("wglReleaseTexImageARB");
		wglSetPbufferAttrib = (WGLSetPbufferAttribProc)osg::getGLExtensionFuncPtr("wglSetPbufferAttribARB");
    }
#endif
}


#if DEBUGGING
    fprintf( fp, "nformats = %d\n\n", nformats );
    int values[MAX_ATTRIBS];
	int iatr[MAX_ATTRIBS] = { WGL_PIXEL_TYPE_ARB, WGL_COLOR_BITS_ARB,
		                      WGL_RED_BITS_ARB, WGL_GREEN_BITS_ARB, WGL_BLUE_BITS_ARB,
							  WGL_ALPHA_BITS_ARB, WGL_DEPTH_BITS_ARB, WGL_STENCIL_BITS_ARB, WGL_ACCUM_BITS_ARB,
							  WGL_DOUBLE_BUFFER_ARB, WGL_SUPPORT_OPENGL_ARB, WGL_ACCELERATION_ARB };
	int niatr = 12;
    for ( int j = 0; j < MAX_ATTRIBS; j++ )
        {
        values[j] = false;
        iattributes[j] = iattributes[2*j];
        }
    for ( unsigned int i = 0; i < nformats; i++ )
        {
        if ( !wglGetPixelFormatAttribivARB( hdc, pformat[i], 0, niatr, iatr, values ) )
            {
            fprintf( stderr, "pbuffer creation error:  wglGetPixelFormatAttribiv() failed\n" );
            exit( -1 );
            }
		fprintf( fp, "%d. pformat = %d\n", i, pformat[i] );
		fprintf( fp, "--------------------\n" );
        for ( int k = 0; k < niatr; k++ )
            {
            if ( iatr[k] == WGL_PIXEL_TYPE_ARB )
                {
                if ( values[k] == WGL_TYPE_COLORINDEX_ARB )
                    fprintf( fp, " Pixel type = WGL_TYPE_COLORINDEX_ARB\n" );
                if ( values[k] == WGL_TYPE_RGBA_ARB )
                    fprintf( fp, " Pixel type = WGL_TYPE_RGBA_ARB\n" );
                }
            if ( iatr[k] == WGL_COLOR_BITS_ARB )
                {
                fprintf( fp, " Color bits = %d\n", values[k] );
                }
            if ( iatr[k] == WGL_RED_BITS_ARB )
                {
                fprintf( fp, "      red         %d\n", values[k] );
                }
            if ( iatr[k] == WGL_GREEN_BITS_ARB )
                {
                fprintf( fp, "      green       %d\n", values[k] );
                }
            if ( iatr[k] == WGL_BLUE_BITS_ARB )
                {
                fprintf( fp, "      blue        %d\n", values[k] );
                }
            if ( iatr[k] == WGL_ALPHA_BITS_ARB )
                {
                fprintf( fp, "      alpha       %d\n", values[k] );
                }
            if ( iatr[k] == WGL_DEPTH_BITS_ARB )
                {
                fprintf( fp, " Depth bits =   %d\n", values[k] );
                }
            if ( iatr[k] == WGL_STENCIL_BITS_ARB )
                {
                fprintf( fp, " Stencil bits = %d\n", values[k] );
                }
            if ( iatr[k] == WGL_ACCUM_BITS_ARB )
                {
                fprintf( fp, " Accum bits =   %d\n", values[k] );
                }
            if ( iatr[k] == WGL_DOUBLE_BUFFER_ARB )
                {
                fprintf( fp, " Double Buffer  = %d\n", values[k] );
                }
            if ( iatr[k] == WGL_SUPPORT_OPENGL_ARB )
                {
                fprintf( fp, " Support OpenGL = %d\n", values[k] );
                }
            if ( iatr[k] == WGL_ACCELERATION_ARB )
                {
				if ( values[k] == WGL_FULL_ACCELERATION_ARB )
					fprintf( fp, " Acceleration   = WGL_FULL_ACCELERATION_ARB\n" );
				if ( values[k] == WGL_GENERIC_ACCELERATION_ARB )
					fprintf( fp, " Acceleration   = WGL_GENERIC_ACCELERATION_ARB\n" );
                }
            }
        fprintf( fp, "\n" );
        }
    fprintf( fp, "selected pformat = %d\n", format );
#endif



