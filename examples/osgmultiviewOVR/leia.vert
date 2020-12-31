#version 330

#pragma import_defines ( NUM_VIEWS )

#ifdef NUM_VIEWS
    #extension GL_OVR_multiview2 : enable

    layout(num_views = NUM_VIEWS) in;

    uniform mat4 osg_ProjectionMatrices[NUM_VIEWS];
    #define osg_ProjectionMatrix osg_ProjectionMatrices[gl_ViewID_OVR]
#else
    uniform mat4 osg_ProjectionMatrix;
#endif

uniform mat4 osg_ModelViewMatrix;

in vec4 osg_Vertex;
in vec4 osg_Color;
in vec4 osg_MultiTexCoord0;

out vec4 color;
out vec2 texcoord;

void main(void)
{
    mat4 mvp = osg_ProjectionMatrix * osg_ModelViewMatrix;

    color = osg_Color;
    texcoord = osg_MultiTexCoord0.xy;

    gl_Position = mvp * osg_Vertex;
}
