#include	"FTVectorGlyph.h"
#include	"FTVectoriser.h"
#include	"FTGL.h"



FTVectorGlyph::FTVectorGlyph( FT_Glyph glyph)
:	FTGlyph(),
	vectoriser(0),
	numPoints(0),
	numContours(0),
	contourLength(0),
	data(0),
	glList(0)
{
	if( ft_glyph_format_outline != glyph->format)
	{
		return;
	}

	vectoriser = new FTVectoriser( glyph);
	
	vectoriser->Ingest();
	numContours = vectoriser->contours();
	contourLength = new int[ numContours];
	
	int c;
	for( c = 0; c < numContours; ++c)
	{
		contourLength[c] = vectoriser->contourSize( c);
	}
	
	numPoints = vectoriser->points();
	data = new double[ numPoints * 3];
	vectoriser->Output( data);
	
	advance = glyph->advance.x >> 16;

	delete vectoriser;
	
	if ( ( numContours < 1) || ( numPoints < 3))
		return;
		
	glList = glGenLists(1);
	int d = 0;

	glNewList( glList, GL_COMPILE);
		for( c = 0; c < numContours; ++c)
		{
			glBegin( GL_LINE_LOOP);
			for( int p = 0; p < ( contourLength[c]); ++p)
			{
				glVertex2dv( data + d);
				d += 3;
			}
			glEnd();
		}
	glEndList();

	// discard glyph image (bitmap or not)
	FT_Done_Glyph( glyph); // Why does this have to be HERE
}


FTVectorGlyph::~FTVectorGlyph()
{
	delete [] data;
	delete [] contourLength;
}


float FTVectorGlyph::Render( const FT_Vector& pen)
{
	if( glList)
	{
		glTranslatef( pen.x, pen.y, 0);
			glCallList( glList);
		glTranslatef( -pen.x, -pen.y, 0);
	}
	
	return advance;
}




