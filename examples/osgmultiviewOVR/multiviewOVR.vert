varying vec3 texcoord;

void main(void)
{
    texcoord = gl_MultiTexCoord0.xyz;
    gl_Position   = gl_ModelViewProjectionMatrix * gl_Vertex;
}
