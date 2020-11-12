#version 330

uniform sampler2D texture;

in vec2 texcoord;

out vec4 fragColor;

void main(void)
{
    fragColor = texture2D( texture, texcoord.xy);
}
