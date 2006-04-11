uniform sampler2D baseTexture;
varying vec2 texCoord;
varying vec4 colour;

void main (void)
{
    gl_FragColor = colour * texture2D( baseTexture, texCoord);
}
