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
#include "atmosphere.h"
#include "chunk.h"
#include "readwrite.h"


/*!
 * \defgroup atmosphere Atmosphere Settings
 *
 * \author J.E. Hoffmann <je-h@gmx.net>
 */


static Lib3dsBool
fog_read(Lib3dsFog *fog, FILE *f)
{
  Lib3dsChunk c;
  Lib3dsWord chunk;

  if (!lib3ds_chunk_read_start(&c, LIB3DS_FOG, f)) {
    return(LIB3DS_FALSE);
  }
  fog->near_plane=lib3ds_float_read(f);
  fog->near_density=lib3ds_float_read(f);
  fog->far_plane=lib3ds_float_read(f);
  fog->far_density=lib3ds_float_read(f);
  lib3ds_chunk_read_tell(&c, f);
  
  while ((chunk=lib3ds_chunk_read_next(&c, f))!=0) {
    switch (chunk) {
      case LIB3DS_LIN_COLOR_F:
        {
          int i;
          for (i=0; i<3; ++i) {
            fog->col[i]=lib3ds_float_read(f);
          }
        }
        break;
      case LIB3DS_COLOR_F:
        break;
      case LIB3DS_FOG_BGND:
        {
          fog->fog_background=LIB3DS_TRUE;
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
layer_fog_read(Lib3dsLayerFog *fog, FILE *f)
{
  Lib3dsChunk c;
  Lib3dsWord chunk;

  if (!lib3ds_chunk_read_start(&c, LIB3DS_LAYER_FOG, f)) {
    return(LIB3DS_FALSE);
  }
  fog->near_y=lib3ds_float_read(f);
  fog->far_y=lib3ds_float_read(f);
  fog->density=lib3ds_float_read(f);
  fog->flags=lib3ds_dword_read(f);
  lib3ds_chunk_read_tell(&c, f);
  
  while ((chunk=lib3ds_chunk_read_next(&c, f))!=0) {
    switch (chunk) {
      case LIB3DS_LIN_COLOR_F:
        lib3ds_rgb_read(fog->col,f);
        break;
      case LIB3DS_COLOR_F:
        lib3ds_rgb_read(fog->col,f);
        break;
      default:
        lib3ds_chunk_unknown(chunk);
    }
  }
  
  lib3ds_chunk_read_end(&c, f);
  return(LIB3DS_TRUE);
}


static Lib3dsBool
distance_cue_read(Lib3dsDistanceCue *cue, FILE *f)
{
  Lib3dsChunk c;
  Lib3dsWord chunk;

  if (!lib3ds_chunk_read_start(&c, LIB3DS_DISTANCE_CUE, f)) {
    return(LIB3DS_FALSE);
  }
  cue->near_plane=lib3ds_float_read(f);
  cue->near_dimming=lib3ds_float_read(f);
  cue->far_plane=lib3ds_float_read(f);
  cue->far_dimming=lib3ds_float_read(f);
  lib3ds_chunk_read_tell(&c, f);
  
  while ((chunk=lib3ds_chunk_read_next(&c, f))!=0) {
    switch (chunk) {
      case LIB3DS_DCUE_BGND:
        {
          cue->cue_background=LIB3DS_TRUE;
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
 * \ingroup atmosphere
 */
Lib3dsBool
lib3ds_atmosphere_read(Lib3dsAtmosphere *atmosphere, FILE *f)
{
  Lib3dsChunk c;

  if (!lib3ds_chunk_read(&c, f)) {
    return(LIB3DS_FALSE);
  }
  
  switch (c.chunk) {
      case LIB3DS_FOG:
        {
          lib3ds_chunk_read_reset(&c, f);
          if (!fog_read(&atmosphere->fog, f)) {
            return(LIB3DS_FALSE);
          }
        }
        break;
      case LIB3DS_LAYER_FOG:
        {
          lib3ds_chunk_read_reset(&c, f);
          if (!layer_fog_read(&atmosphere->layer_fog, f)) {
            return(LIB3DS_FALSE);
          }
        }
        break;
      case LIB3DS_DISTANCE_CUE:
        {
          lib3ds_chunk_read_reset(&c, f);
          if (!distance_cue_read(&atmosphere->dist_cue, f)) {
            return(LIB3DS_FALSE);
          }
        }
        break;
      case LIB3DS_USE_FOG:
        {
          atmosphere->fog.use=LIB3DS_TRUE;
        }
        break;
      case LIB3DS_USE_LAYER_FOG:
        {
          atmosphere->fog.use=LIB3DS_TRUE;
        }
        break;
      case LIB3DS_USE_DISTANCE_CUE:
        {
          atmosphere->dist_cue.use=LIB3DS_TRUE;
        }
        break;
  }

  return(LIB3DS_TRUE);
}


/*!
 * \ingroup atmosphere
 */
Lib3dsBool
lib3ds_atmosphere_write(Lib3dsAtmosphere *atmosphere, FILE *f)
{
  if (atmosphere->fog.use) { /*---- LIB3DS_FOG ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_FOG;
    if (!lib3ds_chunk_write_start(&c,f)) {
      return(LIB3DS_FALSE);
    }
    lib3ds_float_write(atmosphere->fog.near_plane,f);
    lib3ds_float_write(atmosphere->fog.near_density,f);
    lib3ds_float_write(atmosphere->fog.far_plane,f);
    lib3ds_float_write(atmosphere->fog.far_density,f);
    {
      Lib3dsChunk c;
      c.chunk=LIB3DS_COLOR_F;
      c.size=18;
      lib3ds_chunk_write(&c,f);
      lib3ds_rgb_write(atmosphere->fog.col,f);
    }
    if (atmosphere->fog.fog_background) {
      Lib3dsChunk c;
      c.chunk=LIB3DS_FOG_BGND;
      c.size=6;
      lib3ds_chunk_write(&c,f);
    }
    if (!lib3ds_chunk_write_end(&c,f)) {
      return(LIB3DS_FALSE);
    }
  }

  if (atmosphere->layer_fog.use) { /*---- LIB3DS_LAYER_FOG ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_LAYER_FOG;
    c.size=40;
    lib3ds_chunk_write(&c,f);
    lib3ds_float_write(atmosphere->layer_fog.near_y,f);
    lib3ds_float_write(atmosphere->layer_fog.far_y,f);
    lib3ds_float_write(atmosphere->layer_fog.near_y,f);
    lib3ds_dword_write(atmosphere->layer_fog.flags,f);
    {
      Lib3dsChunk c;
      c.chunk=LIB3DS_COLOR_F;
      c.size=18;
      lib3ds_chunk_write(&c,f);
      lib3ds_rgb_write(atmosphere->fog.col,f);
    }
  }

  if (atmosphere->dist_cue.use) { /*---- LIB3DS_DISTANCE_CUE ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_DISTANCE_CUE;
    if (!lib3ds_chunk_write_start(&c,f)) {
      return(LIB3DS_FALSE);
    }
    lib3ds_float_write(atmosphere->dist_cue.near_plane,f);
    lib3ds_float_write(atmosphere->dist_cue.near_dimming,f);
    lib3ds_float_write(atmosphere->dist_cue.far_plane,f);
    lib3ds_float_write(atmosphere->dist_cue.far_dimming,f);
    if (atmosphere->dist_cue.cue_background) {
      Lib3dsChunk c;
      c.chunk=LIB3DS_DCUE_BGND;
      c.size=6;
      lib3ds_chunk_write(&c,f);
    }
    if (!lib3ds_chunk_write_end(&c,f)) {
      return(LIB3DS_FALSE);
    }
  }

  if (atmosphere->fog.use) { /*---- LIB3DS_USE_FOG ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_USE_FOG;
    c.size=6;
    lib3ds_chunk_write(&c,f);
  }

  if (atmosphere->layer_fog.use) { /*---- LIB3DS_USE_LAYER_FOG ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_USE_LAYER_FOG;
    c.size=6;
    lib3ds_chunk_write(&c,f);
  }

  if (atmosphere->dist_cue.use) { /*---- LIB3DS_USE_DISTANCE_CUE ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_USE_V_GRADIENT;
    c.size=6;
    lib3ds_chunk_write(&c,f);
  }
  
  return(LIB3DS_TRUE);
}


/*!

\typedef Lib3dsAtmosphere
  \ingroup atmosphere
  \sa _Lib3dsAtmosphere

*/

