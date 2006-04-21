uniform sampler2D baseTexture;
varying vec4 colour;

void main (void)
{
    gl_FragColor = colour * texture2D( baseTexture, gl_TexCoord[0].xy);
}
