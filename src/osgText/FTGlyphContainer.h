#ifndef		__FTGlyphContainer__
#define		__FTGlyphContainer__

#include "FTGL.h"

#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

//#include "FTGL.h"
class FTFace;
class FTGlyph;

using namespace std;

/**
 * FTGlyphContainer holds the post processed FTGlyph objects.
 *
 * @see FTGlyph
 */
class FTGL_EXPORT FTGlyphContainer
{
	public:
		/**
		 * Constructor
		 *
		 * @param face		The Freetype face
		 * @param numGlyphs	the number of glyphs in this face
		 * @param p			A flag to indicate preprocessing of glyphs.
		 *					Not used.
		 */
		FTGlyphContainer( FTFace* face, unsigned int numGlyphs, bool p = false);

		/**
		 * Destructor
		 */
		virtual ~FTGlyphContainer();

		/**
		 * Adds a glyph to this glyph list.
		 *
		 * @param glyph		
		 * @return			<code>true</code>
		 */
		bool Add( FTGlyph* glyph);

		/**
		* Returns the kerned advance width for a glyph.
		*
		* @param index	glyph index of the character
		* @param next	the next glyph in a string
		* @return 		advance width
		*/
		float Advance( unsigned int index, unsigned int next);
		
		/**
		 * renders a character
		 * @param index	the glyph to be rendered
		 * @param next	the next glyph in the string. Used for kerning.
		 * @param pen	the position to render the glyph
		 * @return 		The distance to advance the pen position after rendering
		 */
		FT_Vector& render( unsigned int index, unsigned int next, FT_Vector pen);
		
		/**
		 * Queries the Font for errors.
		 *
		 * @return	The current error code.
		 */
		virtual FT_Error Error() const { return err;}

	private:
		/**
		 * A flag to indicate preprocessing of glyphs. Not used.
		 */
		bool preCache;

		/**
		 * How meny glyphs are stored in this container
		 */
		int numGlyphs;

		/**
		 * The current Freetype face
		 */
		FTFace* face;

		/**
		 * The kerning vector for the current pair of glyphs
		 */
		FT_Vector kernAdvance;

		/**
		 * The advance for the glyph being rendered
		 */
		float advance;

		/**
		 * A structure to hold the glyphs
		 */
		vector<FTGlyph*> glyphs;
//		typedef pair<int, FTGlyph*> CHARREF; // glyphIndex, glyph
//		vector<CHARREF> glyphs;
//		map< int, FTGlyph*> CHARREF; // charCode, glyph

		/**
		 * Current error code. Zero means no error.
		 */
		FT_Error err;
		
};


#endif	//	__FTGlyphContainer__
