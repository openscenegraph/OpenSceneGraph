#ifndef		__FTOutlineGlyph__
#define		__FTOutlineGlyph__

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "FTGlyph.h"

class FTVectoriser;

class	FTOutlineGlyph : public FTGlyph
{
	public:
		// methods
		FTOutlineGlyph( FT_Glyph glyph);
		virtual ~FTOutlineGlyph();
		virtual float Render( const FT_Vector& pen);
		
		// attributes
	
	private:
		// methods
		
		// attributes
		FTVectoriser* vectoriser;
		int numPoints;
		int numContours;
		int* contourLength;
		double* data;
		int glList;
	
};


#endif	//	__FTOutlineGlyph__
