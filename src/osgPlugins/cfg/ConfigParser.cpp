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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* If NAME_PREFIX is specified substitute the variables and functions
   names.  */
#define yyparse ConfigParser_parse
#define yylex   ConfigParser_lex
#define yyerror ConfigParser_error
#define yylval  ConfigParser_lval
#define yychar  ConfigParser_char
#define yydebug ConfigParser_debug
#define yynerrs ConfigParser_nerrs


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




/* Copy the first part of user declarations.  */



#ifndef WIN32
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#endif

#ifndef WIN32
#define SUPPORT_CPP 1
#endif

#include <osgDB/fstream>

#include <string.h>
#include <stdio.h>
#include <string>

#include "FlexLexer.h"

#include "CameraConfig.h"


using namespace std;
using namespace osgProducer;

static void ConfigParser_error( const char * );
static CameraConfig *cfg = 0L;
static std::string fileName = "(stdin)";

static yyFlexLexer *flexer = 0L;

static int yylex()
{
    if( flexer == 0L )
        fprintf( stderr, "Internal error!\n" );
    return flexer->yylex();
}



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
    char * string;
    int    integer;
    float  real;
    bool boolean;
} YYSTYPE;
/* Line 191 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */


#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
     || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))                \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)        \
      do                    \
    {                    \
      register YYSIZE_T yyi;        \
      for (yyi = 0; yyi < (Count); yyi++)    \
        (To)[yyi] = (From)[yyi];        \
    }                    \
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)                    \
    do                                    \
      {                                    \
    YYSIZE_T yynewbytes;                        \
    YYCOPY (&yyptr->Stack, Stack, yysize);                \
    Stack = &yyptr->Stack;                        \
    yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
    yyptr += yynewbytes / sizeof (*yyptr);                \
      }                                    \
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  32
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   279

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  76
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  45
/* YYNRULES -- Number of rules. */
#define YYNRULES  122
/* YYNRULES -- Number of states. */
#define YYNSTATES  266

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   326

#define YYTRANSLATE(YYX)                         \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    75,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    72,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    73,     2,    74,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    29,    33,    35,    37,    39,    45,
      50,    52,    53,    55,    58,    59,    66,    68,    71,    72,
      76,    78,    85,    89,    93,   100,   102,   104,   105,   111,
     113,   116,   121,   128,   134,   140,   144,   146,   148,   153,
     155,   158,   159,   168,   179,   186,   195,   204,   215,   220,
     221,   228,   230,   233,   234,   238,   240,   247,   254,   258,
     262,   266,   270,   277,   281,   285,   289,   293,   295,   297,
     299,   301,   303,   304,   311,   312,   318,   320,   324,   326,
     329,   332,   335,   337,   339,   341,   344,   347,   350,   353,
     356,   359,   362,   365,   368,   371,   374,   377,   379,   382,
     383,   389,   391,   394,   398,   400,   402,   404,   406,   408,
     410,   412,   414
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      77,     0,    -1,    78,    -1,    79,    -1,    78,    79,    -1,
     105,    -1,    99,    -1,    87,    -1,   110,    -1,    84,    -1,
      83,    -1,    80,    -1,    81,    -1,    80,    81,    -1,    68,
      82,    72,    -1,    69,    -1,    70,    -1,    71,    -1,    55,
     116,   118,   118,    72,    -1,    34,    73,    85,    74,    -1,
      86,    -1,    -1,    87,    -1,    86,    87,    -1,    -1,    35,
     117,    88,    73,    89,    74,    -1,    90,    -1,    89,    90,
      -1,    -1,    24,   117,    72,    -1,    99,    -1,    36,   114,
     114,   114,   114,    72,    -1,    61,   120,    72,    -1,    62,
     120,    72,    -1,    46,   114,   114,   114,   114,    72,    -1,
      96,    -1,    91,    -1,    -1,    41,    73,    92,    93,    74,
      -1,    94,    -1,    93,    94,    -1,    45,   114,   114,    72,
      -1,    42,   114,   114,   114,   114,    72,    -1,    43,   114,
     114,   114,    72,    -1,    44,   114,   114,   114,    72,    -1,
      57,    95,    72,    -1,    58,    -1,    59,    -1,    37,    73,
      97,    74,    -1,    98,    -1,    97,    98,    -1,    -1,    40,
     114,   114,   114,   114,   114,   114,    72,    -1,    40,   114,
     114,   114,   114,   114,   114,   114,   114,    72,    -1,    39,
     114,   114,   114,   114,    72,    -1,    39,   114,   114,   114,
     114,   114,   114,    72,    -1,    38,   114,   114,   114,   114,
     114,   114,    72,    -1,    38,   114,   114,   114,   114,   114,
     114,   114,   114,    72,    -1,    45,   114,   114,    72,    -1,
      -1,    24,   117,   100,    73,   101,    74,    -1,   102,    -1,
     101,   102,    -1,    -1,     3,   117,    72,    -1,   105,    -1,
      25,   116,   116,   116,   116,    72,    -1,    26,   115,   115,
     115,   115,    72,    -1,    27,   117,    72,    -1,    28,   116,
      72,    -1,    29,   116,    72,    -1,    30,   120,    72,    -1,
      56,   116,   116,   116,   116,    72,    -1,    60,   120,    72,
      -1,    31,   103,    72,    -1,    63,   117,    72,    -1,    64,
     104,    72,    -1,    32,    -1,    33,    -1,    65,    -1,    66,
      -1,    67,    -1,    -1,     3,   117,   106,    73,   108,    74,
      -1,    -1,     3,   107,    73,   108,    74,    -1,   109,    -1,
     108,    75,   109,    -1,     4,    -1,     5,   119,    -1,     6,
     116,    -1,     7,   116,    -1,     8,    -1,     9,    -1,    10,
      -1,    11,   116,    -1,    12,   116,    -1,    13,   116,    -1,
      14,   116,    -1,    15,   116,    -1,    16,   116,    -1,    17,
     116,    -1,    18,   116,    -1,    19,   116,    -1,    20,   116,
      -1,    21,   116,    -1,    22,   116,    -1,    23,    -1,   116,
     116,    -1,    -1,    47,   111,    73,   112,    74,    -1,   113,
      -1,   112,   113,    -1,    24,   117,    72,    -1,    51,    -1,
      49,    -1,    51,    -1,    49,    -1,    54,    -1,    54,    -1,
      50,    -1,    52,    -1,    53,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   144,   144,   147,   147,   150,   150,   150,   150,   150,
     150,   150,   153,   153,   157,   165,   166,   167,   170,   176,
     179,   182,   182,   182,   186,   185,   194,   194,   197,   199,
     203,   207,   211,   215,   219,   223,   224,   228,   227,   237,
     237,   241,   245,   249,   253,   257,   264,   265,   268,   271,
     271,   274,   276,   280,   284,   288,   292,   296,   300,   307,
     306,   317,   318,   321,   323,   327,   331,   335,   339,   343,
     347,   351,   355,   359,   363,   368,   372,   379,   380,   384,
     385,   386,   391,   390,   400,   399,   410,   410,   413,   417,
     422,   426,   431,   435,   439,   443,   447,   452,   457,   461,
     465,   469,   473,   477,   481,   485,   489,   493,   497,   504,
     503,   509,   509,   513,   519,   523,   530,   536,   542,   548,
     555,   563,   563
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "PRTOKEN_VISUAL", "PRTOKEN_SET_SIMPLE", 
  "PRTOKEN_VISUAL_ID", "PRTOKEN_BUFFER_SIZE", "PRTOKEN_LEVEL", 
  "PRTOKEN_RGBA", "PRTOKEN_DOUBLEBUFFER", "PRTOKEN_STEREO", 
  "PRTOKEN_AUX_BUFFERS", "PRTOKEN_RED_SIZE", "PRTOKEN_GREEN_SIZE", 
  "PRTOKEN_BLUE_SIZE", "PRTOKEN_ALPHA_SIZE", "PRTOKEN_DEPTH_SIZE", 
  "PRTOKEN_STENCIL_SIZE", "PRTOKEN_ACCUM_RED_SIZE", 
  "PRTOKEN_ACCUM_GREEN_SIZE", "PRTOKEN_ACCUM_BLUE_SIZE", 
  "PRTOKEN_ACCUM_ALPHA_SIZE", "PRTOKEN_SAMPLES", "PRTOKEN_SAMPLE_BUFFERS", 
  "PRTOKEN_RENDER_SURFACE", "PRTOKEN_WINDOW_RECT", "PRTOKEN_INPUT_RECT", 
  "PRTOKEN_HOSTNAME", "PRTOKEN_DISPLAY", "PRTOKEN_SCREEN", 
  "PRTOKEN_BORDER", "PRTOKEN_DRAWABLE_TYPE", "PRTOKEN_WINDOW_TYPE", 
  "PRTOKEN_PBUFFER_TYPE", "PRTOKEN_CAMERA_GROUP", "PRTOKEN_CAMERA", 
  "PRTOKEN_PROJECTION_RECT", "PRTOKEN_LENS", "PRTOKEN_FRUSTUM", 
  "PRTOKEN_PERSPECTIVE", "PRTOKEN_ORTHO", "PRTOKEN_OFFSET", 
  "PRTOKEN_ROTATE", "PRTOKEN_TRANSLATE", "PRTOKEN_SCALE", "PRTOKEN_SHEAR", 
  "PRTOKEN_CLEAR_COLOR", "PRTOKEN_INPUT_AREA", "PRTOKEN_ERROR", 
  "PRTOKEN_INTEGER", "PRTOKEN_HEX_INTEGER", "PRTOKEN_FLOAT", 
  "PRTOKEN_TRUE", "PRTOKEN_FALSE", "PRTOKEN_QUOTED_STRING", 
  "PRTOKEN_STEREO_SYSTEM_COMMANDS", 
  "PRTOKEN_CUSTOM_FULL_SCREEN_RECTANGLE", "PRTOKEN_METHOD", 
  "PRTOKEN_PREMULTIPLY", "PRTOKEN_POSTMULTIPLY", 
  "PRTOKEN_OVERRIDE_REDIRECT", "PRTOKEN_SHARELENS", "PRTOKEN_SHAREVIEW", 
  "PRTOKEN_READ_DRAWABLE", "PRTOKEN_SET_RTT_MODE", 
  "PRTOKEN_RTT_MODE_NONE", "PRTOKEN_RTT_MODE_RGB", 
  "PRTOKEN_RTT_MODE_RGBA", "PRTOKEN_THREAD_MODEL", 
  "PRTOKEN_SINGLE_THREADED", "PRTOKEN_THREAD_PER_CAMERA", 
  "PRTOKEN_THREAD_PER_RENDER_SURFACE", "';'", "'{'", "'}'", "','", 
  "$accept", "config", "entries", "entry", "system_params", 
  "system_param", "threadModelDirective", "stereo_param", "camera_group", 
  "camera_group_attributes", "cameras", "camera", "@1", 
  "camera_attributes", "camera_attribute", "camera_offset", "@2", 
  "camera_offset_attributes", "camera_offset_attribute", 
  "offset_multiply_method", "lens", "lens_attributes", "lens_attribute", 
  "render_surface", "@3", "render_surface_attributes", 
  "render_surface_attribute", "drawableType", "rtt_mode", "visual", "@4", 
  "@5", "visual_attributes", "visual_attribute", "input_area", "@6", 
  "input_area_entries", "input_area_entry", "real", "floatparam", 
  "intparam", "name", "string", "hex_integer", "bool", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,    59,   123,   125,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    76,    77,    78,    78,    79,    79,    79,    79,    79,
      79,    79,    80,    80,    81,    82,    82,    82,    83,    84,
      85,    86,    86,    86,    88,    87,    89,    89,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    92,    91,    93,
      93,    94,    94,    94,    94,    94,    95,    95,    96,    97,
      97,    98,    98,    98,    98,    98,    98,    98,    98,   100,
      99,   101,   101,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   103,   103,   104,
     104,   104,   106,   105,   107,   105,   108,   108,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   111,
     110,   112,   112,   113,   114,   114,   115,   116,   117,   118,
     119,   120,   120
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     3,     1,     1,     1,     5,     4,
       1,     0,     1,     2,     0,     6,     1,     2,     0,     3,
       1,     6,     3,     3,     6,     1,     1,     0,     5,     1,
       2,     4,     6,     5,     5,     3,     1,     1,     4,     1,
       2,     0,     8,    10,     6,     8,     8,    10,     4,     0,
       6,     1,     2,     0,     3,     1,     6,     6,     3,     3,
       3,     3,     6,     3,     3,     3,     3,     1,     1,     1,
       1,     1,     0,     6,     0,     5,     1,     3,     1,     2,
       2,     2,     1,     1,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     2,     0,
       5,     1,     2,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,    84,     0,     0,     0,   109,     0,     0,     0,     2,
       3,    11,    12,    10,     9,     7,     6,     5,     8,   118,
       0,    82,    59,    21,    24,     0,   117,     0,    15,    16,
      17,     0,     1,     4,    13,     0,     0,     0,     0,    20,
      22,     0,     0,   119,     0,    14,    88,     0,     0,     0,
      92,    93,    94,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   107,     0,    86,     0,     0,
      63,    19,    23,    28,     0,     0,   111,     0,   120,    89,
      90,    91,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,    85,     0,   108,     0,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    61,    65,     0,     0,     0,     0,     0,     0,     0,
       0,    26,    36,    35,    30,     0,   110,   112,    18,    87,
      83,    82,     0,   116,     0,     0,     0,     0,   121,   122,
       0,    77,    78,     0,     0,     0,     0,    79,    80,    81,
       0,    60,    62,    59,   115,   114,     0,    51,    37,     0,
       0,     0,    25,    27,   113,    64,     0,     0,    68,    69,
      70,    71,    74,     0,    73,    75,    76,    29,     0,     0,
       0,     0,     0,     0,    49,     0,     0,    32,    33,     0,
       0,     0,     0,     0,     0,     0,     0,    48,    50,     0,
       0,     0,     0,     0,     0,    39,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    46,
      47,     0,    38,    40,     0,    66,    67,    72,    31,     0,
       0,     0,    58,     0,     0,     0,     0,    45,    34,     0,
       0,     0,     0,     0,     0,    41,     0,    54,     0,     0,
       0,    43,    44,     0,     0,     0,    42,    56,     0,    55,
      52,     0,     0,     0,    57,    53
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     8,     9,    10,    11,    12,    31,    13,    14,    38,
      39,    15,    41,   120,   121,   122,   185,   204,   205,   221,
     123,   183,   184,    16,    37,   110,   111,   143,   150,    17,
      36,    20,    66,    67,    18,    25,    75,    76,   156,   134,
      68,    21,    44,    79,   140
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -130
static const short yypact[] =
{
      36,   -51,   -51,   -66,   -51,  -130,   -33,   -34,    26,    36,
    -130,    10,  -130,  -130,  -130,  -130,  -130,  -130,  -130,  -130,
      13,  -130,  -130,    57,  -130,    22,  -130,    45,  -130,  -130,
    -130,    39,  -130,  -130,  -130,   230,    50,    51,    60,    57,
    -130,    52,   111,  -130,    45,  -130,  -130,    86,   -33,   -33,
    -130,  -130,  -130,   -33,   -33,   -33,   -33,   -33,   -33,   -33,
     -33,   -33,   -33,   -33,   -33,  -130,   -62,  -130,   -33,   230,
     202,  -130,  -130,    66,   -51,   -10,  -130,    65,  -130,  -130,
    -130,  -130,  -130,  -130,  -130,  -130,  -130,  -130,  -130,  -130,
    -130,  -130,  -130,  -130,  -130,   230,  -130,   -54,   -51,   -33,
      96,   -51,   -33,   -33,    16,    73,   -33,    16,   -51,    15,
       3,  -130,  -130,   -51,    24,    76,    77,    24,    16,    16,
      48,  -130,  -130,  -130,  -130,    67,  -130,  -130,  -130,  -130,
    -130,    79,   -33,  -130,    96,    80,    81,    87,  -130,  -130,
      93,  -130,  -130,    94,   -33,   100,   109,  -130,  -130,  -130,
     110,  -130,  -130,   112,  -130,  -130,    24,   131,  -130,    24,
     114,   115,  -130,  -130,  -130,  -130,   -33,    96,  -130,  -130,
    -130,  -130,  -130,   -33,  -130,  -130,  -130,  -130,    24,    24,
      24,    24,    24,   -30,  -130,    88,    24,  -130,  -130,   -33,
      96,   -33,    24,    24,    24,    24,    24,  -130,  -130,    24,
      24,    24,    24,    61,    72,  -130,    24,   116,   117,   118,
     122,    24,    24,    24,   126,    24,    24,    24,    24,  -130,
    -130,   127,  -130,  -130,   128,  -130,  -130,  -130,  -130,    24,
      24,    24,  -130,    24,    24,    24,   134,  -130,  -130,    24,
     -32,    24,    24,   135,   137,  -130,    24,  -130,    24,    24,
     140,  -130,  -130,   -27,   141,   -26,  -130,  -130,    24,  -130,
    -130,    24,   142,   144,  -130,  -130
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -130,  -130,  -130,   209,  -130,   208,  -130,  -130,  -130,  -130,
    -130,   -12,  -130,  -130,   101,  -130,  -130,  -130,    18,  -130,
    -130,  -130,    41,   -55,  -130,  -130,   145,  -130,  -130,   -69,
    -130,  -130,   156,   159,  -130,  -130,  -130,   151,   -38,  -129,
      -6,     0,   212,  -130,   -31
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned short yytable[] =
{
      27,   112,    22,    19,    24,   167,    98,    23,   179,   180,
     181,    40,    94,    95,    74,   182,    26,   154,   124,   155,
     130,    95,   154,   154,   155,   155,    32,    72,    99,   100,
     101,   102,   103,   104,   105,    28,    29,    30,   190,     1,
     247,   112,    80,    81,   197,   257,   260,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,   106,
       2,   208,    96,   107,   126,   124,   108,   109,   138,   139,
       3,     4,   113,   154,   125,   155,   145,   151,     7,   159,
     147,   148,   149,     5,   114,   115,    35,   160,   161,   116,
     113,     6,     4,   132,   117,    42,   136,   137,   131,    43,
     144,   135,   114,   115,     7,   141,   142,   116,   146,   118,
     119,    45,   117,   153,   199,   200,   201,   202,   178,   219,
     220,   186,   162,    69,    70,    73,   166,   118,   119,   203,
     199,   200,   201,   202,    71,    74,    78,   128,   173,   164,
     192,   193,   194,   195,   196,   203,   222,   133,   206,   157,
     158,   165,   168,   169,   210,   211,   212,   213,   214,   170,
     189,   215,   216,   217,   218,   171,   172,   191,   224,   179,
     180,   181,   174,   229,   230,   231,   182,   233,   234,   235,
     236,   175,   176,   207,   177,   209,   187,   188,   225,   226,
     227,   239,   240,   241,   228,   242,   243,   244,   232,   237,
     238,   246,   248,   249,   250,    98,   245,   251,   253,   252,
     254,   255,   256,   259,   264,   258,   265,   261,    33,    34,
     262,   163,   223,   263,   198,    97,   127,    99,   100,   101,
     102,   103,   104,   105,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,   129,   152,    77,     0,   106,     0,
       0,     0,   107,     0,     0,   108,   109,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    26
};

static const short yycheck[] =
{
       6,    70,     2,    54,     4,   134,     3,    73,    38,    39,
      40,    23,    74,    75,    24,    45,    49,    49,    73,    51,
      74,    75,    49,    49,    51,    51,     0,    39,    25,    26,
      27,    28,    29,    30,    31,    69,    70,    71,   167,     3,
      72,   110,    48,    49,    74,    72,    72,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    56,
      24,   190,    68,    60,    74,   120,    63,    64,    52,    53,
      34,    35,    24,    49,    74,    51,   107,    74,    68,   117,
      65,    66,    67,    47,    36,    37,    73,   118,   119,    41,
      24,    55,    35,    99,    46,    73,   102,   103,    98,    54,
     106,   101,    36,    37,    68,    32,    33,    41,   108,    61,
      62,    72,    46,   113,    42,    43,    44,    45,   156,    58,
      59,   159,    74,    73,    73,    73,   132,    61,    62,    57,
      42,    43,    44,    45,    74,    24,    50,    72,   144,    72,
     178,   179,   180,   181,   182,    57,    74,    51,   186,    73,
      73,    72,    72,    72,   192,   193,   194,   195,   196,    72,
     166,   199,   200,   201,   202,    72,    72,   173,   206,    38,
      39,    40,    72,   211,   212,   213,    45,   215,   216,   217,
     218,    72,    72,   189,    72,   191,    72,    72,    72,    72,
      72,   229,   230,   231,    72,   233,   234,   235,    72,    72,
      72,   239,   240,   241,   242,     3,    72,    72,   246,    72,
     248,   249,    72,    72,    72,   253,    72,   255,     9,    11,
     258,   120,   204,   261,   183,    69,    75,    25,    26,    27,
      28,    29,    30,    31,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    95,   110,    44,    -1,    56,    -1,
      -1,    -1,    60,    -1,    -1,    63,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,    24,    34,    35,    47,    55,    68,    77,    78,
      79,    80,    81,    83,    84,    87,    99,   105,   110,    54,
     107,   117,   117,    73,   117,   111,    49,   116,    69,    70,
      71,    82,     0,    79,    81,    73,   106,   100,    85,    86,
      87,    88,    73,    54,   118,    72,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,   108,   109,   116,    73,
      73,    74,    87,    73,    24,   112,   113,   118,    50,   119,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,    74,    75,   116,   108,     3,    25,
      26,    27,    28,    29,    30,    31,    56,    60,    63,    64,
     101,   102,   105,    24,    36,    37,    41,    46,    61,    62,
      89,    90,    91,    96,    99,   117,    74,   113,    72,   109,
      74,   117,   116,    51,   115,   117,   116,   116,    52,    53,
     120,    32,    33,   103,   116,   120,   117,    65,    66,    67,
     104,    74,   102,   117,    49,    51,   114,    73,    73,   114,
     120,   120,    74,    90,    72,    72,   116,   115,    72,    72,
      72,    72,    72,   116,    72,    72,    72,    72,   114,    38,
      39,    40,    45,    97,    98,    92,   114,    72,    72,   116,
     115,   116,   114,   114,   114,   114,   114,    74,    98,    42,
      43,    44,    45,    57,    93,    94,   114,   116,   115,   116,
     114,   114,   114,   114,   114,   114,   114,   114,   114,    58,
      59,    95,    74,    94,   114,    72,    72,    72,    72,   114,
     114,   114,    72,   114,   114,   114,   114,    72,    72,   114,
     114,   114,   114,   114,   114,    72,   114,    72,   114,   114,
     114,    72,    72,   114,   114,   114,    72,    72,   114,    72,
      72,   114,   114,   114,    72,    72
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok        (yyerrstatus = 0)
#define yyclearin    (yychar = YYEMPTY)
#define YYEMPTY        (-2)
#define YYEOF        0

#define YYACCEPT    goto yyacceptlab
#define YYABORT        goto yyabortlab
#define YYERROR        goto yyerrlab1

/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL        goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                    \
do                                \
  if (yychar == YYEMPTY && yylen == 1)                \
    {                                \
      yychar = (Token);                        \
      yylval = (Value);                        \
      yytoken = YYTRANSLATE (yychar);                \
      YYPOPSTACK;                        \
      goto yybackup;                        \
    }                                \
  else                                \
    {                                 \
      yyerror ("syntax error: cannot back up");\
      YYERROR;                            \
    }                                \
while (0)

#define YYTERROR    1
#define YYERRCODE    256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)            \
do {                        \
  if (yydebug)                    \
    YYFPRINTF Args;                \
} while (0)

# define YYDSYMPRINT(Args)            \
do {                        \
  if (yydebug)                    \
    yysymprint Args;                \
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)        \
do {                                \
  if (yydebug)                            \
    {                                \
      YYFPRINTF (stderr, "%s ", Title);                \
      yysymprint (stderr,                     \
                  Token, Value);    \
      YYFPRINTF (stderr, "\n");                    \
    }                                \
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                \
do {                                \
  if (yydebug)                            \
    yy_stack_print ((Bottom), (Top));                \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)        \
do {                    \
  if (yydebug)                \
    yy_reduce_print (Rule);        \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef    YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

}

/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short    yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;        /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
    /* Give user a chance to reallocate the stack. Use copies of
       these so that the &'s don't force the real ones into
       memory.  */
    YYSTYPE *yyvs1 = yyvs;
    short *yyss1 = yyss;


    /* Each stack pointer address is followed by the size of the
       data in use in that stack, in bytes.  This used to be a
       conditional around just the two extra args, but that might
       be undefined if yyoverflow is a macro.  */
    yyoverflow ("parser stack overflow",
            &yyss1, yysize * sizeof (*yyssp),
            &yyvs1, yysize * sizeof (*yyvsp),

            &yystacksize);

    yyss = yyss1;
    yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
    goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
    yystacksize = YYMAXDEPTH;

      {
    short *yyss1 = yyss;
    union yyalloc *yyptr =
      (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
    if (! yyptr)
      goto yyoverflowlab;
    YYSTACK_RELOCATE (yyss);
    YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
    if (yyss1 != yyssa)
      YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
          (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
    YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
    goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 14:

    {
            osgProducer::CameraGroup::ThreadModel tm = (osgProducer::CameraGroup::ThreadModel)yyvsp[-1].integer;
            cfg->setThreadModelDirective( tm );
        ;}
    break;

  case 15:

    { yyval.integer = CameraGroup::SingleThreaded; ;}
    break;

  case 16:

    { yyval.integer = CameraGroup::ThreadPerCamera; ;}
    break;

  case 17:

    { yyval.integer = CameraGroup::ThreadPerRenderSurface; ;}
    break;

  case 18:

    {
           cfg->addStereoSystemCommand( yyvsp[-3].integer, yyvsp[-2].string, yyvsp[-1].string );
        ;}
    break;

  case 24:

    {
        cfg->beginCamera( yyvsp[0].string );
     ;}
    break;

  case 25:

    {
        cfg->endCamera();
     ;}
    break;

  case 29:

    {
        cfg->setCameraRenderSurface( yyvsp[-1].string );
    ;}
    break;

  case 30:

    {
        cfg->setCameraRenderSurface();
    ;}
    break;

  case 31:

    {
        cfg->setCameraProjectionRectangle( yyvsp[-4].real, yyvsp[-3].real, yyvsp[-2].real, yyvsp[-1].real );
    ;}
    break;

  case 32:

    {
        cfg->setCameraShareLens( yyvsp[-1].boolean );
    ;}
    break;

  case 33:

    {
        cfg->setCameraShareView( yyvsp[-1].boolean );
    ;}
    break;

  case 34:

    {
        cfg->setCameraClearColor( yyvsp[-4].real, yyvsp[-3].real, yyvsp[-2].real, yyvsp[-1].real );
    ;}
    break;

  case 37:

    {
        cfg->beginCameraOffset();
    ;}
    break;

  case 38:

    {
        cfg->endCameraOffset();
    ;}
    break;

  case 41:

    {
        cfg->shearCameraOffset( yyvsp[-2].real, yyvsp[-1].real );
      ;}
    break;

  case 42:

    {
        cfg->rotateCameraOffset( yyvsp[-4].real, yyvsp[-3].real, yyvsp[-2].real, yyvsp[-1].real );
      ;}
    break;

  case 43:

    {
        cfg->translateCameraOffset( yyvsp[-3].real, yyvsp[-2].real, yyvsp[-1].real );
      ;}
    break;

  case 44:

    {
        cfg->scaleCameraOffset( yyvsp[-3].real, yyvsp[-2].real, yyvsp[-1].real );
      ;}
    break;

  case 45:

    {
        cfg->setCameraOffsetMultiplyMethod( (osgProducer::Camera::Offset::MultiplyMethod)yyvsp[-1].integer );
      ;}
    break;

  case 46:

    { yyval.integer = osgProducer::Camera::Offset::PreMultiply; ;}
    break;

  case 47:

    { yyval.integer = osgProducer::Camera::Offset::PostMultiply; ;}
    break;

  case 52:

    {
          cfg->setCameraOrtho( yyvsp[-6].real, yyvsp[-5].real, yyvsp[-4].real, yyvsp[-3].real, yyvsp[-2].real, yyvsp[-1].real );
      ;}
    break;

  case 53:

    {
          cfg->setCameraOrtho( yyvsp[-8].real, yyvsp[-7].real, yyvsp[-6].real, yyvsp[-5].real, yyvsp[-4].real, yyvsp[-3].real, yyvsp[-2].real, yyvsp[-1].real );
      ;}
    break;

  case 54:

    {
          cfg->setCameraPerspective( yyvsp[-4].real, yyvsp[-3].real, yyvsp[-2].real, yyvsp[-1].real );
      ;}
    break;

  case 55:

    {
          cfg->setCameraPerspective( yyvsp[-6].real, yyvsp[-5].real, yyvsp[-4].real, yyvsp[-3].real, yyvsp[-2].real, yyvsp[-1].real );
      ;}
    break;

  case 56:

    {
          cfg->setCameraFrustum( yyvsp[-6].real, yyvsp[-5].real, yyvsp[-4].real, yyvsp[-3].real, yyvsp[-2].real, yyvsp[-1].real );
      ;}
    break;

  case 57:

    {
          cfg->setCameraFrustum( yyvsp[-8].real, yyvsp[-7].real, yyvsp[-6].real, yyvsp[-5].real, yyvsp[-4].real, yyvsp[-3].real, yyvsp[-2].real, yyvsp[-1].real );
      ;}
    break;

  case 58:

    {
          cfg->setCameraLensShear( yyvsp[-2].real, yyvsp[-1].real );
      ;}
    break;

  case 59:

    {
         cfg->beginRenderSurface( yyvsp[0].string );
     ;}
    break;

  case 60:

    {
         cfg->endRenderSurface();
     ;}
    break;

  case 64:

    {
        cfg->setRenderSurfaceVisualChooser( yyvsp[-1].string );
    ;}
    break;

  case 65:

    {
        cfg->setRenderSurfaceVisualChooser();
    ;}
    break;

  case 66:

    {
        cfg->setRenderSurfaceWindowRectangle( yyvsp[-4].integer, yyvsp[-3].integer, yyvsp[-2].integer, yyvsp[-1].integer );
    ;}
    break;

  case 67:

    {
        cfg->setRenderSurfaceInputRectangle( yyvsp[-4].real, yyvsp[-3].real, yyvsp[-2].real, yyvsp[-1].real );
    ;}
    break;

  case 68:

    {
        cfg->setRenderSurfaceHostName( std::string(yyvsp[-1].string) );
    ;}
    break;

  case 69:

    {
        cfg->setRenderSurfaceDisplayNum( yyvsp[-1].integer );
    ;}
    break;

  case 70:

    {
        cfg->setRenderSurfaceScreen( yyvsp[-1].integer );
    ;}
    break;

  case 71:

    {    
        cfg->setRenderSurfaceBorder( yyvsp[-1].boolean );
    ;}
    break;

  case 72:

    {
        cfg->setRenderSurfaceCustomFullScreenRectangle( yyvsp[-4].integer, yyvsp[-3].integer, yyvsp[-2].integer, yyvsp[-1].integer );
    ;}
    break;

  case 73:

    {
        cfg->setRenderSurfaceOverrideRedirect( yyvsp[-1].boolean );
    ;}
    break;

  case 74:

    {
        osgProducer::RenderSurface::DrawableType drawableType = (RenderSurface::DrawableType)yyvsp[-1].integer;
        cfg->setRenderSurfaceDrawableType( drawableType );
    ;}
    break;

  case 75:

    {
        cfg->setRenderSurfaceReadDrawable( yyvsp[-1].string );
    ;}
    break;

  case 76:

    {
        cfg->setRenderSurfaceRenderToTextureMode( (osgProducer::RenderSurface::RenderToTextureMode)yyvsp[-1].integer );
    ;}
    break;

  case 77:

    { yyval.integer =  RenderSurface::DrawableType_Window; ;}
    break;

  case 78:

    { yyval.integer =  RenderSurface::DrawableType_PBuffer; ;}
    break;

  case 79:

    { yyval.integer = RenderSurface::RenderToTextureMode_None; ;}
    break;

  case 80:

    { yyval.integer = RenderSurface::RenderToRGBTexture; ;}
    break;

  case 81:

    { yyval.integer = RenderSurface::RenderToRGBATexture; ;}
    break;

  case 82:

    {
         cfg->beginVisual( yyvsp[0].string );
     ;}
    break;

  case 83:

    {
         cfg->endVisual();
     ;}
    break;

  case 84:

    {
         cfg->beginVisual();
     ;}
    break;

  case 85:

    {
         cfg->endVisual();
     ;}
    break;

  case 88:

    {
          cfg->setVisualSimpleConfiguration();
      ;}
    break;

  case 89:

    {
          cfg->setVisualByID( yyvsp[0].integer );
      ;}
    break;

  case 90:

    {
          cfg->addVisualAttribute( VisualChooser::BufferSize, yyvsp[0].integer );
      ;}
    break;

  case 91:

    {
          cfg->addVisualAttribute( VisualChooser::Level, yyvsp[0].integer );
      ;}
    break;

  case 92:

    {
          cfg->addVisualAttribute( VisualChooser::RGBA );
      ;}
    break;

  case 93:

    {
          cfg->addVisualAttribute( VisualChooser::DoubleBuffer );
      ;}
    break;

  case 94:

    {
          cfg->addVisualAttribute( VisualChooser::Stereo );
      ;}
    break;

  case 95:

    {
          cfg->addVisualAttribute( VisualChooser::AuxBuffers, yyvsp[0].integer );
      ;}
    break;

  case 96:

    {
          cfg->addVisualAttribute( VisualChooser::RedSize, yyvsp[0].integer );
      ;}
    break;

  case 97:

    {
          cfg->addVisualAttribute( VisualChooser::GreenSize, yyvsp[0].integer );
      ;}
    break;

  case 98:

    {
          cfg->addVisualAttribute( VisualChooser::BlueSize, yyvsp[0].integer );
      ;}
    break;

  case 99:

    {
          cfg->addVisualAttribute( VisualChooser::AlphaSize, yyvsp[0].integer );
      ;}
    break;

  case 100:

    {
          cfg->addVisualAttribute( VisualChooser::DepthSize, yyvsp[0].integer );
      ;}
    break;

  case 101:

    {
          cfg->addVisualAttribute( VisualChooser::StencilSize, yyvsp[0].integer );
      ;}
    break;

  case 102:

    {
          cfg->addVisualAttribute( VisualChooser::AccumRedSize, yyvsp[0].integer );
      ;}
    break;

  case 103:

    {
          cfg->addVisualAttribute( VisualChooser::AccumGreenSize, yyvsp[0].integer );
      ;}
    break;

  case 104:

    {
          cfg->addVisualAttribute( VisualChooser::AccumBlueSize, yyvsp[0].integer );
      ;}
    break;

  case 105:

    {
          cfg->addVisualAttribute( VisualChooser::AccumAlphaSize, yyvsp[0].integer );
      ;}
    break;

  case 106:

    {
          cfg->addVisualAttribute( VisualChooser::Samples, yyvsp[0].integer );
      ;}
    break;

  case 107:

    {
          cfg->addVisualAttribute( VisualChooser::SampleBuffers );
      ;}
    break;

  case 108:

    {
          cfg->addVisualExtendedAttribute( yyvsp[-1].integer, yyvsp[0].integer );
      ;}
    break;

  case 109:

    { cfg->beginInputArea(); ;}
    break;

  case 110:

    { cfg->endInputArea(); ;}
    break;

  case 113:

    {
            cfg->addInputAreaEntry( yyvsp[-1].string );
        ;}
    break;

  case 114:

    {
        yyval.real = osg::asciiToFloat(flexer->YYText());
    ;}
    break;

  case 115:

    {
        yyval.real = osg::asciiToFloat(flexer->YYText());
    ;}
    break;

  case 116:

    {
        yyval.real = osg::asciiToFloat(flexer->YYText());
    ;}
    break;

  case 117:

    {
        yyval.integer = atoi( flexer->YYText() );
    ;}
    break;

  case 118:

    {
        yyval.string = strdup( flexer->YYText() );
    ;}
    break;

  case 119:

    {
        yyval.string = strdup( flexer->YYText() );
    ;}
    break;

  case 120:

    {
        unsigned int n;
        sscanf( flexer->YYText(), "0x%x", &n );
        yyval.integer = n;
    ;}
    break;

  case 121:

    { yyval.boolean = true;;}
    break;

  case 122:

    { yyval.boolean = false; ;}
    break;


    }

/* Line 991 of yacc.c.  */


  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
    {
      YYSIZE_T yysize = 0;
      int yytype = YYTRANSLATE (yychar);
      char *yymsg;
      int yyx, yycount;

      yycount = 0;
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  */
      for (yyx = yyn < 0 ? -yyn : 0;
           yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
        if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
          yysize += yystrlen (yytname[yyx]) + 15, yycount++;
      yysize += yystrlen ("syntax error, unexpected ") + 1;
      yysize += yystrlen (yytname[yytype]);
      yymsg = (char *) YYSTACK_ALLOC (yysize);
      if (yymsg != 0)
        {
          char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
          yyp = yystpcpy (yyp, yytname[yytype]);

          if (yycount < 5)
        {
          yycount = 0;
          for (yyx = yyn < 0 ? -yyn : 0;
               yyx < (int) (sizeof (yytname) / sizeof (char *));
               yyx++)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
              {
            const char *yyq = ! yycount ? ", expecting " : " or ";
            yyp = yystpcpy (yyp, yyq);
            yyp = yystpcpy (yyp, yytname[yyx]);
            yycount++;
              }
        }
          yyerror (yymsg);
          YYSTACK_FREE (yymsg);
        }
      else
        yyerror ("syntax error; also virtual memory exhausted");
    }
      else
#endif /* YYERROR_VERBOSE */
    yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
     error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
      /* Pop the error token.  */
          YYPOPSTACK;
      /* Pop the rest of the stack.  */
      while (yyss < yyssp)
        {
          YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
          yydestruct (yystos[*yyssp], yyvsp);
          YYPOPSTACK;
        }
      YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;

#if 0
/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__) \
    && !defined __cplusplus
  __attribute__ ((__unused__))
#endif


  goto yyerrlab2;
#endif

/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
  yyerrstatus = 3;    /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
    {
      yyn += YYTERROR;
      if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
        {
          yyn = yytable[yyn];
          if (0 < yyn)
        break;
        }
    }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
    YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}





static void yyerror( const char *errmsg )
{
    fprintf( stderr, 
        "CameraConfig::parseFile(\"%s\") : %s - Line %d at or before \"%s\"\n",
        fileName.c_str(),
        errmsg, 
        flexer->lineno(),
        flexer->YYText() );
}

bool CameraConfig::parseFile( const std::string &file )
{
    fileName.clear();

    fileName = findFile(file);

    if( fileName.empty() )
    {
        fprintf( stderr, "CameraConfig::parseFile() - Can't find file \"%s\".\n", file.c_str() );
        return false;
    }

    bool retval = true;

#if defined (SUPPORT_CPP)

    const char *cpp_path =
  #if defined(__APPLE__)
                "/usr/bin/cpp";
  #else
                "/lib/cpp";
  #endif

    if( access( cpp_path, X_OK ) == 0 )
    {

        int pd[2];
        int result = pipe( pd );

        flexer = new yyFlexLexer;
        if( fork() == 0 )
        {
            // we don't want to read from the pipe in the child, so close it.
            close( pd[0] );
            close( 1 );
            result = dup( pd[1] );


            /* This was here to allow reading a config file from stdin.
             * This has never been directly supported, so commenting out.
            if( fileName.empty() )
                execlp( cpp_path, "cpp",  "-P", 0L );
            else
            */
            execlp( cpp_path, "cpp",  "-P", fileName.c_str(), (char *)NULL );

            // This should not execute unless an error happens
            perror( "execlp" );
        }
        else
        {
            close( pd[1]);
            close( 0 );
            result = dup( pd[0] );

            cfg = this;

            retval = ConfigParser_parse() == 0 ? true : false;

            // Wait on child process to finish
            int stat;
            wait( &stat );
        }
    }
    else
#endif
    {
        osgDB::ifstream ifs(fileName.c_str());
        flexer = new yyFlexLexer(&ifs);
        cfg = this;
        retval = ConfigParser_parse() == 0 ? true : false;
        ifs.close();
        delete flexer;
    }
    return retval;
}


