%{
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

// Where is malloc.h really needed?
#if !defined(__APPLE__) && !defined(macintosh)
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

%}

%union {
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
};

%token <s_value> STRING
%token <s_value> QUOTED_STRING
%token <f_value> FLOAT
%token <i_value> INT
%token SEPARATOR DEF UN_MATERIAL DIFFUSE_COLOR COORDINATE3 INDEXED_FACE_SET INDEXED_TRIANGLE_STRIP_SET 
%token A_POINT COORD_INDEX TEXTURE_COORD_INDEX NORMAL_INDEX TEXTURE_COORDINATE TEXTURE2
%token MATRIX_TRANSFORM MATRIX
%token LISTA_VACIA FINPOLY
%token TWO_SIDED
%token VECTOR
%token VRML_HEADER
%token TRANSFORM
%token USE

%type <vlist> points
%type <plist> polylist
%type <vindex> vertexilist
%type <tcoord> coords
%type <nodo> node_contents
%type <nodo> node
%type <attribute> attr
%type <matrix> matrix
%start vrml_file


%%

vrml_file: VRML_HEADER node { root_node=$2; }
;

node: 
    UN_MATERIAL '{' node_contents '}'         { $$=new Material($3); delete $3;}
    | TEXTURE2 '{' node_contents '}'         { $$=new Texture2($3); delete $3;}
    | COORDINATE3 '{' A_POINT '[' points ']' '}' { $$=new Coordinate3($5); delete $5; }
    | INDEXED_FACE_SET '{' COORD_INDEX '[' polylist ']' '}' {$$=new IndexedFaceSet($5); delete $5; }
    | INDEXED_FACE_SET '{' COORD_INDEX '[' polylist ']' TEXTURE_COORD_INDEX '[' polylist ']' '}' {$$=new IndexedFaceSet($5,$9); delete $5; delete $9; }
    | INDEXED_FACE_SET '{' COORD_INDEX '[' polylist ']' NORMAL_INDEX '[' polylist ']' '}' {$$=new IndexedFaceSet($5); delete $5;  delete $9; }
    | INDEXED_FACE_SET '{' TWO_SIDED COORD_INDEX '[' polylist ']' '}' {$$=new IndexedFaceSet($6); delete $6; $$->setTwoSided(); }
    | INDEXED_FACE_SET '{' TWO_SIDED COORD_INDEX '[' polylist ']' TEXTURE_COORD_INDEX '[' polylist ']' '}' {$$=new IndexedFaceSet($6,$10); delete $6; delete $10; $$->setTwoSided(); $$->setTwoSided(); }
    | INDEXED_FACE_SET '{' TWO_SIDED COORD_INDEX '[' polylist ']' NORMAL_INDEX '[' polylist ']' '}' {$$=new IndexedFaceSet($6); delete $6;  delete $10; $$->setTwoSided(); }

    | INDEXED_TRIANGLE_STRIP_SET '{' COORD_INDEX '[' polylist ']' '}' {$$=new IndexedTriStripSet($5); delete $5; }
    | INDEXED_TRIANGLE_STRIP_SET '{' COORD_INDEX '[' polylist ']' TEXTURE_COORD_INDEX '[' polylist ']' '}' {$$=new IndexedTriStripSet($5,$9); delete $5; delete $9; }
    | INDEXED_TRIANGLE_STRIP_SET '{' COORD_INDEX '[' polylist ']' NORMAL_INDEX '[' polylist ']' '}' {$$=new IndexedTriStripSet($5); delete $5;  delete $9; }
    | MATRIX_TRANSFORM '{' MATRIX matrix '}' { $$=new MatrixTransform($4); }
    | TEXTURE_COORDINATE '{' A_POINT '[' coords ']' '}' { $$=new TextureCoordinate($5); delete $5; }
    | TRANSFORM '{' node_contents '}'	{ $$=new Transform($3); delete $3; }
    | STRING '{' node_contents '}'	 	 { $$=$3; }
    | SEPARATOR '{' node_contents '}' 	 {$$=new Separator($3); delete $3; }
;

node_contents:		{ $$=new MyNode(); }
    | node_contents attr { $1->addAttribute($2->getName(), $2); $$=$1; }
    | node_contents DEF STRING node { NodeCache::addNode($3,$4); $1->addChild($4); $$=$1; }
    | node_contents node { $1->addChild($2); $$=$1; }
    | node_contents USE STRING {$1->addChild(NodeCache::getNode($3)); $$=$1; }
    | VECTOR '[' points ']' { delete $3;$$=new MyNode(); }
;

points: points FLOAT FLOAT FLOAT     { $1->push_back(osg::Vec3($2,$3,$4)); $$=$1; }
	| 			      { $$=new VertexList(); }
;

polylist: polylist vertexilist     { $1->push_back($2);$$=$1; }
	| 			   { $$=new PolygonList(); }
;

vertexilist: INT vertexilist  { $2->push_back($1);$$=$2; }
	| FINPOLY		   { $$=new VertexIndexList(); }
;

matrix: 
	FLOAT FLOAT FLOAT FLOAT
	FLOAT FLOAT FLOAT FLOAT
	FLOAT FLOAT FLOAT FLOAT
	FLOAT FLOAT FLOAT FLOAT {Matrix m; m[0]=$1; m[1]=$2; m[2]=$3;m[3]=$4;m[4]=$5;m[5]=$6;m[6]=$7;m[7]=$8;m[8]=$9;m[9]=$10;m[10]=$11;m[11]=$12;m[12]=$13;m[13]=$14;m[14]=$15;m[15]=$16; memcpy($$,m,sizeof(m));}

;

attr:     STRING FLOAT { $$=new AtrFloat($1,$2); }
        | STRING STRING { $$=new AtrString($1,$2); }
	| STRING FLOAT FLOAT { $$=new AtrVec($1,$2,$3); }
	| STRING FLOAT FLOAT FLOAT { $$=new AtrVec($1,$2,$3,$4); }
	| STRING FLOAT FLOAT FLOAT FLOAT { $$=new AtrVec($1,$2,$3,$4,$5); }
        | STRING QUOTED_STRING { $$=new AtrString($1,$2); }
	| STRING '[' points ']' { $$=new AtrVec3List($1,$3); }
;

coords: coords FLOAT FLOAT { $1->push_back(TextureCoordVal($2,$3));$$=$1; }
       |                   { $$=new TextureCoordList(); }
;

%%

void yyerror(char *str,...)
{
    printf("There was an error near line %d : \"%s\"!!!\n",yyline,str);fflush(stdout);
}

MyNode *getRoot() { return root_node; }
