#ifndef		__FTGLBitmapFont__
#define		__FTGLBitmapFont__


#include "FTGL.h"

#include "FTFont.h"

class FTBitmapGlyph;

/**
 * FTGLBitmapFont is a specialisation of the FTFont class for handling
 * Bitmap fonts
 *
 * @see		FTFont
 */
class FTGL_EXPORT FTGLBitmapFont : public FTFont
{
	public:
		/**
		 * Constructor
		 */
		FTGLBitmapFont();

		/**
		 * Destructor
		 */
		~FTGLBitmapFont();
		
		/**
		 * Renders a string of characters
		 * 
		 * @param string	'C' style string to be output.	 
		 */
		void render( const char* string);

		/**
		 * Renders a string of characters
		 * 
		 * @param string	'C' style wide string to be output.	 
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
		
};
#endif	//	__FTGLBitmapFont__
