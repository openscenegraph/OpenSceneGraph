uniform vec3 delta;
uniform float inversePeriod;
uniform float startTime;
uniform float osg_FrameTime;
varying vec4 colour;

void main(void)
{
    float offset = gl_MultiTexCoord0.x;

    vec3 v = gl_Vertex.xyz + delta * fract( (osg_FrameTime - startTime)*inversePeriod - offset);
    
    colour = gl_Color;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(v,1.0);
}
