#ifndef        __FTBitmapGlyph__
#define        __FTBitmapGlyph__

#include "FTGL.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include    "FTGlyph.h"


/**
 * FTBitmapGlyph is a specialisation of FTGlyph for creating bitmaps.
 *
 * It provides the interface between Freetype glyphs and their openGL
 * renderable counterparts. This is an abstract class and derived classes
 * must implement the <code>render</code> function. 
 * 
 * @see FTGlyphContainer
 *
 */
class FTGL_EXPORT FTBitmapGlyph : public FTGlyph
{
    public:
        /**
         * Constructor
         *
         * @param glyph    The Freetype glyph to be processed
         */
        FTBitmapGlyph( FT_Glyph glyph);

        /**
         * Destructor
         */
        virtual ~FTBitmapGlyph();

        /**
         * Renders this glyph at the current pen position.
         *
         * @param pen    The current pen position.
         * @return        The advance distance for this glyph.
         */
        virtual float Render( const FT_Vector& pen);
        
    private:
        /**
         * The width of the glyph 'image'
         */
        int destWidth;

        /**
         * The height of the glyph 'image'
         */
        int destHeight;
        
        /**
         * Pointer to the 'image' data
         */
        unsigned char* data;
        
};


#endif    //    __FTBitmapGlyph__

