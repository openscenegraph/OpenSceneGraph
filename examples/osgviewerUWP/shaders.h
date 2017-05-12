#pragma once 

#include <osg/GL>

//
// vertex color shader
//
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


//
// Flat shaded diffuse texture shader
//
const char* TextureShaderVert =
"#version 100\n"
"attribute vec4 osg_Vertex;\n"
"attribute vec4 osg_MultiTexCoord0;\n"
"uniform mat4 osg_ModelViewProjectionMatrix;\n"
"varying vec4 texcoord0;\n"
"void main()\n"
"{\n"
"    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
"    texcoord0 = osg_MultiTexCoord0;\n"
"}\n";

const char* TextureShaderFrag =
"#version 100\n"
"varying lowp vec4 texcoord0;\n"
"uniform sampler2D texture0;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(texture0, texcoord0.xy);\n"
"}\n";


//
// Point sprite particle shader
//
const char* ParticleShaderVert =
"#version 100\n"
"attribute vec4 osg_Vertex;\n"
"attribute vec4 osg_Color;\n"
"uniform mat4 osg_ModelViewProjectionMatrix;\n"
"varying vec4 vertColor;\n"
"void main()\n"
"{\n"
"    gl_PointSize = 10.0;\n"
"    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
"    vertColor = osg_Color;\n"
"}\n";

const char* ParticleShaderFrag =
"#version 100\n"
"varying lowp vec4 vertColor;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vertColor;//vec4(0.3,0.3,0.3,1.0);\n"
"}\n";