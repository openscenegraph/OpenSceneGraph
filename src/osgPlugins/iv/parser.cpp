
/*  A Bison parser, made from parser.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	STRING	257
#define	QUOTED_STRING	258
#define	FLOAT	259
#define	INT	260
#define	SEPARATOR	261
#define	DEF	262
#define	UN_MATERIAL	263
#define	DIFFUSE_COLOR	264
#define	COORDINATE3	265
#define	INDEXED_FACE_SET	266
#define	A_POINT	267
#define	COORD_INDEX	268
#define	TEXTURE_COORD_INDEX	269
#define	NORMAL_INDEX	270
#define	TEXTURE_COORDINATE	271
#define	TEXTURE2	272
#define	MATRIX_TRANSFORM	273
#define	MATRIX	274
#define	LISTA_VACIA	275
#define	FINPOLY	276
#define	DOBLE_CARA	277
#define	VECTOR	278
#define	VRML_HEADER	279
#define	TRANSFORM	280
#define	USE	281

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

#include <malloc.h>
#include "mynode.h"
#include <stdlib.h>
#include "geometry.h"
#include "nodecache.h"

#include "material.h"
#include "coordinate3.h"
#include "separator.h"
#include "matrixtransform.h"
#include "indexedfaceset.h"
#include "texturecoordinate.h"
#include "texture2.h"
#include "transform.h"

#include "atrfloat.h"
#include "atrstring.h"
#include "atrvec.h"
#include "string.h"
extern int yyline;
extern int yylex();
void yyerror(char *str,...);
static MyNode *root_node;


#line 52 "parser.y"
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
} YYSTYPE;
#ifndef YYDEBUG
#define YYDEBUG 1
#endif

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		120
#define	YYFLAG		-32768
#define	YYNTBASE	32

#define YYTRANSLATE(x) ((unsigned)(x) <= 281 ? yytranslate[x] : 41)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    30,     2,    31,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    28,     2,    29,     2,     2,     2,     2,     2,
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
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     8,    13,    21,    29,    41,    53,    62,    75,
    88,    94,   102,   107,   112,   117,   118,   121,   126,   129,
   133,   138,   143,   144,   147,   148,   151,   153,   170,   173,
   176,   180,   185,   191,   194,   198
};

static const short yyrhs[] = {    25,
    33,     0,     9,    28,    34,    29,     0,    18,    28,    34,
    29,     0,    11,    28,    13,    30,    35,    31,    29,     0,
    12,    28,    14,    30,    36,    31,    29,     0,    12,    28,
    14,    30,    36,    31,    15,    30,    36,    31,    29,     0,
    12,    28,    14,    30,    36,    31,    16,    30,    36,    31,
    29,     0,    12,    28,    23,    14,    30,    36,    31,    29,
     0,    12,    28,    23,    14,    30,    36,    31,    15,    30,
    36,    31,    29,     0,    12,    28,    23,    14,    30,    36,
    31,    16,    30,    36,    31,    29,     0,    19,    28,    20,
    38,    29,     0,    17,    28,    13,    30,    40,    31,    29,
     0,    26,    28,    34,    29,     0,     3,    28,    34,    29,
     0,     7,    28,    34,    29,     0,     0,    34,    39,     0,
    34,     8,     3,    33,     0,    34,    33,     0,    34,    27,
     3,     0,    24,    30,    35,    31,     0,    35,     5,     5,
     5,     0,     0,    36,    37,     0,     0,     6,    37,     0,
    22,     0,     5,     5,     5,     5,     5,     5,     5,     5,
     5,     5,     5,     5,     5,     5,     5,     5,     0,     3,
     5,     0,     3,     3,     0,     3,     5,     5,     0,     3,
     5,     5,     5,     0,     3,     5,     5,     5,     5,     0,
     3,     4,     0,    40,     5,     5,     0,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    92,    95,    97,    98,    99,   100,   101,   102,   103,   104,
   105,   106,   107,   108,   109,   112,   113,   114,   115,   116,
   117,   120,   121,   124,   125,   128,   129,   132,   140,   141,
   142,   143,   144,   145,   148,   149
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","STRING",
"QUOTED_STRING","FLOAT","INT","SEPARATOR","DEF","UN_MATERIAL","DIFFUSE_COLOR",
"COORDINATE3","INDEXED_FACE_SET","A_POINT","COORD_INDEX","TEXTURE_COORD_INDEX",
"NORMAL_INDEX","TEXTURE_COORDINATE","TEXTURE2","MATRIX_TRANSFORM","MATRIX","LISTA_VACIA",
"FINPOLY","DOBLE_CARA","VECTOR","VRML_HEADER","TRANSFORM","USE","'{'","'}'",
"'['","']'","vrml_file","node","node_contents","points","polylist","vertexilist",
"matrix","attr","coords", NULL
};
#endif

static const short yyr1[] = {     0,
    32,    33,    33,    33,    33,    33,    33,    33,    33,    33,
    33,    33,    33,    33,    33,    34,    34,    34,    34,    34,
    34,    35,    35,    36,    36,    37,    37,    38,    39,    39,
    39,    39,    39,    39,    40,    40
};

static const short yyr2[] = {     0,
     2,     4,     4,     7,     7,    11,    11,     8,    12,    12,
     5,     7,     4,     4,     4,     0,     2,     4,     2,     3,
     4,     4,     0,     2,     0,     2,     1,    16,     2,     2,
     3,     4,     5,     2,     3,     0
};

static const short yydefact[] = {     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     1,    16,    16,    16,     0,     0,     0,    16,     0,    16,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    23,     0,     0,     0,    14,    19,    17,    15,     2,
    23,    25,     0,    36,     3,     0,     0,    13,     0,    30,
    34,    29,     0,    20,     0,     0,    25,     0,     0,    11,
     0,    21,    31,    18,     0,     0,    27,     0,    24,     0,
     0,     0,     0,     0,    32,     4,    26,     0,     0,     5,
     0,    35,    12,     0,    22,    33,    25,    25,     0,     0,
     8,     0,     0,     0,    25,    25,     0,     0,     0,     0,
     0,     0,     6,     7,     0,     0,     0,     9,    10,     0,
     0,     0,     0,     0,     0,     0,    28,     0,     0,     0
};

static const short yydefgoto[] = {   118,
    37,    22,    49,    56,    69,    47,    38,    58
};

static const short yypact[] = {   -20,
   121,   -22,     2,    16,    23,    25,    33,    43,    44,    45,
-32768,     1,     1,     1,    35,    20,    62,     1,    13,     1,
    46,     0,    38,    51,    56,    59,    67,    84,    76,   116,
    89,-32768,    10,   119,   120,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   101,-32768,-32768,   132,   112,-32768,    -3,-32768,
-32768,   137,   121,-32768,     5,    60,-32768,    11,   138,-32768,
   139,-32768,   140,-32768,   117,    -2,-32768,     6,-32768,    68,
   143,   122,   144,   145,   147,-32768,-32768,   123,   124,-32768,
     8,-32768,-32768,   150,-32768,-32768,-32768,-32768,   126,   127,
-32768,   153,    98,   103,-32768,-32768,   154,   131,   133,   104,
   105,   156,-32768,-32768,   134,   135,   160,-32768,-32768,   161,
   162,   163,   164,   165,   166,   167,-32768,   173,   174,-32768
};

static const short yypgoto[] = {-32768,
    -1,    99,   136,   -56,   109,-32768,-32768,-32768
};


#define	YYLAST		177


static const short yytable[] = {    11,
    70,    61,    33,    66,     1,    12,     3,    34,     4,    61,
     5,     6,    50,    51,    52,    71,     7,     8,     9,    67,
    78,    79,    89,    90,    21,    10,    35,    62,    36,    13,
    93,    94,    30,    26,    80,    65,    91,    12,   100,   101,
    33,    72,    27,    14,     3,    34,     4,    25,     5,     6,
    15,    64,    16,    33,     7,     8,     9,     3,    34,     4,
    17,     5,     6,    10,    35,    66,    39,     7,     8,     9,
    18,    19,    20,    66,    28,    32,    10,    35,    33,    40,
    43,    67,     3,    34,     4,    41,     5,     6,    42,    67,
    68,    33,     7,     8,     9,     3,    34,     4,    81,     5,
     6,    10,    35,    66,    45,     7,     8,     9,    66,    66,
    66,    23,    24,    44,    10,    35,    29,    48,    31,    67,
    46,    53,    54,     2,    67,    67,    67,     3,    98,     4,
    57,     5,     6,    99,   105,   106,    59,     7,     8,     9,
    60,    63,    73,    74,    75,    76,    10,    82,    84,    85,
    83,    86,    87,    88,    92,    95,    96,    97,   102,   103,
   107,   104,   108,   109,   110,   111,   112,   113,   114,   115,
   116,   117,   119,   120,    77,     0,    55
};

static const short yycheck[] = {     1,
    57,     5,     3,     6,    25,    28,     7,     8,     9,     5,
    11,    12,     3,     4,     5,     5,    17,    18,    19,    22,
    15,    16,    15,    16,    24,    26,    27,    31,    29,    28,
    87,    88,    20,    14,    29,    31,    29,    28,    95,    96,
     3,    31,    23,    28,     7,     8,     9,    13,    11,    12,
    28,    53,    28,     3,    17,    18,    19,     7,     8,     9,
    28,    11,    12,    26,    27,     6,    29,    17,    18,    19,
    28,    28,    28,     6,    13,    30,    26,    27,     3,    29,
    14,    22,     7,     8,     9,    30,    11,    12,    30,    22,
    31,     3,    17,    18,    19,     7,     8,     9,    31,    11,
    12,    26,    27,     6,    29,    17,    18,    19,     6,     6,
     6,    13,    14,    30,    26,    27,    18,    29,    20,    22,
     5,     3,     3,     3,    22,    22,    22,     7,    31,     9,
    30,    11,    12,    31,    31,    31,     5,    17,    18,    19,
    29,     5,     5,     5,     5,    29,    26,     5,     5,     5,
    29,     5,    30,    30,     5,    30,    30,     5,     5,    29,
     5,    29,    29,    29,     5,     5,     5,     5,     5,     5,
     5,     5,     0,     0,    66,    -1,    41
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

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

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/lib/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
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
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
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

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 92 "parser.y"
{ root_node=yyvsp[0].nodo; ;
    break;}
case 2:
#line 96 "parser.y"
{ yyval.nodo=new Material(yyvsp[-1].nodo); delete yyvsp[-1].nodo;;
    break;}
case 3:
#line 97 "parser.y"
{ yyval.nodo=new Texture2(yyvsp[-1].nodo); delete yyvsp[-1].nodo;;
    break;}
case 4:
#line 98 "parser.y"
{ yyval.nodo=new Coordinate3(yyvsp[-2].vlist); delete yyvsp[-2].vlist; ;
    break;}
case 5:
#line 99 "parser.y"
{yyval.nodo=new IndexedFaceSet(yyvsp[-2].plist); delete yyvsp[-2].plist; ;
    break;}
case 6:
#line 100 "parser.y"
{yyval.nodo=new IndexedFaceSet(yyvsp[-6].plist,yyvsp[-2].plist); delete yyvsp[-6].plist; delete yyvsp[-2].plist; ;
    break;}
case 7:
#line 101 "parser.y"
{yyval.nodo=new IndexedFaceSet(yyvsp[-6].plist); delete yyvsp[-6].plist;  delete yyvsp[-2].plist; ;
    break;}
case 8:
#line 102 "parser.y"
{yyval.nodo=new IndexedFaceSet(yyvsp[-2].plist); delete yyvsp[-2].plist; yyval.nodo->setTwoSided(); ;
    break;}
case 9:
#line 103 "parser.y"
{yyval.nodo=new IndexedFaceSet(yyvsp[-6].plist,yyvsp[-2].plist); delete yyvsp[-6].plist; delete yyvsp[-2].plist; yyval.nodo->setTwoSided(); yyval.nodo->setTwoSided(); ;
    break;}
case 10:
#line 104 "parser.y"
{yyval.nodo=new IndexedFaceSet(yyvsp[-6].plist); delete yyvsp[-6].plist;  delete yyvsp[-2].plist; yyval.nodo->setTwoSided(); ;
    break;}
case 11:
#line 105 "parser.y"
{ yyval.nodo=new MatrixTransform(yyvsp[-1].matrix); ;
    break;}
case 12:
#line 106 "parser.y"
{ yyval.nodo=new TextureCoordinate(yyvsp[-2].tcoord); delete yyvsp[-2].tcoord; ;
    break;}
case 13:
#line 107 "parser.y"
{ yyval.nodo=new Transform(yyvsp[-1].nodo); delete yyvsp[-1].nodo; ;
    break;}
case 14:
#line 108 "parser.y"
{ yyval.nodo=yyvsp[-1].nodo; ;
    break;}
case 15:
#line 109 "parser.y"
{yyval.nodo=new Separator(yyvsp[-1].nodo); delete yyvsp[-1].nodo; ;
    break;}
case 16:
#line 112 "parser.y"
{ yyval.nodo=new MyNode(); ;
    break;}
case 17:
#line 113 "parser.y"
{ yyvsp[-1].nodo->addAttribute(yyvsp[0].attribute->getName(), yyvsp[0].attribute); yyval.nodo=yyvsp[-1].nodo; ;
    break;}
case 18:
#line 114 "parser.y"
{ NodeCache::addNode(yyvsp[-1].s_value,yyvsp[0].nodo); yyvsp[-3].nodo->addChild(yyvsp[0].nodo); yyval.nodo=yyvsp[-3].nodo; ;
    break;}
case 19:
#line 115 "parser.y"
{ yyvsp[-1].nodo->addChild(yyvsp[0].nodo); yyval.nodo=yyvsp[-1].nodo; ;
    break;}
case 20:
#line 116 "parser.y"
{yyvsp[-2].nodo->addChild(NodeCache::getNode(yyvsp[0].s_value)); yyval.nodo=yyvsp[-2].nodo; ;
    break;}
case 21:
#line 117 "parser.y"
{ delete yyvsp[-1].vlist;yyval.nodo=new MyNode(); ;
    break;}
case 22:
#line 120 "parser.y"
{ yyvsp[-3].vlist->push_back(osg::Vec3(yyvsp[-2].f_value,yyvsp[-1].f_value,yyvsp[0].f_value)); yyval.vlist=yyvsp[-3].vlist; ;
    break;}
case 23:
#line 121 "parser.y"
{ yyval.vlist=new VertexList(); ;
    break;}
case 24:
#line 124 "parser.y"
{ yyvsp[-1].plist->push_back(yyvsp[0].vindex);yyval.plist=yyvsp[-1].plist; ;
    break;}
case 25:
#line 125 "parser.y"
{ yyval.plist=new PolygonList(); ;
    break;}
case 26:
#line 128 "parser.y"
{ yyvsp[0].vindex->push_back(yyvsp[-1].i_value);yyval.vindex=yyvsp[0].vindex; ;
    break;}
case 27:
#line 129 "parser.y"
{ yyval.vindex=new VertexIndexList(); ;
    break;}
case 28:
#line 136 "parser.y"
{Matrix m; m[0]=yyvsp[-15].f_value; m[1]=yyvsp[-14].f_value; m[2]=yyvsp[-13].f_value;m[3]=yyvsp[-12].f_value;m[4]=yyvsp[-11].f_value;m[5]=yyvsp[-10].f_value;m[6]=yyvsp[-9].f_value;m[7]=yyvsp[-8].f_value;m[8]=yyvsp[-7].f_value;m[9]=yyvsp[-6].f_value;m[10]=yyvsp[-5].f_value;m[11]=yyvsp[-4].f_value;m[12]=yyvsp[-3].f_value;m[13]=yyvsp[-2].f_value;m[14]=yyvsp[-1].f_value;m[15]=yyvsp[0].f_value; memcpy(yyval.matrix,m,sizeof(m));;
    break;}
case 29:
#line 140 "parser.y"
{ yyval.attribute=new AtrFloat(yyvsp[-1].s_value,yyvsp[0].f_value); ;
    break;}
case 30:
#line 141 "parser.y"
{ yyval.attribute=new AtrString(yyvsp[-1].s_value,yyvsp[0].s_value); ;
    break;}
case 31:
#line 142 "parser.y"
{ yyval.attribute=new AtrVec(yyvsp[-2].s_value,yyvsp[-1].f_value,yyvsp[0].f_value); ;
    break;}
case 32:
#line 143 "parser.y"
{ yyval.attribute=new AtrVec(yyvsp[-3].s_value,yyvsp[-2].f_value,yyvsp[-1].f_value,yyvsp[0].f_value); ;
    break;}
case 33:
#line 144 "parser.y"
{ yyval.attribute=new AtrVec(yyvsp[-4].s_value,yyvsp[-3].f_value,yyvsp[-2].f_value,yyvsp[-1].f_value,yyvsp[0].f_value); ;
    break;}
case 34:
#line 145 "parser.y"
{ yyval.attribute=new AtrString(yyvsp[-1].s_value,yyvsp[0].s_value); ;
    break;}
case 35:
#line 148 "parser.y"
{ yyvsp[-2].tcoord->push_back(TextureCoordVal(yyvsp[-1].f_value,yyvsp[0].f_value));yyval.tcoord=yyvsp[-2].tcoord; ;
    break;}
case 36:
#line 149 "parser.y"
{ yyval.tcoord=new TextureCoordList(); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

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

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 152 "parser.y"


void yyerror(char *str,...)
{
    printf("There was an error near line %d : \"%s\"!!!\n",yyline,str);fflush(stdout);
}

MyNode *getRoot() { return root_node; }
