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
#include "viewport.h"
#include "chunk.h"
#include "readwrite.h"
#include <stdlib.h>
#include <string.h>


/*!
 * \defgroup viewport Viewport and default view settings
 *
 * \author J.E. Hoffmann <je-h@gmx.net>
 */


/*!
 * \ingroup viewport 
 */
Lib3dsBool
lib3ds_viewport_read(Lib3dsViewport *viewport, iostream *strm)
{
  Lib3dsChunk c;
  Lib3dsWord chunk;

  if (!lib3ds_chunk_read_start(&c, 0, strm)) {
    return(LIB3DS_FALSE);
  }
  
  switch (c.chunk) {
    case LIB3DS_VIEWPORT_LAYOUT:
      {
        int cur=0;
        viewport->layout.style=lib3ds_word_read(strm);
        viewport->layout.active=lib3ds_intw_read(strm);
        lib3ds_intw_read(strm);
        viewport->layout.swap=lib3ds_intw_read(strm);
        lib3ds_intw_read(strm);
        viewport->layout.swap_prior=lib3ds_intw_read(strm);
        viewport->layout.swap_view=lib3ds_intw_read(strm);
        lib3ds_chunk_read_tell(&c, strm);
        while ((chunk=lib3ds_chunk_read_next(&c, strm))!=0) {
          switch (chunk) {
            case LIB3DS_VIEWPORT_SIZE:
              {
                viewport->layout.position[0]=lib3ds_word_read(strm);
                viewport->layout.position[1]=lib3ds_word_read(strm);
                viewport->layout.size[0]=lib3ds_word_read(strm);
                viewport->layout.size[1]=lib3ds_word_read(strm);
              }
              break;
            case LIB3DS_VIEWPORT_DATA_3:
              {
                lib3ds_viewport_set_views(viewport,cur+1);
                lib3ds_intw_read(strm);
                viewport->layout.viewL[cur].axis_lock=lib3ds_word_read(strm);
                viewport->layout.viewL[cur].position[0]=lib3ds_intw_read(strm);
                viewport->layout.viewL[cur].position[1]=lib3ds_intw_read(strm);
                viewport->layout.viewL[cur].size[0]=lib3ds_intw_read(strm);
                viewport->layout.viewL[cur].size[1]=lib3ds_intw_read(strm);
                viewport->layout.viewL[cur].type=lib3ds_word_read(strm);
                viewport->layout.viewL[cur].zoom=lib3ds_float_read(strm);
                lib3ds_vector_read(viewport->layout.viewL[cur].center,strm);
                viewport->layout.viewL[cur].horiz_angle=lib3ds_float_read(strm);
                viewport->layout.viewL[cur].vert_angle=lib3ds_float_read(strm);
                strm->read(viewport->layout.viewL[cur].camera,11);
                ++cur;
              }
              break;
            case LIB3DS_VIEWPORT_DATA:
              /* 3DS R2 & R3 chunk
                 unsupported */
              break;
            default:
              lib3ds_chunk_unknown(chunk);
          }
        }
      }
      break;
    case LIB3DS_DEFAULT_VIEW:
      {
        memset(&viewport->default_view,0,sizeof(Lib3dsDefaultView));
        while ((chunk=lib3ds_chunk_read_next(&c, strm))!=0) {
          switch (chunk) {
            case LIB3DS_VIEW_TOP:
              {
                viewport->default_view.type=LIB3DS_VIEW_TYPE_TOP;
                lib3ds_vector_read(viewport->default_view.position,strm);
                viewport->default_view.width=lib3ds_float_read(strm);
              }
              break;
            case LIB3DS_VIEW_BOTTOM:
              {
                viewport->default_view.type=LIB3DS_VIEW_TYPE_BOTTOM;
                lib3ds_vector_read(viewport->default_view.position,strm);
                viewport->default_view.width=lib3ds_float_read(strm);
              }
              break;
            case LIB3DS_VIEW_LEFT:
              {
                viewport->default_view.type=LIB3DS_VIEW_TYPE_LEFT;
                lib3ds_vector_read(viewport->default_view.position,strm);
                viewport->default_view.width=lib3ds_float_read(strm);
              }
              break;
            case LIB3DS_VIEW_RIGHT:
              {
                viewport->default_view.type=LIB3DS_VIEW_TYPE_RIGHT;
                lib3ds_vector_read(viewport->default_view.position,strm);
                viewport->default_view.width=lib3ds_float_read(strm);
              }
              break;
            case LIB3DS_VIEW_FRONT:
              {
                viewport->default_view.type=LIB3DS_VIEW_TYPE_FRONT;
                lib3ds_vector_read(viewport->default_view.position,strm);
                viewport->default_view.width=lib3ds_float_read(strm);
              }
              break;
            case LIB3DS_VIEW_BACK:
              {
                viewport->default_view.type=LIB3DS_VIEW_TYPE_BACK;
                lib3ds_vector_read(viewport->default_view.position,strm);
                viewport->default_view.width=lib3ds_float_read(strm);
              }
              break;
            case LIB3DS_VIEW_USER:
              {
                viewport->default_view.type=LIB3DS_VIEW_TYPE_USER;
                lib3ds_vector_read(viewport->default_view.position,strm);
                viewport->default_view.width=lib3ds_float_read(strm);
                viewport->default_view.horiz_angle=lib3ds_float_read(strm);
                viewport->default_view.vert_angle=lib3ds_float_read(strm);
                viewport->default_view.roll_angle=lib3ds_float_read(strm);
              }
              break;
            case LIB3DS_VIEW_CAMERA:
              {
                viewport->default_view.type=LIB3DS_VIEW_TYPE_CAMERA;
                strm->read(viewport->default_view.camera,11);
              }
              break;
            default:
              lib3ds_chunk_unknown(chunk);
          }
        }
      }
      break;
  }

  lib3ds_chunk_read_end(&c, strm);
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup viewport 
 */
void
lib3ds_viewport_set_views(Lib3dsViewport *viewport, Lib3dsDword views)
{
  ASSERT(viewport);
  if (viewport->layout.views) {
    if (views) {
      viewport->layout.views=views;
      viewport->layout.viewL=(Lib3dsView*)realloc(viewport->layout.viewL, sizeof(Lib3dsView)*views);
    }
    else {
      free(viewport->layout.viewL);
      viewport->layout.views=0;
      viewport->layout.viewL=0;
    }
  }
  else {
    viewport->layout.views=views;
    viewport->layout.viewL= views ? (Lib3dsView*)calloc(sizeof(Lib3dsView),views) : 0;
  }
}


/*!
 * \ingroup viewport 
 */
Lib3dsBool
lib3ds_viewport_write(Lib3dsViewport *viewport, iostream *strm)
{
  if (viewport->layout.views) {
    Lib3dsChunk c;
    unsigned i;

    c.chunk=LIB3DS_VIEWPORT_LAYOUT;
    if (!lib3ds_chunk_write_start(&c,strm)) {
      return(LIB3DS_FALSE);
    }

    lib3ds_word_write(viewport->layout.style,strm);
    lib3ds_intw_write(viewport->layout.active,strm);
    lib3ds_intw_write(0,strm);
    lib3ds_intw_write(viewport->layout.swap,strm);
    lib3ds_intw_write(0,strm);
    lib3ds_intw_write(viewport->layout.swap_prior,strm);
    lib3ds_intw_write(viewport->layout.swap_view,strm);
    
    {
      Lib3dsChunk c;
      c.chunk=LIB3DS_VIEWPORT_SIZE;
      c.size=14;
      lib3ds_chunk_write(&c,strm);
      lib3ds_intw_write(viewport->layout.position[0],strm);
      lib3ds_intw_write(viewport->layout.position[1],strm);
      lib3ds_intw_write(viewport->layout.size[0],strm);
      lib3ds_intw_write(viewport->layout.size[1],strm);
    }

    for (i=0; i<viewport->layout.views; ++i) {
      Lib3dsChunk c;
      c.chunk=LIB3DS_VIEWPORT_DATA_3;
      c.size=55;
      lib3ds_chunk_write(&c,strm);

      lib3ds_intw_write(0,strm);
      lib3ds_word_write(viewport->layout.viewL[i].axis_lock,strm);
      lib3ds_intw_write(viewport->layout.viewL[i].position[0],strm);
      lib3ds_intw_write(viewport->layout.viewL[i].position[1],strm);
      lib3ds_intw_write(viewport->layout.viewL[i].size[0],strm);
      lib3ds_intw_write(viewport->layout.viewL[i].size[1],strm);
      lib3ds_word_write(viewport->layout.viewL[i].type,strm);
      lib3ds_float_write(viewport->layout.viewL[i].zoom,strm);
      lib3ds_vector_write(viewport->layout.viewL[i].center,strm);
      lib3ds_float_write(viewport->layout.viewL[i].horiz_angle,strm);
      lib3ds_float_write(viewport->layout.viewL[i].vert_angle,strm);
      strm->write(viewport->layout.viewL[i].camera,11);
    }

    if (!lib3ds_chunk_write_end(&c,strm)) {
      return(LIB3DS_FALSE);
    }
  }

  if (viewport->default_view.type) {
    Lib3dsChunk c;

    c.chunk=LIB3DS_DEFAULT_VIEW;
    if (!lib3ds_chunk_write_start(&c,strm)) {
      return(LIB3DS_FALSE);
    }

    switch (viewport->default_view.type) {
      case LIB3DS_VIEW_TYPE_TOP:
        {
          Lib3dsChunk c;
          c.chunk=LIB3DS_VIEW_TOP;
          c.size=22;
          lib3ds_chunk_write(&c,strm);
          lib3ds_vector_write(viewport->default_view.position,strm);
          lib3ds_float_write(viewport->default_view.width,strm);
        }
        break;
      case LIB3DS_VIEW_TYPE_BOTTOM:
        {
          Lib3dsChunk c;
          c.chunk=LIB3DS_VIEW_BOTTOM;
          c.size=22;
          lib3ds_chunk_write(&c,strm);
          lib3ds_vector_write(viewport->default_view.position,strm);
          lib3ds_float_write(viewport->default_view.width,strm);
        }
        break;
      case LIB3DS_VIEW_TYPE_LEFT:
        {
          Lib3dsChunk c;
          c.chunk=LIB3DS_VIEW_LEFT;
          c.size=22;
          lib3ds_chunk_write(&c,strm);
          lib3ds_vector_write(viewport->default_view.position,strm);
          lib3ds_float_write(viewport->default_view.width,strm);
        }
        break;
      case LIB3DS_VIEW_TYPE_RIGHT:
        {
          Lib3dsChunk c;
          c.chunk=LIB3DS_VIEW_RIGHT;
          c.size=22;
          lib3ds_chunk_write(&c,strm);
          lib3ds_vector_write(viewport->default_view.position,strm);
          lib3ds_float_write(viewport->default_view.width,strm);
        }
        break;
      case LIB3DS_VIEW_TYPE_FRONT:
        {
          Lib3dsChunk c;
          c.chunk=LIB3DS_VIEW_FRONT;
          c.size=22;
          lib3ds_chunk_write(&c,strm);
          lib3ds_vector_write(viewport->default_view.position,strm);
          lib3ds_float_write(viewport->default_view.width,strm);
        }
        break;
      case LIB3DS_VIEW_TYPE_BACK:
        {
          Lib3dsChunk c;
          c.chunk=LIB3DS_VIEW_BACK;
          c.size=22;
          lib3ds_chunk_write(&c,strm);
          lib3ds_vector_write(viewport->default_view.position,strm);
          lib3ds_float_write(viewport->default_view.width,strm);
        }
        break;
      case LIB3DS_VIEW_TYPE_USER:
        {
          Lib3dsChunk c;
          c.chunk=LIB3DS_VIEW_USER;
          c.size=34;
          lib3ds_chunk_write(&c,strm);
          lib3ds_vector_write(viewport->default_view.position,strm);
          lib3ds_float_write(viewport->default_view.width,strm);
          lib3ds_float_write(viewport->default_view.horiz_angle,strm);
          lib3ds_float_write(viewport->default_view.vert_angle,strm);
          lib3ds_float_write(viewport->default_view.roll_angle,strm);
        }
        break;
      case LIB3DS_VIEW_TYPE_CAMERA:
        {
          Lib3dsChunk c;
          c.chunk=LIB3DS_VIEW_CAMERA;
          c.size=17;
          lib3ds_chunk_write(&c,strm);
          strm->write(viewport->default_view.camera,11); // NAH Potential issue here
        }
        break;
    }

    if (!lib3ds_chunk_write_end(&c,strm)) {
      return(LIB3DS_FALSE);
    }
  }
  return(LIB3DS_TRUE);
}


/*!

\typedef Lib3dsViewport
  \ingroup viewport
  \sa _Lib3dsViewport

*/
