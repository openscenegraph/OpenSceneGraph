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
#define LIB3DS_EXPORT
#include "file.h"
#include "chunk.h"
#include "readwrite.h"
#include "material.h"
#include "mesh.h"
#include "camera.h"
#include "light.h"
#include "node.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "config.h"
#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif


/*!
 * \defgroup file Files
 *
 * \author J.E. Hoffmann <je-h@gmx.net>
 */


/*!
 * Loads a .3DS file from disk into memory.
 *
 * \param filename  The filename of the .3DS file
 *
 * \return   A pointer to the Lib3dsFile structure containing the
 *           data of the .3DS file. 
 *           If the .3DS file can not be loaded NULL is returned.
 *
 * \note     To free the returned pointer use lib3ds_free.
 *
 * \see lib3ds_file_save
 * \see lib3ds_file_new
 * \see lib3ds_file_free
 *
 * \ingroup file
 */
Lib3dsFile*
lib3ds_file_load(const char *filename)
{
  FILE *f;
  Lib3dsFile *file;

  f=fopen(filename, "rb");
  if (!f) {
    return(0);
  }
  file=lib3ds_file_new();
  if (!file) {
    fclose(f);
    return(0);
  }
  
  if (!lib3ds_file_read(file, f)) {
    free(file);
    fclose(f);
    return(0);
  }
  fclose(f);
  return(file);
}


/*!
 * Saves a .3DS file from memory to disk.
 *
 * \param file      A pointer to a Lib3dsFile structure containing the
 *                  the data that should be stored.
 * \param filename  The filename of the .3DS file to store the data in.
 *
 * \return          TRUE on success, FALSE otherwise.
 *
 * \see lib3ds_file_load
 *
 * \ingroup file
 */
Lib3dsBool
lib3ds_file_save(Lib3dsFile *file, const char *filename)
{
  FILE *f;

  f=fopen(filename, "wb");
  if (!f) {
    return(LIB3DS_FALSE);
  }
  
  if (!lib3ds_file_write(file, f)) {
    fclose(f);
    return(LIB3DS_FALSE);
  }
  fclose(f);
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup file
 */
Lib3dsFile*
lib3ds_file_new()
{
  Lib3dsFile *file;

  file=(Lib3dsFile*)calloc(sizeof(Lib3dsFile),1);
  if (!file) {
    return(0);
  }
  file->mesh_version=3;
  file->master_scale=1.0f;
  file->keyf_revision=5;
  strcpy(file->name, "LIB3DS");
  return(file);
}


/*!
 * \ingroup file
 */
void
lib3ds_file_free(Lib3dsFile* file)
{
  ASSERT(file);
  lib3ds_viewport_set_views(&file->viewport,0);
  {
    Lib3dsMaterial *p,*q;
    
    for (p=file->materials; p; p=q) {
      q=p->next;
      lib3ds_material_free(p);
    }
    file->materials=0;
  }
  {
    Lib3dsCamera *p,*q;
    
    for (p=file->cameras; p; p=q) {
      q=p->next;
      lib3ds_camera_free(p);
    }
    file->cameras=0;
  }
  {
    Lib3dsLight *p,*q;
    
    for (p=file->lights; p; p=q) {
      q=p->next;
      lib3ds_light_free(p);
    }
    file->lights=0;
  }
  {
    Lib3dsMesh *p,*q;
    
    for (p=file->meshes; p; p=q) {
      q=p->next;
      lib3ds_mesh_free(p);
    }
    file->meshes=0;
  }
  {
    Lib3dsNode *p,*q;
  
    for (p=file->nodes; p; p=q) {
      q=p->next;
      lib3ds_node_free(p);
    }
  }
  free(file);
}


/*!
 * \ingroup file
 */
void
lib3ds_file_eval(Lib3dsFile *file, Lib3dsFloat t)
{
  Lib3dsNode *p;

  for (p=file->nodes; p!=0; p=p->next) {
    lib3ds_node_eval(p, t);
  }
}


static Lib3dsBool
named_object_read(Lib3dsFile *file, FILE *f)
{
  Lib3dsChunk c;
  char name[64];
  Lib3dsWord chunk;

  if (!lib3ds_chunk_read_start(&c, LIB3DS_NAMED_OBJECT, f)) {
    return(LIB3DS_FALSE);
  }
  if (!lib3ds_string_read(name, 64, f)) {
    return(LIB3DS_FALSE);
  }
  lib3ds_chunk_dump_info("  NAME=%s", name);
  lib3ds_chunk_read_tell(&c, f);

  while ((chunk=lib3ds_chunk_read_next(&c, f))!=0) {
    switch (chunk) {
      case LIB3DS_N_TRI_OBJECT:
        {
          Lib3dsMesh *mesh;

          mesh=lib3ds_mesh_new(name);
          if (!mesh) {
            return(LIB3DS_FALSE);
          }
          lib3ds_chunk_read_reset(&c, f);
          if (!lib3ds_mesh_read(mesh, f)) {
            return(LIB3DS_FALSE);
          }
          lib3ds_file_insert_mesh(file, mesh);
        }
        break;
      case LIB3DS_N_CAMERA:
        {
          Lib3dsCamera *camera;

          camera=lib3ds_camera_new(name);
          if (!camera) {
            return(LIB3DS_FALSE);
          }
          lib3ds_chunk_read_reset(&c, f);
          if (!lib3ds_camera_read(camera, f)) {
            return(LIB3DS_FALSE);
          }
          lib3ds_file_insert_camera(file, camera);
        }
        break;
      case LIB3DS_N_DIRECT_LIGHT:
        {
          Lib3dsLight *light;

          light=lib3ds_light_new(name);
          if (!light) {
            return(LIB3DS_FALSE);
          }
          lib3ds_chunk_read_reset(&c, f);
          if (!lib3ds_light_read(light, f)) {
            return(LIB3DS_FALSE);
          }
          lib3ds_file_insert_light(file, light);
        }
        break;
      default:
        lib3ds_chunk_unknown(chunk);
    }
  }
  
  lib3ds_chunk_read_end(&c, f);
  return(LIB3DS_TRUE);
}


static Lib3dsBool
ambient_read(Lib3dsFile *file, FILE *f)
{
  Lib3dsChunk c;
  Lib3dsWord chunk;
  Lib3dsBool have_lin=LIB3DS_FALSE;

  if (!lib3ds_chunk_read_start(&c, LIB3DS_AMBIENT_LIGHT, f)) {
    return(LIB3DS_FALSE);
  }

  while ((chunk=lib3ds_chunk_read_next(&c, f))!=0) {
    switch (chunk) {
      case LIB3DS_LIN_COLOR_F:
        {
          int i;
          for (i=0; i<3; ++i) {
            file->ambient[i]=lib3ds_float_read(f);
          }
        }
        have_lin=LIB3DS_TRUE;
        break;
      case LIB3DS_COLOR_F:
        {
          /* gamma corrected color chunk
             replaced in 3ds R3 by LIN_COLOR_24 */
          if (!have_lin) {
            int i;
            for (i=0; i<3; ++i) {
              file->ambient[i]=lib3ds_float_read(f);
            }
          }
        }
        break;
      default:
        lib3ds_chunk_unknown(chunk);
    }
  }
  
  lib3ds_chunk_read_end(&c, f);
  return(LIB3DS_TRUE);
}


static Lib3dsBool
mdata_read(Lib3dsFile *file, FILE *f)
{
  Lib3dsChunk c;
  Lib3dsWord chunk;

  if (!lib3ds_chunk_read_start(&c, LIB3DS_MDATA, f)) {
    return(LIB3DS_FALSE);
  }
  
  while ((chunk=lib3ds_chunk_read_next(&c, f))!=0) {
    switch (chunk) {
      case LIB3DS_MESH_VERSION:
        {
          file->mesh_version=lib3ds_intd_read(f);
        }
        break;
      case LIB3DS_MASTER_SCALE:
        {
          file->master_scale=lib3ds_float_read(f);
        }
        break;
      case LIB3DS_SHADOW_MAP_SIZE:
      case LIB3DS_LO_SHADOW_BIAS:
      case LIB3DS_HI_SHADOW_BIAS:
      case LIB3DS_SHADOW_SAMPLES:
      case LIB3DS_SHADOW_RANGE:
      case LIB3DS_SHADOW_FILTER:
      case LIB3DS_RAY_BIAS:
        {
          lib3ds_chunk_read_reset(&c, f);
          if (!lib3ds_shadow_read(&file->shadow, f)) {
            return(LIB3DS_FALSE);
          }
        }
        break;
      case LIB3DS_VIEWPORT_LAYOUT:
      case LIB3DS_DEFAULT_VIEW:
        {
          lib3ds_chunk_read_reset(&c, f);
          if (!lib3ds_viewport_read(&file->viewport, f)) {
            return(LIB3DS_FALSE);
          }
        }
        break;
      case LIB3DS_O_CONSTS:
        {
          int i;
          for (i=0; i<3; ++i) {
            file->construction_plane[i]=lib3ds_float_read(f);
          }
        }
        break;
      case LIB3DS_AMBIENT_LIGHT:
        {
          lib3ds_chunk_read_reset(&c, f);
          if (!ambient_read(file, f)) {
            return(LIB3DS_FALSE);
          }
        }
        break;
      case LIB3DS_BIT_MAP:
      case LIB3DS_SOLID_BGND:
      case LIB3DS_V_GRADIENT:
      case LIB3DS_USE_BIT_MAP:
      case LIB3DS_USE_SOLID_BGND:
      case LIB3DS_USE_V_GRADIENT:
        {
          lib3ds_chunk_read_reset(&c, f);
          if (!lib3ds_background_read(&file->background, f)) {
            return(LIB3DS_FALSE);
          }
        }
        break;
      case LIB3DS_FOG:
      case LIB3DS_LAYER_FOG:
      case LIB3DS_DISTANCE_CUE:
      case LIB3DS_USE_FOG:
      case LIB3DS_USE_LAYER_FOG:
      case LIB3DS_USE_DISTANCE_CUE:
        {
          lib3ds_chunk_read_reset(&c, f);
          if (!lib3ds_atmosphere_read(&file->atmosphere, f)) {
            return(LIB3DS_FALSE);
          }
        }
        break;
      case LIB3DS_MAT_ENTRY:
        {
          Lib3dsMaterial *material;

          material=lib3ds_material_new();
          if (!material) {
            return(LIB3DS_FALSE);
          }
          lib3ds_chunk_read_reset(&c, f);
          if (!lib3ds_material_read(material, f)) {
            return(LIB3DS_FALSE);
          }
          lib3ds_file_insert_material(file, material);
        }
        break;
      case LIB3DS_NAMED_OBJECT:
        {
          lib3ds_chunk_read_reset(&c, f);
          if (!named_object_read(file, f)) {
            return(LIB3DS_FALSE);
          }
        }
        break;
      default:
        lib3ds_chunk_unknown(chunk);
    }
  }

  lib3ds_chunk_read_end(&c, f);
  return(LIB3DS_TRUE);
}


static Lib3dsBool
kfdata_read(Lib3dsFile *file, FILE *f)
{
  Lib3dsChunk c;
  Lib3dsWord chunk;

  if (!lib3ds_chunk_read_start(&c, LIB3DS_KFDATA, f)) {
    return(LIB3DS_FALSE);
  }
  
  while ((chunk=lib3ds_chunk_read_next(&c, f))!=0) {
    switch (chunk) {
      case LIB3DS_KFHDR:
        {
          file->keyf_revision=lib3ds_word_read(f);
          if (!lib3ds_string_read(file->name, 12+1, f)) {
            return(LIB3DS_FALSE);
          }
          file->frames=lib3ds_intd_read(f);
        }
        break;
      case LIB3DS_KFSEG:
        {
          file->segment_from=lib3ds_intd_read(f);
          file->segment_to=lib3ds_intd_read(f);
        }
        break;
      case LIB3DS_KFCURTIME:
        {
          file->current_frame=lib3ds_intd_read(f);
        }
        break;
      case LIB3DS_VIEWPORT_LAYOUT:
      case LIB3DS_DEFAULT_VIEW:
        {
          lib3ds_chunk_read_reset(&c, f);
          if (!lib3ds_viewport_read(&file->viewport_keyf, f)) {
            return(LIB3DS_FALSE);
          }
        }
        break;
      case LIB3DS_AMBIENT_NODE_TAG:
        {
          Lib3dsNode *node;

          node=lib3ds_node_new_ambient();
          if (!node) {
            return(LIB3DS_FALSE);
          }
          lib3ds_chunk_read_reset(&c, f);
          if (!lib3ds_node_read(node, file, f)) {
            return(LIB3DS_FALSE);
          }
          lib3ds_file_insert_node(file, node);
        }
        break;
      case LIB3DS_OBJECT_NODE_TAG:
        {
          Lib3dsNode *node;

          node=lib3ds_node_new_object();
          if (!node) {
            return(LIB3DS_FALSE);
          }
          lib3ds_chunk_read_reset(&c, f);
          if (!lib3ds_node_read(node, file, f)) {
            return(LIB3DS_FALSE);
          }
          lib3ds_file_insert_node(file, node);
        }
        break;
      case LIB3DS_CAMERA_NODE_TAG:
        {
          Lib3dsNode *node;

          node=lib3ds_node_new_camera();
          if (!node) {
            return(LIB3DS_FALSE);
          }
          lib3ds_chunk_read_reset(&c, f);
          if (!lib3ds_node_read(node, file, f)) {
            return(LIB3DS_FALSE);
          }
          lib3ds_file_insert_node(file, node);
        }
        break;
      case LIB3DS_TARGET_NODE_TAG:
        {
          Lib3dsNode *node;

          node=lib3ds_node_new_target();
          if (!node) {
            return(LIB3DS_FALSE);
          }
          lib3ds_chunk_read_reset(&c, f);
          if (!lib3ds_node_read(node, file, f)) {
            return(LIB3DS_FALSE);
          }
          lib3ds_file_insert_node(file, node);
        }
        break;
      case LIB3DS_LIGHT_NODE_TAG:
      case LIB3DS_SPOTLIGHT_NODE_TAG:
        {
          Lib3dsNode *node;

          node=lib3ds_node_new_light();
          if (!node) {
            return(LIB3DS_FALSE);
          }
          lib3ds_chunk_read_reset(&c, f);
          if (!lib3ds_node_read(node, file, f)) {
            return(LIB3DS_FALSE);
          }
          lib3ds_file_insert_node(file, node);
        }
        break;
      case LIB3DS_L_TARGET_NODE_TAG:
        {
          Lib3dsNode *node;

          node=lib3ds_node_new_spot();
          if (!node) {
            return(LIB3DS_FALSE);
          }
          lib3ds_chunk_read_reset(&c, f);
          if (!lib3ds_node_read(node, file, f)) {
            return(LIB3DS_FALSE);
          }
          lib3ds_file_insert_node(file, node);
        }
        break;
      default:
        lib3ds_chunk_unknown(chunk);
    }
  }

  lib3ds_chunk_read_end(&c, f);
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup file
 */
Lib3dsBool
lib3ds_file_read(Lib3dsFile *file, FILE *f)
{
  Lib3dsChunk c;
  Lib3dsWord chunk;

  if (!lib3ds_chunk_read_start(&c, 0, f)) {
    return(LIB3DS_FALSE);
  }
  switch (c.chunk) {
    case LIB3DS_MDATA:
      {
        lib3ds_chunk_read_reset(&c, f);
        if (!mdata_read(file, f)) {
          return(LIB3DS_FALSE);
        }
      }
      break;
    case LIB3DS_M3DMAGIC:
    case LIB3DS_MLIBMAGIC:
    case LIB3DS_CMAGIC:
      {
        while ((chunk=lib3ds_chunk_read_next(&c, f))!=0) {
          switch (chunk) {
            case LIB3DS_M3D_VERSION:
              {
                file->mesh_version=lib3ds_dword_read(f);
              }
              break;
            case LIB3DS_MDATA:
              {
                lib3ds_chunk_read_reset(&c, f);
                if (!mdata_read(file, f)) {
                  return(LIB3DS_FALSE);
                }
              }
              break;
            case LIB3DS_KFDATA:
              {
                lib3ds_chunk_read_reset(&c, f);
                if (!kfdata_read(file, f)) {
                  return(LIB3DS_FALSE);
                }
              }
              break;
            default:
              lib3ds_chunk_unknown(chunk);
          }
        }
      }
      break;
    default:
      lib3ds_chunk_unknown(c.chunk);
      return(LIB3DS_FALSE);
  }

  lib3ds_chunk_read_end(&c, f);
  return(LIB3DS_TRUE);
}


static Lib3dsBool
colorf_write(Lib3dsRgba rgb, FILE *f)
{
  Lib3dsChunk c;

  c.chunk=LIB3DS_COLOR_F;
  c.size=18;
  lib3ds_chunk_write(&c,f);
  lib3ds_rgb_write(rgb,f);

  c.chunk=LIB3DS_LIN_COLOR_F;
  c.size=18;
  lib3ds_chunk_write(&c,f);
  lib3ds_rgb_write(rgb,f);
  return(LIB3DS_TRUE);
}


static Lib3dsBool
mdata_write(Lib3dsFile *file, FILE *f)
{
  Lib3dsChunk c;

  c.chunk=LIB3DS_MDATA;
  if (!lib3ds_chunk_write_start(&c,f)) {
    return(LIB3DS_FALSE);
  }

  { /*---- LIB3DS_MESH_VERSION ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_MESH_VERSION;
    c.size=10;
    lib3ds_chunk_write(&c,f);
    lib3ds_intd_write(file->mesh_version,f);
  }
  { /*---- LIB3DS_MASTER_SCALE ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_MASTER_SCALE;
    c.size=10;
    lib3ds_chunk_write(&c,f);
    lib3ds_float_write(file->master_scale,f);
  }
  { /*---- LIB3DS_O_CONSTS ----*/
    int i;
    for (i=0; i<3; ++i) {
      if (fabs(file->construction_plane[i])>LIB3DS_EPSILON) {
        break;
      }
    }
    if (i<3) {
      Lib3dsChunk c;
      c.chunk=LIB3DS_O_CONSTS;
      c.size=18;
      lib3ds_chunk_write(&c,f);
      lib3ds_vector_write(file->construction_plane,f);
    }
  }
  
  { /*---- LIB3DS_AMBIENT_LIGHT ----*/
    int i;
    for (i=0; i<3; ++i) {
      if (fabs(file->ambient[i])>LIB3DS_EPSILON) {
        break;
      }
    }
    if (i<3) {
      Lib3dsChunk c;
      c.chunk=LIB3DS_AMBIENT_LIGHT;
      c.size=42;
      lib3ds_chunk_write(&c,f);
      colorf_write(file->ambient,f);
    }
  }
  lib3ds_background_write(&file->background, f);
  lib3ds_atmosphere_write(&file->atmosphere, f);
  lib3ds_shadow_write(&file->shadow, f);
  lib3ds_viewport_write(&file->viewport, f);
  {
    Lib3dsMaterial *p;
    for (p=file->materials; p!=0; p=p->next) {
      if (!lib3ds_material_write(p,f)) {
        return(LIB3DS_FALSE);
      }
    }
  }
  {
    Lib3dsCamera *p;
    Lib3dsChunk c;
    
    for (p=file->cameras; p!=0; p=p->next) {
      c.chunk=LIB3DS_NAMED_OBJECT;
      if (!lib3ds_chunk_write_start(&c,f)) {
        return(LIB3DS_FALSE);
      }
      lib3ds_string_write(p->name,f);
      lib3ds_camera_write(p,f);
      if (!lib3ds_chunk_write_end(&c,f)) {
        return(LIB3DS_FALSE);
      }
    }
  }
  {
    Lib3dsLight *p;
    Lib3dsChunk c;
    
    for (p=file->lights; p!=0; p=p->next) {
      c.chunk=LIB3DS_NAMED_OBJECT;
      if (!lib3ds_chunk_write_start(&c,f)) {
        return(LIB3DS_FALSE);
      }
      lib3ds_string_write(p->name,f);
      lib3ds_light_write(p,f);
      if (!lib3ds_chunk_write_end(&c,f)) {
        return(LIB3DS_FALSE);
      }
    }
  }
  {
    Lib3dsMesh *p;
    Lib3dsChunk c;
    
    for (p=file->meshes; p!=0; p=p->next) {
      c.chunk=LIB3DS_NAMED_OBJECT;
      if (!lib3ds_chunk_write_start(&c,f)) {
        return(LIB3DS_FALSE);
      }
      lib3ds_string_write(p->name,f);
      lib3ds_mesh_write(p,f);
      if (!lib3ds_chunk_write_end(&c,f)) {
        return(LIB3DS_FALSE);
      }
    }
  }

  if (!lib3ds_chunk_write_end(&c,f)) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}



static Lib3dsBool
nodes_write(Lib3dsNode *node, Lib3dsFile *file, FILE *f)
{
  {
    Lib3dsNode *p;
    for (p=node->childs; p!=0; p=p->next) {
      if (!lib3ds_node_write(p, file, f)) {
        return(LIB3DS_FALSE);
      }
      nodes_write(p, file, f);
    }
  }
  return(LIB3DS_TRUE);
}


static Lib3dsBool
kfdata_write(Lib3dsFile *file, FILE *f)
{
  Lib3dsChunk c;
  
  c.chunk=LIB3DS_KFDATA;
  if (!lib3ds_chunk_write_start(&c,f)) {
    return(LIB3DS_FALSE);
  }
  
  { /*---- LIB3DS_KFHDR ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_KFHDR;
    c.size=6 + 2 + strlen(file->name)+1 +4;
    lib3ds_chunk_write(&c,f);
    lib3ds_intw_write(file->keyf_revision,f);
    lib3ds_string_write(file->name, f);
    lib3ds_intd_write(file->frames, f);
  }
  { /*---- LIB3DS_KFSEG ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_KFSEG;
    c.size=14;
    lib3ds_chunk_write(&c,f);
    lib3ds_intd_write(file->segment_from,f);
    lib3ds_intd_write(file->segment_to,f);
  }
  { /*---- LIB3DS_KFCURTIME ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_KFCURTIME;
    c.size=10;
    lib3ds_chunk_write(&c,f);
    lib3ds_intd_write(file->current_frame,f);
  }
  lib3ds_viewport_write(&file->viewport_keyf, f);
  
  {
    Lib3dsNode *p;
    for (p=file->nodes; p!=0; p=p->next) {
      if (!lib3ds_node_write(p, file, f)) {
        return(LIB3DS_FALSE);
      }
      if (!nodes_write(p, file, f)) {
        return(LIB3DS_FALSE);
      }
    }
  }
  
  if (!lib3ds_chunk_write_end(&c,f)) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup file
 */
Lib3dsBool
lib3ds_file_write(Lib3dsFile *file, FILE *f)
{
  Lib3dsChunk c;

  c.chunk=LIB3DS_M3DMAGIC;
  if (!lib3ds_chunk_write_start(&c,f)) {
    LIB3DS_ERROR_LOG;
    return(LIB3DS_FALSE);
  }

  { /*---- LIB3DS_M3D_VERSION ----*/
    Lib3dsChunk c;

    c.chunk=LIB3DS_M3D_VERSION;
    c.size=10;
    lib3ds_chunk_write(&c,f);
    lib3ds_dword_write(file->mesh_version, f);
  }

  if (!mdata_write(file, f)) {
    return(LIB3DS_FALSE);
  }
  if (!kfdata_write(file, f)) {
    return(LIB3DS_FALSE);
  }

  if (!lib3ds_chunk_write_end(&c,f)) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup file
 */
void
lib3ds_file_insert_material(Lib3dsFile *file, Lib3dsMaterial *material)
{
  Lib3dsMaterial *p,*q;
  
  ASSERT(file);
  ASSERT(material);
  ASSERT(!material->next);

  q=0;
  for (p=file->materials; p!=0; p=p->next) {
    if (strcmp(material->name, p->name)<0) {
      break;
    }
    q=p;
  }
  if (!q) {
    material->next=file->materials;
    file->materials=material;
  }
  else {
    material->next=q->next;
    q->next=material;
  }
}


/*!
 * \ingroup file
 */
void
lib3ds_file_remove_material(Lib3dsFile *file, Lib3dsMaterial *material)
{
  Lib3dsMaterial *p,*q;

  ASSERT(file);
  ASSERT(material);
  ASSERT(file->materials);
  for (p=0,q=file->materials; q; p=q,q=q->next) {
    if (q==material) {
      break;
    }
  }
  if (!q) {
    ASSERT(LIB3DS_FALSE);
    return;
  }
  if (!p) {
    file->materials=material->next;
  }
  else {
    p->next=q->next;
  }
  material->next=0;
}


/*!
 * \ingroup file
 */
Lib3dsMaterial*
lib3ds_file_material_by_name(Lib3dsFile *file, const char *name)
{
  Lib3dsMaterial *p;

  ASSERT(file);
  for (p=file->materials; p!=0; p=p->next) {
    if (strcmp(p->name,name)==0) {
      return(p);
    }
  }
  return(0);
}


/*!
 * \ingroup file
 */
void
lib3ds_file_dump_materials(Lib3dsFile *file)
{
  Lib3dsMaterial *p;

  ASSERT(file);
  for (p=file->materials; p!=0; p=p->next) {
    lib3ds_material_dump(p);
  }
}


/*!
 * \ingroup file
 */
void
lib3ds_file_insert_mesh(Lib3dsFile *file, Lib3dsMesh *mesh)
{
  Lib3dsMesh *p,*q;
  
  ASSERT(file);
  ASSERT(mesh);
  ASSERT(!mesh->next);

  q=0;
  for (p=file->meshes; p!=0; p=p->next) {
    if (strcmp(mesh->name, p->name)<0) {
      break;
    }
    q=p;
  }
  if (!q) {
    mesh->next=file->meshes;
    file->meshes=mesh;
  }
  else {
    mesh->next=q->next;
    q->next=mesh;
  }
}


/*!
 * \ingroup file
 */
void
lib3ds_file_remove_mesh(Lib3dsFile *file, Lib3dsMesh *mesh)
{
  Lib3dsMesh *p,*q;

  ASSERT(file);
  ASSERT(mesh);
  ASSERT(file->meshes);
  for (p=0,q=file->meshes; q; p=q,q=q->next) {
    if (q==mesh) {
      break;
    }
  }
  if (!q) {
    ASSERT(LIB3DS_FALSE);
    return;
  }
  if (!p) {
    file->meshes=mesh->next;
  }
  else {
    p->next=q->next;
  }
  mesh->next=0;
}


/*!
 * \ingroup file
 */
Lib3dsMesh*
lib3ds_file_mesh_by_name(Lib3dsFile *file, const char *name)
{
  Lib3dsMesh *p;

  ASSERT(file);
  for (p=file->meshes; p!=0; p=p->next) {
    if (strcmp(p->name,name)==0) {
      return(p);
    }
  }
  return(0);
}


/*!
 * \ingroup file
 */
void
lib3ds_file_dump_meshes(Lib3dsFile *file)
{
  Lib3dsMesh *p;

  ASSERT(file);
  for (p=file->meshes; p!=0; p=p->next) {
    lib3ds_mesh_dump(p);
  }
}


static void
dump_instances(Lib3dsNode *node, const char* parent)
{
  Lib3dsNode *p;
  char name[255];

  ASSERT(node);
  ASSERT(parent);
  strcpy(name, parent);
  strcat(name, ".");
  strcat(name, node->name);
  if (node->type==LIB3DS_OBJECT_NODE) {
    printf("  %s : %s\n", name, node->data.object.instance);
  }
  for (p=node->childs; p!=0; p=p->next) {
    dump_instances(p, parent);
  }
}


/*!
 * \ingroup file
 */
void
lib3ds_file_dump_instances(Lib3dsFile *file)
{
  Lib3dsNode *p;

  ASSERT(file);
  for (p=file->nodes; p!=0; p=p->next) {
    dump_instances(p,"");
  }
}


/*!
 * \ingroup file
 */
void
lib3ds_file_insert_camera(Lib3dsFile *file, Lib3dsCamera *camera)
{
  Lib3dsCamera *p,*q;
  
  ASSERT(file);
  ASSERT(camera);
  ASSERT(!camera->next);

  q=0;
  for (p=file->cameras; p!=0; p=p->next) {
    if (strcmp(camera->name, p->name)<0) {
      break;
    }
    q=p;
  }
  if (!q) {
    camera->next=file->cameras;
    file->cameras=camera;
  }
  else {
    camera->next=q->next;
    q->next=camera;
  }
}


/*!
 * \ingroup file
 */
void
lib3ds_file_remove_camera(Lib3dsFile *file, Lib3dsCamera *camera)
{
  Lib3dsCamera *p,*q;

  ASSERT(file);
  ASSERT(camera);
  ASSERT(file->cameras);
  for (p=0,q=file->cameras; q; p=q,q=q->next) {
    if (q==camera) {
      break;
    }
  }
  if (!q) {
    ASSERT(LIB3DS_FALSE);
    return;
  }
  if (!p) {
    file->cameras=camera->next;
  }
  else {
    p->next=q->next;
  }
  camera->next=0;
}


/*!
 * \ingroup file
 */
Lib3dsCamera*
lib3ds_file_camera_by_name(Lib3dsFile *file, const char *name)
{
  Lib3dsCamera *p;

  ASSERT(file);
  for (p=file->cameras; p!=0; p=p->next) {
    if (strcmp(p->name,name)==0) {
      return(p);
    }
  }
  return(0);
}


/*!
 * \ingroup file
 */
void
lib3ds_file_dump_cameras(Lib3dsFile *file)
{
  Lib3dsCamera *p;

  ASSERT(file);
  for (p=file->cameras; p!=0; p=p->next) {
    lib3ds_camera_dump(p);
  }
}


/*!
 * \ingroup file
 */
void
lib3ds_file_insert_light(Lib3dsFile *file, Lib3dsLight *light)
{
  Lib3dsLight *p,*q;
  
  ASSERT(file);
  ASSERT(light);
  ASSERT(!light->next);

  q=0;
  for (p=file->lights; p!=0; p=p->next) {
    if (strcmp(light->name, p->name)<0) {
      break;
    }
    q=p;
  }
  if (!q) {
    light->next=file->lights;
    file->lights=light;
  }
  else {
    light->next=q->next;
    q->next=light;
  }
}


/*!
 * \ingroup file
 */
void
lib3ds_file_remove_light(Lib3dsFile *file, Lib3dsLight *light)
{
  Lib3dsLight *p,*q;

  ASSERT(file);
  ASSERT(light);
  ASSERT(file->lights);
  for (p=0,q=file->lights; q; p=q,q=q->next) {
    if (q==light) {
      break;
    }
  }
  if (!q) {
    ASSERT(LIB3DS_FALSE);
    return;
  }
  if (!p) {
    file->lights=light->next;
  }
  else {
    p->next=q->next;
  }
  light->next=0;
}


/*!
 * \ingroup file
 */
Lib3dsLight*
lib3ds_file_light_by_name(Lib3dsFile *file, const char *name)
{
  Lib3dsLight *p;

  ASSERT(file);
  for (p=file->lights; p!=0; p=p->next) {
    if (strcmp(p->name,name)==0) {
      return(p);
    }
  }
  return(0);
}


/*!
 * \ingroup file
 */
void
lib3ds_file_dump_lights(Lib3dsFile *file)
{
  Lib3dsLight *p;

  ASSERT(file);
  for (p=file->lights; p!=0; p=p->next) {
    lib3ds_light_dump(p);
  }
}


/*!
 * \ingroup file
 */
void
lib3ds_file_bounding_box(Lib3dsFile *file, Lib3dsVector min, Lib3dsVector max)
{
  Lib3dsBool init=LIB3DS_FALSE;

  {
    Lib3dsVector lmin, lmax;
    Lib3dsMesh *p=file->meshes;

    if (!init && p) {
      init = LIB3DS_TRUE;
      lib3ds_mesh_bounding_box(p, min, max);
      p = p->next;  
    }
    while (p) {
      lib3ds_mesh_bounding_box(p, lmin, lmax);
      lib3ds_vector_min(min, lmin);
      lib3ds_vector_max(max, lmax);
      p=p->next;
    }
  }
  {
    Lib3dsCamera *p=file->cameras;
    if (!init && p) {
      init = LIB3DS_TRUE;
      lib3ds_vector_copy(min, p->position);
      lib3ds_vector_copy(max, p->position);
    }

    while (p) {
      lib3ds_vector_min(min, p->position);
      lib3ds_vector_max(max, p->position);
      lib3ds_vector_min(min, p->target);
      lib3ds_vector_max(max, p->target);
      p=p->next;
    }
  }
  {
    Lib3dsLight *p=file->lights;
    if (!init && p) {
      init = LIB3DS_TRUE;
      lib3ds_vector_copy(min, p->position);
      lib3ds_vector_copy(max, p->position);
    }

    while (p) {
      lib3ds_vector_min(min, p->position);
      lib3ds_vector_max(max, p->position);
      if (p->spot_light) {
        lib3ds_vector_min(min, p->spot);
        lib3ds_vector_max(max, p->spot);
      }
      p=p->next;
    }
  }
}  


/*!
 * \ingroup file
 */
Lib3dsNode*
lib3ds_file_node_by_name(Lib3dsFile *file, const char* name, Lib3dsNodeTypes type)
{
  Lib3dsNode *p,*q;

  ASSERT(file);
  for (p=file->nodes; p!=0; p=p->next) {
    if ((p->type==type) && (strcmp(p->name, name)==0)) {
      return(p);
    }
    q=lib3ds_node_by_name(p, name, type);
    if (q) {
      return(q);
    }
  }
  return(0);
}


/*!
 * \ingroup file
 */
Lib3dsNode*
lib3ds_file_node_by_id(Lib3dsFile *file, Lib3dsWord node_id)
{
  Lib3dsNode *p,*q;

  ASSERT(file);
  for (p=file->nodes; p!=0; p=p->next) {
    if (p->node_id==node_id) {
      return(p);
    }
    q=lib3ds_node_by_id(p, node_id);
    if (q) {
      return(q);
    }
  }
  return(0);
}


/*!
 * \ingroup file
 */
void
lib3ds_file_insert_node(Lib3dsFile *file, Lib3dsNode *node)
{
  Lib3dsNode *parent,*p,*n;
  
  ASSERT(node);
  ASSERT(!node->next);
  ASSERT(!node->parent);

  parent=0;
  if (node->parent_id!=LIB3DS_NO_PARENT) {
    parent=lib3ds_file_node_by_id(file, node->parent_id);
  }
  node->parent=parent;
  
  if (!parent) {
    for (p=0,n=file->nodes; n!=0; p=n,n=n->next) {
      if (strcmp(n->name, node->name)>0) {
        break;
      }
    }
    if (!p) {
      node->next=file->nodes;
      file->nodes=node;
    }
    else {
      node->next=p->next;
      p->next=node;
    }
  }
  else {
    for (p=0,n=parent->childs; n!=0; p=n,n=n->next) {
      if (strcmp(n->name, node->name)>0) {
        break;
      }
    }
    if (!p) {
      node->next=parent->childs;
      parent->childs=node;
    }
    else {
      node->next=p->next;
      p->next=node;
    }
  }

  if (node->node_id!=LIB3DS_NO_PARENT) {
    for (n=file->nodes; n!=0; n=p) {
      p=n->next;
      if (n->parent_id==node->node_id) {
        lib3ds_file_remove_node(file, n);
        lib3ds_file_insert_node(file, n);
      }
    }
  }
}


/*!
 * \ingroup file
 */
Lib3dsBool
lib3ds_file_remove_node(Lib3dsFile *file, Lib3dsNode *node)
{
  Lib3dsNode *p,*n;

  if (node->parent) {
    for (p=0,n=node->parent->childs; n; p=n,n=n->next) {
      if (n==node) {
        break;
      }
    }
    if (!n) {
      return(LIB3DS_FALSE);
    }
    
    if (!p) {
      node->parent->childs=n->next;
    }
    else {
      p->next=n->next;
    }
  }
  else {
    for (p=0,n=file->nodes; n; p=n,n=n->next) {
      if (n==node) {
        break;
      }
    }
    if (!n) {
      return(LIB3DS_FALSE);
    }
    
    if (!p) {
      file->nodes=n->next;
    }
    else {
      p->next=n->next;
    }
  }
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup file
 */
void
lib3ds_file_dump_nodes(Lib3dsFile *file)
{
  Lib3dsNode *p;

  ASSERT(file);
  for (p=file->nodes; p!=0; p=p->next) {
    lib3ds_node_dump(p,1);
  }
}


/*!

\typedef Lib3dsFile
  \ingroup file
  \sa _Lib3dsFile

*/
