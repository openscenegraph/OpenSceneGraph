#include	"FTGLPolygonFont.h"
#include	"FTGlyphContainer.h"
#include	"FTGL.h"
#include	"FTPolyGlyph.h"



FTGLPolygonFont::FTGLPolygonFont()
:	tempGlyph(0)
{}


FTGLPolygonFont::~FTGLPolygonFont()
{}


bool FTGLPolygonFont::MakeGlyphList()
{
	for( unsigned int n = 0; n < numGlyphs; ++n)
	{
		FT_Glyph* ftGlyph = face.Glyph( n, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
		
		if( ftGlyph)
		{
			tempGlyph = new FTPolyGlyph( *ftGlyph);
			glyphList->Add( tempGlyph);
		}
		else
		{
			err = face.Error();
		}
	}
	
	return !err;
}
