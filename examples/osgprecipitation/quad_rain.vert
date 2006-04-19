uniform float inversePeriod;
uniform vec4 particleColour;
uniform float particleSize;

uniform float osg_FrameTime;
uniform float osg_DeltaFrameTime;

varying vec4 colour;
varying vec2 texCoord;

void main(void)
{
    float offset = gl_Vertex.z;
    float startTime = gl_MultiTexCoord1.x;
    texCoord = gl_MultiTexCoord0.xy;

    vec4 v_previous = gl_Vertex;
    v_previous.z = fract( (osg_FrameTime - startTime)*inversePeriod - offset);
    
    vec4 v_current =  v_previous;
    v_current.z += (osg_DeltaFrameTime*inversePeriod);
    

    colour = particleColour;
    
    vec4 v1 = gl_ModelViewMatrix * v_current;
    vec4 v2 = gl_TextureMatrix[0] * v_previous;
    
    vec3 dv = v2.xyz - v1.xyz;
    
    vec2 dv_normalized = normalize(dv.xy);
    dv.xy += dv_normalized * particleSize;
    vec2 dp = vec2( -dv_normalized.y, dv_normalized.x ) * particleSize;
    
    float area = length(dv.xy);
    colour.a = 0.05+(particleSize)/area;
    

    v1.xyz += dv*texCoord.y;
    v1.xy += dp*texCoord.x;
    
    gl_Position = gl_ProjectionMatrix * v1;
}
