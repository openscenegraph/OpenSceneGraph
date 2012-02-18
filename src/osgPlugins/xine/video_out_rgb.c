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
 * video_out_rgb.c, general purpose rgb video output xine driver
 *			by Claudio "KLaN" Ciccani <klan82@cheapnet.it>
 *
 * ...someway based on video_ou_xshm.c
 *
 *
 * NOTE: this driver is not able to display videos itself;
 *       however it can be used in every graphics environment
 *       and not only for playback (either video processing,
 *       frame dumping ... what you want)
 *
 *
 *  TODO: a good scaling function, hue, saturation,
 *        contrast, applying effects and many more
 *        (eg. SSE2 support, if someone buy me a new procesor).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#include <fcntl.h>
#include <unistd.h>

#include "xine.h"
#include "xine/xine_internal.h"
#include "xine/xineutils.h"
#include "xine/video_out.h"
#include "video_out_rgb.h"


#define THIS  "video_out_rgb"

static int s_debugMessages = 0; 

#define EVAL(exp) \
{\
	if(!(exp))\
	{\
		fprintf(stderr, THIS ": <\"" #exp "\"> evaluation failed !!\n");\
		fflush(stderr);\
		goto FAILURE;\
	}\
}

#define CHECK(exp) \
{\
	if(!(exp))\
	{\
		fprintf(stderr, THIS ": <\"" #exp "\"> check failed !!\n");\
		fflush(stderr);\
	}\
}

#define release(ptr) \
{\
	if(ptr) free(ptr);\
	ptr = NULL;\
}

#ifdef __cplusplus
extern "C" {
#endif

static inline void
clear(void* dest, uint32_t size)
{
#ifdef COMPILE_ASSEMBLY
	__asm__ __volatile__(

		"xorl %%eax, %%eax\n\t"
		"movl %1, %%ecx\n\t"
		"rep; stosl\n\t"
		"movl %2, %%ecx\n\t"
		"rep; stosb\n\t"
		
		: "=&D" (dest)
		: "r" (size >> 2), "r" (size & 3), "0" (dest)
		: "eax", "ecx");
#else
	memset(dest, 0, size);
#endif
}

#ifdef DEBUG

static inline uint64_t rdtsc()
{
	uint64_t t;
	
	__asm__ __volatile__(
		".byte 0x0f, 0x31\n\t"
		: "=A" (t));
	
	return(t);
}

#endif /* DEBUG */



/*
 * I'm using this formula for colorspace conversion:
 *
 *	R = Y + (1.420705 * U)
 *	G = Y - (0.698001 * U) - (0.337633 * V)
 *	B = Y + (1.732446 * V)
 *
 * First YUV chunks are converted and put into a
 * rgb_planar_t structure, then packed into the
 * specified RGB pixel format.
 * I'm using this method because I need RGB levels to
 * be separated for effects stuff (currently only
 * selecting levels).
 *
 */


/* floating point factors; used by 3DNow and SSE */
static const float fp_factors[] =
{
        1.420705, 1.420705,
        0.698001, 0.698001,
        0.337633, 0.337633,
        1.732446, 1.732446
};


/* used by MMX;
 * each factor is multiplied by 2^14 and rounded
 */
static const int16_t wd_factors[] =
{
	0x4000,  23277, 0x4000,  23277,
	0x4000,  28384, 0x4000,  28384,
	 11436,   5532,  11436,   5532
};


/* used by normal C function;
 * each factor is multiplied by 2^16 and rounded
 */
static const int32_t dw_factors[] =
{
	 93107,
	 45744,
	 22127,
	113538,
};


/* used by 3DNow for rounding values */
static const float round[] =
{
	0.555555,
	0.555555
};

static const uint32_t lsub[] =
{
	128, 128
};

static const uint32_t lmask[] =
{
	0xff, 0xff
};

static const uint16_t wmask[] =
{
	0xff, 0xff,
	0xff, 0xff
};



#ifdef COMPILE_ASSEMBLY

static void
__3dnow_convert_yuy2(uint8_t* yuv[], rgb_planar_t* rgb,
			 uint32_t pitches[], uint32_t width, uint32_t height)
{
	uint8_t* yuv_data = yuv[0];
	uint8_t* r_buffer = rgb->r;
	uint8_t* g_buffer = rgb->g;
	uint8_t* b_buffer = rgb->b;
	uint32_t samples = (width * height) >> 2; /* 4 pixels at once */


	__asm__ __volatile__("femms\n\t");

	while(samples--)
	{
		__asm__ __volatile__(

			"prefetch 192(%0)\n\t"

			"movq (%0), %%mm1\n\t" /* mm1 = [v2 y3 u2 y2 v0 y1 u0 y0] */

			"movq %%mm1, %%mm3\n\t" /* mm3 = [v2 y3 u2 y2 v0 y1 u0 y0] */
			"pand (%1), %%mm3\n\t" /*  mm3 = [0 y3 0 y2 0 y1 0 y0] */

			"movq %%mm1, %%mm2\n\t" /* mm2 = [v2 y3 u2 y2 v0 y1 u0 y0] */
			"psrld $8, %%mm2\n\t" /* mm2 = [0 v2 y3 u2 0 v0 y1 u0] */
			"pand (%2), %%mm2\n\t" /* mm2 = [0 0 0 u2 0 0 0 u0] */
			"psubd (%3), %%mm2\n\t" /* mm2 = [u2 - 128 | u2 - 128] */
			"pi2fd %%mm2, %%mm2\n\t"

			"psrld $24, %%mm1\n\t" /* mm1 = [0 0 0 v2 0 0 0 v0] */
			"psubd (%3), %%mm1\n\t" /* mm1 = [v2 - 128 | v0 - 128] */
			"pi2fd %%mm1, %%mm1\n\t"

			"addl $8, %0\n\t"

			: "=&r" (yuv_data)
			: "r" (wmask), "r" (lmask), "r" (lsub), "0" (yuv_data)
			: "memory");


		__asm__ __volatile__(

			"prefetchw 320(%0)\n\t"

			"movq %%mm1, %%mm0\n\t" /* mm0 = [v2 | v0] */
			"pfmul (%3), %%mm0\n\t" /* mm0 = [v2 * 1.420705 | v0 * 1.420705] */
			"pfadd (%4), %%mm0\n\t" /* rounded */
			"pf2id %%mm0, %%mm0\n\t"
			"packssdw %%mm0, %%mm0\n\t" /* mm0 = [v2 * 1.420705 | v0 * 1.420705 | ...] */
			"punpckhwd %%mm0, %%mm0\n\t" /* mm0 = [v2 * 1.420705 | v2 * 1.420705 |
							 	v0 * 1.420705 | v0 * 1.420705] */
			"paddw %%mm3, %%mm0\n\t" /* mm0 = [0 r3 0 r2 0 r1 0 r0] */
			"packuswb %%mm0, %%mm0\n\t" /* mm0 = [r3 r2 r1 r0 r3 r2 r1 r0] */
			"movd %%mm0, (%0)\n\t"

			"prefetchw 320(%2)\n\t"

			"movq %%mm2, %%mm0\n\t" /* mm0 = [u2 | u0] */
			"pfmul 24(%3), %%mm0\n\t" /* mm0 = [u2 * 1.732446 | u0 * 1.732446] */
			"pfadd (%4), %%mm0\n\t" /* rounded */
			"pf2id %%mm0, %%mm0\n\t"
			"packssdw %%mm0, %%mm0\n\t" /* mm0 = [u2 * 1.732446 | u0 * 1.732446 | ...] */
			"punpckhwd %%mm0, %%mm0\n\t" /* mm0 = [u2 * 1.732446 | u2 * 1.732446 |
								u0 * 1.732446 | u0 * 1.732446] */
			"paddw %%mm3, %%mm0\n\t" /* mm0 = [0 b3 0 b2 0 b1 0 b0] */
			"packuswb %%mm0, %%mm0\n\t" /* mm0 = [b3 b2 b1 b0 b3 b2 b1 b0] */
			"movd %%mm0, (%2)\n\t"

			"prefetchw 320(%1)\n\t"

			"pfmul 8(%3), %%mm1\n\t" /* mm1 = [v2 * 0.698001 | v0 * 0.698001] */
			"pfmul 16(%3), %%mm2\n\t" /* mm2 = [u2 * 0.337633 | u0 * 0.337633] */
			"pfadd %%mm1, %%mm2\n\t" /* mm2 = [(v2 * 0.698001) + (u2 * 0.337633) | ...] */
			"pfadd (%4), %%mm2\n\t" /* rounded */
			"pf2id %%mm2, %%mm2\n\t"
			"packssdw %%mm2, %%mm2\n\t" /* mm2 = [(v2 * 0.698001) + (u2 * 0.337633) |
								(v0 * 0.698001) + (u0 * 0.337633)| ...] */
			"punpckhwd %%mm2, %%mm2\n\t" /* mm2 = [(v2 * 0.698001) + (u2 * 0.337633) |
								(v2 * 0.698001) + (u2 * 0.337633)| ...] */
			"psubw %%mm2, %%mm3\n\t" /* mm3 = [0 g3 0 g2 0 g1 0 g0] */
			"packuswb %%mm3, %%mm3\n\t" /* mm3 = [g3 g2 g1 g0 g3 g2 g1 g0] */
			"movd %%mm3, (%1)\n\t"

			"addl $4, %0\n\t"
			"addl $4, %1\n\t"
			"addl $4, %2\n\t"

			: "=&r" (r_buffer), "=&r" (g_buffer), "=&r" (b_buffer)
			: "r" (fp_factors), "r" (round),
			  "0" (r_buffer), "1" (g_buffer), "2" (b_buffer)
			: "memory");

	}

	__asm__ __volatile__("femms\n\t");
}


static void
__3dnow_convert_yv12(uint8_t* yuv[], rgb_planar_t* rgb,
			 uint32_t pitches[], uint32_t width, uint32_t height)
{
	uint8_t* y_data = yuv[0];
	uint8_t* u_data = yuv[1];
	uint8_t* v_data = yuv[2];
	uint8_t* r_buffer = rgb->r;
	uint8_t* g_buffer = rgb->g;
	uint8_t* b_buffer = rgb->b;
	uint32_t line_size = width >> 2; /* 4 pixels at once */
	uint32_t samples   = line_size * (height >> 1);


	__asm__ __volatile__(
		"femms\n\t"
		"pxor %%mm7, %%mm7\n\t"
		::: "memory");

	while(samples--)
	{
		__asm__ __volatile__(

			"prefetch 192(%0)\n\t"
			"prefetch 192(%1)\n\t"
			"prefetch 192(%2)\n\t"
			"prefetch 192(%3)\n\t"

			"movd (%0), %%mm6\n\t" /* mm6 = [0 0 0 0 y03 y02 y01 y00] */
			"movd (%1), %%mm5\n\t" /* mm5 = [0 0 0 0 y13 y12 y11 y10] */
			"movd (%2), %%mm1\n\t" /* mm1 = [0 0 0 0 u12 u8 u4 u0] */
			"movd (%3), %%mm0\n\t" /* mm0 = [0 0 0 0 v12 v8 v4 v0] */

			"punpcklbw %%mm7, %%mm6\n\t" /* mm6 = [0 y03 0 y02 0 y01 0 y00] */
			"punpcklbw %%mm7, %%mm5\n\t" /* mm5 = [0 y13 0 y12 0 y11 0 y10] */

			"punpcklbw %%mm7, %%mm1\n\t" /* mm1 = [0 u12 0 u8 0 u4 0 u0] */
			"punpcklwd %%mm7, %%mm1\n\t" /* mm1 = [0 0 0 u4 0 0 0 u0] */
			"psubd (%4), %%mm1\n\t" /* mm1 = [u4 - 128 | u0 - 128] */
			"pi2fd %%mm1, %%mm1\n\t"

			"punpcklbw %%mm7, %%mm0\n\t" /* mm0 = [0 v12 0 v8 0 v4 0 v0] */
			"punpcklwd %%mm7, %%mm0\n\t" /* mm0 = [0 0 0 v4 0 0 0 v0] */
			"psubd (%4), %%mm0\n\t" /* mm0 = [v4 - 128 | v0 - 128] */
			"pi2fd %%mm0, %%mm0\n\t"

			:: "r" (y_data), "r" (y_data + pitches[0]),
			   "r" (u_data), "r" (v_data), "r" (lsub)
			: "memory");


		__asm__ __volatile__(

			"movq %%mm0, %%mm3\n\t" /* mm3 = [v4 | v0] */
			"pfmul (%0), %%mm3\n\t" /* mm3 = [v4 * 1.420705 | v0 * 1.420705] */
			"pfadd (%1), %%mm3\n\t" /* rounded */
			"pf2id %%mm3, %%mm3\n\t"
			"packssdw %%mm3, %%mm3\n\t" /* mm3 = [v4 * 1.420705 | v0 * 1.420705 | ...] */
			"punpckhwd %%mm3, %%mm3\n\t" /* mm3 = [v4 * 1.420705 | v4 * 1.420705 |
							 	v0 * 1.420705 | v0 * 1.420705] */

			"movq %%mm1, %%mm2\n\t" /* mm2 = [u4 | u0] */
			"pfmul 24(%0), %%mm2\n\t" /* mm2 = [u4 * 1.732446 | u0 * 1.732446] */
			"pfadd (%1), %%mm2\n\t" /* rounded */
			"pf2id %%mm2, %%mm2\n\t"
			"packssdw %%mm2, %%mm2\n\t" /* mm2 = [u4 * 1.732446 | u0 * 1.732446 | ...] */
			"punpckhwd %%mm2, %%mm2\n\t" /* mm2 = [u4 * 1.732446 | u4 * 1.732446 |
								u0 * 1.732446 | u0 * 1.732446] */

			"pfmul 8(%0), %%mm0\n\t" /* mm0 = [v4 * 0.698001 | v0 * 0.698001] */
			"pfmul 16(%0), %%mm1\n\t" /* mm1 = [u4 * 0.337633 | u0 * 0.337633] */
			"pfadd %%mm0, %%mm1\n\t" /* mm1 = [(v4 * 0.698001) + (u4 * 0.337633) | ...] */
			"pfadd (%1), %%mm1\n\t" /* rounded */
			"pf2id %%mm1, %%mm1\n\t"
			"packssdw %%mm1, %%mm1\n\t" /* mm1 = [(v4 * 0.698001) + (u4 * 0.337633) |
								(v0 * 0.698001) + (u0 * 0.337633)| ...] */
			"punpckhwd %%mm1, %%mm1\n\t" /* mm1 = [(v4 * 0.698001) + (u4 * 0.337633) |
								(v4 * 0.698001) + (u4 * 0.337633)| ...] */

			:: "r" (fp_factors), "r" (round)
			: "memory");


		__asm__ __volatile__(

			"prefetchw 320(%0)\n\t"

			"movq %%mm6, %%mm0\n\t"
			"paddw %%mm3, %%mm0\n\t" /* mm0 = [0 r03 0 r02 0 r01 0 r00] */
			"packuswb %%mm0, %%mm0\n\t" /* mm0 = [r03 r02 r01 r00 r03 r02 r01 r00] */
			"movd %%mm0, (%0)\n\t"
			"paddw %%mm5, %%mm3\n\t" /* mm3 = [0 r13 0 r12 0 r11 0 r10] */
			"packuswb %%mm3, %%mm3\n\t" /* mm3 = [r13 r12 r11 r10 r13 r12 r11 r10] */
			"movd %%mm3, (%1)\n\t"

			"addl $4, %0\n\t"

			: "=&r" (r_buffer)
			: "r" (r_buffer + width), "0" (r_buffer)
			: "memory");


		__asm__ __volatile__(

			"prefetchw 320(%0)\n\t"

			"movq %%mm6, %%mm0\n\t"
			"paddw %%mm2, %%mm0\n\t" /* mm0 = [0 b03 0 b02 0 b01 0 b00] */
			"packuswb %%mm0, %%mm0\n\t" /* mm0 = [b03 b02 b01 b00 b03 b02 b01 b00] */
			"movd %%mm0, (%0)\n\t"
			"paddw %%mm5, %%mm2\n\t" /* mm2 = [0 b13 0 b12 0 b11 0 b10] */
			"packuswb %%mm2, %%mm2\n\t" /* mm2 = [b13 b12 b11 b10 b13 b12 b11 b10] */
			"movd %%mm2, (%1)\n\t"

			"addl $4, %0\n\t"

			: "=&r" (b_buffer)
			: "r" (b_buffer + width), "0" (b_buffer)
			: "memory");


		__asm__ __volatile__(

			"prefetchw 320(%0)\n\t"

			"psubw %%mm1, %%mm6\n\t" /* mm6 = [0 g03 0 g02 0 g01 0 g00] */
			"packuswb %%mm6, %%mm6\n\t" /* mm6 = [g03 g02 g01 g00 g03 g02 g01 g00] */
			"movd %%mm6, (%0)\n\t"
			"psubw %%mm1, %%mm5\n\t" /* mm5 = [0 g13 0 g12 0 g11 0 g10] */
			"packuswb %%mm5, %%mm5\n\t" /* mm5 = [g13 g12 g11 g10 g13 g12 g11 g10] */
			"movd %%mm5, (%1)\n\t"

			"addl $4, %0\n\t"

			: "=&r" (g_buffer)
			: "r" (g_buffer + width), "0" (g_buffer)
			: "memory");


		y_data += 4;
		u_data += 2;
		v_data += 2;

		if(!(--line_size))
		{
			line_size = width >> 2;
			y_data   += pitches[0];
			r_buffer += width;
			g_buffer += width;
			b_buffer += width;
		}
	}

	__asm__ __volatile__("femms\n\t");
}



/* packed floats operations are slower with SSE;
 * therefore I process only 2 pixels at once in both functions
 */

static void
__sse_convert_yuy2(uint8_t* yuv[], rgb_planar_t* rgb,
			uint32_t pitches[], uint32_t width, uint32_t height)
{
	uint8_t* yuv_data = yuv[0];
	uint8_t* r_buffer = rgb->r;
	uint8_t* g_buffer = rgb->g;
	uint8_t* b_buffer = rgb->b;
	uint32_t samples = (width * height) >> 1;



	__asm__ __volatile__(
		"pxor %%mm7, %%mm7\n\t"
		"movss   (%0), %%xmm7\n\t"
		"movss  8(%0), %%xmm6\n\t"
		"movss 16(%0), %%xmm5\n\t"
		"movss 24(%0), %%xmm4\n\t"
		:: "r" (fp_factors)
		: "memory");

	while(samples--)
	{
		__asm__ __volatile__(

			"prefetchnta 192(%0)\n\t"

			"movd (%0), %%mm3\n\t" /* mm3 = [0 0 0 0 v0 y1 u0 y0] */
			"punpcklwd %%mm7, %%mm3\n\t" /* mm3 = [0 0 v0 y1 0 0 u0 y0] */
			"pand (%1), %%mm3\n\t" /* mm3 = [0 0 0 y1 0 0 0 y0] */

			"movzbl 1(%0), %%eax\n\t" /* eax = u0 */
			"subl $128, %%eax\n\t" /* eax = u0 - 128 */
			"cvtsi2ss %%eax, %%xmm2\n\t" /* xmm2 = [0 | 0 | 0 | u0 - 128] */

			"movzbl 3(%0), %%eax\n\t" /* eax = v0 */
			"subl $128, %%eax\n\t" /* eax = v0 - 128 */
			"cvtsi2ss %%eax, %%xmm1\n\t" /* xmm1 = [0 | 0 | 0 | v0 - 128] */

			"addl $4, %0\n\t"

			: "=&r" (yuv_data)
			: "r" (lmask), "0" (yuv_data)
			: "eax");


		__asm__ __volatile__(

			"prefetchnta 320(%0)\n\t"

			"movss %%xmm1, %%xmm0\n\t" /* xmm0 = [0 | 0 | 0 | v0] */
			"mulss %%xmm7, %%xmm0\n\t" /* xmm0 = [0 | 0 | 0 | v0 * 1.420705] */
			"cvtps2pi %%xmm0, %%mm0\n\t" /* mm0 = [0 | v0 * 1.420705] */
			"punpckldq %%mm0, %%mm0\n\t" /* mm0 = [v0 * 1.420705 | v0 * 1.420705] */
			"paddd %%mm3, %%mm0\n\t" /* mm0 = [r1 | r0] */
			"packssdw %%mm0, %%mm0\n\t" /* mm0 = [0 r1 0 r0 0 r1 0 r0] */
			"packuswb %%mm0, %%mm0\n\t" /* mm0 = [r1 r0 r1 r0 r1 r0 r1 r0] */
			"movd %%mm0, (%0)\n\t"

			"prefetchnta 320(%2)\n\t"

			"movss %%xmm2, %%xmm0\n\t" /* xmm0 = [0 | 0 | 0 | u0] */
			"mulss %%xmm4, %%xmm0\n\t" /* xmm0 = [0 | 0 | 0 | u0 * 1.732446] */
			"cvtps2pi %%xmm0, %%mm0\n\t" /* mm0 = [0 | u0 * 1.732446] */
			"punpckldq %%mm0, %%mm0\n\t" /* mm0 = [u0 * 1.732446 | u0 * 1.732446] */
			"paddd %%mm3, %%mm0\n\t" /* mm0 = [b1 | b0] */
			"packssdw %%mm0, %%mm0\n\t" /* mm0 = [0 b1 0 b0 0 b1 0 b0] */
			"packuswb %%mm0, %%mm0\n\t" /* mm0 = [b1 b0 b1 b0 b1 b0 b1 b0] */
			"movd %%mm0, (%2)\n\t"

			"prefetchnta 320(%1)\n\t"

			"mulss %%xmm6, %%xmm1\n\t" /* xmm1 = [0 | 0 | 0 | v0 * 0.698001] */
			"mulss %%xmm5, %%xmm2\n\t" /* xmm2 = [0 | 0 | 0 | u0 * 0.337633] */
			"addss %%xmm2, %%xmm1\n\t" /* xmm1 = [0 | 0 | 0 | (v0 * 0.698001) + (u0 * 0.337633)] */
			"cvtps2pi %%xmm1, %%mm0\n\t" /* mm0 = [0 | (v0 * 0.698001) + (u0 * 0.337633)] */
			"punpckldq %%mm0, %%mm0\n\t" /* mm0 = [(v0 * 0.698001) + (u0 * 0.337633) |
								(v0 * 0.698001) + (u0 * 0.337633)] */
			"psubd %%mm0, %%mm3\n\t" /* mm3 = [g1 | g0] */
			"packssdw %%mm3, %%mm3\n\t" /* mm3 = [0 g1 0 g0 0 g1 0 g0] */
			"packuswb %%mm3, %%mm3\n\t" /* mm3 = [g1 g0 g1 g0 g1 g0 g1 g0] */
			"movd %%mm3, (%1)\n\t"

			"addl $2, %0\n\t"
			"addl $2, %1\n\t"
			"addl $2, %2\n\t"

			: "=&r" (r_buffer), "=&r" (g_buffer), "=&r" (b_buffer)
			: "0" (r_buffer), "1" (g_buffer), "2" (b_buffer)
			: "memory");
	}


	__asm__ __volatile__("emms\n\t");

}


static void
__sse_convert_yv12(uint8_t* yuv[], rgb_planar_t* rgb,
			 uint32_t pitches[], uint32_t width, uint32_t height)
{
	uint8_t* y_data = yuv[0];
	uint8_t* u_data = yuv[1];
	uint8_t* v_data = yuv[2];
	uint8_t* r_buffer = rgb->r;
	uint8_t* g_buffer = rgb->g;
	uint8_t* b_buffer = rgb->b;
	uint32_t line_size = width >> 1;
	uint32_t samples   = line_size * (height >> 1);


	__asm__ __volatile__(
		"pxor %%mm7, %%mm7\n\t"
		"movss   (%0), %%xmm7\n\t"
		"movss  8(%0), %%xmm6\n\t"
		"movss 16(%0), %%xmm5\n\t"
		"movss 24(%0), %%xmm4\n\t"
		:: "r" (fp_factors)
		: "memory");

	while(samples--)
	{
		__asm__ __volatile__(

			"prefetchnta 192(%0)\n\t"
			"prefetchnta 192(%1)\n\t"
			"prefetchnta 192(%2)\n\t"
			"prefetchnta 192(%3)\n\t"

			"movd (%0), %%mm3\n\t" /* mm3 = [0 0 0 0 y03 y02 y01 y00] */
			"punpcklbw %%mm7, %%mm3\n\t" /* mm6 = [0 y03 0 y02 0 y01 0 y00] */
			"movd (%1), %%mm2\n\t" /* mm2 = [0 0 0 0 y13 y12 y11 y10] */
			"punpcklbw %%mm7, %%mm2\n\t" /* mm2 = [0 y13 0 y12 0 y11 0 y10] */
			"punpckldq %%mm2, %%mm3\n\t" /* mm3 = [0 y11 0 y10 0 y01 0 y00] */

			"movzbl (%2), %%eax\n\t" /* eax = u0 */
			"subl $128, %%eax\n\t" /* eax = u0 - 128 */
			"cvtsi2ss %%eax, %%xmm1\n\t" /* xmm1 = [0 | 0 | 0 | u0 - 128] */

			"movzbl (%3), %%eax\n\t" /* eax = v0 */
			"subl $128, %%eax\n\t" /* eax = v0 - 128 */
			"cvtsi2ss %%eax, %%xmm0\n\t" /* xmm0 = [0 | 0 | 0 | v0 - 128] */

			:: "r" (y_data), "r" (y_data + pitches[0]),
			   "r" (u_data), "r" (v_data)
			: "eax");


		__asm__ __volatile__(

			"prefetchnta 320(%0)\n\t"

			"movss %%xmm0, %%xmm3\n\t" /* xmm3 = [0 | 0 | 0 | v0] */
			"mulss %%xmm7, %%xmm3\n\t" /* xmm3 = [0 | 0 | 0 | v0 * 1.420705] */
			"cvtps2pi %%xmm3, %%mm0\n\t" /* mm0 = [0 | v0 * 1.420705] */
			"punpckldq %%mm0, %%mm0\n\t" /* mm0 = [v0 * 1.420705 | v0 * 1.420705] */
			"packssdw %%mm0, %%mm0\n\t" /* mm0 = [v0 * 1.420705 | v0 * 1.420705 | ...] */
			"paddw %%mm3, %%mm0\n\t" /* mm0 = [0 r11 0 r10 0 r01 0 r00] */
			"packuswb %%mm0, %%mm0\n\t" /* mm0 =  [r11 r10 r01 r00 r11 r10 r01 r00] */
			"movd %%mm0, (%0)\n\t"
			"psrld $16, %%mm0\n\t" /* mm0 = [0 0 r11 r10 0 0 r11 r10] */
			"movd %%mm0, (%1)\n\t"

			"addl $2, %0\n\t"

			: "=&r" (r_buffer)
			: "r" (r_buffer + width), "0" (r_buffer)
			: "memory");


		__asm__ __volatile__(

			"prefetchnta 320(%0)\n\t"

			"movss %%xmm1, %%xmm3\n\t" /* xmm3 = [0 | 0 | 0 | u0] */
			"mulss %%xmm4, %%xmm3\n\t" /* xmm3 = [0 | 0 | 0 | u0 * 1.732446] */
			"cvtps2pi %%xmm3, %%mm0\n\t" /* mm0 = [0 | u0 * 1.732446] */
			"punpckldq %%mm0, %%mm0\n\t" /* mm0 = [u0 * 1.732446 | u0 * 1.732446] */
			"packssdw %%mm0, %%mm0\n\t" /* mm0 = [u0 * 1.732446 | u0 * 1.732446 | ...] */
			"paddw %%mm3, %%mm0\n\t" /* mm0 = [0 b11 0 b10 0 b01 0 b00] */
			"packuswb %%mm0, %%mm0\n\t" /* mm0 = [b11 b10 b01 b00 b11 b10 b01 b00] */
			"movd %%mm0, (%0)\n\t"
			"psrld $16, %%mm0\n\t" /* mm0 = [0 0 b11 b10 0 0 b11 b10] */
			"movd %%mm0, (%1)\n\t"

			"addl $2, %0\n\t"

			: "=&r" (b_buffer)
			: "r" (b_buffer + width), "0" (b_buffer)
			: "memory");


		__asm__ __volatile__(

			"prefetchnta 320(%0)\n\t"

			"mulss %%xmm6, %%xmm0\n\t" /* xmm0 = [0 | 0 | 0 | (v0 * 0.698001)] */
			"mulss %%xmm5, %%xmm1\n\t" /* xmm1 = [0 | 0 | 0 | (u0 * 0.337633)] */
			"addss %%xmm1, %%xmm0\n\t" /* xmm0 = [0 | 0 | 0 | (v0 * 0.698001) + (u0 * 0.337633)] */
			"cvtps2pi %%xmm0, %%mm0\n\t" /* mm0 = [0 | (v0 * 0.698001) + (u0 * 0.337633)] */
			"punpckldq %%mm0, %%mm0\n\t" /* mm0 = [(v0 * 0.698001) + (u0 * 0.337633) | ...] */
			"packssdw %%mm0, %%mm0\n\t" /* mm0 = [(short) (v0 * 0.698001) + (u0 * 0.337633) / ..] */
			"psubw %%mm0, %%mm3\n\t" /* mm3 = [0 g11 0 g10 0 g01 0 g00] */
			"packuswb %%mm3, %%mm3\n\t" /* mm3 = [g11 g10 g01 g00 g11 g10 g01 g00] */
			"movd %%mm3, (%0)\n\t"
			"psrld $16, %%mm3\n\t" /* mm3 = [0 0 g11 g10 0 0 g11 g10] */
			"movd %%mm3, (%1)\n\t"

			"addl $2, %0\n\t"

			: "=&r" (g_buffer)
			: "r" (g_buffer + width), "0" (g_buffer)
			: "memory");


		y_data += 2;
		u_data++;
		v_data++;

		if(!(--line_size))
		{
			line_size = width >> 1;
			y_data   += pitches[0];
			r_buffer += width;
			g_buffer += width;
			b_buffer += width;
		}
	}

	__asm__ __volatile__("emms\n\t");
}



static void
__mmx_convert_yuy2(uint8_t* yuv[], rgb_planar_t* rgb,
			uint32_t pitches[], uint32_t width, uint32_t height)
{
	uint8_t* yuv_data = yuv[0];
	uint8_t* r_buffer = rgb->r;
	uint8_t* g_buffer = rgb->g;
	uint8_t* b_buffer = rgb->b;
	uint32_t samples = (width * height) >> 2; /* 4 pixels at once */


	while(samples--)
	{
		__asm__ __volatile__(

			"movq (%0), %%mm0\n\t" /* mm1 = [v2 y3 u2 y2 v0 y1 u0 y0] */

			"movq %%mm0, %%mm3\n\t" /* mm3 = [v2 y3 u2 y2 v0 y1 u0 y0] */
			"pand (%1), %%mm3\n\t" /* mm3 = [0 0 0 y2 0 0 0 y0] */
			"movq %%mm0, %%mm2\n\t" /* mm2 = [v2 y3 u2 y2 v0 y1 u0 y0] */
			"psrld $16, %%mm2\n\t" /* mm2 = [0 0 v2 y3 0 0 v0 y1] */
			"pand (%1), %%mm2\n\t" /* mm2 = [0 0 0 y3 0 0 0 y1] */

			"movq %%mm0, %%mm1\n\t" /* mm1 = [v2 y3 u2 y2 v0 y1 u0 y0] */
			"psrld $8, %%mm1\n\t" /* mm1 = [0 v2 y3 u2 0 v0 y1 u0] */
			"pand (%1), %%mm1\n\t" /* mm1 = [0 0 0 u2 0 0 0 u0] */
			"psubd (%2), %%mm1\n\t" /* mm1 = [u2 - 128 | u0 - 128] */
			"packssdw %%mm1, %%mm1\n\t" /* mm1 = [0 u2 0 u0 0 u2 0 u0] */

			"psrld $24, %%mm0\n\t" /* mm0 = [0 0 0 v2 0 0 0 v0] */
			"psubd (%2), %%mm0\n\t" /* mm0 = [v2 - 128 | v0 - 128] */
			"packssdw %%mm0, %%mm0\n\t" /* mm0 = [0 u2 0 u0 0 u2 0 u0] */

			"addl $8, %0\n\t"

			: "=&r" (yuv_data)
			: "r" (lmask), "r" (lsub), "0" (yuv_data)
			: "memory");


		__asm__ __volatile__(

			"packssdw %%mm3, %%mm4\n\t" /* mm4 = [? ? ? ? 0 y2 0 y0] */
			"punpckhwd %%mm0, %%mm4\n\t" /* mm4 = [0 v2 0 y2 0 v0 0 y0] */
			"pmaddwd (%3), %%mm4\n\t" /* mm4 = [r2 << 14 | r0 << 14] */
			"psrad $14,%%mm4\n\t" /* mm4 = [0 0 0 r2 0 0 0 r0] */
			"packssdw %%mm4, %%mm4\n\t" /* mm4 = [0 r2 0 r0 0 r2 0 r0] */
			"packssdw %%mm2, %%mm5\n\t" /* mm5 = [? ? ? ? 0 y3 0 y1] */
			"punpckhwd %%mm0, %%mm5\n\t" /* mm5 = [0 v2 0 y3 0 v0 0 y1] */
			"pmaddwd (%3), %%mm5\n\t" /* mm5 = [r3 << 14| r1 << 14] */
			"psrad $14, %%mm5\n\t" /* mm5 = [0 0 0 r3 0 0 0 r1] */
			"packssdw %%mm5, %%mm5\n\t" /* mm5 = [0 r3 0 r1 0 r3 0 r1] */
			"punpckhwd %%mm5, %%mm4\n\t" /* mm4 = [0 r3 0 r2 0 r1 0 r0] */
			"packuswb %%mm4, %%mm4\n\t" /* mm4 = [r3 r2 r1 r0 r3 r2 r1 r0] */
			"movd %%mm4, (%0)\n\t"

			"packssdw %%mm3, %%mm4\n\t" /* mm4 = [? ? ? ? 0 y2 0 y0] */
			"punpckhwd %%mm1, %%mm4\n\t" /* mm4 = [0 u2 0 y2 0 u0 0 y0] */
			"pmaddwd 8(%3), %%mm4\n\t" /* mm4 = [b2 << 14 | b0 << 14] */
			"psrad $14, %%mm4\n\t" /* mm4 = [0 0 0 b2 0 0 0 b0] */
			"packssdw %%mm4, %%mm4\n\t" /* mm4 = [0 b2 0 b0 0 b2 0 b0] */
			"packssdw %%mm2, %%mm5\n\t" /* mm5 = [? ? ? ? 0 y3 0 y1] */
			"punpckhwd %%mm1, %%mm5\n\t" /* mm5 = [0 u2 0 y3 0 u0 0 y1] */
			"pmaddwd 8(%3), %%mm5\n\t" /* mm5 = [b3 << 14 | b1 << 14] */
			"psrad $14, %%mm5\n\t" /* mm5 = [0 0 0 b3 0 0 0 b1] */
			"packssdw %%mm5, %%mm5\n\t" /* mm5 = [0 b3 0 b1 0 b3 0 b1] */
			"punpckhwd %%mm5, %%mm4\n\t" /* mm4 = [0 b3 0 b2 0 b1 0 b0] */
			"packuswb %%mm4, %%mm4\n\t" /* mm4 = [b3 b2 b1 b0 b3 b2 b1 b0] */
			"movd %%mm4, (%2)\n\t"

			"punpckhwd %%mm1, %%mm0\n\t" /* mm0 = [0 u2 0 v2 0 u0 0 v0] */
			"pmaddwd 16(%3), %%mm0\n\t" /* mm0 = [(u2 * 5532) + (v2 * 11436) | ...] */
			"pslld $14, %%mm3\n\t"
			"psubd %%mm0, %%mm3\n\t" /* mm3 = [g2 << 14 | g0 << 14] */
			"psrad $14, %%mm3\n\t" /* mm3 = [0 0 0 g2 0 0 0 g0] */
			"packssdw %%mm3, %%mm3\n\t" /* mm3 = [0 g2 0 g0 0 g2 0 g0] */
			"pslld $14, %%mm2\n\t"
			"psubd %%mm0, %%mm2\n\t" /* mm2 = [g3 << 14 | g1 << 14] */
			"psrad $14, %%mm2\n\t" /* mm2 = [0 0 0 g3 0 0 0 g1] */
			"packssdw %%mm2, %%mm2\n\t" /* mm2 = [0 g3 0 g1 0 g3 0 g1] */
			"punpckhwd %%mm2, %%mm3\n\t" /* mm3 = [0 g3 0 g2 0 g1 0 g0] */
			"packuswb %%mm3, %%mm3\n\t" /* mm3 = [g3 g2 g1 g0 g3 g2 g1 g0] */
			"movd %%mm3, (%1)\n\t"

			"addl $4, %0\n\t"
			"addl $4, %1\n\t"
			"addl $4, %2\n\t"

			: "=&r" (r_buffer), "=&r" (g_buffer), "=&r" (b_buffer)
			: "r" (wd_factors), "0" (r_buffer), "1" (g_buffer), "2" (b_buffer)
			: "memory");
	}


	__asm__ __volatile__("emms\n\t");

}


static void
__mmx_convert_yv12(uint8_t* yuv[], rgb_planar_t* rgb,
			 uint32_t pitches[], uint32_t width, uint32_t height)
{
	static const uint16_t wsub[] =
	{
		128, 128,
		128, 128
	};
	uint8_t* y_data = yuv[0];
	uint8_t* u_data = yuv[1];
	uint8_t* v_data = yuv[2];
	uint8_t* r_buffer = rgb->r;
	uint8_t* g_buffer = rgb->g;
	uint8_t* b_buffer = rgb->b;
	uint32_t line_size = width >> 1; /* 2 pixels at once */
	uint32_t samples   = line_size * (height >> 1);


	__asm__ __volatile__(
		"pxor %%mm7, %%mm7\n\t"
		::: "memory");

	while(samples--)
	{
		__asm__ __volatile__(

			"movd (%0), %%mm3\n\t" /* mm3 = [0 0 0 0 y03 y02 y01 y00] */
			"movd (%1), %%mm2\n\t" /* mm2 = [0 0 0 0 y13 y12 y11 y10] */
			"movd (%2), %%mm1\n\t" /* mm1 = [0 0 0 0 u12 u8 u4 u0] */
			"movd (%3), %%mm0\n\t" /* mm0 = [0 0 0 0 v12 v8 v4 v0] */

			"punpcklbw %%mm7, %%mm3\n\t" /* mm3 = [0 y03 0 y02 0 y01 0 y00] */
			"punpcklbw %%mm7, %%mm2\n\t" /* mm2 = [0 y13 0 y12 0 y11 0 y10] */

			"punpckldq %%mm1, %%mm1\n\t" /* mm1 = [u12 u8 u4 u0 u12 u8 u4 u0] */
			"pand (%5), %%mm1\n\t" /* mm1 = [0 0 0 u0 0 0 0 u0] */
			"packssdw %%mm1, %%mm1\n\t" /* mm1 = [0 u0 0 u0 0 u0 0 u0] */
			"psubw (%4), %%mm1\n\t" /* mm1 = [u0 -128 | u0 -128 | ...] */

			"punpckldq %%mm0, %%mm0\n\t" /* mm0 = [v12 v8 v4 v0 v12 v8 v4 v0] */
			"pand (%5), %%mm0\n\t" /* mm1 = [0 0 0 v0 0 0 0 v0] */
			"packssdw %%mm0, %%mm0\n\t" /* mm1 = [0 v0 0 v0 0 v0 0 v0] */
			"psubw (%4), %%mm0\n\t" /* mm1 = [v0 -128 | v0 -128 | ...] */

			:: "r" (y_data), "r" (y_data + pitches[0]),
			   "r" (u_data), "r" (v_data), "r" (wsub), "r" (lmask)
			: "memory");


		__asm__ __volatile__(

			"movq %%mm3, %%mm4\n\t"
			"punpcklwd %%mm0, %%mm4\n\t" /* mm4 = [0 v0 0 y01 0 v0 0 y00] */
			"pmaddwd (%2), %%mm4\n\t" /* mm4 = [r01 << 14 | r00 << 14] */
			"psrad $14, %%mm4\n\t" /* mm4 = [r01 | r00] */
			"movq %%mm2, %%mm5\n\t"
			"punpcklwd %%mm0, %%mm5\n\t" /* mm5 = [0 v0 0 y11 0 v0 0 y10] */
			"pmaddwd (%2), %%mm5\n\t" /* mm5 = [r11 << 14 | r10 << 14] */
			"psrad $14, %%mm5\n\t" /* mm5 = [r11 | r10] */
			"packssdw %%mm5, %%mm4\n\t" /* mm4 = [r11 | r10 | r01 | r00] */
			"packuswb %%mm4, %%mm4\n\t" /* mm4 = [r11 r10 r01 r00 r11 r10 r01 r00] */
			"movd %%mm4, (%0)\n\t"
			"psrld $16, %%mm4\n\t" /* mm4 = [0 0 r11 r10 0 0 r11 r10] */
			"movd %%mm4, (%1)\n\t"
			"addl $2, %0\n\t"

			: "=&r" (r_buffer)
			: "r" (r_buffer + width), "r" (wd_factors), "0" (r_buffer)
			: "memory");


		__asm__ __volatile__(

			"movq %%mm3, %%mm4\n\t"
			"punpcklwd %%mm1, %%mm4\n\t" /* mm4 = [0 u0 0 y01 0 u0 0 y00] */
			"pmaddwd 8(%2), %%mm4\n\t" /* mm4 = [b01 << 14 | b00 << 14] */
			"psrad $14, %%mm4\n\t" /* mm4 = [b01 | b00] */
			"movq %%mm2, %%mm5\n\t"
			"punpckhwd %%mm1, %%mm5\n\t" /* mm5 = [0 u0 0 y11 0 u0 0 y10] */
			"pmaddwd 8(%2), %%mm5\n\t" /* mm5 = [b11 << 14 | b10 << 14] */
			"psrad $14, %%mm5\n\t" /* mm5 = [b11 | b10] */
			"packssdw %%mm5, %%mm4\n\t" /* mm4 = [b11 | b10 | b01 | b00] */
			"packuswb %%mm4, %%mm4\n\t" /* mm4 = [b11 b10 b01 b00 r11 b10 b01 b00] */
			"movd %%mm4, (%0)\n\t"
			"psrld $16, %%mm4\n\t" /* mm4 = [0 0 b11 b10 0 0 b11 b10] */
			"movd %%mm4, (%1)\n\t"
			"addl $2, %0\n\t"

			: "=&r" (b_buffer)
			: "r" (b_buffer + width), "r" (wd_factors), "0" (b_buffer)
			: "memory");


		__asm__ __volatile__(

			"punpcklwd %%mm1, %%mm0\n\t" /* mm0 = [0 u0 0 v0 0 u0 0 v0] */
			"pmaddwd 16(%2), %%mm0\n\t" /* mm0 = [(u0 * 5532) + (v0 * 11436) | ...] */
			"punpcklwd %%mm7, %%mm3\n\t" /* mm3 = [0 0 0 y01 0 0 0 y00] */
			"pslld $14, %%mm3\n\t" /* mm3 = [y01 << 14 | y00 << 14] */
			"psubd %%mm0, %%mm3\n\t" /* mm3 = [g01 << 14 | g00 << 14] */
			"psrad $14, %%mm3\n\t" /* mm3 = [g01 | g00] */
			"punpcklwd %%mm7, %%mm2\n\t" /* mm2 = [0 0 0 y11 0 0 0 y10] */
			"pslld $14, %%mm2\n\t" /* mm2 = [y11 << 14 | y10 << 14] */
			"psubd %%mm0, %%mm2\n\t" /* mm2 = [g11 << 14 | g10 << 14] */
			"psrad $14, %%mm2\n\t" /* mm2 = [g11 | g10] */
			"packssdw %%mm2, %%mm3\n\t" /* mm3 = [g11 | g10 | g01 | g00] */
			"packuswb %%mm3, %%mm3\n\t" /* mm3 = [g11 g10 g01 g00 g11 g10 g01 g00] */
			"movd %%mm3, (%0)\n\t"
			"psrld $16, %%mm3\n\t" /* mm3 = [0 0 g11 g10 0 0 g11 g10] */
			"movd %%mm3, (%1)\n\t"
			"addl $2, %0\n\t"

			: "=&r" (g_buffer)
			: "r" (g_buffer + width), "r" (wd_factors), "0" (g_buffer)
			: "memory");


		y_data += 2;
		u_data++;
		v_data++;

		if(!(--line_size))
		{
			line_size = width >> 1;
			y_data   += pitches[0];
			r_buffer += width;
			g_buffer += width;
			b_buffer += width;
		}
	}

	__asm__ __volatile__("emms\n\t");
}

#endif


#define range(x)  ((x < 0) ? 0 : ((x > 0xff) ? 0xff : x))

static void
__dummy_convert_yuy2(uint8_t* yuv[], rgb_planar_t* rgb,
			 uint32_t pitches[], uint32_t width, uint32_t height)
{
	uint8_t* yuv_data = yuv[0];
	uint8_t* r_buffer = rgb->r;
	uint8_t* g_buffer = rgb->g;
	uint8_t* b_buffer = rgb->b;
	uint32_t samples = (width * height) >> 1; /* 2 pixels at once */


	while(samples--)
	{
		int u, y, v, y2;
		int m1, m2, m3, m4;
		int r, g, b;

		y  = *(yuv_data)     << 16;
		u  = *(yuv_data + 1) - 128;
		y2 = *(yuv_data + 2) << 16;
		v  = *(yuv_data + 3) - 128;

		m1 = v * dw_factors[0];
		m2 = v * dw_factors[1];
		m3 = u * dw_factors[2];
		m4 = u * dw_factors[3];

		r = (int) (y + m1) >> 16;
		g = (int) (y - m2 - m3) >> 16;
		b = (int) (y + m4) >> 16;

		*(r_buffer) = range(r);
		*(g_buffer) = range(g);
		*(b_buffer) = range(b);

		r = (int) (y2 + m1) >> 16;
		g = (int) (y2 - m2 - m3) >> 16;
		b = (int) (y2 + m4) >> 16;

		*(r_buffer + 1) = range(r);
		*(g_buffer + 1) = range(g);
		*(b_buffer + 1) = range(b);

		yuv_data += 4;
		r_buffer += 2;
		g_buffer += 2;
		b_buffer += 2;
	}

}


static void
__dummy_convert_yv12(uint8_t* yuv[], rgb_planar_t* rgb,
			 uint32_t pitches[], uint32_t width, uint32_t height)
{
	uint8_t* y_data = yuv[0];
	uint8_t* u_data = yuv[1];
	uint8_t* v_data = yuv[2];
	uint8_t* r_buffer = rgb->r;
	uint8_t* g_buffer = rgb->g;
	uint8_t* b_buffer = rgb->b;
	uint32_t line_size = width >> 1; /* 2 pixels at once */
	uint32_t samples   = line_size * (height >> 1);


	while(samples--)
	{
		int y, u, v;
		int m1, m2, m3, m4;
		int r, g, b;

		u  = *(u_data++) - 128;
		v  = *(v_data++) - 128;

		m1 = v * dw_factors[0];
		m2 = v * dw_factors[1];
		m3 = u * dw_factors[2];
		m4 = u * dw_factors[3];

		y = *(y_data) << 16;

		r = (int) (y + m1) >> 16;
		g = (int) (y - m2 - m3) >> 16;
		b = (int) (y + m4) >> 16;

		*(r_buffer) = range(r);
		*(g_buffer) = range(g);
		*(b_buffer) = range(b);

		y = *(y_data + 1) << 16;

		r = (int) (y + m1) >> 16;
		g = (int) (y - m2 - m3) >> 16;
		b = (int) (y + m4) >> 16;

		*(r_buffer + 1) = range(r);
		*(g_buffer + 1) = range(g);
		*(b_buffer + 1) = range(b);

		y = *(y_data + pitches[0]) << 16;

		r = (int) (y + m1) >> 16;
		g = (int) (y - m2 - m3) >> 16;
		b = (int) (y + m4) >> 16;

		*(r_buffer + width) = range(r);
		*(g_buffer + width) = range(g);
		*(b_buffer + width) = range(b);

		y = *(y_data + pitches[0] + 1) << 16;

		r = (int) (y + m1) >> 16;
		g = (int) (y - m2 - m3) >> 16;
		b = (int) (y + m4) >> 16;

		*(r_buffer + width + 1) = range(r);
		*(g_buffer + width + 1) = range(g);
		*(b_buffer + width + 1) = range(b);

		y_data   += 2;
		r_buffer += 2;
		g_buffer += 2;
		b_buffer += 2;

		if(!(--line_size))
		{
			line_size = width >> 1;
			y_data   += pitches[0];
			r_buffer += width;
			g_buffer += width;
			b_buffer += width;
		}
	}

}



static const rgbout_converter_t convert_methods[] =
{
/*        <name>   <accel>      <convert yuy2>        <convert yv12>    */
	{    NULL,        0, __dummy_convert_yuy2, __dummy_convert_yv12},
#ifdef COMPILE_ASSEMBLY
	{   "MMX",   MM_MMX,   __mmx_convert_yuy2,   __mmx_convert_yv12},
	{   "SSE",   MM_SSE,   __sse_convert_yuy2,   __sse_convert_yv12},
	{"3DNow!", MM_3DNOW, __3dnow_convert_yuy2, __3dnow_convert_yv12}
#endif
/* currently 3DNow is the best function, therefore it's preferred on AMD cpus */
};






static void
__pack_argb(rgb_planar_t* data, void* dest,
		uint32_t pixels, uint32_t accel)
{
	static const uint32_t alpha[] =
	{
		0xffffffff,
		0xffffffff
	};
	uint8_t* buffer = (uint8_t*) dest;
	uint8_t* r_data = data->r;
	uint8_t* g_data = data->g;
	uint8_t* b_data = data->b;
	uint32_t rest = pixels;

#ifdef COMPILE_ASSEMBLY

	if((accel & MM_MMXEXT) == MM_MMXEXT)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"por (%0), %%mm3\n\t"
			:: "r" (alpha)
			: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"prefetchw 320(%3)\n\t"

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"punpcklbw %%mm3, %%mm0\n\t" /* mm0 = [ff r3 ff r2 ff r1 ff r0] */
				"punpcklbw %%mm1, %%mm2\n\t" /* mm2 = [g3 b3 g2 b2 g1 b1 g0 b0] */

				"movq %%mm2, %%mm4\n\t" /* mm2 = [g3 b3 g2 b2 g1 b1 g0 b0] */
				"punpcklwd %%mm0, %%mm4\n\t" /* mm4 = [ff r1 g1 b1 ff r0 g0 b0] */
				"movntq %%mm4, (%3)\n\t"

				"punpckhwd %%mm0, %%mm2\n\t" /* mm2 = [ff r3 g3 b3 ff r2 g2 b2] */
				"movntq %%mm2, 8(%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $16, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");
		}

		__asm__ __volatile__("sfence; emms\n\t");

	}
	else if((accel & MM_MMX) == MM_MMX)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"por (%0), %%mm3\n\t"
			:: "r" (alpha)
			: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"punpcklbw %%mm3, %%mm0\n\t" /* mm0 = [ff r3 ff r2 ff r1 ff r0] */
				"punpcklbw %%mm1, %%mm2\n\t" /* mm2 = [g3 b3 g2 b2 g1 b1 g0 b0] */

				"movq %%mm2, %%mm4\n\t" /* mm2 = [g3 b3 g2 b2 g1 b1 g0 b0] */
				"punpcklwd %%mm0, %%mm4\n\t" /* mm4 = [ff r1 g1 b1 ff r0 g0 b0] */
				"movq %%mm4, (%3)\n\t"

				"punpckhwd %%mm0, %%mm2\n\t" /* mm2 = [ff r3 g3 b3 ff r2 g2 b2] */
				"movq %%mm2, 8(%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $16, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");
		}

		__asm__ __volatile__("emms\n\t");

	}
#endif

	while(rest--)
	{
		*(buffer)     = *(b_data++);
		*(buffer + 1) = *(g_data++);
		*(buffer + 2) = *(r_data++);
		*(buffer + 3) = 0xff;

		buffer += 4;
	}

}


static void
__pack_argb1555(rgb_planar_t* data, void* dest,
		uint32_t pixels, uint32_t accel)
{
	static const uint16_t alpha[] =
	{
		0x8000,
		0x8000,
		0x8000,
		0x8000
	};
	uint16_t* buffer = (uint16_t*) dest;
	uint8_t* r_data = data->r;
	uint8_t* g_data = data->g;
	uint8_t* b_data = data->b;
	int32_t rest = pixels;

#ifdef COMPILE_ASSEMBLY

	if((accel & MM_MMXEXT) == MM_MMXEXT)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"pxor %%mm3, %%mm3\n\t"
			"movq (%0), %%mm4\n\t"
			:: "r" (alpha)
			: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"prefetchw 320(%3)\n\t"

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"punpcklbw %%mm3, %%mm0\n\t" /* mm0 = [0 r3 0 r2 0 r1 0 r0] */
				"punpcklbw %%mm3, %%mm1\n\t" /* mm1 = [0 g3 0 g2 0 g1 0 g0] */
				"punpcklbw %%mm3, %%mm2\n\t" /* mm2 = [0 b3 0 b2 0 b1 0 b0] */

				"psrlw $3, %%mm0\n\t" /* word = [00000000 000rrrrr] */
				"psrlw $3, %%mm1\n\t" /* word = [00000000 000ggggg] */
				"psrlw $3, %%mm2\n\t" /* word = [00000000 000bbbbb] */

				"psllw $10, %%mm0\n\t" /* word = [0rrrrr00 00000000] */
				"psllw $5,  %%mm1\n\t" /* word = [000000gg ggg00000] */
				"por %%mm2, %%mm1\n\t" /* word = [000000gg gggbbbbb] */
				"por %%mm1, %%mm0\n\t" /* word = [0rrrrrgg gggbbbbb] */
				"por %%mm4, %%mm0\n\t" /* word = [1rrrrrgg gggbbbbb] */

				"movntq %%mm0, (%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $8, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");

		}

		__asm__ __volatile__("sfence; emms\n\t");

	}
	else if((accel & MM_MMX) == MM_MMX)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"pxor %%mm3, %%mm3\n\t"
			"movq (%0), %%mm4\n\t"
			:: "r" (alpha)
			: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"punpcklbw %%mm3, %%mm0\n\t" /* mm0 = [0 r3 0 r2 0 r1 0 r0] */
				"punpcklbw %%mm3, %%mm1\n\t" /* mm1 = [0 g3 0 g2 0 g1 0 g0] */
				"punpcklbw %%mm3, %%mm2\n\t" /* mm2 = [0 b3 0 b2 0 b1 0 b0] */

				"psrlw $3, %%mm0\n\t" /* word = [00000000 000rrrrr] */
				"psrlw $3, %%mm1\n\t" /* word = [00000000 000ggggg] */
				"psrlw $3, %%mm2\n\t" /* word = [00000000 000bbbbb] */

				"psllw $10, %%mm0\n\t" /* word = [0rrrrr00 00000000] */
				"psllw $5,  %%mm1\n\t" /* word = [000000gg ggg00000] */
				"por %%mm2, %%mm1\n\t" /* word = [000000gg gggbbbbb] */
				"por %%mm1, %%mm0\n\t" /* word = [0rrrrrgg gggbbbbb] */
				"por %%mm4, %%mm0\n\t" /* word = [arrrrrgg gggbbbbb] */

				"movq %%mm0, (%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $8, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");

		}

		__asm__ __volatile__("emms\n\t");

	}

#endif
	while(rest--)
	{
		uint16_t r5, g5, b5;

		r5 = *(r_data++) >> 3;
		g5 = *(g_data++) >> 3;
		b5 = *(b_data++) >> 3;

		*(buffer++) = (alpha[0] | (r5 << 10) | (g5 << 5) | b5);
	}

}


static void
__pack_rgb32(rgb_planar_t* data, void* dest,
		uint32_t pixels, uint32_t accel)
{
	uint8_t* buffer = (uint8_t*) dest;
	uint8_t* r_data = data->r;
	uint8_t* g_data = data->g;
	uint8_t* b_data = data->b;
	uint32_t rest = pixels;

#ifdef COMPILE_ASSEMBLY

	if((accel & MM_MMXEXT) == MM_MMXEXT)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"pxor %%mm3, %%mm3\n\t"
			::: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"prefetchw 320(%0)\n\t"

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"punpcklbw %%mm3, %%mm0\n\t" /* mm0 = [0 r3 0 r2 0 r1 0 r0] */
				"punpcklbw %%mm1, %%mm2\n\t" /* mm2 = [g3 b3 g2 b2 g1 b1 g0 b0] */

				"movq %%mm2, %%mm4\n\t" /* mm2 = [g3 b3 g2 b2 g1 b1 g0 b0] */
				"punpcklwd %%mm0, %%mm4\n\t" /* mm4 = [0 r1 g1 b1 0 r0 g0 b0] */
				"movntq %%mm4, (%3)\n\t"

				"punpckhwd %%mm0, %%mm2\n\t" /* mm2 = [0 r3 g3 b3 0 r2 g2 b2] */
				"movntq %%mm2, 8(%3)\n\t"

				"addl  $4, %0\n\t"
				"addl  $4, %1\n\t"
				"addl  $4, %2\n\t"
				"addl $16, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");
		}

		__asm__ __volatile__("sfence; emms\n\t");

	}
	else if((accel & MM_MMX) == MM_MMX)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"pxor %%mm3, %%mm3\n\t"
			::: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"punpcklbw %%mm3, %%mm0\n\t" /* mm0 = [0 r3 0 r2 0 r1 0 r0] */
				"punpcklbw %%mm1, %%mm2\n\t" /* mm2 = [g3 b3 g2 b2 g1 b1 g0 b0] */

				"movq %%mm2, %%mm4\n\t" /* mm2 = [g3 b3 g2 b2 g1 b1 g0 b0] */
				"punpcklwd %%mm0, %%mm4\n\t" /* mm4 = [0 r1 g1 b1 0 r0 g0 b0] */
				"movq %%mm4, (%3)\n\t"

				"punpckhwd %%mm0, %%mm2\n\t" /* mm2 = [0 r3 g3 b3 0 r2 g2 b2] */
				"movq %%mm2, 8(%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $16, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");
		}

		__asm__ __volatile__("emms\n\t");
	}
#endif

	while(rest--)
	{
		*(buffer)     = *(b_data++);
		*(buffer + 1) = *(g_data++);
		*(buffer + 2) = *(r_data++);

		buffer += 4;
	}

}


static void
__pack_rgb24(rgb_planar_t* data, void* dest,
		uint32_t pixels, uint32_t accel)
{
	uint8_t* buffer = (uint8_t*) dest;
	uint8_t* r_data = data->r;
	uint8_t* g_data = data->g;
	uint8_t* b_data = data->b;
	uint32_t rest = pixels;


/* MMXEXT doesn't speed up here */
#ifdef COMPILE_ASSEMBLY

	if((accel & MM_MMX) == MM_MMX)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"pxor %%mm3, %%mm3\n\t"
			::: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"punpcklbw %%mm1, %%mm2\n\t" /* mm1 = [g3 b3 g2 b2 g1 b1 g0 b0] */
				"punpcklbw %%mm3, %%mm0\n\t" /* mm3 = [0 r3 0 r2 0 r1 0 r0] */

				"movq %%mm2, %%mm4\n\t" /* mm4 = [g3 b3 g2 b2 g1 b1 g0 b0] */
				"punpcklwd %%mm0, %%mm4\n\t" /* mm4 = [0 r1 g1 b1 0 r0 g0 b0] */
				"movd %%mm4, (%3)\n\t"
				"punpckhdq %%mm4, %%mm4\n\t" /* mm4 = [0 r1 g1 b1 0 r1 g1 b1] */
				"movd %%mm4, 3(%3)\n\t"

				"punpckhwd %%mm0, %%mm2\n\t" /* mm2 = [0 r3 g3 b3 0 r2 g2 b2] */
				"movd %%mm2, 6(%3)\n\t"
				"punpckhdq %%mm2, %%mm2\n\t" /* mm2 = [0 r3 g3 b3 0 r3 g3 b3] */
				"movd %%mm2, 9(%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $12, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");

		}

		__asm__ __volatile__("emms\n\t");

	}
#endif

	while(rest--)
	{
		*(buffer)     = *(b_data++);
		*(buffer + 1) = *(g_data++);
		*(buffer + 2) = *(r_data++);

		buffer += 3;
	}

}


static void
__pack_rgb16(rgb_planar_t* data, void* dest,
		uint32_t pixels, uint32_t accel)
{
	uint16_t* buffer = (uint16_t*) dest;
	uint8_t* r_data = data->r;
	uint8_t* g_data = data->g;
	uint8_t* b_data = data->b;
	uint32_t rest = pixels;

#ifdef COMPILE_ASSEMBLY

	if((accel & MM_MMXEXT) == MM_MMXEXT)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"pxor %%mm3, %%mm3\n\t"
			::: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"prefetchw 320(%3)\n\t"

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"punpcklbw %%mm3, %%mm0\n\t" /* mm0 = [0 r3 0 r2 0 r1 0 r0] */
				"punpcklbw %%mm3, %%mm1\n\t" /* mm1 = [0 g3 0 g2 0 g1 0 g0] */
				"punpcklbw %%mm3, %%mm2\n\t" /* mm2 = [0 b3 0 b2 0 b1 0 b0] */

				"psrlw $3, %%mm0\n\t" /* word = [00000000 000rrrrr] */
				"psrlw $2, %%mm1\n\t" /* word = [00000000 00gggggg] */
				"psrlw $3, %%mm2\n\t" /* word = [00000000 000bbbbb] */

				"psllw $11, %%mm0\n\t" /* word = [rrrrr000 00000000] */
				"psllw $5,  %%mm1\n\t" /* word = [00000ggg ggg00000] */
				"por %%mm2, %%mm1\n\t" /* word = [00000ggg gggbbbbb] */
				"por %%mm1, %%mm0\n\t" /* word = [rrrrrggg gggbbbbb] */

				"movntq %%mm0, (%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $8, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");
			}

		__asm__ __volatile__("sfence; emms\n\t");

	}
	else if((accel & MM_MMX) == MM_MMX)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"pxor %%mm3, %%mm3\n\t"
			::: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"punpcklbw %%mm3, %%mm0\n\t" /* mm0 = [0 r3 0 r2 0 r1 0 r0] */
				"punpcklbw %%mm3, %%mm1\n\t" /* mm1 = [0 g3 0 g2 0 g1 0 g0] */
				"punpcklbw %%mm3, %%mm2\n\t" /* mm2 = [0 b3 0 b2 0 b1 0 b0] */

				"psrlw $3, %%mm0\n\t" /* word = [00000000 000rrrrr] */
				"psrlw $2, %%mm1\n\t" /* word = [00000000 00gggggg] */
				"psrlw $3, %%mm2\n\t" /* word = [00000000 000bbbbb] */

				"psllw $11, %%mm0\n\t" /* word = [rrrrr000 00000000] */
				"psllw $5,  %%mm1\n\t" /* word = [00000ggg ggg00000] */
				"por %%mm2, %%mm1\n\t" /* word = [00000ggg gggbbbbb] */
				"por %%mm1, %%mm0\n\t" /* word = [rrrrrggg gggbbbbb] */

				"movq %%mm0, (%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $8, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");
			}

		__asm__ __volatile__("emms\n\t");

	}
#endif

	while(rest--)
	{
		uint16_t r16, g16, b16;

		r16 = *(r_data++) >> 3;
		g16 = *(g_data++) >> 2;
		b16 = *(b_data++) >> 3;

		*(buffer++) = ((r16 << 11) | (g16 << 5) | b16);
	}

}


static void
__pack_bgra(rgb_planar_t* data, void* dest,
		 uint32_t pixels, uint32_t accel)
{
	static const uint32_t alpha[] =
	{
		0xffffffff,
		0xffffffff
	};
	uint8_t* buffer = (uint8_t*) dest;
	uint8_t* r_data = data->r;
	uint8_t* g_data = data->g;
	uint8_t* b_data = data->b;
	uint32_t rest = pixels;


#ifdef COMPILE_ASSEMBLY
	if((accel & MM_MMXEXT) == MM_MMXEXT)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		while(n--)
		{
			__asm__ __volatile__(

				"prefetchw 320(%3)\n\t"

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"por (%4), %%mm3\n\t" /* mm3 = [ff ff ff ff ff ff ff ff] */
				"punpcklbw %%mm0, %%mm3\n\t" /* mm3 = [r3 ff r2 ff r1 ff r0 ff] */
				"punpcklbw %%mm2, %%mm1\n\t" /* mm1 = [b3 g3 b2 g2 b1 g1 b0 g0] */

				"movq %%mm3, %%mm4\n\t" /* mm4 = [r3 ff r2 ff r1 ff r0 ff] */
				"punpcklwd %%mm1, %%mm4\n\t" /* mm4 = [b1 g1 r1 ff b1 g1 r1 ff] */
				"movntq %%mm4, (%3)\n\t"

				"punpckhwd %%mm1, %%mm3\n\t" /* mm3 = [b3 g3 r3 ff b2 g2 r2 ff] */
				"movntq %%mm0, 8(%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $16, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "r" (alpha), "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");
		}

		__asm__ __volatile__("sfence; emms\n\t");

	}
	else if((accel & MM_MMX) == MM_MMX)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		while(n--)
		{
			__asm__ __volatile__(

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"por (%4), %%mm3\n\t" /* mm3 = [ff ff ff ff ff ff ff ff] */
				"punpcklbw %%mm0, %%mm3\n\t" /* mm3 = [r3 ff r2 ff r1 ff r0 ff] */
				"punpcklbw %%mm2, %%mm1\n\t" /* mm1 = [b3 g3 b2 g2 b1 g1 b0 g0] */

				"movq %%mm3, %%mm4\n\t" /* mm4 = [r3 ff r2 ff r1 ff r0 ff] */
				"punpcklwd %%mm1, %%mm4\n\t" /* mm4 = [b1 g1 r1 ff b1 g1 r1 ff] */
				"movq %%mm4, (%3)\n\t"

				"punpckhwd %%mm1, %%mm3\n\t" /* mm3 = [b3 g3 r3 ff b2 g2 r2 ff] */
				"movq %%mm0, 8(%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $16, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "r" (alpha), "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");
		}

		__asm__ __volatile__("emms\n\t");

	}

#endif
	while(rest--)
	{
		*(buffer)     = 0xff;
		*(buffer + 1) = *(r_data++);
		*(buffer + 2) = *(g_data++);
		*(buffer + 3) = *(b_data++);

		buffer += 4;
	}

}


static void
__pack_bgra5551(rgb_planar_t* data, void* dest,
		 uint32_t pixels, uint32_t accel)
{
	static const uint16_t alpha[] =
	{
		1, 1, 1, 1
	};
	uint16_t* buffer = (uint16_t*) dest;
	uint8_t* r_data = data->r;
	uint8_t* g_data = data->g;
	uint8_t* b_data = data->b;
	uint32_t rest = pixels;


#ifdef COMPILE_ASSEMBLY
	if((accel & MM_MMXEXT) == MM_MMXEXT)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"pxor %%mm3, %%mm3\n\t"
			::: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"prefetchw 320(%3)\n\t"

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"punpcklbw %%mm3, %%mm0\n\t" /* mm0 = [0 r3 0 r2 0 r1 0 r0] */
				"punpcklbw %%mm3, %%mm1\n\t" /* mm1 = [0 g3 0 g2 0 g1 0 g0] */
				"punpcklbw %%mm3, %%mm2\n\t" /* mm2 = [0 b3 0 b2 0 b1 0 b0] */

				"psrlw $2, %%mm0\n\t" /* word = [00000000 00rrrrrr] */
				"psrlw $3, %%mm1\n\t" /* word = [00000000 000ggggg] */
				"psrlw $3, %%mm2\n\t" /* word = [00000000 000bbbbb] */

				"psllw $11, %%mm2\n\t" /* word = [bbbbb000 00000000] */
				"psllw $6, %%mm1\n\t"  /* word = [00000ggg gg000000] */
				"por (%4), %%mm0\n\t"  /* word = [00000000 00rrrrra] */
				"por %%mm2, %%mm1\n\t" /* word = [bbbbbggg gg000000] */
				"por %%mm1, %%mm0\n\t" /* word = [bbbbbggg ggrrrrra] */

				"movntq %%mm0, (%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $8, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "r" (alpha), "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");
		}

		__asm__ __volatile__("sfence; emms\n\t");

	}
	else if((accel & MM_MMX) == MM_MMX)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"pxor %%mm3, %%mm3\n\t"
			::: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"punpcklbw %%mm3, %%mm0\n\t" /* mm0 = [0 r3 0 r2 0 r1 0 r0] */
				"punpcklbw %%mm3, %%mm1\n\t" /* mm1 = [0 g3 0 g2 0 g1 0 g0] */
				"punpcklbw %%mm3, %%mm2\n\t" /* mm2 = [0 b3 0 b2 0 b1 0 b0] */

				"psrlw $2, %%mm0\n\t" /* word = [00000000 00rrrrrr] */
				"psrlw $3, %%mm1\n\t" /* word = [00000000 000ggggg] */
				"psrlw $3, %%mm2\n\t" /* word = [00000000 000bbbbb] */

				"psllw $11, %%mm2\n\t" /* word = [bbbbb000 00000000] */
				"psllw $6, %%mm1\n\t"  /* word = [00000ggg gg000000] */
				"por (%4), %%mm0\n\t"  /* word = [00000000 00rrrrra] */
				"por %%mm2, %%mm1\n\t" /* word = [bbbbbggg gg000000] */
				"por %%mm1, %%mm0\n\t" /* word = [bbbbbggg ggrrrrra] */

				"movq %%mm0, (%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $8, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "r" (alpha), "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");
		}

		__asm__ __volatile__("emms\n\t");

	}
#endif

	while(rest--)
	{
		uint16_t r5, g5, b5;

		r5 = *(r_data++) >> 2;
		g5 = *(g_data++) >> 3;
		b5 = *(b_data++) >> 3;

		*(buffer++) = ((b5 << 11) | (g5 << 6) | r5 | 1);
	}

}


static void
__pack_bgr32(rgb_planar_t* data, void* dest,
		 uint32_t pixels, uint32_t accel)
{
	uint8_t* buffer = (uint8_t*) dest;
	uint8_t* r_data = data->r;
	uint8_t* g_data = data->g;
	uint8_t* b_data = data->b;
	uint32_t rest = pixels;

#ifdef COMPILE_ASSEMBLY

	if((accel & MM_MMXEXT) == MM_MMXEXT)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"pxor %%mm3, %%mm3\n\t"
			::: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"prefetchw 320(%0)\n\t"

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"punpcklbw %%mm1, %%mm0\n\t" /* mm0 = [g3 r3 g2 r2 g1 r1 g0 r0] */
				"punpcklbw %%mm3, %%mm2\n\t" /* mm2 = [b3 0 b2 0 b1 0 b0 0] */

				"movq %%mm0, %%mm4\n\t" /* mm4 = [g3 r3 g2 r2 g1 r1 g0 r0] */
				"punpcklwd %%mm2, %%mm4\n\t" /* mm4 = [0 b1 g1 r1 0 b0 g0 r0] */
				"movntq %%mm4, (%3)\n\t"

				"punpckhwd %%mm2, %%mm0\n\t" /* mm0 = [0 b3 g3 r3 0 b2 g2 r2] */
				"movntq %%mm0, 8(%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $16, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");
		}

		__asm__ __volatile__("sfence; emms\n\t");

	}
	else if((accel & MM_MMX) == MM_MMX)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"pxor %%mm3, %%mm3\n\t"
			::: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"punpcklbw %%mm1, %%mm0\n\t" /* mm0 = [g3 r3 g2 r2 g1 r1 g0 r0] */
				"punpcklbw %%mm3, %%mm2\n\t" /* mm2 = [b3 0 b2 0 b1 0 b0 0] */

				"movq %%mm0, %%mm4\n\t" /* mm4 = [g3 r3 g2 r2 g1 r1 g0 r0] */
				"punpcklwd %%mm2, %%mm4\n\t" /* mm4 = [0 b1 g1 r1 0 b0 g0 r0] */
				"movq %%mm4, (%3)\n\t"

				"punpckhwd %%mm2, %%mm0\n\t" /* mm0 = [0 b3 g3 r3 0 b2 g2 r2] */
				"movq %%mm0, 8(%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $16, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");
		}

		__asm__ __volatile__("emms\n\t");

	}
#endif

	while(rest--)
	{
		*(buffer)     = *(r_data++);
		*(buffer + 1) = *(g_data++);
		*(buffer + 2) = *(b_data++);

		buffer += 4;
	}

}


static void
__pack_bgr24(rgb_planar_t* data, void* dest,
		 uint32_t pixels, uint32_t accel)
{
	uint8_t* buffer = (uint8_t*) dest;
	uint8_t* r_data = data->r;
	uint8_t* g_data = data->g;
	uint8_t* b_data = data->b;
	uint32_t rest = pixels;


/* MMXEXT doesn't speed up here */

#ifdef COMPILE_ASSEMBLY
	if((accel & MM_MMX) == MM_MMX)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"pxor %%mm3, %%mm3\n\t"
			::: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"movd (%0), %%mm0\n\t" /* mm0 = [0 0 0 0 r3 r2 r1 r0] */
				"movd (%1), %%mm1\n\t" /* mm1 = [0 0 0 0 g3 g2 g1 g0] */
				"movd (%2), %%mm2\n\t" /* mm2 = [0 0 0 0 b3 b2 b1 b0] */

				"punpcklbw %%mm1, %%mm0\n\t" /* mm1 = [g3 r3 g2 r2 g1 r1 g0 r0] */
				"punpcklbw %%mm3, %%mm2\n\t" /* mm3 = [0 b3 0 b2 0 b1 0 b0] */

				"movq %%mm0, %%mm4\n\t" /* mm4 = [g3 r3 g2 r2 g1 r1 g0 r0] */
				"punpcklwd %%mm2, %%mm4\n\t" /* mm4 = [0 b1 g1 r1 0 b0 g0 r0] */
				"movd %%mm4, (%3)\n\t"
				"punpckhdq %%mm4, %%mm4\n\t" /* mm4 = [0 b1 g1 r1 0 b1 g1 r1] */
				"movd %%mm4, 3(%3)\n\t"

				"punpckhwd %%mm2, %%mm0\n\t" /* mm2 = [0 b3 g3 r3 0 b2 g2 r2] */
				"movd %%mm0, 6(%3)\n\t"
				"punpckhdq %%mm0, %%mm0\n\t" /* mm2 = [0 b3 g3 r3 0 b3 g3 r3] */
				"movd %%mm0, 9(%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $12, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");
		}

		__asm__ __volatile__("emms\n\t");

	}
#endif


	while(rest--)
	{
		*(buffer)     = *(r_data++);
		*(buffer + 1) = *(g_data++);
		*(buffer + 2) = *(b_data++);

		buffer += 3;
	}

}


static void
__pack_bgr16(rgb_planar_t* data, void* dest,
		 uint32_t pixels, uint32_t accel)
{
	uint16_t* buffer = (uint16_t*) dest;
	uint8_t* r_data = data->r;
	uint8_t* g_data = data->g;
	uint8_t* b_data = data->b;
	uint32_t rest = pixels;


#ifdef COMPILE_ASSEMBLY
	if((accel & MM_MMXEXT) == MM_MMXEXT)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"pxor %%mm3, %%mm3\n\t"
			::: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"prefetchw 320(%0)\n\t"

				"movd (%0), %%mm0\n\t"  /* mm0 = [0 | r3r2r1r0] */
				"movd (%1), %%mm1\n\t"  /* mm1 = [0 | g3g2g1g0] */
				"movd (%2), %%mm2\n\t"  /* mm2 = [0 | b3b2b1b0] */

				"punpcklbw %%mm3, %%mm0\n\t" /* mm0 = [0 r3 0 r2 0 r1 0 r0] */
				"punpcklbw %%mm3, %%mm1\n\t" /* mm1 = [0 g3 0 g2 0 g1 0 g0] */
				"punpcklbw %%mm3, %%mm2\n\t" /* mm2 = [0 b3 0 b2 0 b1 0 b0] */

				"psrlw $3, %%mm0\n\t" /* word = [00000000 000rrrrr] */
				"psrlw $2, %%mm1\n\t" /* word = [00000000 00gggggg] */
				"psrlw $3, %%mm2\n\t" /* word = [00000000 000bbbbb] */

				"psllw $11, %%mm2\n\t" /* word = [bbbbb000 00000000] */
				"psllw $5,  %%mm1\n\t" /* word = [00000ggg ggg00000] */
				"por %%mm2, %%mm1\n\t" /* word = [bbbbbggg ggg00000] */
				"por %%mm1, %%mm0\n\t" /* word = [bbbbbggg gggrrrrr] */

				"movntq %%mm0, (%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $8, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");
		}

		__asm__ __volatile__("sfence; emms\n\t");

	}
	else if((accel & MM_MMX) == MM_MMX)
	{
		uint32_t n = pixels >> 2; /* (width * height) / 4 */
		rest = pixels & 3; /* pixels - (n * 4) */


		__asm__ __volatile__(
			"pxor %%mm3, %%mm3\n\t"
			::: "memory");

		while(n--)
		{
			__asm__ __volatile__(

				"movd (%0), %%mm0\n\t"  /* mm0 = [0 | r3r2r1r0] */
				"movd (%1), %%mm1\n\t"  /* mm1 = [0 | g3g2g1g0] */
				"movd (%2), %%mm2\n\t"  /* mm2 = [0 | b3b2b1b0] */

				"punpcklbw %%mm3, %%mm0\n\t" /* mm0 = [0 r3 0 r2 0 r1 0 r0] */
				"punpcklbw %%mm3, %%mm1\n\t" /* mm1 = [0 g3 0 g2 0 g1 0 g0] */
				"punpcklbw %%mm3, %%mm2\n\t" /* mm2 = [0 b3 0 b2 0 b1 0 b0] */

				"psrlw $3, %%mm0\n\t" /* word = [00000000 000rrrrr] */
				"psrlw $2, %%mm1\n\t" /* word = [00000000 00gggggg] */
				"psrlw $3, %%mm2\n\t" /* word = [00000000 000bbbbb] */

				"psllw $11, %%mm2\n\t" /* word = [bbbbb000 00000000] */
				"psllw $5,  %%mm1\n\t" /* word = [00000ggg ggg00000] */
				"por %%mm2, %%mm1\n\t" /* word = [bbbbbggg ggg00000] */
				"por %%mm1, %%mm0\n\t" /* word = [bbbbbggg gggrrrrr] */

				"movq %%mm0, (%3)\n\t"

				"addl $4, %0\n\t"
				"addl $4, %1\n\t"
				"addl $4, %2\n\t"
				"addl $8, %3\n\t"

				: "=&r" (r_data), "=&r" (g_data), "=&r" (b_data), "=&r" (buffer)
				: "0" (r_data), "1" (g_data), "2" (b_data), "3" (buffer)
				: "memory");
		}

		__asm__ __volatile__("emms\n\t");

	}
#endif


	while(rest--)
	{
		uint16_t r16, g16, b16;

		r16 = *(r_data++) >> 3;
		g16 = *(g_data++) >> 2;
		b16 = *(b_data++) >> 3;

		*(buffer++) = ((b16 << 11) | (g16 << 5) | r16);
	}

}



static const rgbout_packer_t pack_methods[] =
{
/*  	name / pixel id / pixel size / scratch / function */
	{    "ARGB",     PX_ARGB, 4, 0,     __pack_argb},
	{"ARGB1555", PX_ARGB1555, 2, 0, __pack_argb1555},
	{   "RGB32",    PX_RGB32, 4, 0,    __pack_rgb32},
	{   "RGB24",    PX_RGB24, 3, 1,    __pack_rgb24},
	{   "RGB16",    PX_RGB16, 2, 0,    __pack_rgb16},
	{    "BGRA",     PX_BGRA, 4, 0,     __pack_bgra},
	{"BGRA5551", PX_BGRA5551, 2, 0, __pack_bgra5551},
	{   "BGR32",    PX_BGR32, 4, 0,    __pack_bgr32},
	{   "BGR24",    PX_BGR24, 3, 1,    __pack_bgr24},
	{   "BGR26",    PX_BGR16, 2, 0,    __pack_bgr16}
};






static uint32_t
rgbout_get_capabilities(vo_driver_t* vo_driver)
{
	return(VO_CAP_YV12 | VO_CAP_YUY2);
}


static void
rgbout_frame_field(vo_frame_t* vo_frame, int flags)
{
	/* nothing */
}


static void
rgbout_frame_dispose(vo_frame_t* vo_frame)
{
	rgbout_frame_t* frame = (rgbout_frame_t*) vo_frame;

	EVAL(vo_frame != NULL)

	release(frame->chunk[0]);
	release(frame->chunk[1]);
	release(frame->chunk[2]);
	free(frame);

FAILURE:
	return;
}


static vo_frame_t*
rgbout_alloc_frame(vo_driver_t* vo_driver)
{
	rgbout_driver_t* this = (rgbout_driver_t*) vo_driver;
	rgbout_frame_t* frame = NULL;

	EVAL(vo_driver != NULL);

	EVAL(frame = (rgbout_frame_t*) malloc(sizeof(rgbout_frame_t)));
	clear(frame, sizeof(rgbout_frame_t));
	this->frame = frame;

	pthread_mutex_init(&(frame->vo_frame.mutex), NULL);

	frame->vo_frame.proc_slice = NULL;
 	frame->vo_frame.proc_frame = NULL;
	frame->vo_frame.field      = rgbout_frame_field;
	frame->vo_frame.dispose    = rgbout_frame_dispose;
	frame->vo_frame.driver     = vo_driver;

	if(this->frame_width && this->frame_height)
		frame->pixels = (this->frame_width * this->frame_height);

	return((vo_frame_t*) frame);

FAILURE:
	return(NULL);
}


static void
rgbout_update_frame_format(vo_driver_t* vo_driver, vo_frame_t* vo_frame,
		uint32_t width, uint32_t height, double ratio,
		int format, int flags)
{
	rgbout_driver_t* this = (rgbout_driver_t*) vo_driver;
	rgbout_frame_t* frame = (rgbout_frame_t*) vo_frame;
	static int told = 0;


	EVAL(vo_driver != NULL);
	EVAL(vo_frame  != NULL);
	CHECK(width  != 0);
	CHECK(height != 0);

	frame->width  = width + (width & 1); /* if odd, round to next even number */
	frame->height = height + (height & 1); /* if odd, round to next even number */
	frame->pixels = frame->width * frame->height;
	frame->ratio  = ratio;
	frame->format = format;

	this->lastframe_width  = this->frame_width;
	this->lastframe_height = this->frame_height;
	this->frame_width      = frame->width;
	this->frame_height     = frame->height;

	release(frame->chunk[0]);
	release(frame->chunk[1]);
	release(frame->chunk[2]);

CHECK_FRAME:
	switch(frame->format)
	{
		case XINE_IMGFMT_YV12:
		{
			if(!(told))
			{
				if (s_debugMessages) fprintf(stderr, THIS ": yuv format is YV12\n");
				told = 1;
			}
			
			this->convert = convert_methods[this->cm].convert_yv12;
			frame->vo_frame.pitches[0] = (frame->width > 7) ? frame->width : 8;
			frame->vo_frame.pitches[1] = (frame->width > 15) ? (frame->width >> 1) : 8;
			frame->vo_frame.pitches[2] = frame->vo_frame.pitches[1];
			frame->vo_frame.base[0] = (uint8_t*) xine_xmalloc_aligned(16,
							 	frame->vo_frame.pitches[0] * frame->height,
							 	&(frame->chunk[0]));
			frame->vo_frame.base[1] = (uint8_t*) xine_xmalloc_aligned(16,
								frame->vo_frame.pitches[1] * (frame->height >> 1),
								&(frame->chunk[1]));
			frame->vo_frame.base[2] = (uint8_t*) xine_xmalloc_aligned(16,
								frame->vo_frame.pitches[2] * (frame->height >> 1),
								&(frame->chunk[2]));
		}
		break;

		case XINE_IMGFMT_YUY2:
		{
			if(!(told))
			{
				if (s_debugMessages) fprintf(stderr, THIS ": yuv format is YUY2\n");
				told = 1;
			}
			
			this->convert = convert_methods[this->cm].convert_yuy2;
			frame->vo_frame.pitches[0] = (frame->width > 3) ? (frame->width << 1) : 8;
			frame->vo_frame.pitches[1] = 0;
			frame->vo_frame.pitches[2] = 0;
			frame->vo_frame.base[0] = (uint8_t*) xine_xmalloc_aligned(16,
								frame->vo_frame.pitches[0] * frame->height,
								&(frame->chunk[0]));
			frame->vo_frame.base[1] = NULL;
			frame->vo_frame.base[2] = NULL;
		}
		break;

		default:
		{
			if(!(told))
			{
				if (s_debugMessages) fprintf(stderr, THIS
					": unknown yuv format [%#x], assuming YV12\n",
					frame->format);
				told = 1;
			}

			frame->format = XINE_IMGFMT_YV12;
			goto CHECK_FRAME;
		}
		break;
	}


	if(this->frame_width != this->lastframe_width ||
		this->frame_height != this->lastframe_height)
	{
		release(this->buffer.r);
		release(this->buffer.g);
		release(this->buffer.b);
		release(this->outbuffer);
	}

/* some malloc call can fail;
 * therefore always check each buffer
 */
	if(!(this->buffer.r))
		this->buffer.r  = (uint8_t*) xine_xmalloc(frame->pixels + 4);

	if(!(this->buffer.g))
		this->buffer.g  = (uint8_t*) xine_xmalloc(frame->pixels + 4);

	if(!(this->buffer.b))
		this->buffer.b  = (uint8_t*) xine_xmalloc(frame->pixels + 4);

	if(!(this->outbuffer))
	{
		uint32_t outbuffer_size = (frame->pixels * pack_methods[this->pm].pixelsize)
					   + pack_methods[this->pm].scratch;
		EVAL(this->outbuffer = xine_xmalloc(outbuffer_size));
		clear(this->outbuffer, outbuffer_size);
	}


FAILURE:
	return;
}


/* the only public callback, currently */
static int
rgbout_update_visual(vo_driver_t* vo_driver,
			rgbout_visual_info_t* new_visual)
{
	rgbout_driver_t* this = (rgbout_driver_t*) vo_driver;
	uint32_t i    = 0;
	uint8_t found = 0;
	uint32_t pm   = 0;

	EVAL(vo_driver  != NULL);
	EVAL(new_visual != NULL);
	EVAL(new_visual->callback != NULL);

	for(; i < (sizeof(pack_methods) / sizeof(rgbout_packer_t)); i++)
	{
		if(pack_methods[i].id == new_visual->format)
		{
			found = 1;
			pm = i;

			if(pack_methods[pm].pixelsize != pack_methods[this->pm].pixelsize ||
			   pack_methods[pm].scratch > pack_methods[this->pm].scratch)
			{
				release(this->outbuffer);
			}

			break;
		}
	}

	if(!(found))
	{
		if (s_debugMessages) fprintf(stderr, THIS ": unknown pixel format [%i]\n",
				new_visual->format);
		goto FAILURE;
	}

	this->pm     = pm;
	this->pack   = pack_methods[pm].pack;
	this->levels = new_visual->levels;
	this->render = new_visual->callback;

	return(1);

FAILURE:
	return(0);
}


static void
rgbout_display_frame(vo_driver_t* vo_driver, vo_frame_t* vo_frame)
{
	rgbout_driver_t* this = (rgbout_driver_t*) vo_driver;
	rgbout_frame_t* frame = (rgbout_frame_t*) vo_frame;
#ifdef DEBUG
	static int skip_frames = 2; /* start speed test at frame n+1 */
	static int test_frames = 3; /* test for n frames */
#endif
	

	EVAL(vo_driver != NULL);
	EVAL(vo_frame  != NULL);
	EVAL(this->buffer.r  != NULL);
	EVAL(this->buffer.g  != NULL);
	EVAL(this->buffer.b  != NULL);
	EVAL(this->outbuffer != NULL);

	switch(frame->format)
	{
		case XINE_IMGFMT_YV12:
		{
			EVAL(frame->vo_frame.base[0] != NULL);
			EVAL(frame->vo_frame.base[1] != NULL);
			EVAL(frame->vo_frame.base[2] != NULL);
		}
		break;

		case XINE_IMGFMT_YUY2:
		{
			EVAL(frame->vo_frame.base[0] != NULL);
		}
		break;
	}

#ifdef DEBUG
	
	if(test_frames && skip_frames < 1)
	{
		uint64_t t;

		t = rdtsc();
		this->convert(frame->vo_frame.base, &(this->buffer),
				frame->vo_frame.pitches, frame->width, frame->height);
		t = rdtsc() - t;
		fprintf(stderr, THIS ": [ %lli nanosec.] %s conversion speed test\n",
				t, (convert_methods[this->cm].name)
				    ? convert_methods[this->cm].name : "C");

		if(!(this->levels & PXLEVEL_R))
			clear(this->buffer.r, frame->pixels);
		if(!(this->levels & PXLEVEL_G))
			clear(this->buffer.g, frame->pixels);
		if(!(this->levels & PXLEVEL_B))
			clear(this->buffer.b, frame->pixels);

		t = rdtsc();
		this->pack(&(this->buffer), this->outbuffer, frame->pixels, this->accel);
		t = rdtsc() - t;
		fprintf(stderr, THIS ": [ %lli nanosec.] %s packing speed test\n",
				t, pack_methods[this->pm].name);
		
		test_frames--;
	}
	else
	{
		this->convert(frame->vo_frame.base, &(this->buffer),
				frame->vo_frame.pitches, frame->width, frame->height);

                if(!(this->levels & PXLEVEL_R))
			clear(this->buffer.r, frame->pixels);
		if(!(this->levels & PXLEVEL_G))
			clear(this->buffer.g, frame->pixels);
		if(!(this->levels & PXLEVEL_B))
			clear(this->buffer.b, frame->pixels);
					
		this->pack(&(this->buffer), this->outbuffer, frame->pixels, this->accel);
	}

	skip_frames--;
	
#else /* ! DEBUG */
	
	this->convert(frame->vo_frame.base, &(this->buffer),
				frame->vo_frame.pitches, frame->width, frame->height);

	if(!(this->levels & PXLEVEL_R))
		clear(this->buffer.r, frame->pixels);
	if(!(this->levels & PXLEVEL_G))
		clear(this->buffer.g, frame->pixels);
	if(!(this->levels & PXLEVEL_B))
		clear(this->buffer.b, frame->pixels);

	this->pack(&(this->buffer), this->outbuffer, frame->pixels, this->accel);

#endif /* DEBUG */

	this->render(frame->width, frame->height, this->outbuffer, this->user_data);

FAILURE:
	frame->vo_frame.free(&(frame->vo_frame));
}


static int
rgbout_get_property(vo_driver_t* vo_driver, int property)
{
	return(0);
}


static int
rgbout_set_property(vo_driver_t* vo_driver, int property, int value)
{
	return(0);
}


static void
rgbout_get_property_min_max(vo_driver_t* vo_driver,
			int property, int *min, int *max)
{
	*(min) = 0;
	*(max) = 0;
}


static int
rgbout_gui_data_exchange(vo_driver_t* vo_driver,
				int data_type, void *data)
{
	fprintf(stderr, THIS ": data exchange not implemented, yet\n");
	return(0);
}


static int
rgbout_redraw_needed(vo_driver_t* vo_driver)
{
	/* not needed */
	return(0);
}


static void
rgbout_dispose(vo_driver_t* vo_driver)
{
	rgbout_driver_t* this = (rgbout_driver_t*) vo_driver;

	EVAL(vo_driver != NULL);

	release(this->buffer.r);
	release(this->buffer.g);
	release(this->buffer.b);
	release(this->outbuffer);
	free(this);

FAILURE:
	return;
}


static vo_driver_t*
open_plugin(video_driver_class_t* vo_class, const void *vo_visual)
{
	rgbout_visual_info_t* visual = (rgbout_visual_info_t*) vo_visual;
	rgbout_driver_t* this        = NULL;

	EVAL(vo_class  != NULL);
	EVAL(vo_visual != NULL);
	EVAL(visual->callback != NULL);

	EVAL(this = (rgbout_driver_t*) malloc(sizeof(rgbout_driver_t)));
	clear(this, sizeof(rgbout_driver_t));

	this->vo_driver.get_capabilities     = rgbout_get_capabilities;
	this->vo_driver.alloc_frame          = rgbout_alloc_frame;
	this->vo_driver.update_frame_format  = rgbout_update_frame_format;
	this->vo_driver.overlay_begin        = NULL;
	this->vo_driver.overlay_blend        = NULL;
	this->vo_driver.overlay_end          = NULL;
	this->vo_driver.display_frame        = rgbout_display_frame;
	this->vo_driver.get_property         = rgbout_get_property;
	this->vo_driver.set_property         = rgbout_set_property;
	this->vo_driver.get_property_min_max = rgbout_get_property_min_max;
	this->vo_driver.gui_data_exchange    = rgbout_gui_data_exchange;
	this->vo_driver.redraw_needed        = rgbout_redraw_needed;
	this->vo_driver.dispose              = rgbout_dispose;
	this->update_visual                  = rgbout_update_visual;


	this->accel = xine_mm_accel(); 
        
        // from Ruben,
        // disable MMXEXT since its causing a crash at present under P4's.
        this->accel &= ~MM_MMXEXT;

	{
		uint32_t i = (sizeof(convert_methods) /
				 sizeof(rgbout_converter_t));

		while(i--)
		{
			if((this->accel & convert_methods[i].required_accel) ==
				convert_methods[i].required_accel)
			{
				this->cm      = i;
				this->convert = convert_methods[i].convert_yv12; /* default */

				if(convert_methods[i].name)
				{
					if (s_debugMessages) fprintf(stderr, THIS
						": using %s acceleration\n",
						convert_methods[i].name);
				}

				break;
			}
		}

		if(!(this->convert))
			this->convert = convert_methods[0].convert_yv12;
	}
	
	this->levels = visual->levels;

	if(this->levels & ~(PXLEVEL_ALL))
	{
		if (s_debugMessages) fprintf(stderr, THIS ": wrong levels flag [%#x], assuming PXLEVEL_ALL\n",
				visual->levels);
		this->levels = PXLEVEL_ALL;
	}

	this->user_data = visual->user_data;
	this->render = visual->callback;

	{
		uint32_t i = 0;

		for(; i < (sizeof(pack_methods) / sizeof(rgbout_packer_t)); i++)
		{
			if(pack_methods[i].id == visual->format)
			{
				this->pm   = i;
				this->pack = pack_methods[i].pack;

				if(pack_methods[i].name)
				{
					if (s_debugMessages) fprintf(stderr, THIS
						": packing pixels in %s format\n",
						pack_methods[i].name);
				}

				break;
			}
		}

		if(!(this->pack))
		{
			if (s_debugMessages) fprintf(stderr, THIS
				": unknown rgb pixel format [%i]\n", visual->format);
			goto FAILURE;
		}
	}

	return(&(this->vo_driver));

FAILURE:
	release(this);
	return(NULL);
}


static char*
get_identifier(video_driver_class_t* vo_class)
{
	return("RGBout");
}


static char*
get_description(video_driver_class_t* vo_class)
{
	return("General purpose RGB video output plugin.");
}


static void
dispose_class(video_driver_class_t* vo_class)
{
	rgbout_class_t* rgb_class = (rgbout_class_t*) vo_class;

	EVAL(vo_class != NULL);

	free(rgb_class);

FAILURE:
	return;
}


static void*
init_class(xine_t* xine, void* vo_visual)
{
	rgbout_class_t* rgb_class = NULL;

	EVAL(xine != NULL);
	EVAL(vo_visual != NULL);

	EVAL(rgb_class = (rgbout_class_t*) malloc(sizeof(rgbout_class_t)));
	clear(rgb_class, sizeof(rgbout_class_t));

	rgb_class->driver_class.open_plugin     = open_plugin;
#if XINE_MAJOR_VERSION < 1 || (XINE_MAJOR_VERSION == 1 && XINE_MINOR_VERSION < 2)
	rgb_class->driver_class.get_identifier  = get_identifier;
	rgb_class->driver_class.get_description = get_description;
#else
	rgb_class->driver_class.identifier  = get_identifier(NULL);
	rgb_class->driver_class.description = get_description(NULL);
#endif

	rgb_class->driver_class.dispose         = dispose_class;

	return(rgb_class);

FAILURE:
	release(rgb_class);
	return(NULL);
}



static vo_info_t vo_info_rgbout =
{
	8,
	XINE_VISUAL_TYPE_RGBOUT
};


plugin_info_t xine_plugin_info[] =
{
  /* type, API, "name", version, special_info, init_function */
  { PLUGIN_VIDEO_OUT, 20, "rgb", XINE_VERSION_CODE, &vo_info_rgbout, init_class},
  { PLUGIN_VIDEO_OUT, 21, "rgb", XINE_VERSION_CODE, &vo_info_rgbout, init_class},
  { PLUGIN_NONE, 0, "", 0, NULL, NULL}
};

void register_rgbout_plugin(xine_t *self)
{
     xine_register_plugins(self, xine_plugin_info);
}

#ifdef __cplusplus
}
#endif
