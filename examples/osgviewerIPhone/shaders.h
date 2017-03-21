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

