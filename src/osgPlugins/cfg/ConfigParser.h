/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     PRTOKEN_VISUAL = 258,
     PRTOKEN_SET_SIMPLE = 259,
     PRTOKEN_VISUAL_ID = 260,
     PRTOKEN_BUFFER_SIZE = 261,
     PRTOKEN_LEVEL = 262,
     PRTOKEN_RGBA = 263,
     PRTOKEN_DOUBLEBUFFER = 264,
     PRTOKEN_STEREO = 265,
     PRTOKEN_AUX_BUFFERS = 266,
     PRTOKEN_RED_SIZE = 267,
     PRTOKEN_GREEN_SIZE = 268,
     PRTOKEN_BLUE_SIZE = 269,
     PRTOKEN_ALPHA_SIZE = 270,
     PRTOKEN_DEPTH_SIZE = 271,
     PRTOKEN_STENCIL_SIZE = 272,
     PRTOKEN_ACCUM_RED_SIZE = 273,
     PRTOKEN_ACCUM_GREEN_SIZE = 274,
     PRTOKEN_ACCUM_BLUE_SIZE = 275,
     PRTOKEN_ACCUM_ALPHA_SIZE = 276,
     PRTOKEN_SAMPLES = 277,
     PRTOKEN_SAMPLE_BUFFERS = 278,
     PRTOKEN_RENDER_SURFACE = 279,
     PRTOKEN_WINDOW_RECT = 280,
     PRTOKEN_INPUT_RECT = 281,
     PRTOKEN_HOSTNAME = 282,
     PRTOKEN_DISPLAY = 283,
     PRTOKEN_SCREEN = 284,
     PRTOKEN_BORDER = 285,
     PRTOKEN_DRAWABLE_TYPE = 286,
     PRTOKEN_WINDOW_TYPE = 287,
     PRTOKEN_PBUFFER_TYPE = 288,
     PRTOKEN_CAMERA_GROUP = 289,
     PRTOKEN_CAMERA = 290,
     PRTOKEN_PROJECTION_RECT = 291,
     PRTOKEN_LENS = 292,
     PRTOKEN_FRUSTUM = 293,
     PRTOKEN_PERSPECTIVE = 294,
     PRTOKEN_ORTHO = 295,
     PRTOKEN_OFFSET = 296,
     PRTOKEN_ROTATE = 297,
     PRTOKEN_TRANSLATE = 298,
     PRTOKEN_SCALE = 299,
     PRTOKEN_SHEAR = 300,
     PRTOKEN_CLEAR_COLOR = 301,
     PRTOKEN_INPUT_AREA = 302,
     PRTOKEN_ERROR = 303,
     PRTOKEN_INTEGER = 304,
     PRTOKEN_HEX_INTEGER = 305,
     PRTOKEN_FLOAT = 306,
     PRTOKEN_TRUE = 307,
     PRTOKEN_FALSE = 308,
     PRTOKEN_QUOTED_STRING = 309,
     PRTOKEN_STEREO_SYSTEM_COMMANDS = 310,
     PRTOKEN_CUSTOM_FULL_SCREEN_RECTANGLE = 311,
     PRTOKEN_METHOD = 312,
     PRTOKEN_PREMULTIPLY = 313,
     PRTOKEN_POSTMULTIPLY = 314,
     PRTOKEN_OVERRIDE_REDIRECT = 315,
     PRTOKEN_SHARELENS = 316,
     PRTOKEN_SHAREVIEW = 317,
     PRTOKEN_READ_DRAWABLE = 318,
     PRTOKEN_SET_RTT_MODE = 319,
     PRTOKEN_RTT_MODE_NONE = 320,
     PRTOKEN_RTT_MODE_RGB = 321,
     PRTOKEN_RTT_MODE_RGBA = 322,
     PRTOKEN_THREAD_MODEL = 323,
     PRTOKEN_SINGLE_THREADED = 324,
     PRTOKEN_THREAD_PER_CAMERA = 325,
     PRTOKEN_THREAD_PER_RENDER_SURFACE = 326
   };
#endif
#define PRTOKEN_VISUAL 258
#define PRTOKEN_SET_SIMPLE 259
#define PRTOKEN_VISUAL_ID 260
#define PRTOKEN_BUFFER_SIZE 261
#define PRTOKEN_LEVEL 262
#define PRTOKEN_RGBA 263
#define PRTOKEN_DOUBLEBUFFER 264
#define PRTOKEN_STEREO 265
#define PRTOKEN_AUX_BUFFERS 266
#define PRTOKEN_RED_SIZE 267
#define PRTOKEN_GREEN_SIZE 268
#define PRTOKEN_BLUE_SIZE 269
#define PRTOKEN_ALPHA_SIZE 270
#define PRTOKEN_DEPTH_SIZE 271
#define PRTOKEN_STENCIL_SIZE 272
#define PRTOKEN_ACCUM_RED_SIZE 273
#define PRTOKEN_ACCUM_GREEN_SIZE 274
#define PRTOKEN_ACCUM_BLUE_SIZE 275
#define PRTOKEN_ACCUM_ALPHA_SIZE 276
#define PRTOKEN_SAMPLES 277
#define PRTOKEN_SAMPLE_BUFFERS 278
#define PRTOKEN_RENDER_SURFACE 279
#define PRTOKEN_WINDOW_RECT 280
#define PRTOKEN_INPUT_RECT 281
#define PRTOKEN_HOSTNAME 282
#define PRTOKEN_DISPLAY 283
#define PRTOKEN_SCREEN 284
#define PRTOKEN_BORDER 285
#define PRTOKEN_DRAWABLE_TYPE 286
#define PRTOKEN_WINDOW_TYPE 287
#define PRTOKEN_PBUFFER_TYPE 288
#define PRTOKEN_CAMERA_GROUP 289
#define PRTOKEN_CAMERA 290
#define PRTOKEN_PROJECTION_RECT 291
#define PRTOKEN_LENS 292
#define PRTOKEN_FRUSTUM 293
#define PRTOKEN_PERSPECTIVE 294
#define PRTOKEN_ORTHO 295
#define PRTOKEN_OFFSET 296
#define PRTOKEN_ROTATE 297
#define PRTOKEN_TRANSLATE 298
#define PRTOKEN_SCALE 299
#define PRTOKEN_SHEAR 300
#define PRTOKEN_CLEAR_COLOR 301
#define PRTOKEN_INPUT_AREA 302
#define PRTOKEN_ERROR 303
#define PRTOKEN_INTEGER 304
#define PRTOKEN_HEX_INTEGER 305
#define PRTOKEN_FLOAT 306
#define PRTOKEN_TRUE 307
#define PRTOKEN_FALSE 308
#define PRTOKEN_QUOTED_STRING 309
#define PRTOKEN_STEREO_SYSTEM_COMMANDS 310
#define PRTOKEN_CUSTOM_FULL_SCREEN_RECTANGLE 311
#define PRTOKEN_METHOD 312
#define PRTOKEN_PREMULTIPLY 313
#define PRTOKEN_POSTMULTIPLY 314
#define PRTOKEN_OVERRIDE_REDIRECT 315
#define PRTOKEN_SHARELENS 316
#define PRTOKEN_SHAREVIEW 317
#define PRTOKEN_READ_DRAWABLE 318
#define PRTOKEN_SET_RTT_MODE 319
#define PRTOKEN_RTT_MODE_NONE 320
#define PRTOKEN_RTT_MODE_RGB 321
#define PRTOKEN_RTT_MODE_RGBA 322
#define PRTOKEN_THREAD_MODEL 323
#define PRTOKEN_SINGLE_THREADED 324
#define PRTOKEN_THREAD_PER_CAMERA 325
#define PRTOKEN_THREAD_PER_RENDER_SURFACE 326




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
    char * string;
    int    integer;
    float  real;
    bool boolean;
} YYSTYPE;
/* Line 1249 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE ConfigParser_lval;



