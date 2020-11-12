#extension GL_EXT_texture_array : enable

uniform sampler2DArray texture;
varying vec3 texcoord;

void main(void)
{
    gl_FragColor = texture2DArray( texture, texcoord.xyz);
}
