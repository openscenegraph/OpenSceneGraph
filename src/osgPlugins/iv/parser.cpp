/* A Bison parser, made from parser.y
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

# define	STRING	257
# define	QUOTED_STRING	258
# define	FLOAT	259
# define	INT	260
# define	SEPARATOR	261
# define	DEF	262
# define	UN_MATERIAL	263
# define	DIFFUSE_COLOR	264
# define	COORDINATE3	265
# define	INDEXED_FACE_SET	266
# define	INDEXED_TRIANGLE_STRIP_SET	267
# define	A_POINT	268
# define	COORD_INDEX	269
# define	TEXTURE_COORD_INDEX	270
# define	NORMAL_INDEX	271
# define	TEXTURE_COORDINATE	272
# define	TEXTURE2	273
# define	MATRIX_TRANSFORM	274
# define	MATRIX	275
# define	LISTA_VACIA	276
# define	FINPOLY	277
# define	TWO_SIDED	278
# define	VECTOR	279
# define	VRML_HEADER	280
# define	TRANSFORM	281
# define	USE	282

#line 1 "parser.y"

/*
 * osgDB::wrl - a VRML 1.0 loader for OpenSceneGraph
 * Copyright (C) 2002 Ruben Lopez <ryu@gpul.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef WIN32
#  pragma warning (disable:4786)
#  pragma warning (disable:4541)
#endif

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include "mynode.h"
#include <stdlib.h>
#include "geometry.h"
#include "nodecache.h"

#include "material.h"
#include "coordinate3.h"
#include "separator.h"
#include "matrixtransform.h"
#include "indexedfaceset.h"
#include "indexedtristripset.h"
#include "texturecoordinate.h"
#include "texture2.h"
#include "transform.h"

#include "atrfloat.h"
#include "atrstring.h"
#include "atrvec.h"
#include "atrvec3list.h"
#include "string.h"
extern int yyline;
extern int yylex();
void yyerror(char *str,...);
static MyNode *root_node;


#line 54 "parser.y"
#ifndef YYSTYPE
typedef union {
  char *s_value;
  float f_value;
  MyNode *nodo;
  Attribute *attribute;
  VertexList *vlist;
  VertexIndexList *vindex;
  TextureCoordList *tcoord;
  PolygonList *plist;
  Matrix matrix;
  int i_value;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
#ifndef YYDEBUG
# define YYDEBUG 1
#endif



#define	YYFINAL		140
#define	YYFLAG		-32768
#define	YYNTBASE	33

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 282 ? yytranslate[x] : 42)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    31,     2,    32,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    29,     2,    30,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     3,     8,    13,    21,    29,    41,    53,    62,
      75,    88,    96,   108,   120,   126,   134,   139,   144,   149,
     150,   153,   158,   161,   165,   170,   175,   176,   179,   180,
     183,   185,   202,   205,   208,   212,   217,   223,   226,   231,
     235
};
static const short yyrhs[] =
{
      26,    34,     0,     9,    29,    35,    30,     0,    19,    29,
      35,    30,     0,    11,    29,    14,    31,    36,    32,    30,
       0,    12,    29,    15,    31,    37,    32,    30,     0,    12,
      29,    15,    31,    37,    32,    16,    31,    37,    32,    30,
       0,    12,    29,    15,    31,    37,    32,    17,    31,    37,
      32,    30,     0,    12,    29,    24,    15,    31,    37,    32,
      30,     0,    12,    29,    24,    15,    31,    37,    32,    16,
      31,    37,    32,    30,     0,    12,    29,    24,    15,    31,
      37,    32,    17,    31,    37,    32,    30,     0,    13,    29,
      15,    31,    37,    32,    30,     0,    13,    29,    15,    31,
      37,    32,    16,    31,    37,    32,    30,     0,    13,    29,
      15,    31,    37,    32,    17,    31,    37,    32,    30,     0,
      20,    29,    21,    39,    30,     0,    18,    29,    14,    31,
      41,    32,    30,     0,    27,    29,    35,    30,     0,     3,
      29,    35,    30,     0,     7,    29,    35,    30,     0,     0,
      35,    40,     0,    35,     8,     3,    34,     0,    35,    34,
       0,    35,    28,     3,     0,    25,    31,    36,    32,     0,
      36,     5,     5,     5,     0,     0,    37,    38,     0,     0,
       6,    38,     0,    23,     0,     5,     5,     5,     5,     5,
       5,     5,     5,     5,     5,     5,     5,     5,     5,     5,
       5,     0,     3,     5,     0,     3,     3,     0,     3,     5,
       5,     0,     3,     5,     5,     5,     0,     3,     5,     5,
       5,     5,     0,     3,     4,     0,     3,    31,    36,    32,
       0,    41,     5,     5,     0,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,    94,    97,    99,   100,   101,   102,   103,   104,   105,
     106,   108,   109,   110,   111,   112,   113,   114,   115,   118,
     119,   120,   121,   122,   123,   126,   127,   130,   131,   134,
     135,   138,   146,   147,   148,   149,   150,   151,   152,   155,
     156
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "STRING", "QUOTED_STRING", "FLOAT", "INT", 
  "SEPARATOR", "DEF", "UN_MATERIAL", "DIFFUSE_COLOR", "COORDINATE3", 
  "INDEXED_FACE_SET", "INDEXED_TRIANGLE_STRIP_SET", "A_POINT", 
  "COORD_INDEX", "TEXTURE_COORD_INDEX", "NORMAL_INDEX", 
  "TEXTURE_COORDINATE", "TEXTURE2", "MATRIX_TRANSFORM", "MATRIX", 
  "LISTA_VACIA", "FINPOLY", "TWO_SIDED", "VECTOR", "VRML_HEADER", 
  "TRANSFORM", "USE", "'{'", "'}'", "'['", "']'", "vrml_file", "node", 
  "node_contents", "points", "polylist", "vertexilist", "matrix", "attr", 
  "coords", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    33,    34,    34,    34,    34,    34,    34,    34,    34,
      34,    34,    34,    34,    34,    34,    34,    34,    34,    35,
      35,    35,    35,    35,    35,    36,    36,    37,    37,    38,
      38,    39,    40,    40,    40,    40,    40,    40,    40,    41,
      41
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     2,     4,     4,     7,     7,    11,    11,     8,    12,
      12,     7,    11,    11,     5,     7,     4,     4,     4,     0,
       2,     4,     2,     3,     4,     4,     0,     2,     0,     2,
       1,    16,     2,     2,     3,     4,     5,     2,     4,     3,
       0
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     1,    19,    19,    19,     0,     0,     0,     0,
      19,     0,    19,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    26,     0,     0,     0,    17,
      22,    20,    18,     2,    26,    28,     0,    28,    40,     3,
       0,     0,    16,     0,    33,    37,    32,    26,     0,    23,
       0,     0,    28,     0,     0,     0,    14,     0,    24,    34,
       0,    21,     0,     0,    30,     0,    27,     0,     0,     0,
       0,     0,     0,    35,    38,     4,    29,     0,     0,     5,
       0,     0,     0,    11,    39,    15,     0,    25,    36,    28,
      28,     0,     0,     8,    28,    28,     0,     0,     0,    28,
      28,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     6,     7,     0,     0,    12,    13,     0,     9,    10,
       0,     0,     0,     0,     0,     0,     0,    31,     0,     0,
       0
};

static const short yydefgoto[] =
{
     138,    40,    24,    53,    61,    76,    51,    41,    64
};

static const short yypact[] =
{
     -16,   157,    -8,    -2,    12,    13,    15,    21,    22,    32,
      38,    44,-32768,    53,    53,    53,    31,    25,    59,    65,
      53,    68,    53,    62,     5,    57,    83,    66,    73,    91,
      88,    94,   109,   102,   135,-32768,     0,   120,   123,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,    99,-32768,-32768,-32768,
     128,   104,-32768,    -4,-32768,-32768,   130,-32768,   157,-32768,
       2,    14,-32768,    16,     4,   136,-32768,   144,-32768,   145,
       6,-32768,   110,    20,-32768,   115,-32768,    24,   141,   146,
     122,   151,   154,   162,-32768,-32768,-32768,   143,   147,-32768,
     156,   150,   152,-32768,-32768,-32768,   177,-32768,-32768,-32768,
  -32768,   158,   159,-32768,-32768,-32768,   183,    48,    49,-32768,
  -32768,    76,    77,   186,   131,   163,    82,    92,   164,   166,
     187,-32768,-32768,   167,   168,-32768,-32768,   190,-32768,-32768,
     194,   195,   196,   197,   198,   199,   200,-32768,   206,   207,
  -32768
};

static const short yypgoto[] =
{
  -32768,     1,   165,   -38,   -47,   137,-32768,-32768,-32768
};


#define	YYLAST		210


static const short yytable[] =
{
      63,    67,    12,    54,    55,    56,    60,    67,    36,    79,
       1,    67,     3,    37,     4,    77,     5,     6,     7,    70,
      73,    13,    73,     8,     9,    10,    73,    14,    68,    13,
      73,    57,    11,    38,    72,    39,    80,    74,    84,    74,
      28,    15,    16,    74,    17,    27,    75,    74,    78,    29,
      18,    19,   107,   108,    73,    73,    90,   111,   112,    71,
      36,    20,   116,   117,     3,    37,     4,    21,     5,     6,
       7,    74,    74,    22,    30,     8,     9,    10,    23,    31,
     114,   115,    73,    73,    11,    38,    36,    42,    73,    33,
       3,    37,     4,    35,     5,     6,     7,    44,    73,    74,
      74,     8,     9,    10,    45,    74,    46,    50,   118,   119,
      11,    38,    36,    43,   123,    74,     3,    37,     4,    47,
       5,     6,     7,    58,   124,    48,    59,     8,     9,    10,
      62,    87,    88,    65,    66,    69,    11,    38,    36,    49,
      85,    81,     3,    37,     4,    89,     5,     6,     7,    82,
      83,    94,    95,     8,     9,    10,    96,    91,    92,    97,
       2,   121,    11,    38,     3,    52,     4,    98,     5,     6,
       7,    93,   101,   102,    99,     8,     9,    10,   100,    25,
      26,   104,   106,   105,    11,    32,   103,    34,   113,   109,
     110,   120,   127,   122,   125,   130,   126,   128,   129,   131,
     132,   133,   134,   135,   136,   137,   139,   140,     0,     0,
      86
};

static const short yycheck[] =
{
      47,     5,     1,     3,     4,     5,    44,     5,     3,     5,
      26,     5,     7,     8,     9,    62,    11,    12,    13,    57,
       6,    29,     6,    18,    19,    20,     6,    29,    32,    29,
       6,    31,    27,    28,    32,    30,    32,    23,    32,    23,
      15,    29,    29,    23,    29,    14,    32,    23,    32,    24,
      29,    29,    99,   100,     6,     6,    32,   104,   105,    58,
       3,    29,   109,   110,     7,     8,     9,    29,    11,    12,
      13,    23,    23,    29,    15,    18,    19,    20,    25,    14,
      32,    32,     6,     6,    27,    28,     3,    30,     6,    21,
       7,     8,     9,    31,    11,    12,    13,    31,     6,    23,
      23,    18,    19,    20,    31,    23,    15,     5,    32,    32,
      27,    28,     3,    30,    32,    23,     7,     8,     9,    31,
      11,    12,    13,     3,    32,    31,     3,    18,    19,    20,
      31,    16,    17,     5,    30,     5,    27,    28,     3,    30,
      30,     5,     7,     8,     9,    30,    11,    12,    13,     5,
       5,     5,    30,    18,    19,    20,     5,    16,    17,     5,
       3,    30,    27,    28,     7,    30,     9,     5,    11,    12,
      13,    30,    16,    17,    31,    18,    19,    20,    31,    14,
      15,    31,     5,    31,    27,    20,    30,    22,     5,    31,
      31,     5,     5,    30,    30,     5,    30,    30,    30,     5,
       5,     5,     5,     5,     5,     5,     0,     0,    -1,    -1,
      73
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

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

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

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
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


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

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
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

#ifdef YYERROR_VERBOSE

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
#endif

#line 315 "/usr/share/bison/bison.simple"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
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

  if (yyssp >= yyss + yystacksize - 1)
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
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
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
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

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

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 1:
#line 94 "parser.y"
{ root_node=yyvsp[0].nodo; ;
    break;}
case 2:
#line 98 "parser.y"
{ yyval.nodo=new Material(yyvsp[-1].nodo); delete yyvsp[-1].nodo;;
    break;}
case 3:
#line 99 "parser.y"
{ yyval.nodo=new Texture2(yyvsp[-1].nodo); delete yyvsp[-1].nodo;;
    break;}
case 4:
#line 100 "parser.y"
{ yyval.nodo=new Coordinate3(yyvsp[-2].vlist); delete yyvsp[-2].vlist; ;
    break;}
case 5:
#line 101 "parser.y"
{yyval.nodo=new IndexedFaceSet(yyvsp[-2].plist); delete yyvsp[-2].plist; ;
    break;}
case 6:
#line 102 "parser.y"
{yyval.nodo=new IndexedFaceSet(yyvsp[-6].plist,yyvsp[-2].plist); delete yyvsp[-6].plist; delete yyvsp[-2].plist; ;
    break;}
case 7:
#line 103 "parser.y"
{yyval.nodo=new IndexedFaceSet(yyvsp[-6].plist); delete yyvsp[-6].plist;  delete yyvsp[-2].plist; ;
    break;}
case 8:
#line 104 "parser.y"
{yyval.nodo=new IndexedFaceSet(yyvsp[-2].plist); delete yyvsp[-2].plist; yyval.nodo->setTwoSided(); ;
    break;}
case 9:
#line 105 "parser.y"
{yyval.nodo=new IndexedFaceSet(yyvsp[-6].plist,yyvsp[-2].plist); delete yyvsp[-6].plist; delete yyvsp[-2].plist; yyval.nodo->setTwoSided(); yyval.nodo->setTwoSided(); ;
    break;}
case 10:
#line 106 "parser.y"
{yyval.nodo=new IndexedFaceSet(yyvsp[-6].plist); delete yyvsp[-6].plist;  delete yyvsp[-2].plist; yyval.nodo->setTwoSided(); ;
    break;}
case 11:
#line 108 "parser.y"
{yyval.nodo=new IndexedTriStripSet(yyvsp[-2].plist); delete yyvsp[-2].plist; ;
    break;}
case 12:
#line 109 "parser.y"
{yyval.nodo=new IndexedTriStripSet(yyvsp[-6].plist,yyvsp[-2].plist); delete yyvsp[-6].plist; delete yyvsp[-2].plist; ;
    break;}
case 13:
#line 110 "parser.y"
{yyval.nodo=new IndexedTriStripSet(yyvsp[-6].plist); delete yyvsp[-6].plist;  delete yyvsp[-2].plist; ;
    break;}
case 14:
#line 111 "parser.y"
{ yyval.nodo=new MatrixTransform(yyvsp[-1].matrix); ;
    break;}
case 15:
#line 112 "parser.y"
{ yyval.nodo=new TextureCoordinate(yyvsp[-2].tcoord); delete yyvsp[-2].tcoord; ;
    break;}
case 16:
#line 113 "parser.y"
{ yyval.nodo=new Transform(yyvsp[-1].nodo); delete yyvsp[-1].nodo; ;
    break;}
case 17:
#line 114 "parser.y"
{ yyval.nodo=yyvsp[-1].nodo; ;
    break;}
case 18:
#line 115 "parser.y"
{yyval.nodo=new Separator(yyvsp[-1].nodo); delete yyvsp[-1].nodo; ;
    break;}
case 19:
#line 118 "parser.y"
{ yyval.nodo=new MyNode(); ;
    break;}
case 20:
#line 119 "parser.y"
{ yyvsp[-1].nodo->addAttribute(yyvsp[0].attribute->getName(), yyvsp[0].attribute); yyval.nodo=yyvsp[-1].nodo; ;
    break;}
case 21:
#line 120 "parser.y"
{ NodeCache::addNode(yyvsp[-1].s_value,yyvsp[0].nodo); yyvsp[-3].nodo->addChild(yyvsp[0].nodo); yyval.nodo=yyvsp[-3].nodo; ;
    break;}
case 22:
#line 121 "parser.y"
{ yyvsp[-1].nodo->addChild(yyvsp[0].nodo); yyval.nodo=yyvsp[-1].nodo; ;
    break;}
case 23:
#line 122 "parser.y"
{yyvsp[-2].nodo->addChild(NodeCache::getNode(yyvsp[0].s_value)); yyval.nodo=yyvsp[-2].nodo; ;
    break;}
case 24:
#line 123 "parser.y"
{ delete yyvsp[-1].vlist;yyval.nodo=new MyNode(); ;
    break;}
case 25:
#line 126 "parser.y"
{ yyvsp[-3].vlist->push_back(osg::Vec3(yyvsp[-2].f_value,yyvsp[-1].f_value,yyvsp[0].f_value)); yyval.vlist=yyvsp[-3].vlist; ;
    break;}
case 26:
#line 127 "parser.y"
{ yyval.vlist=new VertexList(); ;
    break;}
case 27:
#line 130 "parser.y"
{ yyvsp[-1].plist->push_back(yyvsp[0].vindex);yyval.plist=yyvsp[-1].plist; ;
    break;}
case 28:
#line 131 "parser.y"
{ yyval.plist=new PolygonList(); ;
    break;}
case 29:
#line 134 "parser.y"
{ yyvsp[0].vindex->push_back(yyvsp[-1].i_value);yyval.vindex=yyvsp[0].vindex; ;
    break;}
case 30:
#line 135 "parser.y"
{ yyval.vindex=new VertexIndexList(); ;
    break;}
case 31:
#line 142 "parser.y"
{Matrix m; m[0]=yyvsp[-15].f_value; m[1]=yyvsp[-14].f_value; m[2]=yyvsp[-13].f_value;m[3]=yyvsp[-12].f_value;m[4]=yyvsp[-11].f_value;m[5]=yyvsp[-10].f_value;m[6]=yyvsp[-9].f_value;m[7]=yyvsp[-8].f_value;m[8]=yyvsp[-7].f_value;m[9]=yyvsp[-6].f_value;m[10]=yyvsp[-5].f_value;m[11]=yyvsp[-4].f_value;m[12]=yyvsp[-3].f_value;m[13]=yyvsp[-2].f_value;m[14]=yyvsp[-1].f_value;m[15]=yyvsp[0].f_value; memcpy(yyval.matrix,m,sizeof(m));;
    break;}
case 32:
#line 146 "parser.y"
{ yyval.attribute=new AtrFloat(yyvsp[-1].s_value,yyvsp[0].f_value); ;
    break;}
case 33:
#line 147 "parser.y"
{ yyval.attribute=new AtrString(yyvsp[-1].s_value,yyvsp[0].s_value); ;
    break;}
case 34:
#line 148 "parser.y"
{ yyval.attribute=new AtrVec(yyvsp[-2].s_value,yyvsp[-1].f_value,yyvsp[0].f_value); ;
    break;}
case 35:
#line 149 "parser.y"
{ yyval.attribute=new AtrVec(yyvsp[-3].s_value,yyvsp[-2].f_value,yyvsp[-1].f_value,yyvsp[0].f_value); ;
    break;}
case 36:
#line 150 "parser.y"
{ yyval.attribute=new AtrVec(yyvsp[-4].s_value,yyvsp[-3].f_value,yyvsp[-2].f_value,yyvsp[-1].f_value,yyvsp[0].f_value); ;
    break;}
case 37:
#line 151 "parser.y"
{ yyval.attribute=new AtrString(yyvsp[-1].s_value,yyvsp[0].s_value); ;
    break;}
case 38:
#line 152 "parser.y"
{ yyval.attribute=new AtrVec3List(yyvsp[-3].s_value,yyvsp[-1].vlist); ;
    break;}
case 39:
#line 155 "parser.y"
{ yyvsp[-2].tcoord->push_back(TextureCoordVal(yyvsp[-1].f_value,yyvsp[0].f_value));yyval.tcoord=yyvsp[-2].tcoord; ;
    break;}
case 40:
#line 156 "parser.y"
{ yyval.tcoord=new TextureCoordList(); ;
    break;}
}

#line 705 "/usr/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
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
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
yyerrhandle:
  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

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

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 159 "parser.y"


void yyerror(char *str,...)
{
    printf("There was an error near line %d : \"%s\"!!!\n",yyline,str);fflush(stdout);
}

MyNode *getRoot() { return root_node; }
