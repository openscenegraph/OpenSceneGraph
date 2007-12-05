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

#ifndef WIN32
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#endif

#ifndef WIN32
#define SUPPORT_CPP 1
#endif

#include <stdio.h>
#include <fstream>
#include <string>

#include "FlexLexer.h"

#include <Producer/CameraConfig>


using namespace std;
using namespace Producer;

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

%}

%token PRTOKEN_VISUAL
%token PRTOKEN_SET_SIMPLE
%token PRTOKEN_VISUAL_ID
%token PRTOKEN_BUFFER_SIZE
%token PRTOKEN_LEVEL      
%token PRTOKEN_RGBA               
%token PRTOKEN_DOUBLEBUFFER
%token PRTOKEN_STEREO      
%token PRTOKEN_AUX_BUFFERS 
%token PRTOKEN_RED_SIZE    
%token PRTOKEN_GREEN_SIZE  
%token PRTOKEN_BLUE_SIZE     
%token PRTOKEN_ALPHA_SIZE    
%token PRTOKEN_DEPTH_SIZE    
%token PRTOKEN_STENCIL_SIZE  
%token PRTOKEN_ACCUM_RED_SIZE      
%token PRTOKEN_ACCUM_GREEN_SIZE    
%token PRTOKEN_ACCUM_BLUE_SIZE     
%token PRTOKEN_ACCUM_ALPHA_SIZE    
%token PRTOKEN_ACCUM_ALPHA_SIZE    
%token PRTOKEN_SAMPLES
%token PRTOKEN_SAMPLE_BUFFERS    
%token PRTOKEN_RENDER_SURFACE
%token PRTOKEN_WINDOW_RECT
%token PRTOKEN_INPUT_RECT
%token PRTOKEN_HOSTNAME
%token PRTOKEN_DISPLAY
%token PRTOKEN_SCREEN
%token PRTOKEN_BORDER
%token PRTOKEN_DRAWABLE_TYPE
%token PRTOKEN_WINDOW_TYPE
%token PRTOKEN_PBUFFER_TYPE
%token PRTOKEN_CAMERA_GROUP
%token PRTOKEN_CAMERA
%token PRTOKEN_PROJECTION_RECT
%token PRTOKEN_LENS
%token PRTOKEN_FRUSTUM
%token PRTOKEN_PERSPECTIVE
%token PRTOKEN_ORTHO
%token PRTOKEN_OFFSET
%token PRTOKEN_ROTATE
%token PRTOKEN_TRANSLATE
%token PRTOKEN_SCALE
%token PRTOKEN_SHEAR
%token PRTOKEN_CLEAR_COLOR
%token PRTOKEN_INPUT_AREA
%token PRTOKEN_ERROR
%token PRTOKEN_INTEGER
%token PRTOKEN_HEX_INTEGER
%token PRTOKEN_FLOAT
%token PRTOKEN_TRUE
%token PRTOKEN_FALSE
%token PRTOKEN_QUOTED_STRING
%token PRTOKEN_STEREO_SYSTEM_COMMANDS
%token PRTOKEN_CUSTOM_FULL_SCREEN_RECTANGLE
%token PRTOKEN_METHOD
%token PRTOKEN_PREMULTIPLY
%token PRTOKEN_POSTMULTIPLY
%token PRTOKEN_OVERRIDE_REDIRECT
%token PRTOKEN_SHARELENS
%token PRTOKEN_SHAREVIEW
%token PRTOKEN_READ_DRAWABLE
%token PRTOKEN_SET_RTT_MODE
%token PRTOKEN_RTT_MODE_NONE
%token PRTOKEN_RTT_MODE_RGB
%token PRTOKEN_RTT_MODE_RGBA
%token PRTOKEN_THREAD_MODEL
%token PRTOKEN_SINGLE_THREADED
%token PRTOKEN_THREAD_PER_CAMERA
%token PRTOKEN_THREAD_PER_RENDER_SURFACE

%union {
    char * string;
    int    integer;
    float  real;
    bool boolean;
};

%type <string> name string;
%type <integer> intparam offset_multiply_method;
%type <integer> hex_integer;
%type <integer> drawableType rtt_mode;
%type <integer> threadModelDirective;
%type <real>    floatparam;
%type <real>    real;
%type <boolean> bool;

%%

config  : entries
	;

entries : entry | entries entry
	;

entry : visual | render_surface | camera | input_area | camera_group | stereo_param | system_params
	;

system_params : system_param | system_params system_param
    ;

system_param : 
        PRTOKEN_THREAD_MODEL threadModelDirective ';' 
        {
            Producer::CameraGroup::ThreadModel tm = (Producer::CameraGroup::ThreadModel)$2;
            cfg->setThreadModelDirective( tm );
        }
        ;

threadModelDirective:  
          PRTOKEN_SINGLE_THREADED               { $$ = CameraGroup::SingleThreaded; }
        | PRTOKEN_THREAD_PER_CAMERA             { $$ = CameraGroup::ThreadPerCamera; }
        | PRTOKEN_THREAD_PER_RENDER_SURFACE     { $$ = CameraGroup::ThreadPerRenderSurface; }
        ;

stereo_param : PRTOKEN_STEREO_SYSTEM_COMMANDS intparam string string ';'
        {
           cfg->addStereoSystemCommand( $2, $3, $4 );
        };


camera_group: PRTOKEN_CAMERA_GROUP '{' camera_group_attributes '}'
    ;

camera_group_attributes: cameras 
    ;

cameras :  /* null */ | camera | cameras camera
    ;

camera : PRTOKEN_CAMERA name 
	 {
	    cfg->beginCamera( $2 );
	 }
	'{' camera_attributes '}'
	 {
	    cfg->endCamera();
	 };

camera_attributes : camera_attribute | camera_attributes camera_attribute
 	;

camera_attribute :
	/* EMPTY */
	| PRTOKEN_RENDER_SURFACE name ';'
	{
	    cfg->setCameraRenderSurface( $2 );
	}
	| render_surface 
	{
	    cfg->setCameraRenderSurface();
	}
	| PRTOKEN_PROJECTION_RECT real real real real ';'
	{
	    cfg->setCameraProjectionRectangle( $2, $3, $4, $5 );
	}
	| PRTOKEN_SHARELENS bool ';'
	{
	    cfg->setCameraShareLens( $2 );
	}
    | PRTOKEN_SHAREVIEW bool ';'
    {
        cfg->setCameraShareView( $2 );
    }
    | PRTOKEN_CLEAR_COLOR real real real real ';'
    {
        cfg->setCameraClearColor( $2, $3, $4, $5 );
    }
	| lens 
	| camera_offset 
	;

camera_offset : PRTOKEN_OFFSET '{' 
	{
	    cfg->beginCameraOffset();
	}
		camera_offset_attributes '}'
	{
	    cfg->endCameraOffset();
	}
	;

camera_offset_attributes : camera_offset_attribute | camera_offset_attributes camera_offset_attribute
	;

camera_offset_attribute  :
	  PRTOKEN_SHEAR real real ';'
	  {
	    cfg->shearCameraOffset( $2, $3 );
	  }
	| PRTOKEN_ROTATE real real real real ';'
	  {
	    cfg->rotateCameraOffset( $2, $3, $4, $5 );
	  }
	| PRTOKEN_TRANSLATE real real real ';'
	  {
	    cfg->translateCameraOffset( $2, $3, $4 );
	  }
	| PRTOKEN_SCALE real real real ';'
	  {
	    cfg->scaleCameraOffset( $2, $3, $4 );
	  }
	| PRTOKEN_METHOD offset_multiply_method ';'
	  {
	    cfg->setCameraOffsetMultiplyMethod( (Producer::Camera::Offset::MultiplyMethod)$2 );
	  }
	;

offset_multiply_method: 
      PRTOKEN_PREMULTIPLY  { $$ = Producer::Camera::Offset::PreMultiply; }
    | PRTOKEN_POSTMULTIPLY { $$ = Producer::Camera::Offset::PostMultiply; }
    ;

lens : PRTOKEN_LENS '{' lens_attributes '}'
	;

lens_attributes : lens_attribute | lens_attributes lens_attribute
	;

lens_attribute :
	/* Empty */
	| PRTOKEN_ORTHO real real real real real real ';'
	  {
	      cfg->setCameraOrtho( $2, $3, $4, $5, $6, $7 );
	  }
	| PRTOKEN_ORTHO real real real real real real real real ';'
	  {
	      cfg->setCameraOrtho( $2, $3, $4, $5, $6, $7, $8, $9 );
	  }
	| PRTOKEN_PERSPECTIVE real real real real ';'
	  {
	      cfg->setCameraPerspective( $2, $3, $4, $5 );
	  }
	| PRTOKEN_PERSPECTIVE real real real real real real ';'
	  {
	      cfg->setCameraPerspective( $2, $3, $4, $5, $6, $7 );
	  }
	| PRTOKEN_FRUSTUM real real real real real real ';'
	  {
	      cfg->setCameraFrustum( $2, $3, $4, $5, $6, $7 );
	  }
	| PRTOKEN_FRUSTUM real real real real real real real real ';'
	  {
	      cfg->setCameraFrustum( $2, $3, $4, $5, $6, $7, $8, $9 );
	  }
	| PRTOKEN_SHEAR real real ';'
	  {
	      cfg->setCameraLensShear( $2, $3 );
	  }
	;

render_surface : PRTOKEN_RENDER_SURFACE name 
	 {
	     cfg->beginRenderSurface( $2 );
	 }
	 '{' render_surface_attributes '}'
	 {
	     cfg->endRenderSurface();
	 }
	;

render_surface_attributes : 
          render_surface_attribute 
        | render_surface_attributes render_surface_attribute
	;

render_surface_attribute :
	/* Empty */
	| PRTOKEN_VISUAL name ';'
	{
	    cfg->setRenderSurfaceVisualChooser( $2 );
	}
	| visual 
	{
	    cfg->setRenderSurfaceVisualChooser();
	}
	| PRTOKEN_WINDOW_RECT intparam  intparam  intparam  intparam ';'
	{
	    cfg->setRenderSurfaceWindowRectangle( $2, $3, $4, $5 );
	}
    | PRTOKEN_INPUT_RECT floatparam floatparam floatparam floatparam ';'
    {
        cfg->setRenderSurfaceInputRectangle( $2, $3, $4, $5 );
    }
	| PRTOKEN_HOSTNAME name ';'
	{
	    cfg->setRenderSurfaceHostName( std::string($2) );
	}
	| PRTOKEN_DISPLAY intparam ';'
	{
	    cfg->setRenderSurfaceDisplayNum( $2 );
	}
	| PRTOKEN_SCREEN intparam ';'
	{
	    cfg->setRenderSurfaceScreen( $2 );
	}
	| PRTOKEN_BORDER bool ';'
	{    
	    cfg->setRenderSurfaceBorder( $2 );
	}
    | PRTOKEN_CUSTOM_FULL_SCREEN_RECTANGLE intparam  intparam  intparam  intparam ';'
    {
        cfg->setRenderSurfaceCustomFullScreenRectangle( $2, $3, $4, $5 );
    }
    | PRTOKEN_OVERRIDE_REDIRECT bool ';'
    {
        cfg->setRenderSurfaceOverrideRedirect( $2 );
    }
    | PRTOKEN_DRAWABLE_TYPE drawableType ';' 
    {
        Producer::RenderSurface::DrawableType drawableType = (RenderSurface::DrawableType)$2;
        cfg->setRenderSurfaceDrawableType( drawableType );
    }
    | PRTOKEN_READ_DRAWABLE name ';'
    {
        cfg->setRenderSurfaceReadDrawable( $2 );
    }
    | PRTOKEN_SET_RTT_MODE rtt_mode ';'
    {
        cfg->setRenderSurfaceRenderToTextureMode( (Producer::RenderSurface::RenderToTextureMode)$2 );
    }
	;

drawableType: 
      PRTOKEN_WINDOW_TYPE   { $$ =  RenderSurface::DrawableType_Window; }
    | PRTOKEN_PBUFFER_TYPE  { $$ =  RenderSurface::DrawableType_PBuffer; }
    ;

rtt_mode : 
          PRTOKEN_RTT_MODE_NONE  { $$ = RenderSurface::RenderToTextureMode_None; }
        | PRTOKEN_RTT_MODE_RGB   { $$ = RenderSurface::RenderToRGBTexture; }
        | PRTOKEN_RTT_MODE_RGBA  { $$ = RenderSurface::RenderToRGBATexture; }
        ;


visual : PRTOKEN_VISUAL name 
	 {
	     cfg->beginVisual( $2 );
	 }
	'{' visual_attributes '}'
	 {
	     cfg->endVisual();
	 }

	 | PRTOKEN_VISUAL
	 {
	     cfg->beginVisual();
	 }
	'{' visual_attributes '}'
	 {
	     cfg->endVisual();
	 }
	 ;


visual_attributes : visual_attribute | visual_attributes ',' visual_attribute
	;
visual_attribute :
	  PRTOKEN_SET_SIMPLE 
	  {
	      cfg->setVisualSimpleConfiguration();
	  }
	| PRTOKEN_VISUAL_ID hex_integer
	  {
	      cfg->setVisualByID( $2 );
	  }

	| PRTOKEN_BUFFER_SIZE intparam 
	  {
	      cfg->addVisualAttribute( VisualChooser::BufferSize, $2 );
	  }
	| PRTOKEN_LEVEL intparam 
	  {
	      cfg->addVisualAttribute( VisualChooser::Level, $2 );
	  }

	| PRTOKEN_RGBA 
	  {
	      cfg->addVisualAttribute( VisualChooser::RGBA );
	  }
	| PRTOKEN_DOUBLEBUFFER         
	  {
	      cfg->addVisualAttribute( VisualChooser::DoubleBuffer );
	  }
	| PRTOKEN_STEREO               
	  {
	      cfg->addVisualAttribute( VisualChooser::Stereo );
	  }
	| PRTOKEN_AUX_BUFFERS intparam 
	  {
	      cfg->addVisualAttribute( VisualChooser::AuxBuffers, $2 );
	  }
	| PRTOKEN_RED_SIZE intparam             
	  {
	      cfg->addVisualAttribute( VisualChooser::RedSize, $2 );
	  }

	| PRTOKEN_GREEN_SIZE   intparam 
	  {
	      cfg->addVisualAttribute( VisualChooser::GreenSize, $2 );
	  }

	| PRTOKEN_BLUE_SIZE intparam 
	  {
	      cfg->addVisualAttribute( VisualChooser::BlueSize, $2 );
	  }
	| PRTOKEN_ALPHA_SIZE  intparam          
	  {
	      cfg->addVisualAttribute( VisualChooser::AlphaSize, $2 );
	  }
	| PRTOKEN_DEPTH_SIZE  intparam          
	  {
	      cfg->addVisualAttribute( VisualChooser::DepthSize, $2 );
	  }
	| PRTOKEN_STENCIL_SIZE  intparam        
	  {
	      cfg->addVisualAttribute( VisualChooser::StencilSize, $2 );
	  }
	| PRTOKEN_ACCUM_RED_SIZE  intparam      
	  {
	      cfg->addVisualAttribute( VisualChooser::AccumRedSize, $2 );
	  }
	| PRTOKEN_ACCUM_GREEN_SIZE  intparam    
	  {
	      cfg->addVisualAttribute( VisualChooser::AccumGreenSize, $2 );
	  }
	| PRTOKEN_ACCUM_BLUE_SIZE  intparam     
	  {
	      cfg->addVisualAttribute( VisualChooser::AccumBlueSize, $2 );
	  }
	| PRTOKEN_ACCUM_ALPHA_SIZE  intparam    
	  {
	      cfg->addVisualAttribute( VisualChooser::AccumAlphaSize, $2 );
	  }
	| PRTOKEN_SAMPLES  intparam    
	  {
	      cfg->addVisualAttribute( VisualChooser::Samples, $2 );
      }
	| PRTOKEN_SAMPLE_BUFFERS
	  {
	      cfg->addVisualAttribute( VisualChooser::SampleBuffers );
	  }
	| intparam intparam
	  {
	      cfg->addVisualExtendedAttribute( $1, $2 );
	  }
	  ;

input_area : PRTOKEN_INPUT_AREA 
			{ cfg->beginInputArea(); } 
			'{' input_area_entries '}'
			{ cfg->endInputArea(); } 
	;

input_area_entries : input_area_entry | input_area_entries input_area_entry
	;

input_area_entry : 
		PRTOKEN_RENDER_SURFACE name ';'
		{
			cfg->addInputAreaEntry( $2 );
		}
		;

real   : PRTOKEN_FLOAT
	{
	    $$ = atof(flexer->YYText());
	}
	| PRTOKEN_INTEGER
	{
	    $$ = atof(flexer->YYText());
	}
	;


floatparam : PRTOKEN_FLOAT
	{
	    $$ = atof(flexer->YYText());
	}
	;

intparam : PRTOKEN_INTEGER
	{
	    $$ = atoi( flexer->YYText() );
	}
	;

name : PRTOKEN_QUOTED_STRING
	{
	    $$ = strdup( flexer->YYText() );
	}
	;

string : PRTOKEN_QUOTED_STRING
	{
	    $$ = strdup( flexer->YYText() );
	}
	;


hex_integer : PRTOKEN_HEX_INTEGER
	{
	    int n;
	    sscanf( flexer->YYText(), "0x%x", &n );
	    $$ = n;
	}
	;

bool : PRTOKEN_TRUE { $$ = true;} | PRTOKEN_FALSE { $$ = false; }
	;

%%

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

    char *cpp_path =
  #if defined(__APPLE__)
                "/usr/bin/cpp";
  #else
                "/lib/cpp";
  #endif

    if( access( cpp_path, X_OK ) == 0 )
    {

        int pd[2];
        pipe( pd );

        flexer = new yyFlexLexer;
        if( fork() == 0 )
        {
            // we don't want to read from the pipe in the child, so close it.
            close( pd[0] );
            close( 1 );
            dup( pd[1] );


            /* This was here to allow reading a config file from stdin.
             * This has never been directly supported, so commenting out.
            if( fileName.empty() )
                execlp( cpp_path, "cpp",  "-P", 0L );
            else
            */
            execlp( cpp_path, "cpp",  "-P", fileName.c_str(), 0L );

            // This should not execute unless an error happens
            perror( "execlp" );
        }
        else
        {
            close( pd[1]);
            close( 0 );
            dup( pd[0] );

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
        std::ifstream ifs(fileName.c_str());
        flexer = new yyFlexLexer(&ifs);
        cfg = this;
        retval = ConfigParser_parse() == 0 ? true : false;
        ifs.close();
        delete flexer;
    }
    return retval;
}

