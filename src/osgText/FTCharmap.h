#ifndef        __FTCharmap__
#define        __FTCharmap__

#include "FTGL.h"

#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

//#include "FTGL.h"

using namespace std;


/**
 * FTCharmap takes care of specifying the encodeing for a font and mapping
 * character codes to glyph indices.
 *
 * It doesn't preprocess all indices, only on as needed basis. This may seem
 * like a performance penalty but it is quicker than using the 'raw'
 * freetype calls and will save significant amounts of memory when dealing
 * with uncode encoding
 *
 */
class FTGL_EXPORT FTCharmap
{
    public:
        /**
         * Constructor
         */
        FTCharmap( FT_Face ftFace);

        /**
         * Destructor
         */
        virtual ~FTCharmap();

        /**
         * Queries for the current character map code.
         *
         * @return    The current character map code.
         */
        FT_Encoding Encoding() const { return ftEncoding;}
        
        /**
         * Sets the character map for the face.
         * Valid encodings as at Freetype 2.0.4
         *        ft_encoding_none
         *        ft_encoding_symbol
         *        ft_encoding_unicode
         *        ft_encoding_latin_2
         *        ft_encoding_sjis
         *        ft_encoding_gb2312
         *        ft_encoding_big5
         *        ft_encoding_wansung
         *        ft_encoding_johab
         *        ft_encoding_adobe_standard
         *        ft_encoding_adobe_expert
         *        ft_encoding_adobe_custom
         *        ft_encoding_apple_roman
         *
         * @param encoding        the Freetype encoding symbol. See above.
         * @return                <code>true</code> if charmap was valid
         *                        and set correctly
         */
        bool CharMap( FT_Encoding encoding);

        /**
         * Sets the character map for the face.
         *
         * @param encoding        the Freetype encoding symbol. See above.
         * @return                <code>true</code> if charmap was valid
         *                        and set correctly
         */
        bool CharMap( FT_UShort platform, FT_UShort encoding);

        /**
         *    Get the glyph index of the input character.
         *
         * @param index The character code of the requested glyph in the
         *                current encoding eg apple roman.
         * @return        The glyph index for the character.
         */
        unsigned int CharIndex( unsigned int index );

        /**
         * Queries for errors.
         *
         * @return    The current error code.
         */
        FT_Error Error() const { return err;}
        
    protected:
        /**
         * Current character map code.
         */
        FT_Encoding ftEncoding;
        
        /**
         * The current Freetype face.
         */
        FT_Face ftFace;
        
        /**
         * A structure that maps glyph indices to character codes
         *
         * < character code, face glyph index>
         */
        typedef map< unsigned long, unsigned long> CharacterMap;
        CharacterMap charMap;
        
        /**
         * Current error code. Zero means no error.
         */
        FT_Error err;
        
    private:

};


#endif    //    __FTCharmap__
