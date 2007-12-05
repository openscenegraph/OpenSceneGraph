/* -*-c++-*- Producer - Copyright (C) 2001-2004  Don Burns
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
 */

%{
#include <stdio.h>
#include <ctype.h>
#include "ConfigParser.h"

//#define DEBUG
#ifdef DEBUG
#define REPORT printf(" %s\n", yytext);
#else
#define REPORT
#endif
%}

%option noyywrap
%option c++
/*%option yyclass="ConfigParser"*/

DIGIT	    [0-9]
ALPHA	    [A-Za-z]
HEXALNUM	[A-Fa-f0-9]

%%

[/][/]|[#]|[!]      { 
                        #ifdef DEBUG
                        char buff[128];
                        int i = 0;
                        #endif
                        
                        char c;
                        while( (c = yyinput()) != '\n' )
                        {   
                            if( c <= 0 )
                                break;
                            #ifdef DEBUG
                            buff[i++] = c;
                            #endif
                        }
                        #ifdef DEBUG
                        buff[i] = 0;
                        printf( "Single line comment: \"%s\"\n", buff );
                        #endif
                    }

[\n]        { yylineno++;}
[ \t]		{ ; }
[{]			{ REPORT return '{'; }
[}]			{ REPORT return '}'; }
["]			        { 
           		        char c;
	   		            int i = 0;
           		        while( (c = yyinput()) != '"' )
	       		            yytext[i++] = c;
	     		        yytext[i] = 0;
	    		        return PRTOKEN_QUOTED_STRING;
			        }
[,]			        { REPORT return ','; }
[:]			        { REPORT return ':'; }
[;]			        { REPORT return ';'; }
SetSimple		    { REPORT return PRTOKEN_SET_SIMPLE; }
VisualID		    { REPORT return PRTOKEN_VISUAL_ID; }
BUFFER_SIZE		    { REPORT return PRTOKEN_BUFFER_SIZE; }
LEVEL			    { REPORT return PRTOKEN_LEVEL; }
RGBA			    { REPORT return PRTOKEN_RGBA; }
DOUBLEBUFFER	    { REPORT return PRTOKEN_DOUBLEBUFFER; }
STEREO			    { REPORT return PRTOKEN_STEREO; }
AUX_BUFFERS		    { REPORT return PRTOKEN_AUX_BUFFERS; }
RED_SIZE		    { REPORT return PRTOKEN_RED_SIZE; }
GREEN_SIZE		    { REPORT return PRTOKEN_GREEN_SIZE; }
BLUE_SIZE		    { REPORT return PRTOKEN_BLUE_SIZE; }
ALPHA_SIZE		    { REPORT return PRTOKEN_ALPHA_SIZE; }
DEPTH_SIZE		    { REPORT return PRTOKEN_DEPTH_SIZE; }
STENCIL_SIZE		{ REPORT return PRTOKEN_STENCIL_SIZE; }
ACCUM_RED_SIZE		{ REPORT return PRTOKEN_ACCUM_RED_SIZE; }
ACCUM_GREEN_SIZE	{ REPORT return PRTOKEN_ACCUM_GREEN_SIZE; }
ACCUM_BLUE_SIZE		{ REPORT return PRTOKEN_ACCUM_BLUE_SIZE; }
ACCUM_ALPHA_SIZE	{ REPORT return PRTOKEN_ACCUM_ALPHA_SIZE; }
SAMPLES	            { REPORT return PRTOKEN_SAMPLES; }
SAMPLE_BUFFERS	    { REPORT return PRTOKEN_SAMPLE_BUFFERS; }
Visual			    { REPORT return PRTOKEN_VISUAL; }
RenderSurface       { REPORT return PRTOKEN_RENDER_SURFACE; }
WindowRectangle	    { REPORT return PRTOKEN_WINDOW_RECT; }
WindowRect	        { REPORT return PRTOKEN_WINDOW_RECT; }
Hostname		    { REPORT return PRTOKEN_HOSTNAME; }
Display			    { REPORT return PRTOKEN_DISPLAY; }
Screen			    { REPORT return PRTOKEN_SCREEN; }
Border			    { REPORT return PRTOKEN_BORDER; }
InputRectangle      { REPORT return PRTOKEN_INPUT_RECT; }
InputRect           { REPORT return PRTOKEN_INPUT_RECT; }
DrawableType        { REPORT return PRTOKEN_DRAWABLE_TYPE; }
PBUFFER_TYPE        { REPORT return PRTOKEN_PBUFFER_TYPE; }
WINDOW_TYPE         { REPORT return PRTOKEN_WINDOW_TYPE; }
ReadDrawable        { REPORT return PRTOKEN_READ_DRAWABLE; }
RenderToTextureMode { REPORT return PRTOKEN_SET_RTT_MODE; }
RTT_NONE            { REPORT return PRTOKEN_RTT_MODE_NONE; }
RTT_RGB             { REPORT return PRTOKEN_RTT_MODE_RGB; }
RTT_RGBA            { REPORT; return PRTOKEN_RTT_MODE_RGBA; }


CameraGroup         { REPORT return PRTOKEN_CAMERA_GROUP; }
Camera              { REPORT return PRTOKEN_CAMERA; }
ProjectionRectangle { REPORT return PRTOKEN_PROJECTION_RECT; }
ProjectionRect      { REPORT return PRTOKEN_PROJECTION_RECT; }

Lens			    { REPORT return PRTOKEN_LENS; }
ShareLens			{ REPORT return PRTOKEN_SHARELENS; }
ShareView			{ REPORT return PRTOKEN_SHAREVIEW; }
Frustum			    { REPORT return PRTOKEN_FRUSTUM; }
Perspective         { REPORT return PRTOKEN_PERSPECTIVE; }
Ortho			    { REPORT return PRTOKEN_ORTHO; }
ClearColor          { REPORT return PRTOKEN_CLEAR_COLOR; }

Offset              { REPORT return PRTOKEN_OFFSET; }
Rotate			    { REPORT return PRTOKEN_ROTATE; }
Translate		    { REPORT return PRTOKEN_TRANSLATE; }
Scale			    { REPORT return PRTOKEN_SCALE; }
Shear			    { REPORT return PRTOKEN_SHEAR; }
Method              { REPORT return PRTOKEN_METHOD; }
PreMultiply         { REPORT return PRTOKEN_PREMULTIPLY; }
PostMultiply        { REPORT return PRTOKEN_POSTMULTIPLY; }

InputArea		    { REPORT return PRTOKEN_INPUT_AREA; }

StereoSystemCommands { REPORT return PRTOKEN_STEREO_SYSTEM_COMMANDS; }

CustomFullScreenRectangle { REPORT return PRTOKEN_CUSTOM_FULL_SCREEN_RECTANGLE; }
OverrideRedirect    { REPORT return PRTOKEN_OVERRIDE_REDIRECT; }

ThreadModel         { REPORT return PRTOKEN_THREAD_MODEL; }
SingleThreaded         { REPORT return PRTOKEN_SINGLE_THREADED; }
ThreadPerCamera         { REPORT return PRTOKEN_THREAD_PER_CAMERA; }
ThreadPerRenderSurface  { REPORT return PRTOKEN_THREAD_PER_RENDER_SURFACE; }


on|ON|true|yes		{ REPORT return PRTOKEN_TRUE; }
off|OFF|false|no	{ REPORT return PRTOKEN_FALSE; }

[-]?0x{HEXALNUM}+	{ REPORT return PRTOKEN_HEX_INTEGER; }
[-]?{DIGIT}+		{ REPORT return PRTOKEN_INTEGER; }

[-]?{DIGIT}*"."{DIGIT}*[Ff]? 	            { REPORT return PRTOKEN_FLOAT; }
[-]?{DIGIT}+[fF]{1}		                    { REPORT return PRTOKEN_FLOAT; }
[-]?({DIGIT}+)([eE][-+]?{DIGIT}+)?          { REPORT return PRTOKEN_FLOAT; }
[-]?({DIGIT}*\.{DIGIT}+)([eE][-+]?{DIGIT}+)?  { REPORT return PRTOKEN_FLOAT; }




%%


