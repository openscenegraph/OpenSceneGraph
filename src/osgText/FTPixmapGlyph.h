#ifndef        __FTPixmapGlyph__
#define        __FTPixmapGlyph__

#include "FTGL.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include    "FTGlyph.h"


/**
 * FTPixmapGlyph is a specialisation of FTGlyph for creating pixmaps.
 * 
 * @see FTGlyphContainer
 *
 */
class  FTGL_EXPORT FTPixmapGlyph : public FTGlyph
{
    public:
        /**
         * Constructor
         *
         * @param glyph    The Freetype glyph to be processed
         */
        FTPixmapGlyph( FT_Glyph glyph);

        /**
         * Destructor
         */
        virtual ~FTPixmapGlyph();

        /**
         * Renders this glyph at the current pen position.
         *
         * @param pen    The current pen position.
         * @return        The advance distance for this glyph.
         */
        virtual float Render( const FT_Vector& pen);
        
        // attributes
	//
	

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
         * The number of greys or bit depth of the image
         */
        int numGreys;

        /**
         * Pointer to the 'image' data
         */
        unsigned char* data;
        
};


#endif    //    __FTPixmapGlyph__
