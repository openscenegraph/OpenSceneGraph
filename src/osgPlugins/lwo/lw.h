/*
 * Copyright (C) 1998,1999 Janne Löf <jlof@mail.student.oulu.fi>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef LW_H
#define LW_H

#include <osg/GL>
#include <iostream>

#define LW_MAX_POINTS   200
#define LW_MAX_NAME_LEN 500

struct lwMaterial
{
  char name[LW_MAX_NAME_LEN];
  GLfloat r,g,b;
  
  lwMaterial()
  {
    r=1.0f;
    g=1.0f;
    b=1.0f;
    name[0] = 0;
  }
};

struct lwFace
{
  int material;         /* material of this face */
  int index_cnt;        /* number of vertices */
  int *index;           /* index to vertex */
  float *texcoord;      /* u,v texture coordinates */
  
  void init()
  {
    material = 0;         /* material of this face */
    index_cnt = 0;        /* number of vertices */
    index = 0;           /* index to vertex */
    texcoord = 0;      /* u,v texture coordinates */
  }
  
};

struct lwObject
{
  int face_cnt;
  lwFace *face;

  int material_cnt;
  lwMaterial *material;

  int vertex_cnt;
  GLfloat *vertex;
  
  void init()
  {
    face_cnt = 0;
    face = 0;

    material_cnt=0;
    material = 0;

    vertex_cnt=0;
    vertex = 0;
  }

};


typedef int gint32;
typedef unsigned int guint32;
typedef int gint;


gint      lw_is_lwobject(const char     *lw_file);
lwObject *lw_object_read(const char     *lw_file, std::ostream& output);
void      lw_object_free(      lwObject *lw_object);

GLfloat   lw_object_radius(const lwObject *lw_object);
void      lw_object_scale (lwObject *lw_object, GLfloat scale);

#endif /* LW_H */

