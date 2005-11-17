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

#include "old_lw.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MK_ID(a,b,c,d) ((((guint32)(a))<<24)| \
            (((guint32)(b))<<16)| \
            (((guint32)(c))<< 8)| \
            (((guint32)(d))    ))

#define ID_FORM MK_ID('F','O','R','M')
#define ID_LWOB MK_ID('L','W','O','B')
#define ID_PNTS MK_ID('P','N','T','S')
#define ID_SRFS MK_ID('S','R','F','S')
#define ID_SURF MK_ID('S','U','R','F')
#define ID_POLS MK_ID('P','O','L','S')
#define ID_COLR MK_ID('C','O','L','R')

#define ID_CTEX MK_ID('C','T','E','X')
#define ID_DTEX MK_ID('D','T','E','X')
#define ID_STEX MK_ID('S','T','E','X')
#define ID_RTEX MK_ID('R','T','E','X')
#define ID_TTEX MK_ID('T','T','E','X')
#define ID_BTEX MK_ID('B','T','E','X')

#define ID_TIMG MK_ID('T','I','M','G')
#define ID_TFLG MK_ID('T','F','L','G')
#define ID_TSIZ MK_ID('T','S','I','Z')
#define ID_TCTR MK_ID('T','C','T','R')
#define ID_TFAL MK_ID('T','F','A','L')
#define ID_TVEL MK_ID('T','V','E','L')
#define ID_TWRP MK_ID('T','W','R','P')


#define FALSE 0
#define TRUE  1

#define g_return_val_if_fail(expr,val) if (!(expr)) return val;
#define g_return_if_fail(expr) if (!(expr)) return;
#define g_realloc(exp1,exp2) realloc(exp1,exp2)
#define g_malloc0(exp) malloc(exp)
#define g_warning printf
#define g_free free

static gint32 read_char(FILE *f)
{
  int c = fgetc(f);
  g_return_val_if_fail(c != EOF, 0);
  return c;
}

static gint32 read_short(FILE *f)
{
  return (read_char(f)<<8) | read_char(f);
}

static gint32 read_long(FILE *f)
{
  return (read_char(f)<<24) | (read_char(f)<<16) | (read_char(f)<<8) | read_char(f);
}

static GLfloat read_float(FILE *f)
{
  gint32 x = read_long(f);
  return *(GLfloat*)&x;
}

static gint read_string(FILE *f, char *s)
{
  gint c;
  gint cnt = 0;
  do {
    c = read_char(f);
    if (cnt < LW_MAX_NAME_LEN)
      s[cnt] = c;
    else
      s[LW_MAX_NAME_LEN-1] = 0;
    cnt++;
  } while (c != 0);
  /* if length of string (including \0) is odd skip another byte */
  if (cnt%2) {
    read_char(f);
    cnt++;
  }
  return cnt;
}

static void read_srfs(FILE *f, gint nbytes, lwObject *lwo)
{
  int guess_cnt = lwo->material_cnt;

  while (nbytes > 0) {
    lwMaterial *material;

    /* allocate more memory for materials if needed */
    if (guess_cnt <= lwo->material_cnt) {
      guess_cnt += guess_cnt/2 + 4;
      lwo->material = (lwMaterial*) g_realloc(lwo->material, sizeof(lwMaterial)*guess_cnt);
    }
    material = lwo->material + lwo->material_cnt++;

    /* read name */
    nbytes -= read_string(f,material->name);

    /* defaults */
    material->r = 0.7f;
    material->g = 0.7f;
    material->b = 0.7f;
  }
  lwo->material = (lwMaterial*) g_realloc(lwo->material, sizeof(lwMaterial)*lwo->material_cnt);
}


static void read_surf(FILE *f, gint nbytes, lwObject *lwo)
{
  int i;
  char name[LW_MAX_NAME_LEN];
  lwMaterial *material = NULL;

  /* read surface name */
  nbytes -= read_string(f,name);

  /* find material */
  for (i=0; i< lwo->material_cnt; i++) {
    if (strcmp(lwo->material[i].name,name) == 0) {
      material = &lwo->material[i];
      break;
    }
  }
  g_return_if_fail(material != NULL);

  lwTexture* tex = NULL;

  /* read values */
  while (nbytes > 0) {
    gint id = read_long(f);
    gint len = read_short(f);
    nbytes -= 6 + len + (len%2);

    switch (id) {
    case ID_COLR:
      material->r = read_char(f) / 255.0f;
      material->g = read_char(f) / 255.0f;
      material->b = read_char(f) / 255.0f;
      read_char(f); /* dummy */
      break;
    case ID_CTEX:
    case ID_DTEX:
    case ID_STEX:
    case ID_RTEX:
    case ID_TTEX:
    case ID_BTEX:
      len -= read_string(f, name);
      if (id == ID_CTEX) {
        tex = &material->ctex;
      }
      else
        tex = NULL;
      break;
    case ID_TIMG: {
      len -= read_string(f, name);
      if (tex) {
        /* last component of path */
        char* slash = strrchr(name, '/');
        if (!slash)
          slash = strrchr(name, '\\');
        if (slash)
          strcpy(tex->name, slash+1);
        else
          strcpy(tex->name, name);
        //printf("tex name=%s\n", tex->name);
      }
    } break;
    case ID_TFLG:
      if (tex) {
        tex->flags = read_short(f);
      }
      else
        fseek(f, len+(len%2), SEEK_CUR);
      break;
    case ID_TSIZ:
      if (tex) {
        tex->sx = read_float(f);
        tex->sy = read_float(f);
        tex->sz = read_float(f);
      }
      else
        fseek(f, len+(len%2), SEEK_CUR);
      break;
    case ID_TCTR:
      if (tex) {
        tex->cx = read_float(f);
        tex->cy = read_float(f);
        tex->cz = read_float(f);
      }
      else
        fseek(f, len+(len%2), SEEK_CUR);
      break;
    case ID_TFAL:
        if (tex) {
            //float vx,vy,vz;
            /*vx =*/ read_float(f);
            /*vy =*/ read_float(f);
            /*vz =*/ read_float(f);
            //printf("fal %.2f %.2f %.2f\n", vx,vy,vz);
        }
        else
            fseek(f, len+(len%2), SEEK_CUR);
        break;
    case ID_TVEL:
        if (tex) {
            //float vx,vy,vz;
            /*vx =*/ read_float(f);
            /*vy =*/ read_float(f);
            /*vz =*/ read_float(f);
            //printf("vel %.2f %.2f %.2f\n", vx,vy,vz);
        }
        else
            fseek(f, len+(len%2), SEEK_CUR);
        break;
    case ID_TWRP:
      if (tex) {
        tex->u_wrap = (lwTextureWrap) read_short(f);
        tex->v_wrap = (lwTextureWrap) read_short(f);
      }
      else
        fseek(f, len+(len%2), SEEK_CUR);
      break;
    default:
      fseek(f, len+(len%2), SEEK_CUR);
    }
  }
}


static void read_pols(FILE *f, int nbytes, lwObject *lwo)
{
  int guess_cnt = lwo->face_cnt;
  
  while (nbytes > 0) {
    lwFace *face;
    int i;

    /* allocate more memory for polygons if necessary */
    if (guess_cnt <= lwo->face_cnt) {
      guess_cnt += guess_cnt + 4;
      lwo->face = (lwFace*) g_realloc(lwo->face, sizeof(lwFace)*guess_cnt);
    }
    face = lwo->face + lwo->face_cnt++;

    face->init();

    /* number of points in this face */
    face->index_cnt = read_short(f);
    nbytes -= 2;

    /* allocate space for points */
    face->index = (int*) g_malloc0(sizeof(int)*face->index_cnt);
 
    /* read points in */
    for (i=0; i<face->index_cnt; i++) {
      face->index[i] = read_short(f);
      nbytes -= 2;
    }
    
    /* read surface material */
    face->material = read_short(f);
    nbytes -= 2;
    
    /* skip over detail  polygons */
    if (face->material < 0) {
      printf("face->material=%i    ",face->material);
      int det_cnt;
      face->material = -face->material;
      det_cnt = read_short(f);
      nbytes -= 2;
      while (det_cnt-- > 0) {
    int cnt = read_short(f);
    fseek(f, cnt*2+2, SEEK_CUR);
    nbytes -= cnt*2+2;
      }
    }
    face->material -= 1;
  }
  /* readjust to true size */
  lwo->face = (lwFace*) g_realloc(lwo->face, sizeof(lwFace)*lwo->face_cnt);
}



static void read_pnts(FILE *f, gint nbytes, lwObject *lwo)
{
  int i;
  lwo->vertex_cnt = nbytes / 12;
  lwo->vertex = (GLfloat*) g_malloc0(sizeof(GLfloat)*lwo->vertex_cnt*3);
  for (i=0; i<lwo->vertex_cnt; i++) {
    lwo->vertex[i*3+0] = read_float(f);
    lwo->vertex[i*3+1] = read_float(f);
    lwo->vertex[i*3+2] = read_float(f);
  }
}






gint lw_is_lwobject(const char *lw_file)
{
  FILE *f = fopen(lw_file, "rb");
  if (f) {
    gint32 form = read_long(f);
    gint32 nlen = read_long(f);
    gint32 lwob = read_long(f);
    fclose(f);
    if (form == ID_FORM && nlen != 0 && lwob == ID_LWOB)
      return TRUE;
  }
  return FALSE;
}


lwObject *lw_object_read(const char *lw_file, std::ostream& output)
{
  FILE *f = NULL;
  lwObject *lw_object = NULL;

  gint32 form_bytes = 0;
  gint32 read_bytes = 0;

  /* open file */
  f = fopen(lw_file, "rb");
  if (f == NULL) {
    output << "can't open file "<<lw_file<<std::endl;
    return NULL;
  }

  /* check for headers */
  if (read_long(f) != ID_FORM) {
    output << "file "<<lw_file<<" is not an IFF file"<<std::endl;
    fclose(f);
    return NULL;
  }
  form_bytes = read_long(f);
  read_bytes += 4;

  if (read_long(f) != ID_LWOB) {
    output << "file "<<lw_file<<" is not a LWOB file"<<std::endl;
    fclose(f);
    return NULL;
  }

  /* create new lwObject */
  lw_object = (lwObject*) g_malloc0(sizeof(lwObject));
  lw_object->init();

  /* read chunks */
  while (read_bytes < form_bytes) {
    gint32  id     = read_long(f);
    gint32  nbytes = read_long(f);
    read_bytes += 8 + nbytes + (nbytes%2);

    switch (id) {
    case ID_PNTS:
      read_pnts(f, nbytes, lw_object);
      break;
    case ID_POLS:
      read_pols(f, nbytes, lw_object);
      break;
    case ID_SRFS:
      read_srfs(f, nbytes, lw_object);
      break;
    case ID_SURF:
      read_surf(f, nbytes, lw_object);
      break;
    default:
      fseek(f, nbytes + (nbytes%2), SEEK_CUR);
    }
  }

  fclose(f);

  for (int i = 0; i < lw_object->face_cnt; i++) {
      int mati = lw_object->face[i].material;
      if (mati == 0)
          continue;

      /* fetch material */
      lwMaterial* mat = &lw_object->material[mati];
      unsigned int flags = mat->ctex.flags;
      if (flags == 0)
          continue;

      /* calculate texture coordinates */
      lwFace* face = &lw_object->face[i];
      face->texcoord = (float*) g_malloc0(face->index_cnt * sizeof(float) * 2);
      for (int j = 0; j < face->index_cnt; j++) {
          int vi = face->index[j];
          GLfloat* vtx = &lw_object->vertex[vi*3];

          GLfloat u,v;
          u = v = 0.0f;
          if (flags & X_AXIS) {
              u = (vtx[1] - mat->ctex.cy) / mat->ctex.sy;
              v = (vtx[2] - mat->ctex.cz) / mat->ctex.sz;
          }
          else if (flags & Y_AXIS) {
              u = (vtx[0] - mat->ctex.cx) / mat->ctex.sx;
              v = (vtx[2] - mat->ctex.cz) / mat->ctex.sz;
          }
          else if (flags & Z_AXIS) {
              u = (vtx[0] - mat->ctex.cx) / mat->ctex.sx;
              v = (vtx[1] - mat->ctex.cy) / mat->ctex.sy;
          }
          face->texcoord[j*2] = u + 0.5f;
          face->texcoord[j*2+1] = v + 0.5f;
          //printf("%.2f %.2f\n", u,v);
      }
  }

  return lw_object;
}







void lw_object_free(lwObject *lw_object)
{
  g_return_if_fail(lw_object != NULL);
 
  if (lw_object->face) {
    int i;
    for (i=0; i<lw_object->face_cnt; i++) {
      g_free(lw_object->face[i].index);
      if (lw_object->face[i].texcoord)
          g_free(lw_object->face[i].texcoord);
    }
    g_free(lw_object->face);
  }
  g_free(lw_object->material);
  g_free(lw_object->vertex);
  g_free(lw_object);
}


GLfloat lw_object_radius(const lwObject *lwo)
{
  int i;
  double max_radius = 0.0;

  g_return_val_if_fail(lwo != NULL, 0.0);

  for (i=0; i<lwo->vertex_cnt; i++) {
    GLfloat *v = &lwo->vertex[i*3];
    double r = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    if (r > max_radius)
      max_radius = r;
  }
  return (float)sqrt(max_radius);
}

void lw_object_scale(lwObject *lwo, GLfloat scale)
{
  int i;

  g_return_if_fail(lwo != NULL);

  for (i=0; i<lwo->vertex_cnt; i++) {
    lwo->vertex[i*3+0] *= scale;
    lwo->vertex[i*3+1] *= scale;
    lwo->vertex[i*3+2] *= scale;
  }
}


