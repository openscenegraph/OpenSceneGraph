#ifndef		__FTGlyph__
#define		__FTGlyph__

#include "FTGL.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

//#include "FTGL.h"

/**
 * FTGlyph is the base class for FTGL glyphs.
 *
 * It provides the interface between Freetype glyphs and their openGL
 * renderable counterparts. This is an abstract class and derived classes
 * must implement the <code>render</code> function. 
 * 
 * @see FTGlyphContainer
 *
 */
class FTGL_EXPORT FTGlyph
{
	public:
		/**
		 * Constructor
		 */
		FTGlyph();

		/**
		 * Destructor
		 */
		virtual ~FTGlyph();

		/**
		 * Renders this glyph at the current pen position.
		 *
		 * @param pen	The current pen position.
		 * @return		The advance distance for this glyph.
		 */
		virtual float Render( const FT_Vector& pen) = 0;
		
		/**
		 * Return the advance width for this glyph.
		 *
		 * @return	advance width.
		 */
		float Advance() const { return advance;}
		
		/**
		 * Queries for errors.
		 *
		 * @return	The current error code.
		 */
		FT_Error Error() const { return err;}
		
	protected:
		/**
		 * The advance distance for this glyph
		 */
		float advance;

		/**
		 * Vector from the pen position to the topleft corner of the glyph
		 */
		FT_Vector pos;

		/**
		 * Current error code. Zero means no error.
		 */
		FT_Error err;
		
	private:

};


#endif	//	__FTGlyph__

