#version 330
#extension GL_OVR_multiview2 : enable

#define NUM_VIEWS 2

layout(num_views = NUM_VIEWS) in;

uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ProjectionMatrix;

uniform mat4 ovr_projectionMatrix[NUM_VIEWS];
uniform mat4 ovr_viewMatrix[NUM_VIEWS];

in vec4 osg_Vertex;
in vec4 osg_Color;
in vec4 osg_MultiTexCoord0;

out vec4 color;
out vec2 texcoord;

void main(void)
{
    mat4 mvp = ovr_projectionMatrix[gl_ViewID_OVR] * osg_ProjectionMatrix * ovr_viewMatrix[gl_ViewID_OVR] * osg_ModelViewMatrix;

    color = osg_Color;
    texcoord = osg_MultiTexCoord0.xy;

    gl_Position = mvp * osg_Vertex;
}
