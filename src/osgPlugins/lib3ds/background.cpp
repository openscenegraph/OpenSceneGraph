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
#include "background.h"
#include "chunk.h"
#include "readwrite.h"
#include <string.h>
#include <math.h>


/*!
 * \defgroup background Background Settings
 *
 * \author J.E. Hoffmann <je-h@gmx.net>
 */


static Lib3dsBool
solid_bgnd_read(Lib3dsBackground *background, FILE *f)
{
  Lib3dsChunk c;
  Lib3dsWord chunk;
          
  if (!lib3ds_chunk_read_start(&c, LIB3DS_SOLID_BGND, f)) {
    return(LIB3DS_FALSE);
  }

  while ((chunk=lib3ds_chunk_read_next(&c, f))!=0) {
    switch (chunk) {
      case LIB3DS_LIN_COLOR_F:
        lib3ds_rgb_read(background->solid.col, f);
        break;
      case LIB3DS_COLOR_F:
        lib3ds_rgb_read(background->solid.col, f);
        break;
      default:
        lib3ds_chunk_unknown(chunk);
    }
  }
  
  lib3ds_chunk_read_end(&c, f);
  return(LIB3DS_TRUE);
}


static Lib3dsBool
v_gradient_read(Lib3dsBackground *background, FILE *f)
{
  Lib3dsChunk c;
  Lib3dsWord chunk;
  int index[2];
  Lib3dsRgb col[2][3];
  int have_lin=0;
  

  if (!lib3ds_chunk_read_start(&c, LIB3DS_V_GRADIENT, f)) {
    return(LIB3DS_FALSE);
  }
  background->gradient.percent=lib3ds_float_read(f);
  lib3ds_chunk_read_tell(&c, f);

  index[0]=index[1]=0;
  while ((chunk=lib3ds_chunk_read_next(&c, f))!=0) {
    switch (chunk) {
      case LIB3DS_COLOR_F:
        lib3ds_rgb_read(col[0][index[0]],f);
        index[0]++;
        break;
      case LIB3DS_LIN_COLOR_F:
        lib3ds_rgb_read(col[1][index[1]],f);
        index[1]++;
        have_lin=1;
        break;
      default:
        lib3ds_chunk_unknown(chunk);
    }
  }
  {
    int i;
    for (i=0; i<3; ++i) {
      background->gradient.top[i]=col[have_lin][0][i];
      background->gradient.middle[i]=col[have_lin][1][i];
      background->gradient.bottom[i]=col[have_lin][2][i];
    }
  }
  lib3ds_chunk_read_end(&c, f);
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup background
 */
Lib3dsBool
lib3ds_background_read(Lib3dsBackground *background, FILE *f)
{
  Lib3dsChunk c;

  if (!lib3ds_chunk_read(&c, f)) {
    return(LIB3DS_FALSE);
  }
  
  switch (c.chunk) {
    case LIB3DS_BIT_MAP:
      {
        if (!lib3ds_string_read(background->bitmap.name, 64, f)) {
            return(LIB3DS_FALSE);
        }
      }
        break;
      case LIB3DS_SOLID_BGND:
        {
          lib3ds_chunk_read_reset(&c, f);
          if (!solid_bgnd_read(background, f)) {
            return(LIB3DS_FALSE);
          }
        }
        break;
      case LIB3DS_V_GRADIENT:
        {
          lib3ds_chunk_read_reset(&c, f);
          if (!v_gradient_read(background, f)) {
            return(LIB3DS_FALSE);
          }
        }
        break;
      case LIB3DS_USE_BIT_MAP:
        {
          background->bitmap.use=LIB3DS_TRUE;
        }
        break;
      case LIB3DS_USE_SOLID_BGND:
        {
          background->solid.use=LIB3DS_TRUE;
        }
        break;
      case LIB3DS_USE_V_GRADIENT:
        {
          background->gradient.use=LIB3DS_TRUE;
        }
        break;
  }
  
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
colorf_defined(Lib3dsRgba rgb)
{
  int i;
  for (i=0; i<3; ++i) {
    if (fabs(rgb[i])>LIB3DS_EPSILON) {
      break;
    }
  }
  return(i<3);
}


/*!
 * \ingroup background
 */
Lib3dsBool
lib3ds_background_write(Lib3dsBackground *background, FILE *f)
{
  if (strlen(background->bitmap.name)) { /*---- LIB3DS_BIT_MAP ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_BIT_MAP;
    c.size=6+1+strlen(background->bitmap.name);
    lib3ds_chunk_write(&c,f);
    lib3ds_string_write(background->bitmap.name, f);
  }

  if (colorf_defined(background->solid.col)) { /*---- LIB3DS_SOLID_BGND ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_SOLID_BGND;
    c.size=42;
    lib3ds_chunk_write(&c,f);
    colorf_write(background->solid.col,f);
  }

  if (colorf_defined(background->gradient.top) ||
    colorf_defined(background->gradient.middle) ||
    colorf_defined(background->gradient.bottom)) { /*---- LIB3DS_V_GRADIENT ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_V_GRADIENT;
    c.size=118;
    lib3ds_chunk_write(&c,f);
    lib3ds_float_write(background->gradient.percent,f);
    colorf_write(background->gradient.top,f);
    colorf_write(background->gradient.middle,f);
    colorf_write(background->gradient.bottom,f);
  }

  if (background->bitmap.use) { /*---- LIB3DS_USE_BIT_MAP ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_USE_BIT_MAP;
    c.size=6;
    lib3ds_chunk_write(&c,f);
  }

  if (background->solid.use) { /*---- LIB3DS_USE_SOLID_BGND ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_USE_SOLID_BGND;
    c.size=6;
    lib3ds_chunk_write(&c,f);
  }

  if (background->gradient.use) { /*---- LIB3DS_USE_V_GRADIENT ----*/
    Lib3dsChunk c;
    c.chunk=LIB3DS_USE_V_GRADIENT;
    c.size=6;
    lib3ds_chunk_write(&c,f);
  }
  
  return(LIB3DS_TRUE);
}


/*!

\typedef Lib3dsBackground
  \ingroup background
  \sa _Lib3dsBackground

*/
