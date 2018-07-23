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

#ifndef OSG_EXPORT_
#define OSG_EXPORT_ 1

#include<osg/Config>

// disable VisualStudio warnings
#if defined(_MSC_VER) && defined(OSG_DISABLE_MSVC_WARNINGS)
    #pragma warning( disable : 4244 )
    #pragma warning( disable : 4251 )
    #pragma warning( disable : 4275 )
    #pragma warning( disable : 4512 )
    #pragma warning( disable : 4267 )
    #pragma warning( disable : 4702 )
    #pragma warning( disable : 4511 )
#endif

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
    #  if defined( OSG_LIBRARY_STATIC )
    #    define OSG_EXPORT
    #  elif defined( OSG_LIBRARY )
    #    define OSG_EXPORT   __declspec(dllexport)
    #  else
    #    define OSG_EXPORT   __declspec(dllimport)
    #  endif
#else
    #  define OSG_EXPORT
#endif

// set up define for whether member templates are supported by VisualStudio compilers.
#ifdef _MSC_VER
# if (_MSC_VER >= 1300)
#  define __STL_MEMBER_TEMPLATES
# endif
#endif

/* Define NULL pointer value */

#ifndef NULL
    #ifdef  __cplusplus
        #define NULL    0
    #else
        #define NULL    ((void *)0)
    #endif
#endif

// helper macro's for quieten unused variable warnings
#define OSG_UNUSED(VAR) (void)(VAR)
#define OSG_UNUSED2(VAR1, VAR2) (void)(VAR1); (void)(VAR2);
#define OSG_UNUSED3(VAR1, VAR2, VAR3) (void)(VAR1); (void)(VAR2); (void)(VAR2);
#define OSG_UNUSED4(VAR1, VAR2, VAR3, VAR4) (void)(VAR1); (void)(VAR2); (void)(VAR3); (void)(VAR4);
#define OSG_UNUSED5(VAR1, VAR2, VAR3, VAR4, VAR5) (void)(VAR1); (void)(VAR2); (void)(VAR3); (void)(VAR4); (void)(VAR5);

/**

\namespace osg

The core osg library provides the basic scene graph classes such as Nodes,
State and Drawables, and maths and general helper classes.
*/

#endif

