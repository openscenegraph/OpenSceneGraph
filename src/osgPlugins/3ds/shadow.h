/* -*- c -*- */
#ifndef INCLUDED_LIB3DS_SHADOW_H
#define INCLUDED_LIB3DS_SHADOW_H
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
 * Shadow map settings
 * \ingroup shadow
 */
struct _Lib3dsShadow {
    Lib3dsIntw map_size;
    Lib3dsFloat lo_bias;
    Lib3dsFloat hi_bias;
    Lib3dsIntw samples;
    Lib3dsIntd range;
    Lib3dsFloat filter;
    Lib3dsFloat ray_bias;
};

extern LIB3DSAPI Lib3dsBool lib3ds_shadow_read(Lib3dsShadow *shadow, FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_shadow_write(Lib3dsShadow *shadow, FILE *f);

#ifdef __cplusplus
}
#endif
#endif





