#ifndef		__FTOutlineGlyph__
#define		__FTOutlineGlyph__

#include "FTGL.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "FTGlyph.h"

class FTVectoriser;


/**
 * FTOutlineGlyph is a specialisation of FTGlyph for creating outlines.
 * 
 * @see FTGlyphContainer
 * @see FTVectoriser
 *
 */
class FTGL_EXPORT FTOutlineGlyph : public FTGlyph
{
	public:
		/**
		 * Constructor
		 *
		 * @param glyph	The Freetype glyph to be processed
		 */
		FTOutlineGlyph( FT_Glyph glyph);

		/**
		 * Destructor
		 */
		virtual ~FTOutlineGlyph();

		/**
		 * Renders this glyph at the current pen position.
		 *
		 * @param pen	The current pen position.
		 * @return		The advance distance for this glyph.
		 */
		virtual float Render( const FT_Vector& pen);
		
	private:
		/**
		 * An object that helps convert freetype outlines into point
		 * data
		 */
		FTVectoriser* vectoriser;

		/**
		 * The total number of points in the Freetype outline
		 */
		int numPoints;

		/**
		 * The totals number of contours in the Freetype outline
		 */
		int numContours;

		/**
		 * An array containing the number of points in each outline
		 */
		int* contourLength;

		/**
		 * Pointer to the point data
		 */
		double* data;
		
		/**
		 * OpenGL display list
		 */
		int glList;
	
};


#endif	//	__FTOutlineGlyph__

