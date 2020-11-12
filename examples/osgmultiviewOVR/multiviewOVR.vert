#version 330
#extension GL_OVR_multiview2 : enable

#define NUM_VIEWS 2

layout(num_views = NUM_VIEWS) in;

uniform mat4 osg_ModelViewProjectionMatrix;

// uniform mat4 ovr_projectionMatrix[NUM_VIEWS];
// uniform mat4 ovr_viewMatrix[NUM_VIEWS];

in vec4 osg_Vertex;
in vec4 osg_MultiTexCoord0;

out vec2 texcoord;

void main(void)
{
    // gl_ViewID_OVR

    mat4 mvp = osg_ModelViewProjectionMatrix;

    texcoord = osg_MultiTexCoord0.xy;
    gl_Position   = mvp * osg_Vertex;
}
