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
#include "mynode.h"
#include "geometry.h"
#include "parser.hpp"
#include <string.h>
int yyline=1;
%}

DIGIT	[0-9]
%x cindex
%x def
%%
<INITIAL,cindex>[ \t\r,]
<INITIAL,cindex>"\n"   	{ yyline++; }
DEF			{ BEGIN def;return DEF; }
Coordinate3   		{ return COORDINATE3; }
Texture2                { return TEXTURE2; }
Separator		{ return SEPARATOR; }
Material		{ return UN_MATERIAL; }
IndexedFaceSet		{ return INDEXED_FACE_SET; }
IndexedTriangleStripSet	{ return INDEXED_TRIANGLE_STRIP_SET; }
TextureCoordinate2      { return TEXTURE_COORDINATE; }
Transform		{ return TRANSFORM; }
MatrixTransform		{ return MATRIX_TRANSFORM; }
TwoSided                { return TWO_SIDED; }
matrix			{ return MATRIX; }
point			{ return A_POINT; }
coordIndex		{ BEGIN cindex;return COORD_INDEX; }
textureCoordIndex	{ BEGIN cindex;return TEXTURE_COORD_INDEX; }
normalIndex		{ BEGIN cindex;return NORMAL_INDEX; }
vector			{ return VECTOR; }
<cindex>-1		{ return FINPOLY; }
<INITIAL,cindex>\]     	{ BEGIN INITIAL; return ']'; }
USE			{ BEGIN def;return USE; }
"#VRML\ V1.0 ascii" 	{ return VRML_HEADER; }
"#Inventor\ V2."." ascii" { return VRML_HEADER; }
#[^\n\r]*
"-"?{DIGIT}+"."{DIGIT}*	{
    yylval.f_value=strtod(yytext,NULL);
    return FLOAT;
}
"-"?{DIGIT}+"."{DIGIT}*[Ee][\+-]{DIGIT}+  {
    yylval.f_value=strtod(yytext,NULL);
    return FLOAT;
}
"-"?{DIGIT}+[Ee][\+-]{DIGIT}+  {
    yylval.f_value=strtod(yytext,NULL);
    return FLOAT;
}
<INITIAL>"-"?{DIGIT}+		{
    yylval.f_value=(float)strtol(yytext,NULL,10);
    return FLOAT;
}
<cindex>"-"?{DIGIT}+		{
    yylval.i_value=strtol(yytext,NULL,10);
    return INT;
}
<INITIAL,cindex>[,{}\[\]] { return yytext[0]; }
[a-zA-Z0-9\-\_\*]*\ *\[\]
<INITIAL,def>[a-zA-Z0-9\-\_\*\!\.]*	{ BEGIN INITIAL;yylval.s_value=strdup(yytext);return STRING; }
\"[^\"]*\"		{ yylval.s_value=strdup(yytext+1);yylval.s_value[strlen(yylval.s_value)-1]=0;return QUOTED_STRING; }

%%

int yywrap(void)
{
return (1);
}

void flush_scanner(void) {
    yy_flush_buffer(YY_CURRENT_BUFFER);

}
