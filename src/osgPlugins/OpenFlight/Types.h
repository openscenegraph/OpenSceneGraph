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

//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#ifndef FLT_TYPES_H
#define FLT_TYPES_H 1

namespace flt {

#if defined(_MSC_VER)

typedef __int8              int8;
typedef unsigned __int8     uint8;
typedef __int16             int16;
typedef unsigned __int16    uint16;
typedef __int32             int32;	
typedef unsigned __int32    uint32;	
typedef float               float32;
typedef double              float64;

#else

typedef signed char         int8;
typedef unsigned char       uint8;
typedef signed short        int16;
typedef unsigned short      uint16;
typedef signed int     	    int32;	
typedef unsigned int   	    uint32;	
typedef float               float32;
typedef double              float64;

#endif

} // end namespace

#endif
