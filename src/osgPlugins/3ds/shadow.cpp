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
#include "shadow.h"
#include "chunk.h"
#include "readwrite.h"
#include <math.h>


/*!
 * \defgroup shadow Shadow Map Settings
 *
 * \author J.E. Hoffmann <je-h@gmx.net>
 */


/*!
 * \ingroup shadow 
 */
Lib3dsBool
lib3ds_shadow_read(Lib3dsShadow *shadow, FILE *f)
{
  Lib3dsChunk c;

  if (!lib3ds_chunk_read(&c, f)) {
    return(LIB3DS_FALSE);
  }
  
  switch (c.chunk) {
    case LIB3DS_SHADOW_MAP_SIZE:
      {
        shadow->map_size=lib3ds_intw_read(f);
      }
      break;
    case LIB3DS_LO_SHADOW_BIAS:
      {
          shadow->lo_bias=lib3ds_float_read(f);
      }
      break;
    case LIB3DS_HI_SHADOW_BIAS:
      {
        shadow->hi_bias=lib3ds_float_read(f);
      }
      break;
    case LIB3DS_SHADOW_SAMPLES:
      {
        shadow->samples=lib3ds_intw_read(f);
      }
      break;
    case LIB3DS_SHADOW_RANGE:
      {
        shadow->range=lib3ds_intd_read(f);
      }
      break;
    case LIB3DS_SHADOW_FILTER:
      {
        shadow->filter=lib3ds_float_read(f);
      }
      break;
    case LIB3DS_RAY_BIAS:
      {
        shadow->ray_bias=lib3ds_float_read(f);
      }
      break;
  }
  
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup shadow 
 */
Lib3dsBool
lib3ds_shadow_write(Lib3dsShadow *shadow, FILE *f)
{
  if (fabs(shadow->lo_bias)>LIB3DS_EPSILON) { /*---- LIB3DS_LO_SHADOW_BIAS ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_LO_SHADOW_BIAS;
    c.size=10;
    lib3ds_chunk_write(&c,f);
    lib3ds_float_write(shadow->lo_bias,f);
  }

  if (fabs(shadow->hi_bias)>LIB3DS_EPSILON) { /*---- LIB3DS_HI_SHADOW_BIAS ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_HI_SHADOW_BIAS;
    c.size=10;
    lib3ds_chunk_write(&c,f);
    lib3ds_float_write(shadow->hi_bias,f);
  }

  if (shadow->map_size) { /*---- LIB3DS_SHADOW_MAP_SIZE ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_SHADOW_MAP_SIZE;
    c.size=8;
    lib3ds_chunk_write(&c,f);
    lib3ds_intw_write(shadow->map_size,f);
  }
  
  if (shadow->samples) { /*---- LIB3DS_SHADOW_SAMPLES ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_SHADOW_SAMPLES;
    c.size=8;
    lib3ds_chunk_write(&c,f);
    lib3ds_intw_write(shadow->samples,f);
  }

  if (shadow->range) { /*---- LIB3DS_SHADOW_RANGE ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_SHADOW_RANGE;
    c.size=10;
    lib3ds_chunk_write(&c,f);
    lib3ds_intd_write(shadow->range,f);
  }

  if (fabs(shadow->filter)>LIB3DS_EPSILON) { /*---- LIB3DS_SHADOW_FILTER ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_SHADOW_FILTER;
    c.size=10;
    lib3ds_chunk_write(&c,f);
    lib3ds_float_write(shadow->filter,f);
  }
  if (fabs(shadow->ray_bias)>LIB3DS_EPSILON) { /*---- LIB3DS_RAY_BIAS ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_RAY_BIAS;
    c.size=10;
    lib3ds_chunk_write(&c,f);
    lib3ds_float_write(shadow->ray_bias,f);
  }
  return(LIB3DS_TRUE);
}


/*!

\typedef Lib3dsShadow
  \ingroup shadow
  \sa _Lib3dsShadow

*/
