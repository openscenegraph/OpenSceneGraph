#ifndef		__FTGLPolygonFont__
#define		__FTGLPolygonFont__

#include	"FTFont.h"

#include "FTGL.h"


class FTPolyGlyph;

/**
 * FTGLPolygonFont is a specialisation of the FTFont class for handling
 * tesselated Polygon Mesh fonts
 *
 * @see		FTFont
 */
class	FTGLPolygonFont : public FTFont
{
	public:
		// methods
		FTGLPolygonFont();
		~FTGLPolygonFont();
		
		// attributes
		
	private:
		// methods
		bool MakeGlyphList();
		
		// attributes
		FTPolyGlyph* tempGlyph;

		
};
#endif	//	__FTGLPolygonFont__
