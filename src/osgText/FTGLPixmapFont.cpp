#include	"FTGLPixmapFont.h"
#include	"FTGlyphContainer.h"
#include	"FTPixmapGlyph.h"


FTGLPixmapFont::FTGLPixmapFont()
:	tempGlyph(0)
{}


FTGLPixmapFont::~FTGLPixmapFont()
{}


// OPSignature: bool FTGlyphContainer:MakeGlyphList() 
bool FTGLPixmapFont::MakeGlyphList()
{
//	if( preCache)
	for( unsigned int c = 0; c < numGlyphs; ++c)
	{
		FT_Glyph* ftGlyph = face.Glyph( c, FT_LOAD_DEFAULT);
//		FT_HAS_VERTICAL(face)
	
		if( ftGlyph)
		{
			tempGlyph = new FTPixmapGlyph( *ftGlyph);
			glyphList->Add( tempGlyph);
		}
		else
		{
			err = face.Error();
		}
	}
	
	return !err;
}


void FTGLPixmapFont::render( const char* string)
{	
	glPushAttrib( GL_ENABLE_BIT | GL_PIXEL_MODE_BIT);

	glEnable(GL_BLEND);
 	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	FTFont::render( string);

	glPopAttrib();

}


void FTGLPixmapFont::render( const wchar_t* string)
{	
	glPushAttrib( GL_ENABLE_BIT | GL_PIXEL_MODE_BIT);

	glEnable(GL_BLEND);
 	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	FTFont::render( string);

	glPopAttrib();

}

