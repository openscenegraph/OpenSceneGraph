uniform vec3 position;
uniform vec3 dv_i;
uniform vec3 dv_j;
uniform vec3 dv_k;

uniform float inversePeriod;
uniform float startTime;

uniform float osg_FrameTime;

varying vec2 texCoord;

void main(void)
{
    vec3 pos = position + (gl_Vertex.x*dv_i) + (dv_j * gl_Vertex.y);
    
    texCoord = gl_MultiTexCoord0.xy;

    vec3 v_current = pos + dv_k * fract( (osg_FrameTime - startTime)*inversePeriod - gl_Vertex.z);
   
    gl_Position = gl_ModelViewProjectionMatrix * vec4(v_current,1.0);
}
