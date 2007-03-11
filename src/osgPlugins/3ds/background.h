/* -*- c -*- */
#ifndef INCLUDED_LIB3DS_BACKGROUND_H
#define INCLUDED_LIB3DS_BACKGROUND_H
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
 * Bitmap background settings
 * \ingroup background
 */
typedef struct _Lib3dsBitmap {
    Lib3dsBool use;
    char name[64];
} Lib3dsBitmap;

/*!
 * Solid color background settings
 * \ingroup background
 */
typedef struct _Lib3dsSolid {
    Lib3dsBool use;
    Lib3dsRgb col;
} Lib3dsSolid;

/*!
 * Gradient background settings
 * \ingroup background
 */
typedef struct _Lib3dsGradient {
    Lib3dsBool use;
    Lib3dsFloat percent;
    Lib3dsRgb top;
    Lib3dsRgb middle;
    Lib3dsRgb bottom;
} Lib3dsGradient;

/*!
 * Background settings
 * \ingroup background
 */
struct _Lib3dsBackground {
    Lib3dsBitmap bitmap;
    Lib3dsSolid solid;
    Lib3dsGradient gradient;
};

extern LIB3DSAPI Lib3dsBool lib3ds_background_read(Lib3dsBackground *background, FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_background_write(Lib3dsBackground *background, FILE *f);

#ifdef __cplusplus
};
#endif
#endif





