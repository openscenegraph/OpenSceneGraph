#ifndef		__FTGL__
#define		__FTGL__


#ifdef WIN32
	// stl stuff
	#pragma warning( disable : 4251 )
	#pragma warning( disable : 4275 )
	#pragma warning( disable : 4786 )

#endif

#ifndef WIN32

    // non windows, doesn't require nonesense as seen below :-)    
    #ifndef __gl_h_
        #include <GL/gl.h>
    #endif

    // required for compatibility with glext.h style function definitions of 
    // OpenGL extensions, such as in src/osg/Point.cpp.
    #ifndef APIENTRY
        #define APIENTRY
    #endif

#else

    // Under windows avoid including <windows.h>
    // to avoid name space pollution, but Win32's <GL/gl.h> 
    // needs APIENTRY and WINGDIAPI defined properly. 
    // F
    # if 0
    #  define  WIN32_LEAN_AND_MEAN
    #  include <windows.h>
    # else
       // XXX This is from Win32's <windef.h> 
    #  ifndef APIENTRY
    #   define GLUT_APIENTRY_DEFINED
    #   if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
    #    define APIENTRY    __stdcall
    #   else
    #    define APIENTRY
    #   endif
    #  endif
       // XXX This is from Win32's <winnt.h> 
    #  ifndef CALLBACK
    #   if (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_PPC)) && !defined(MIDL_PASS)
    #    define CALLBACK __stdcall
    #   else
    #    define CALLBACK
    #   endif
    #  endif
       // XXX This is from Win32's <wingdi.h> and <winnt.h> 
    #  ifndef WINGDIAPI
    #   define GLUT_WINGDIAPI_DEFINED
    #   define WINGDIAPI __declspec(dllimport)
    #  endif
       // XXX This is from Win32's <ctype.h> 
    #  ifndef _WCHAR_T_DEFINED
    typedef unsigned short wchar_t;
    #   define _WCHAR_T_DEFINED
    #  endif
    # endif

    #ifndef __gl_h_
        #include <GL/gl.h>
    #endif

#endif

#if defined(_MSC_VER)
	#  ifdef FTGL_LIBRARY_STATIC		// staticLib
	#    define FTGL_EXPORT
	#  elif FTGL_LIBRARY				// dynamicLib
	#    define FTGL_EXPORT   __declspec(dllexport)
	#  else
	#    define FTGL_EXPORT   __declspec(dllimport)
	#  endif /* FTGL_LIBRARY */
#else
	#  define FTGL_EXPORT
#endif  


#endif	//	__FTGL__
