#ifndef        __FTTextureGlyph__
#define        __FTTextureGlyph__

#include "FTGL.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "FTGlyph.h"


/**
 * FTTextureGlyph is a specialisation of FTGlyph for creating texture
 * glyphs.
 * 
 * @see FTGlyphContainer
 *
 */
class FTGL_EXPORT FTTextureGlyph : public FTGlyph
{
    public:
        /**
         * Constructor
         *
         * @param glyph        The Freetype glyph to be processed
         * @param id        The index of the texture that this glyph will
         *                    be drawn in
         * @param data        A pointer to the texture memory
         * @param stride    The stride of the texture memory
         * @param height    The height (number of rows) of the texture memory
         * @param u            The texture co-ord for this glyph
         * @param v            The texture co-ord for this glyph
         */
        FTTextureGlyph( FT_Glyph glyph, int id, unsigned char* data, GLsizei stride, GLsizei height, float u, float v);

        /**
         * Destructor
         */
        virtual ~FTTextureGlyph();

        /**
         * Renders this glyph at the current pen position.
         *
         * @param pen    The current pen position.
         * @return        The advance distance for this glyph.
         */
        virtual float Render( const FT_Vector& pen);
        
        /**
         * The texture index of the currently active texture
         *
         * We call glGetIntegerv( GL_TEXTURE_2D_BINDING, activeTextureID);
         * to get the currently active texture to try to reduce the number
         * of texture bind operations
         */
        GLint activeTextureID;
        
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
         * A structure to hold the uv co-ords.
         */
        struct FTPoint
        {
            float x;
            float y;
        };

        /**
         * The texture co-ords of this glyph within the texture.
         */
        FTPoint uv[2];
        
        /**
         * The texture index that this glyph is contained in.
         */
        int glTextureID;
};


#endif    //    __FTTextureGlyph__
