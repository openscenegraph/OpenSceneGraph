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
#include <material.h>
#include <chunk.h>
#include <readwrite.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
//#include <config.h>
#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif

/*!
 * \defgroup material Materials
 *
 * \author J.E. Hoffmann <je-h@gmx.net>
 */

/*!
 * \ingroup material
 */
Lib3dsMaterial*
lib3ds_material_new()
{
    Lib3dsMaterial *material;

    material=(Lib3dsMaterial*)calloc(sizeof(Lib3dsMaterial), 1);
    if (!material)
    {
        return(0);
    }
    return(material);
}


/*!
 * \ingroup material
 */
void
lib3ds_material_free(Lib3dsMaterial *material)
{
    memset(material, 0, sizeof(Lib3dsMaterial));
    free(material);
}


static Lib3dsBool
color_read(Lib3dsRgba rgb, FILE *f)
{
    Lib3dsChunk c;
    Lib3dsWord chunk;
    Lib3dsBool have_lin=LIB3DS_FALSE;

    if (!lib3ds_chunk_read_start(&c, 0, f))
    {
        return(LIB3DS_FALSE);
    }

    while ((chunk=lib3ds_chunk_read_next(&c, f))!=0)
    {
        switch (chunk)
        {
            case LIB3DS_LIN_COLOR_24:
            {
                int i;
                for (i=0; i<3; ++i)
                {
                    rgb[i]=1.0f*lib3ds_byte_read(f)/255.0f;
                }
                rgb[3]=1.0f;
            }
            have_lin=LIB3DS_TRUE;
            break;
            case LIB3DS_COLOR_24:
                /* gamma corrected color chunk
                   replaced in 3ds R3 by LIN_COLOR_24 */
                if (!have_lin)
                {
                    int i;
                    for (i=0; i<3; ++i)
                    {
                        rgb[i]=1.0f*lib3ds_byte_read(f)/255.0f;
                    }
                    rgb[3]=1.0f;
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
int_percentage_read(Lib3dsFloat *p, FILE *f)
{
    Lib3dsChunk c;
    Lib3dsWord chunk;

    if (!lib3ds_chunk_read_start(&c, 0, f))
    {
        return(LIB3DS_FALSE);
    }

    while ((chunk=lib3ds_chunk_read_next(&c, f))!=0)
    {
        switch (chunk)
        {
            case LIB3DS_INT_PERCENTAGE:
            {
                Lib3dsIntw i=lib3ds_intw_read(f);
                *p=(Lib3dsFloat)(1.0*i/100.0);
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
texture_map_read(Lib3dsTextureMap *map, FILE *f)
{
    Lib3dsChunk c;
    Lib3dsWord chunk;

    if (!lib3ds_chunk_read_start(&c, 0, f))
    {
        return(LIB3DS_FALSE);
    }

    while ((chunk=lib3ds_chunk_read_next(&c, f))!=0)
    {
        switch (chunk)
        {
            case LIB3DS_INT_PERCENTAGE:
            {
                map->percent=1.0f*lib3ds_intw_read(f)/100.0f;
            }
            break;
            case LIB3DS_MAT_MAPNAME:
            {
                if (!lib3ds_string_read(map->name, 64, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_MAP_TILING:
            {
                map->flags=lib3ds_word_read(f);
            }
            break;
            case LIB3DS_MAT_MAP_TEXBLUR:
            {
                map->blur=lib3ds_float_read(f);
            }
            break;
            case LIB3DS_MAT_MAP_USCALE:
            {
                map->scale[0]=lib3ds_float_read(f);
            }
            break;
            case LIB3DS_MAT_MAP_VSCALE:
            {
                map->scale[1]=lib3ds_float_read(f);
            }
            break;
            case LIB3DS_MAT_MAP_UOFFSET:
            {
                map->offset[0]=lib3ds_float_read(f);
            }
            break;
            case LIB3DS_MAT_MAP_VOFFSET:
            {
                map->offset[1]=lib3ds_float_read(f);
            }
            break;
            case LIB3DS_MAT_MAP_ANG:
            {
                map->rotation=lib3ds_float_read(f);
            }
            break;
            case LIB3DS_MAT_MAP_COL1:
            {
                map->tint_1[0]=1.0f*lib3ds_byte_read(f)/255.0f;
                map->tint_1[1]=1.0f*lib3ds_byte_read(f)/255.0f;
                map->tint_1[2]=1.0f*lib3ds_byte_read(f)/255.0f;
            }
            break;
            case LIB3DS_MAT_MAP_COL2:
            {
                map->tint_2[0]=1.0f*lib3ds_byte_read(f)/255.0f;
                map->tint_2[1]=1.0f*lib3ds_byte_read(f)/255.0f;
                map->tint_2[2]=1.0f*lib3ds_byte_read(f)/255.0f;
            }
            break;
            case LIB3DS_MAT_MAP_RCOL:
            {
                map->tint_r[0]=1.0f*lib3ds_byte_read(f)/255.0f;
                map->tint_r[1]=1.0f*lib3ds_byte_read(f)/255.0f;
                map->tint_r[2]=1.0f*lib3ds_byte_read(f)/255.0f;
            }
            break;
            case LIB3DS_MAT_MAP_GCOL:
            {
                map->tint_g[0]=1.0f*lib3ds_byte_read(f)/255.0f;
                map->tint_g[1]=1.0f*lib3ds_byte_read(f)/255.0f;
                map->tint_g[2]=1.0f*lib3ds_byte_read(f)/255.0f;
            }
            break;
            case LIB3DS_MAT_MAP_BCOL:
            {
                map->tint_b[0]=1.0f*lib3ds_byte_read(f)/255.0f;
                map->tint_b[1]=1.0f*lib3ds_byte_read(f)/255.0f;
                map->tint_b[2]=1.0f*lib3ds_byte_read(f)/255.0f;
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
 * \ingroup material
 */
void
lib3ds_material_dump(Lib3dsMaterial *material)
{
    ASSERT(material);
    printf("  %s\n", material->name);
    printf("    Ambient:        %1.2f %1.2f %1.2f\n",
        material->ambient[0],material->ambient[1], material->ambient[2]);
    printf("    Diffuse:        %1.2f %1.2f %1.2f\n",
        material->diffuse[0],material->diffuse[1], material->diffuse[2]);
    printf("    Specular:       %1.2f %1.2f %1.2f\n",
        material->specular[0],material->specular[1], material->specular[2]);
    printf("    Shininess:      %1.2f\n", material->shininess);
    printf("    Shin. Strength: %1.2f\n", material->shin_strength);
    printf("    Shading:        %d\n", material->shading);
    if (*material->texture1_map.name)
    {
        printf("    Texture 1 Map:  '%s'\n", material->texture1_map.name);
    }
    if (*material->texture1_mask.name)
    {
        printf("    Texture 1 Mask: '%s'\n", material->texture1_mask.name);
    }
    if (*material->texture2_map.name)
    {
        printf("    Texture 2 Map:  '%s'\n", material->texture2_map.name);
    }
    if (*material->texture2_mask.name)
    {
        printf("    Texture 2 Mask: '%s'\n", material->texture2_mask.name);
    }
}


/*!
 * \ingroup material
 */
Lib3dsBool
lib3ds_material_read(Lib3dsMaterial *material, FILE *f)
{
    Lib3dsChunk c;
    Lib3dsWord chunk;

    ASSERT(material);
    if (!lib3ds_chunk_read_start(&c, LIB3DS_MAT_ENTRY, f))
    {
        return(LIB3DS_FALSE);
    }

    while ((chunk=lib3ds_chunk_read_next(&c, f))!=0)
    {
        switch (chunk)
        {
            case LIB3DS_MAT_NAME:
            {
                if (!lib3ds_string_read(material->name, 64, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_AMBIENT:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!color_read(material->ambient, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_DIFFUSE:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!color_read(material->diffuse, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_SPECULAR:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!color_read(material->specular, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_SHININESS:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!int_percentage_read(&material->shininess, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_SHIN2PCT:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!int_percentage_read(&material->shin_strength, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_TRANSPARENCY:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!int_percentage_read(&material->transparency, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_XPFALL:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!int_percentage_read(&material->falloff, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_USE_XPFALL:
            {
                material->use_falloff=LIB3DS_TRUE;
            }
            break;
            case LIB3DS_MAT_REFBLUR:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!int_percentage_read(&material->blur, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_USE_REFBLUR:
            {
                material->use_blur=LIB3DS_TRUE;
            }
            break;
            case LIB3DS_MAT_SHADING:
            {
                material->shading=lib3ds_intw_read(f);
            }
            break;
            case LIB3DS_MAT_SELF_ILLUM:
            {
                material->self_illum=LIB3DS_TRUE;
            }
            break;
            case LIB3DS_MAT_TWO_SIDE:
            {
                material->two_sided=LIB3DS_TRUE;
            }
            break;
            case LIB3DS_MAT_DECAL:
            {
                material->map_decal=LIB3DS_TRUE;
            }
            break;
            case LIB3DS_MAT_ADDITIVE:
            {
                material->additive=LIB3DS_TRUE;
            }
            break;
            case LIB3DS_MAT_FACEMAP:
            {
                material->face_map=LIB3DS_TRUE;
            }
            break;
            case LIB3DS_MAT_PHONGSOFT:
            {
                material->soften=LIB3DS_TRUE;
            }
            break;
            case LIB3DS_MAT_WIRE:
            {
                material->use_wire=LIB3DS_TRUE;
            }
            break;
            case LIB3DS_MAT_WIREABS:
            {
                material->use_wire_abs=LIB3DS_TRUE;
            }
            break;
            case LIB3DS_MAT_WIRE_SIZE:
            {
                material->wire_size=lib3ds_float_read(f);
            }
            break;
            case LIB3DS_MAT_TEXMAP:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->texture1_map, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_TEXMASK:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->texture1_mask, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_TEX2MAP:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->texture2_map, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_TEX2MASK:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->texture2_mask, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_OPACMAP:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->opacity_map, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_OPACMASK:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->opacity_mask, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_BUMPMAP:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->bump_map, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_BUMPMASK:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->bump_mask, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_SPECMAP:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->specular_map, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_SPECMASK:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->specular_mask, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_SHINMAP:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->shininess_map, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_SHINMASK:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->shininess_mask, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_SELFIMAP:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->self_illum_map, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_SELFIMASK:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->self_illum_mask, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_REFLMAP:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->reflection_map, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_REFLMASK:
            {
                lib3ds_chunk_read_reset(&c, f);
                if (!texture_map_read(&material->reflection_mask, f))
                {
                    return(LIB3DS_FALSE);
                }
            }
            break;
            case LIB3DS_MAT_ACUBIC:
            {
                lib3ds_intb_read(f);
                material->autorefl_map.level=lib3ds_intb_read(f);
                material->autorefl_map.flags=lib3ds_intw_read(f);
                material->autorefl_map.size=lib3ds_intd_read(f);
                material->autorefl_map.frame_step=lib3ds_intd_read(f);
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
color_write(Lib3dsRgba rgb, FILE *f)
{
    Lib3dsChunk c;

    c.chunk=LIB3DS_COLOR_24;
    c.size=9;
    lib3ds_chunk_write(&c,f);
    lib3ds_byte_write((Lib3dsByte)floor(255.0*rgb[0]+0.5),f);
    lib3ds_byte_write((Lib3dsByte)floor(255.0*rgb[1]+0.5),f);
    lib3ds_byte_write((Lib3dsByte)floor(255.0*rgb[2]+0.5),f);

    c.chunk=LIB3DS_LIN_COLOR_24;
    c.size=9;
    lib3ds_chunk_write(&c,f);
    lib3ds_byte_write((Lib3dsByte)floor(255.0*rgb[0]+0.5),f);
    lib3ds_byte_write((Lib3dsByte)floor(255.0*rgb[1]+0.5),f);
    lib3ds_byte_write((Lib3dsByte)floor(255.0*rgb[2]+0.5),f);

    return(LIB3DS_TRUE);
}


static Lib3dsBool
int_percentage_write(Lib3dsFloat p, FILE *f)
{
    Lib3dsChunk c;

    c.chunk=LIB3DS_INT_PERCENTAGE;
    c.size=8;
    lib3ds_chunk_write(&c,f);
    lib3ds_intw_write((Lib3dsByte)floor(100.0*p+0.5),f);

    return(LIB3DS_TRUE);
}


static Lib3dsBool
texture_map_write(Lib3dsWord chunk, Lib3dsTextureMap *map, FILE *f)
{
    Lib3dsChunk c;

    if (strlen(map->name)==0)
    {
        return(LIB3DS_TRUE);
    }
    c.chunk=chunk;
    if (!lib3ds_chunk_write_start(&c,f))
    {
        return(LIB3DS_FALSE);
    }

    int_percentage_write(map->percent,f);

    {                            /*---- LIB3DS_MAT_MAPNAME ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_MAPNAME;
        c.size=6+strlen(map->name)+1;
        lib3ds_chunk_write(&c,f);
        lib3ds_string_write(map->name,f);
    }

    {                            /*---- LIB3DS_MAT_MAP_TILING ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_MAP_TILING;
        c.size=8;
        lib3ds_chunk_write(&c,f);
        lib3ds_word_write((Lib3dsWord)map->flags,f);
    }

    {                            /*---- LIB3DS_MAT_MAP_TEXBLUR ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_MAP_TEXBLUR;
        c.size=10;
        lib3ds_chunk_write(&c,f);
        lib3ds_float_write(map->blur,f);
    }

    {                            /*---- LIB3DS_MAT_MAP_USCALE ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_MAP_USCALE;
        c.size=10;
        lib3ds_chunk_write(&c,f);
        lib3ds_float_write(map->scale[0],f);
    }

    {                            /*---- LIB3DS_MAT_MAP_VSCALE ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_MAP_VSCALE;
        c.size=10;
        lib3ds_chunk_write(&c,f);
        lib3ds_float_write(map->scale[1],f);
    }

    {                            /*---- LIB3DS_MAT_MAP_UOFFSET ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_MAP_UOFFSET;
        c.size=10;
        lib3ds_chunk_write(&c,f);
        lib3ds_float_write(map->offset[0],f);
    }

    {                            /*---- LIB3DS_MAT_MAP_VOFFSET ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_MAP_VOFFSET;
        c.size=10;
        lib3ds_chunk_write(&c,f);
        lib3ds_float_write(map->offset[1],f);
    }

    {                            /*---- LIB3DS_MAT_MAP_ANG ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_MAP_ANG;
        c.size=10;
        lib3ds_chunk_write(&c,f);
        lib3ds_float_write(map->rotation,f);
    }

    {                            /*---- LIB3DS_MAT_MAP_COL1 ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_MAP_COL1;
        c.size=9;
        lib3ds_chunk_write(&c,f);
        lib3ds_byte_write((Lib3dsByte)floor(255.0*map->tint_1[0]+0.5), f);
        lib3ds_byte_write((Lib3dsByte)floor(255.0*map->tint_1[1]+0.5), f);
        lib3ds_byte_write((Lib3dsByte)floor(255.0*map->tint_1[2]+0.5), f);
    }

    {                            /*---- LIB3DS_MAT_MAP_COL2 ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_MAP_COL2;
        c.size=9;
        lib3ds_chunk_write(&c,f);
        lib3ds_byte_write((Lib3dsByte)floor(255.0*map->tint_2[0]+0.5), f);
        lib3ds_byte_write((Lib3dsByte)floor(255.0*map->tint_2[1]+0.5), f);
        lib3ds_byte_write((Lib3dsByte)floor(255.0*map->tint_2[2]+0.5), f);
    }

    {                            /*---- LIB3DS_MAT_MAP_RCOL ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_MAP_RCOL;
        c.size=9;
        lib3ds_chunk_write(&c,f);
        lib3ds_byte_write((Lib3dsByte)floor(255.0*map->tint_r[0]+0.5), f);
        lib3ds_byte_write((Lib3dsByte)floor(255.0*map->tint_r[1]+0.5), f);
        lib3ds_byte_write((Lib3dsByte)floor(255.0*map->tint_r[2]+0.5), f);
    }

    {                            /*---- LIB3DS_MAT_MAP_GCOL ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_MAP_GCOL;
        c.size=9;
        lib3ds_chunk_write(&c,f);
        lib3ds_byte_write((Lib3dsByte)floor(255.0*map->tint_g[0]+0.5), f);
        lib3ds_byte_write((Lib3dsByte)floor(255.0*map->tint_g[1]+0.5), f);
        lib3ds_byte_write((Lib3dsByte)floor(255.0*map->tint_g[2]+0.5), f);
    }

    {                            /*---- LIB3DS_MAT_MAP_BCOL ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_MAP_BCOL;
        c.size=9;
        lib3ds_chunk_write(&c,f);
        lib3ds_byte_write((Lib3dsByte)floor(255.0*map->tint_b[0]+0.5), f);
        lib3ds_byte_write((Lib3dsByte)floor(255.0*map->tint_b[1]+0.5), f);
        lib3ds_byte_write((Lib3dsByte)floor(255.0*map->tint_b[2]+0.5), f);
    }

    if (!lib3ds_chunk_write_end(&c,f))
    {
        return(LIB3DS_FALSE);
    }
    return(LIB3DS_TRUE);
}


/*!
 * \ingroup material
 */
Lib3dsBool
lib3ds_material_write(Lib3dsMaterial *material, FILE *f)
{
    Lib3dsChunk c;

    c.chunk=LIB3DS_MAT_ENTRY;
    if (!lib3ds_chunk_write_start(&c,f))
    {
        return(LIB3DS_FALSE);
    }

    {                            /*---- LIB3DS_MAT_NAME ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_NAME;
        c.size=6+strlen(material->name)+1;
        lib3ds_chunk_write(&c,f);
        lib3ds_string_write(material->name,f);
    }

    {                            /*---- LIB3DS_MAT_AMBIENT ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_AMBIENT;
        c.size=24;
        lib3ds_chunk_write(&c,f);
        color_write(material->ambient,f);
    }

    {                            /*---- LIB3DS_MAT_DIFFUSE ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_DIFFUSE;
        c.size=24;
        lib3ds_chunk_write(&c,f);
        color_write(material->diffuse,f);
    }

    {                            /*---- LIB3DS_MAT_SPECULAR ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_SPECULAR;
        c.size=24;
        lib3ds_chunk_write(&c,f);
        color_write(material->specular,f);
    }

    {                            /*---- LIB3DS_MAT_SHININESS ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_SHININESS;
        c.size=14;
        lib3ds_chunk_write(&c,f);
        int_percentage_write(material->shininess,f);
    }

    {                            /*---- LIB3DS_MAT_SHIN2PCT ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_SHIN2PCT;
        c.size=14;
        lib3ds_chunk_write(&c,f);
        int_percentage_write(material->shin_strength,f);
    }

    {                            /*---- LIB3DS_MAT_TRANSPARENCY ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_TRANSPARENCY;
        c.size=14;
        lib3ds_chunk_write(&c,f);
        int_percentage_write(material->transparency,f);
    }

    {                            /*---- LIB3DS_MAT_XPFALL ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_XPFALL;
        c.size=14;
        lib3ds_chunk_write(&c,f);
        int_percentage_write(material->falloff,f);
    }

    if (material->use_falloff)   /*---- LIB3DS_MAT_USE_XPFALL ----*/
    {
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_USE_XPFALL;
        c.size=6;
        lib3ds_chunk_write(&c,f);
    }

    {                            /*---- LIB3DS_MAT_SHADING ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_SHADING;
        c.size=8;
        lib3ds_chunk_write(&c,f);
        lib3ds_intw_write(material->shading,f);
    }

    {                            /*---- LIB3DS_MAT_REFBLUR ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_REFBLUR;
        c.size=14;
        lib3ds_chunk_write(&c,f);
        int_percentage_write(material->blur,f);
    }

    if (material->use_blur)      /*---- LIB3DS_MAT_USE_REFBLUR ----*/
    {
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_USE_REFBLUR;
        c.size=6;
        lib3ds_chunk_write(&c,f);
    }

    if (material->self_illum)    /*---- LIB3DS_MAT_SELF_ILLUM ----*/
    {
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_SELF_ILLUM;
        c.size=6;
        lib3ds_chunk_write(&c,f);
    }

    if (material->two_sided)     /*---- LIB3DS_MAT_TWO_SIDE ----*/
    {
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_TWO_SIDE;
        c.size=6;
        lib3ds_chunk_write(&c,f);
    }

    if (material->map_decal)     /*---- LIB3DS_MAT_DECAL ----*/
    {
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_DECAL;
        c.size=6;
        lib3ds_chunk_write(&c,f);
    }

    if (material->additive)      /*---- LIB3DS_MAT_ADDITIVE ----*/
    {
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_ADDITIVE;
        c.size=6;
        lib3ds_chunk_write(&c,f);
    }

    if (material->use_wire)      /*---- LIB3DS_MAT_WIRE ----*/
    {
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_WIRE;
        c.size=6;
        lib3ds_chunk_write(&c,f);
    }

    if (material->use_wire_abs)  /*---- LIB3DS_MAT_WIREABS ----*/
    {
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_WIREABS;
        c.size=6;
        lib3ds_chunk_write(&c,f);
    }

    {                            /*---- LIB3DS_MAT_WIRE_SIZE ----*/
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_WIRE_SIZE;
        c.size=10;
        lib3ds_chunk_write(&c,f);
        lib3ds_float_write(material->wire_size,f);
    }

    if (material->face_map)      /*---- LIB3DS_MAT_FACEMAP ----*/
    {
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_FACEMAP;
        c.size=6;
        lib3ds_chunk_write(&c,f);
    }

    if (material->soften)        /*---- LIB3DS_MAT_PHONGSOFT ----*/
    {
        Lib3dsChunk c;
        c.chunk=LIB3DS_MAT_PHONGSOFT;
        c.size=6;
        lib3ds_chunk_write(&c,f);
    }

    if (!texture_map_write(LIB3DS_MAT_TEXMAP, &material->texture1_map, f))
    {
        return(LIB3DS_FALSE);
    }
    if (!texture_map_write(LIB3DS_MAT_TEXMASK, &material->texture1_mask, f))
    {
        return(LIB3DS_FALSE);
    }
    if (!texture_map_write(LIB3DS_MAT_TEX2MAP, &material->texture2_map, f))
    {
        return(LIB3DS_FALSE);
    }
    if (!texture_map_write(LIB3DS_MAT_TEX2MASK, &material->texture2_mask, f))
    {
        return(LIB3DS_FALSE);
    }
    if (!texture_map_write(LIB3DS_MAT_OPACMAP, &material->opacity_map, f))
    {
        return(LIB3DS_FALSE);
    }
    if (!texture_map_write(LIB3DS_MAT_OPACMASK, &material->opacity_mask, f))
    {
        return(LIB3DS_FALSE);
    }
    if (!texture_map_write(LIB3DS_MAT_BUMPMAP, &material->bump_map, f))
    {
        return(LIB3DS_FALSE);
    }
    if (!texture_map_write(LIB3DS_MAT_BUMPMASK, &material->bump_mask, f))
    {
        return(LIB3DS_FALSE);
    }
    if (!texture_map_write(LIB3DS_MAT_SPECMAP, &material->specular_map, f))
    {
        return(LIB3DS_FALSE);
    }
    if (!texture_map_write(LIB3DS_MAT_SPECMASK, &material->specular_mask, f))
    {
        return(LIB3DS_FALSE);
    }
    if (!texture_map_write(LIB3DS_MAT_SHINMAP, &material->shininess_map, f))
    {
        return(LIB3DS_FALSE);
    }
    if (!texture_map_write(LIB3DS_MAT_SHINMASK, &material->shininess_mask, f))
    {
        return(LIB3DS_FALSE);
    }
    if (!texture_map_write(LIB3DS_MAT_SELFIMAP, &material->self_illum_map, f))
    {
        return(LIB3DS_FALSE);
    }
    if (!texture_map_write(LIB3DS_MAT_SELFIMASK, &material->self_illum_mask, f))
    {
        return(LIB3DS_FALSE);
    }
    if (!texture_map_write(LIB3DS_MAT_REFLMAP,  &material->reflection_map, f))
    {
        return(LIB3DS_FALSE);
    }
    if (!texture_map_write(LIB3DS_MAT_REFLMASK,  &material->reflection_mask, f))
    {
        return(LIB3DS_FALSE);
    }

    if (!lib3ds_chunk_write_end(&c,f))
    {
        return(LIB3DS_FALSE);
    }
    return(LIB3DS_TRUE);
}


/*!

\typedef Lib3dsMaterial
  \ingroup material
  \sa _Lib3dsMaterial

*/
