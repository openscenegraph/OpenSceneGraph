uniform vec3 dv_i;
uniform vec3 dv_j;
uniform vec3 dv_k;

uniform float inversePeriod;
uniform float startTime;
uniform vec4 particleColour;
uniform float particleSize;

uniform float osg_FrameTime;
uniform float osg_DeltaFrameTime;
uniform mat4 previousModelViewMatrix;

varying vec4 colour;
varying vec2 texCoord;

void main(void)
{
    vec3 pos = gl_Normal.xyz + (gl_Vertex.x*dv_i) + (dv_j * gl_Vertex.y);
    
    float offset = gl_Vertex.z;
    texCoord = gl_MultiTexCoord0.xy;

    vec3 v_previous = pos + dv_k * fract( (osg_FrameTime - startTime)*inversePeriod - offset);
    vec3 v_current = v_previous + dv_k * (osg_DeltaFrameTime*inversePeriod);
    
    colour = particleColour;
    
    vec4 v1 = gl_ModelViewMatrix * vec4(v_current,1.0);
    vec4 v2 = previousModelViewMatrix * vec4(v_previous,1.0);
    
    vec3 dv = v2.xyz - v1.xyz;
    
    vec2 dv_normalized = normalize(dv.xy);
    dv.xy += dv_normalized * particleSize;
    vec2 dp = vec2( -dv_normalized.y, dv_normalized.x ) * particleSize;
    
    float area = length(dv.xy)*length(dp);
    colour.a = 0.05+(particleSize*particleSize)/area;
    

    v1.xyz += dv*texCoord.y;
    v1.xy += dp*texCoord.x;
    
    gl_Position = gl_ProjectionMatrix * v1;
}
