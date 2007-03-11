/* -*- c -*- */
#ifndef INCLUDED_LIB3DS_MESH_H
#define INCLUDED_LIB3DS_MESH_H
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

#ifndef INCLUDED_LIB3DS_TYPES_H
#include "types.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Triangular mesh point
 * \ingroup mesh
 */
typedef struct _Lib3dsPoint {
    Lib3dsVector pos;
} Lib3dsPoint;

/*!
 * Triangular mesh face
 * \ingroup mesh
 */
struct _Lib3dsFace {
    Lib3dsUserData user;
    char material[64];
    Lib3dsWord points[3];
    Lib3dsWord flags;
    Lib3dsDword smoothing;
    Lib3dsVector normal;
};

/*!
 * Triangular mesh box mapping settings
 * \ingroup mesh
 */
struct _Lib3dsBoxMap {
    char front[64];
    char back[64];
    char left[64];
    char right[64];
    char top[64];
    char bottom[64];
};

/*!
 * Lib3dsMapData maptype
 * \ingroup tracks
 */
typedef enum {
  LIB3DS_MAP_NONE        =0xFFFF,
  LIB3DS_MAP_PLANAR      =0,
  LIB3DS_MAP_CYLINDRICAL =1,
  LIB3DS_MAP_SPHERICAL   =2
} Lib3dsMapType;

/*!
 * Triangular mesh texture mapping data
 * \ingroup mesh
 */
struct _Lib3dsMapData {
    Lib3dsWord maptype;
    Lib3dsVector pos;
    Lib3dsMatrix matrix;
    Lib3dsFloat scale;
    Lib3dsFloat tile[2];
    Lib3dsFloat planar_size[2];
    Lib3dsFloat cylinder_height;
};

/*!
 * Triangular mesh object
 * \ingroup mesh
 */
struct _Lib3dsMesh {
    Lib3dsUserData user;
    Lib3dsMesh *next;
    char name[64];
    Lib3dsByte color;
    Lib3dsMatrix matrix;
    Lib3dsDword points;
    Lib3dsPoint *pointL;
    Lib3dsDword flags;
    Lib3dsWord *flagL;
    Lib3dsDword texels;
    Lib3dsTexel *texelL;
    Lib3dsDword faces;
    Lib3dsFace *faceL;
    Lib3dsBoxMap box_map;
    Lib3dsMapData map_data;
}; 

extern LIB3DSAPI Lib3dsMesh* lib3ds_mesh_new(const char *name);
extern LIB3DSAPI void lib3ds_mesh_free(Lib3dsMesh *mesh);
extern LIB3DSAPI Lib3dsBool lib3ds_mesh_new_point_list(Lib3dsMesh *mesh, Lib3dsDword points);
extern LIB3DSAPI void lib3ds_mesh_free_point_list(Lib3dsMesh *mesh);
extern LIB3DSAPI Lib3dsBool lib3ds_mesh_new_flag_list(Lib3dsMesh *mesh, Lib3dsDword flags);
extern LIB3DSAPI void lib3ds_mesh_free_flag_list(Lib3dsMesh *mesh);
extern LIB3DSAPI Lib3dsBool lib3ds_mesh_new_texel_list(Lib3dsMesh *mesh, Lib3dsDword texels);
extern LIB3DSAPI void lib3ds_mesh_free_texel_list(Lib3dsMesh *mesh);
extern LIB3DSAPI Lib3dsBool lib3ds_mesh_new_face_list(Lib3dsMesh *mesh, Lib3dsDword flags);
extern LIB3DSAPI void lib3ds_mesh_free_face_list(Lib3dsMesh *mesh);
extern LIB3DSAPI void lib3ds_mesh_bounding_box(Lib3dsMesh *mesh, Lib3dsVector min, Lib3dsVector max);
extern LIB3DSAPI void lib3ds_mesh_calculate_normals(Lib3dsMesh *mesh, Lib3dsVector *normalL);
extern LIB3DSAPI void lib3ds_mesh_dump(Lib3dsMesh *mesh);
extern LIB3DSAPI Lib3dsBool lib3ds_mesh_read(Lib3dsMesh *mesh, FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_mesh_write(Lib3dsMesh *mesh, FILE *f);

#ifdef __cplusplus
};
#endif
#endif

