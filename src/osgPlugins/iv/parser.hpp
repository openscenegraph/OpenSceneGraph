#ifndef BISON_PARSER_HPP
# define BISON_PARSER_HPP

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


extern YYSTYPE yylval;

#endif /* not BISON_PARSER_HPP */
