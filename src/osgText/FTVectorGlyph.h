#ifndef		__FTVectorGlyph__
#define		__FTVectorGlyph__

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "FTGlyph.h"

class FTVectoriser;

class	FTVectorGlyph : public FTGlyph
{
	public:
		// methods
		FTVectorGlyph( FT_Glyph glyph);
		virtual ~FTVectorGlyph();
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


#endif	//	__FTVectorGlyph__
