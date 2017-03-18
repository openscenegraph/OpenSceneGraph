#pragma once 

#include <osg/GL>

//
// vertex color shader
//
#if OSG_GLES3_FEATURES

const char* ColorShaderVert =
"#version 300 es\n"
"in vec4 osg_Vertex;\n"
"in vec4 osg_Color;\n"
"out vec4 vertColor;\n"
"uniform mat4 osg_ModelViewProjectionMatrix;\n"
"void main()\n"
"{\n"
"    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
"    vertColor = osg_Color;\n"
"}\n";

const char* ColorShaderFrag =
"#version 300 es\n"
"in lowp vec4 vertColor;\n"
"out lowp vec4 fragColor;\n"
"void main()\n"
"{\n"
"    fragColor = vertColor;\n"
"}\n";

#elif OSG_GLES2_FEATURES

const char* ColorShaderVert =
"#version 100\n"
"attribute vec4 osg_Vertex;\n"
"attribute vec4 osg_Color;\n"
"uniform mat4 osg_ModelViewProjectionMatrix;\n"
"varying vec4 vertColor;\n"
"void main()\n"
"{\n"
"    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
"    vertColor = osg_Color;\n"
"}\n";

const char* ColorShaderFrag =
"#version 100\n"
"varying lowp vec4 vertColor;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vertColor;\n"
"}\n";

#else

const char* ColorShaderVert = NULL;
const char* ColorShaderFrag = NULL;

#endif


//
// text shader
//
#if OSG_GLES3_FEATURES

const char* TextShaderVert =
"#version 300 es\n"
"in vec4 osg_Vertex;\n"
"in vec4 osg_Color;\n"
"in vec4 osg_MultiTexCoord0;\n"
"uniform mat4 osg_ModelViewProjectionMatrix;\n"
"out vec4 vertColor;\n"
"out vec4 texCoord;\n"
"void main()\n"
"{\n"
"    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
"    vertColor = osg_Color;\n"
"    texCoord = osg_MultiTexCoord0;\n"
"}\n";

const char* TextShaderFrag =
"#version 300 es\n"
"in lowp vec4 vertColor;\n"
"in lowp vec4 texCoord;\n"
"uniform sampler2D glyphTexture;\n"
"out lowp vec4 fragColor;\n"
"void main()\n"
"{\n"
"    fragColor = vertColor * texture(glyphTexture, texCoord.xy).a;\n"
"}\n";

#elif OSG_GLES2_FEATURES

const char* TextShaderVert =
"#version 100\n"
"attribute vec4 osg_Vertex;\n"
"attribute vec4 osg_Color;\n"
"attribute vec4 osg_MultiTexCoord0;\n"
"uniform mat4 osg_ModelViewProjectionMatrix;\n"
"varying vec4 vertColor;\n"
"varying vec4 texCoord;\n"
"void main()\n"
"{\n"
"    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
"    vertColor = osg_Color;\n"
"    texCoord = osg_MultiTexCoord0;\n"
"}\n";

const char* TextShaderFrag =
"#version 100\n"
"varying lowp vec4 vertColor;\n"
"varying lowp vec4 texCoord;\n"
"uniform sampler2D glyphTexture;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vertColor * texture2D(glyphTexture, texCoord.xy).a;\n"
"}\n";

#else

const char* TextShaderVert = NULL;
const char* TextShaderFrag = NULL;

#endif
