#include    "FTTextureGlyph.h"
#include    "FTGL.h"


FTTextureGlyph::FTTextureGlyph( FT_Glyph glyph, int id, unsigned char* data, GLsizei stride, GLsizei height, float u, float v)
:    FTGlyph(),
    destWidth(0),
    destHeight(0),
    numGreys(0),
    glTextureID(id)
{
    // This function will always fail if the glyph's format isn't scalable????
    err = FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1);
    if( err || glyph->format != ft_glyph_format_bitmap)
    {
        return;
    }

    FT_BitmapGlyph  bitmap = ( FT_BitmapGlyph)glyph;
    FT_Bitmap*      source = &bitmap->bitmap;

    //check the pixel mode
    //ft_pixel_mode_grays
        
    int srcWidth = source->width;
    int srcHeight = source->rows;
    int srcPitch = source->pitch;
    
    numGreys = source->num_grays;
    advance = glyph->advance.x >> 16;

     pos.x = bitmap->left;
    pos.y = bitmap->top;
    
    destWidth = srcWidth;
    destHeight = srcHeight;
    
    for(int y = 0; y < srcHeight; ++y)
    {
        for(int x = 0; x < srcWidth; ++x)
        {
            *( data + ( y * stride  + x)) = *( source->buffer + ( y * srcPitch) + x);
        }        
    }

//        0    
//        +----+
//        |    |
//        |    |
//        |    |
//        +----+
//             1
    
    uv[0].x = u;
    uv[0].y = v;
    uv[1].x = uv[0].x + ( (float)destWidth / (float)stride);
    uv[1].y = uv[0].y + ( (float)destHeight / (float)height);

    // discard glyph image (bitmap or not)
    // Is this the right place to do this?
    FT_Done_Glyph( glyph);
}


FTTextureGlyph::~FTTextureGlyph()
{

}


float FTTextureGlyph::Render( const FT_Vector& pen)
{
    glGetIntegerv( GL_TEXTURE_2D_BINDING_EXT, &activeTextureID);
    if( activeTextureID != glTextureID)
    {
        glBindTexture( GL_TEXTURE_2D, (GLuint)glTextureID);
    }
    
    glBegin( GL_QUADS);
    glTexCoord2f( uv[0].x, uv[0].y); glVertex2f( pen.x + pos.x,                pen.y + pos.y);
    glTexCoord2f( uv[1].x, uv[0].y); glVertex2f( pen.x + destWidth + pos.x,    pen.y + pos.y);
    glTexCoord2f( uv[1].x, uv[1].y); glVertex2f( pen.x + destWidth + pos.x,    pen.y + pos.y - destHeight);
    glTexCoord2f( uv[0].x, uv[1].y); glVertex2f( pen.x + pos.x,                pen.y + pos.y - destHeight);
    glEnd();

    return advance;

}

