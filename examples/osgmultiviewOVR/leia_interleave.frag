#extension GL_EXT_texture_array : enable

#pragma import_defines ( WINDOW_WIDTH, WINDOW_HEIGHT, INTERLEAVE )

uniform sampler2DArray texture_0;
uniform sampler2DArray texture_1;
uniform sampler2DArray texture_2;
uniform sampler2DArray texture_3;

varying vec2 texcoord;

#ifndef WINDOW_WIDTH
    #define WINDOW_WIDTH 1920
    #define WINDOW_HEIGHT 1080
#endif

#define INTERLEAVE

void main(void)
{
#ifdef INTERLEAVE

    float i = mod(gl_FragCoord.x, 4.0);
    float j = mod(gl_FragCoord.y, 4.0);

    vec3 tc = vec3(gl_FragCoord.x / WINDOW_WIDTH, gl_FragCoord.y / WINDOW_HEIGHT, i);

    if (j < 1.0) gl_FragColor = texture2DArray( texture_0, tc);
    else if (j < 2.0) gl_FragColor = texture2DArray( texture_1, tc);
    else if (j < 3.0) gl_FragColor = texture2DArray( texture_2, tc);
    else gl_FragColor = texture2DArray( texture_3, tc);

#else
    float cell_width = WINDOW_WIDTH/4.0;
    float cell_height = WINDOW_HEIGHT/4.0;

    float i = floor(gl_FragCoord.x / cell_width);
    float j = floor(gl_FragCoord.y / cell_height);

    vec3 tc = vec3(gl_FragCoord.x / cell_width - i, gl_FragCoord.y / cell_height - j, i);

    if (j < 1.0) gl_FragColor = texture2DArray( texture_0, tc);
    else if (j < 2.0) gl_FragColor = texture2DArray( texture_1, tc);
    else if (j < 3.0) gl_FragColor = texture2DArray( texture_2, tc);
    else gl_FragColor = texture2DArray( texture_3, tc);
#endif
}
