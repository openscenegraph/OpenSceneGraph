#version 330
#extension GL_OVR_multiview2 : enable

layout(num_views = 2) in;

uniform mat4 osg_ModelViewProjectionMatrix;

in vec4 osg_Vertex;
in vec4 osg_MultiTexCoord0;

out vec2 texcoord;

void main(void)
{
    texcoord = osg_MultiTexCoord0.xy;
    gl_Position   = osg_ModelViewProjectionMatrix * osg_Vertex;
}
