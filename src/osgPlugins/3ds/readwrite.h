/* -*- c -*- */
#ifndef INCLUDED_LIB3DS_READWRITE_H
#define INCLUDED_LIB3DS_READWRITE_H
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

extern LIB3DSAPI void setByteOrder();

extern LIB3DSAPI Lib3dsByte lib3ds_byte_read(FILE *f);
extern LIB3DSAPI Lib3dsWord lib3ds_word_read(FILE *f);
extern LIB3DSAPI Lib3dsDword lib3ds_dword_read(FILE *f);
extern LIB3DSAPI Lib3dsIntb lib3ds_intb_read(FILE *f);
extern LIB3DSAPI Lib3dsIntw lib3ds_intw_read(FILE *f);
extern LIB3DSAPI Lib3dsIntd lib3ds_intd_read(FILE *f);
extern LIB3DSAPI Lib3dsFloat lib3ds_float_read(FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_vector_read(Lib3dsVector v, FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_rgb_read(Lib3dsRgb rgb, FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_string_read(char *s, int buflen, FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_byte_write(Lib3dsByte b, FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_word_write(Lib3dsWord w, FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_dword_write(Lib3dsDword d, FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_intb_write(Lib3dsIntb b, FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_intw_write(Lib3dsIntw w, FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_intd_write(Lib3dsIntd d, FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_float_write(Lib3dsFloat l, FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_vector_write(Lib3dsVector v, FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_rgb_write(Lib3dsRgb rgb, FILE *f);
extern LIB3DSAPI Lib3dsBool lib3ds_string_write(const char *s, FILE *f);

#ifdef __cplusplus
};
#endif
#endif

