#ifndef		__FTGLOutlineFont__
#define		__FTGLOutlineFont__

#include "FTGL.h"

#include "FTFont.h"


class FTOutlineGlyph;

/**
 * FTGLOutlineFont is a specialisation of the FTFont class for handling
 * Vector Outline fonts
 *
 * @see		FTFont
 */
class FTGL_EXPORT FTGLOutlineFont : public FTFont
{
	public:
		/**
		 * Default Constructor
		 */
		FTGLOutlineFont();
		
		/**
		 * Destructor
		 */
		~FTGLOutlineFont();
		
		/**
		 * Renders a string of characters
		 * 
		 * @param string	'C' style string to be output.	 
		 */
		void render( const char* string);
		
		/**
		 * Renders a string of characters
		 * 
		 * @param string	wchar_t string to be output.	 
		 */
		void render( const wchar_t* string);

		// attributes
		
	private:
		/**
		 * Constructs the internal glyph cache.
		 *
		 * This a list of glyphs processed for openGL rendering NOT
		 * freetype glyphs
		 */
		bool MakeGlyphList();
		
		/**
		 * A temporary FTOutlineGlyph used for building the glyphList
		 */
		FTOutlineGlyph* tempGlyph;
	
};
#endif // __FTGLOutlineFont__
