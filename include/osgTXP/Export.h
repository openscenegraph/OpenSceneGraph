//C++ header - Open Scene Graph - Copyright (C) 1998-2002 Robert Osfield
//Distributed under the terms of the GNU Library General Public License (LGPL)
//as published by the Free Software Foundation.

#ifndef OSGTXP_EXPORT_
#define OSGTXP_EXPORT_ 1

#if defined(_MSC_VER)
    #pragma warning( disable : 4244 )
    #pragma warning( disable : 4251 )
    #pragma warning( disable : 4267 )
    #pragma warning( disable : 4275 )
    #pragma warning( disable : 4290 )
    #pragma warning( disable : 4786 )
#endif

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__) || defined( __MWERKS__)
	#  ifdef OSGTXP_LIBRARY
	#    define OSGTXP_EXPORT   __declspec(dllexport)
	#  else
	#    define OSGTXP_EXPORT   __declspec(dllimport)
	#  endif /* OSGTXP_LIBRARY */
#else
	#  define OSGTXP_EXPORT
#endif  

#endif
