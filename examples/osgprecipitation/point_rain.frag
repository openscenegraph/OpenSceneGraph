uniform sampler2D baseTexture;
uniform vec4 particleColour;
varying vec2 texCoord;

void main (void)
{
    gl_FragColor = particleColour * texture2D( baseTexture, texCoord);
}
