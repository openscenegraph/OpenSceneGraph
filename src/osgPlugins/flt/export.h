#ifndef __FLT_EXPORT_H
#define __FLT_EXPORT_H

#define FLT_DLL

#ifdef WIN32
#pragma warning( disable : 4251 )
#pragma warning( disable : 4275 )
#pragma warning( disable : 4786 )
#endif

#if defined(FLT_DLL) && defined(_MSC_VER)
#  ifdef FLT_LIBRARY
#    define FLT_EXPORT   __declspec(dllexport)
#  else
#    define FLT_EXPORT   __declspec(dllimport)
#  endif                                          /* FLT_LIBRARY */
#else
#  define FLT_EXPORT
#endif                                            /* FLT_DLL && _MSC_VER */

#endif
