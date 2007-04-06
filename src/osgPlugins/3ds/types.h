/* -*- c -*- */
#ifndef INCLUDED_LIB3DS_TYPES_H
#define INCLUDED_LIB3DS_TYPES_H
/*
 * The 3D Studio File Format Library
 * Copyright (C) 1996-2001 by J.E. Hoffmann <je-h@gmx.net>
 * All rights reserved.
 *
 * This program is  free  software;  you can redistribute it and/or modify it
 * under the terms of the  GNU Lesser General Public License  as published by 
 * the  Free Software Foundation;  either version 2.1 of the License,  or (at 
 * your option) any later version.
 *
 * This  program  is  distributed in  the  hope that it will  be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public  
 * License for more details.
 *
 * You should  have received  a copy of the GNU Lesser General Public License
 * along with  this program;  if not, write to the  Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id$
 */
#ifdef __cplusplus
extern "C" {
#endif

/*
 * #if defined (_LIB3DS_DLL) && defined(_WIN32) && (!defined(__GNUC__))
 * #ifdef LIB3DS_EXPORT
 * #define LIB3DSAPI __declspec(dllexport)
 * #else               
 * #define LIB3DSAPI __declspec(dllimport)
 * #endif           
 * #else
 * #define LIB3DSAPI
 * #endif
 */
// within the OSG just use the libary inline, no need to export symbols.
#define LIB3DSAPI

#define LIB3DS_TRUE 1
#define LIB3DS_FALSE 0

typedef int Lib3dsBool;
typedef unsigned char Lib3dsByte;
typedef unsigned short Lib3dsWord;
typedef unsigned int Lib3dsDword;
typedef signed char Lib3dsIntb;
typedef signed short Lib3dsIntw;
typedef signed int Lib3dsIntd;
typedef float Lib3dsFloat;
typedef double Lib3dsDouble;

typedef float Lib3dsVector[3];
typedef float Lib3dsTexel[2];
typedef float Lib3dsQuat[4];
typedef float Lib3dsMatrix[4][4];
typedef float Lib3dsRgb[3];
typedef float Lib3dsRgba[4];

#define LIB3DS_EPSILON (1e-8)
#define LIB3DS_PI 3.14159265358979323846
#define LIB3DS_TWOPI (2.0*LIB3DS_PI)
#define LIB3DS_HALFPI (LIB3DS_PI/2.0)
#define LIB3DS_DEG(x) ((180.0/LIB3DS_PI)*(x))
#define LIB3DS_RAD(x) ((LIB3DS_PI/180.0)*(x))
  
#ifndef INCLUDED_STDIO_H
#define INCLUDED_STDIO_H
#include <stdio.h>
#endif

#ifdef _DEBUG
  #ifndef ASSERT
  #include <assert.h>
  #define ASSERT(__expr) assert(__expr)
  #endif
  #define LIB3DS_ERROR_LOG \
    {printf("\t***LIB3DS_ERROR_LOG*** %s : %d\n", __FILE__, __LINE__);}
#else 
  #ifndef ASSERT
  #define ASSERT(__expr)
  #endif
  #define LIB3DS_ERROR_LOG
#endif

typedef struct _Lib3dsFile Lib3dsFile;
typedef struct _Lib3dsBackground Lib3dsBackground;
typedef struct _Lib3dsAtmosphere Lib3dsAtmosphere;
typedef struct _Lib3dsShadow Lib3dsShadow;
typedef struct _Lib3dsViewport Lib3dsViewport;
typedef struct _Lib3dsMaterial Lib3dsMaterial;
typedef struct _Lib3dsFace Lib3dsFace; 
typedef struct _Lib3dsBoxMap Lib3dsBoxMap; 
typedef struct _Lib3dsMapData Lib3dsMapData; 
typedef struct _Lib3dsMesh Lib3dsMesh;
typedef struct _Lib3dsCamera Lib3dsCamera;
typedef struct _Lib3dsLight Lib3dsLight;
typedef struct _Lib3dsBoolKey Lib3dsBoolKey;
typedef struct _Lib3dsBoolTrack Lib3dsBoolTrack;
typedef struct _Lib3dsLin1Key Lib3dsLin1Key;
typedef struct _Lib3dsLin1Track Lib3dsLin1Track;
typedef struct _Lib3dsLin3Key Lib3dsLin3Key;
typedef struct _Lib3dsLin3Track Lib3dsLin3Track;
typedef struct _Lib3dsQuatKey Lib3dsQuatKey;
typedef struct _Lib3dsQuatTrack Lib3dsQuatTrack;
typedef struct _Lib3dsMorphKey Lib3dsMorphKey;
typedef struct _Lib3dsMorphTrack Lib3dsMorphTrack;

typedef enum _Lib3dsNodeTypes {
  LIB3DS_UNKNOWN_NODE =0,
  LIB3DS_AMBIENT_NODE =1,
  LIB3DS_OBJECT_NODE  =2,
  LIB3DS_CAMERA_NODE  =3,
  LIB3DS_TARGET_NODE  =4,
  LIB3DS_LIGHT_NODE   =5,
  LIB3DS_SPOT_NODE    =6
} Lib3dsNodeTypes;

typedef struct _Lib3dsNode Lib3dsNode;

typedef union _Lib3dsUserData {
    void *p;
    Lib3dsIntd i;
    Lib3dsDword d;
    Lib3dsFloat f;
    Lib3dsMaterial *material;
    Lib3dsMesh *mesh;
    Lib3dsCamera *camera;
    Lib3dsLight *light;
    Lib3dsNode *node;
} Lib3dsUserData;

#ifdef __cplusplus
}
#endif
#endif









