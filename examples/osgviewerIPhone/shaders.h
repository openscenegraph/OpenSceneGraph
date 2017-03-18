#pragma once 

#include <osg/GL>

//
// vertex color shader
//
#if OSG_GLES3_FEATURES

const char* ColorShaderVert =
"#version 300 es\n"
"in vec4 osg_Vertex;\n"
"uniform mat4 osg_ModelViewProjectionMatrix;\n"
"void main()\n"
"{\n"
"    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
"}\n";

const char* ColorShaderFrag =
"#version 300 es\n"
"uniform lowp vec4 color;\n"
"out lowp vec4 fragColor;\n"
"void main()\n"
"{\n"
"    fragColor = color;\n"
"}\n";

#elif OSG_GLES2_FEATURES

const char* ColorShaderVert =
"#version 100\n"
"attribute vec4 osg_Vertex;\n"
"uniform mat4 osg_ModelViewProjectionMatrix;\n"
"void main()\n"
"{\n"
"    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
"}\n";

const char* ColorShaderFrag =
"#version 100\n"
"uniform lowp vec4 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = color;\n"
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
"in vec4 osg_MultiTexCoord0;\n"
"uniform mat4 osg_ModelViewProjectionMatrix;\n"
"out vec4 texCoord;\n"
"void main()\n"
"{\n"
"    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
"    texCoord = osg_MultiTexCoord0;\n"
"}\n";

const char* TextShaderFrag =
"#version 300 es\n"
"in lowp vec4 texCoord;\n"
"uniform sampler2D glyphTexture;\n"
"uniform lowp vec4 color;\n"
"out lowp vec4 fragColor;\n"
"void main()\n"
"{\n"
"    //lowp vec4 gc = texture(glyphTexture, texCoord.xy);\n"
"    fragColor = color * texture(glyphTexture, texCoord.xy).a;\n"
"}\n";

#elif OSG_GLES2_FEATURES

const char* TextShaderVert =
"#version 100\n"
"attribute vec4 osg_Vertex;\n"
"attribute vec4 osg_MultiTexCoord0;\n"
"uniform mat4 osg_ModelViewProjectionMatrix;\n"
"varying vec4 texCoord;\n"
"void main()\n"
"{\n"
"    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
"    texCoord = osg_MultiTexCoord0;\n"
"}\n";

const char* TextShaderFrag =
"#version 100\n"
"varying lowp vec4 texCoord;\n"
"uniform sampler2D glyphTexture;\n"
"uniform lowp vec4 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = color * texture2D(glyphTexture, texCoord.xy).a;\n"
"}\n";

#else

const char* TextShaderVert = NULL;
const char* TextShaderFrag = NULL;

#endif
