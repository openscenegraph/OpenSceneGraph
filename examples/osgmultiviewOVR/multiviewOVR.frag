#version 330

uniform sampler2D texture;

in vec4 color;
in vec2 texcoord;

out vec4 fragColor;

void main(void)
{
    fragColor = color * texture2D( texture, texcoord.xy);
    if (fragColor.a==0.0) discard;
}
