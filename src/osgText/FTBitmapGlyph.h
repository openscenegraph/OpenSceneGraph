#ifndef		__FTBitmapGlyph__
#define		__FTBitmapGlyph__

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include	"FTGlyph.h"


class	FTBitmapGlyph : public FTGlyph
{
	public:
		/**
		 * Constructor
		 *
		 * @param
		 */
		FTBitmapGlyph( FT_Glyph glyph);

		/**
		 *
		 */
		virtual ~FTBitmapGlyph();

		/**
		 *
		 * @param pen	
		 *
		 * @return		
		 */
		virtual float Render( const FT_Vector& pen);
		
		// attributes
		
	private:
		// methods
		
		// attributes
		int destWidth;
		int destHeight;
		
		unsigned char* data;
		
};


#endif	//	__FTBitmapGlyph__
