#ifndef		__FTGLOutlineFont
#define		__FTGLOutlineFont


#include	"FTFont.h"
#include "FTGL.h"


class FTOutlineGlyph;

/**
 * FTGLOutlineFont is a specialisation of the FTFont class for handling
 * Vector Outline fonts
 *
 * @see		FTFont
 */
class	FTGLOutlineFont : public FTFont
{
	public:
		// methods
		FTGLOutlineFont();
		~FTGLOutlineFont();
		
		void render( const char* string);
		void render( const wchar_t* string);

		// attributes
		
	private:
		// methods
		bool MakeGlyphList();
		
		// attributes
		FTOutlineGlyph* tempGlyph;
	
};
#endif
