#include    "FTBitmapGlyph.h"


FTBitmapGlyph::FTBitmapGlyph( FT_Glyph glyph)
:    FTGlyph(),
    destWidth(0),
    destHeight(0),
    data(0)
{
    // This function will always fail if the glyph's format isn't scalable????
    FT_Error err = FT_Glyph_To_Bitmap( &glyph, ft_render_mode_mono, 0, 1);
    if( err || ft_glyph_format_bitmap != glyph->format)
    {return;}

    advance = glyph->advance.x >> 16;

    FT_BitmapGlyph  bitmap = (FT_BitmapGlyph)glyph;
    FT_Bitmap*      source = &bitmap->bitmap;

    //check the pixel mode
    //ft_pixel_mode_grays
        
    int srcWidth = source->width;
    int srcHeight = source->rows;
    int srcPitch = source->pitch;
    
    if (srcPitch*srcHeight==0) 
    {
        FT_Done_Glyph( glyph );
        return;
    }

    pos.x = bitmap->left;
    pos.y = srcHeight - bitmap->top;
    
   // FIXME What about dest alignment?
    destWidth = srcWidth;
    destHeight = srcHeight;
    
    data = osgNew unsigned char[srcPitch * destHeight];
    
    for(int y = 0; y < srcHeight; ++y)
    {
        --destHeight;
        for(int x = 0; x < srcPitch; ++x)
        {
            *( data + ( destHeight * srcPitch + x)) = *( source->buffer + ( y * srcPitch) + x);
        }        
    }

    destHeight = srcHeight;

    // discard glyph image (bitmap or not)
    // Is this the right place to do this?
    FT_Done_Glyph( glyph );
}


FTBitmapGlyph::~FTBitmapGlyph()
{
    osgDelete [] data;
}


float FTBitmapGlyph::Render( const FT_Vector& pen)
{
    if( data != 0 )
    {
        // Move the glyph origin
        glBitmap( 0, 0, 0.0, 0.0, pen.x + pos.x, pen.y - pos.y, (const GLubyte *)0 );

        glBitmap( destWidth, destHeight, 0.0f, 0.0, 0.0, 0.0, (const GLubyte *)data);

        // Restore the glyph origin
        glBitmap( 0, 0, 0.0, 0.0, -pen.x - pos.x, -pen.y + pos.y, (const GLubyte *)0 );
    }
    
    return advance;
}
