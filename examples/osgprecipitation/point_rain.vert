uniform vec3 dv_i;
uniform vec3 dv_j;
uniform vec3 dv_k;

uniform float inversePeriod;
uniform float startTime;
uniform vec4 particleColour;
uniform float particleSize;

uniform float osg_FrameTime;

varying vec4 colour;
varying vec2 texCoord;

void main(void)
{
    vec3 pos = gl_Normal.xyz + (gl_Vertex.x*dv_i) + (dv_j * gl_Vertex.y);
    
    texCoord = gl_MultiTexCoord0.xy;

    vec3 v_current = pos + dv_k * fract( (osg_FrameTime - startTime)*inversePeriod - gl_Vertex.z);
   
    colour = particleColour;

    gl_Position = gl_ModelViewProjectionMatrix * vec4(v_current,1.0);

    float pointSize = min(abs(1280*particleSize / gl_Position.w), 20.0);
    gl_PointSize = max(ceil(pointSize),2);
    
    colour.a = 0.05+(pointSize)/(gl_PointSize);
}
