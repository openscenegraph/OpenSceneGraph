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


extern YYSTYPE yylval;
