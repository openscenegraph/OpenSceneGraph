/*
 * Copyright (C) 2000-2003 the xine project and Claudio "KLaN" Ciccani
 *
 * This file is part of xine, a free video player.
 *
 * xine is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 *
 * video_out_rgb.h, data types definitions for video_out_rgb.c
 *			by Claudio "KLaN" Ciccani <klan82@cheapnet.it>
 *
 *
 */

#ifndef VIDEO_OUT_RGB_H
#define VIDEO_OUT_RGB_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
	uint8_t *r;
	uint8_t *g;
	uint8_t *b;

} rgb_planar_t;



typedef enum
{
/*	PX_RGB_PLANAR = 1, ? */
	PX_ARGB      = 2,  /* 32 bits [a:8@24, r:8@26, g:8@8, b:8@0] */
	PX_ARGB1555  = 3,  /* 16 bits [a:1@15, r:5@10, g:5@5, b:5@0] */
	PX_RGB32     = 4,  /* 32 bits [r:8@16, g:8@8, b:8@0] */
	PX_RGB24     = 5,  /* 24 bits [r:8@16, g:8@8, b:8@0] */
	PX_RGB16     = 6,  /* 16 bits [r:5@11, g:6@5, b:5@0] */
	PX_BGRA      = 7,  /* 32 bits [a:8@0, r:8@8, g:8@16, b:8@24] */
	PX_BGRA5551  = 8,  /* 16 bits [a:1@0, r:5@1, g:5@6, b:5@11] */
	PX_BGR32     = 9,  /* 32 bits [r:8@0, g:8@8, b:8@16] */
	PX_BGR24     = 10, /* 24 bits [r:8@0, g:8@8, b:8@16] */
	PX_BGR16     = 11  /* 16 bits [r:5@0, g:6@5, b:5@11] */

} rgb_pixel_format_t;


typedef enum
{
	PXLEVEL_NONE = 0,
	PXLEVEL_R    = (1 << 0),
	PXLEVEL_G    = (1 << 1),
	PXLEVEL_B    = (1 << 2),
	PXLEVEL_ALL  = 7 /* PX_LEVEL_R | PX_LEVEL_G | PX_LEVEL_B */

} rgb_pixel_levels_t;



/*
 * Applications that want to use this driver must provide a
 * callabck function (eg. for rendering frames);
 * RGBout will pass it a buffer containing pixels in the format
 * specified by "format" (generally you have only to BLIT
 * the buffer if you want to display the frame).
 * "levels" selects which RGB level is visible (if you dont't
 * need this feature, set it to PXLEVEL_ALL).
 *
 * N.B.: DO NOT FREE THE BUFFER
 *
 */

typedef struct
{
	rgb_pixel_format_t format;
	rgb_pixel_levels_t levels;
        void* user_data;
	void (*callback) (uint32_t width, uint32_t height, void* imageData, void* userData);

} rgbout_visual_info_t;



typedef struct
{
	vo_frame_t vo_frame;

	uint32_t width;
	uint32_t height;
	uint32_t pixels;
	uint32_t format;
	double ratio;

	void* chunk[3];

} rgbout_frame_t;



typedef struct rgb_driver_s
{
	vo_driver_t vo_driver;

	config_values_t* config;

	rgbout_frame_t* frame;

	uint32_t frame_width;
	uint32_t frame_height;
	uint32_t lastframe_width;
	uint32_t lastframe_height;

	rgb_planar_t buffer;
	void* outbuffer;

	uint32_t accel;
	uint8_t cm, pm; /* conversion method, packing method */

	uint8_t levels; /* RGB levels mask */
        void* user_data;

	/* private functions */
	void  (*convert) (uint8_t* yuv[], rgb_planar_t* rgb,
				 uint32_t pitches[], uint32_t width, uint32_t height);
	void  (*pack)   (rgb_planar_t* rgb, void* dest, uint32_t pixels, uint32_t accel);
	void  (*render) (uint32_t width, uint32_t height, void* data, void* userData);

	/* public callback */
	int    (*update_visual) (vo_driver_t* vo_driver,
					 rgbout_visual_info_t* new_visual);

} rgbout_driver_t;



typedef struct
{
	video_driver_class_t driver_class;

} rgbout_class_t;



typedef struct
{
	const char* name;
	uint32_t required_accel;

	void  (*convert_yuy2) (uint8_t* yuv[], rgb_planar_t* rgb,
				 uint32_t pitches[], uint32_t width, uint32_t height);
	void  (*convert_yv12) (uint8_t* yuv[], rgb_planar_t* rgb,
				 uint32_t pitches[], uint32_t width, uint32_t height);

} rgbout_converter_t;


typedef struct
{
	const char* name;
	rgb_pixel_format_t id;
	uint8_t pixelsize;
	uint8_t scratch;

	void  (*pack) (rgb_planar_t* rgb, void* dest, uint32_t pixels, uint32_t accel);

} rgbout_packer_t;

void register_rgbout_plugin(xine_t *self);

#define XINE_VISUAL_TYPE_RGBOUT 100

#ifdef __cplusplus
}
#endif

#endif /* VIDEO_OUT_RGB_H */

