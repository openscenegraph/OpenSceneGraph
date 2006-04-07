uniform vec3 delta;
uniform float inversePeriod;
uniform float startTime;
uniform float osg_FrameTime;
uniform float osg_DeltaFrameTime;
uniform mat4 previousModelViewMatrix;

varying vec4 colour;
varying vec2 texCoord;

void main(void)
{
    const float particleSize = 0.01;
    const float particleSize2 = 0.0001;//particleSize*particleSize;

    float offset = gl_MultiTexCoord0.z;
    texCoord = gl_MultiTexCoord0.xy;

    vec3 v_previous = gl_Vertex.xyz + delta * fract( (osg_FrameTime - startTime)*inversePeriod - offset);
    vec3 v_current = v_previous + delta * (osg_DeltaFrameTime*inversePeriod);
    
    colour = gl_Color;
    
    vec4 v1 = gl_ModelViewMatrix * vec4(v_current,1.0);
    vec4 v2 = previousModelViewMatrix * vec4(v_previous,1.0);
    
    vec3 dv = v2.xyz - v1.xyz;
    
    vec2 dv_normalized = normalize(dv.xy);
    dv.xy += dv_normalized * particleSize;
    vec2 dp = vec2( -dv_normalized.y, dv_normalized.x ) * particleSize;
    
    float area = length(dv.xy)*length(dp);
    colour.a = 0.1+(particleSize2)/area;
    

    v1.xyz += dv*texCoord.y;
    v1.xy += dp*texCoord.x;
    
    gl_Position = gl_ProjectionMatrix * v1;
}
