#include	"FTGLBitmapFont.h"
#include	"FTGlyphContainer.h"
#include	"FTBitmapGlyph.h"


FTGLBitmapFont::FTGLBitmapFont()
:	tempGlyph(0)
{}


FTGLBitmapFont::~FTGLBitmapFont()
{}


// OPSignature: bool FTGlyphContainer:MakeGlyphList() 
bool FTGLBitmapFont::MakeGlyphList()
{
//	if( preCache)
	for( unsigned int c = 0; c < numGlyphs; ++c)
	{
		FT_Glyph* ftGlyph = face.Glyph( c, FT_LOAD_DEFAULT);
//		FT_HAS_VERTICAL(face)

		if( ftGlyph)
		{
			tempGlyph = new FTBitmapGlyph( *ftGlyph);
			glyphList->Add( tempGlyph);
		}
		else
		{
			err = face.Error();
		}
	}
	
	return !err;
}


void FTGLBitmapFont::render( const char* string)
{	
	glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT);
	
	// doing this every frame is a bad?
	glPixelStorei( GL_UNPACK_LSB_FIRST, GL_FALSE);
	glPixelStorei( GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1);

	FTFont::render( string);

	glPopClientAttrib();

}


void FTGLBitmapFont::render( const wchar_t* string)
{	
	glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT);
	
	// doing this every frame is a bad?
	glPixelStorei( GL_UNPACK_LSB_FIRST, GL_FALSE);
	glPixelStorei( GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1);

	FTFont::render( string);

	glPopClientAttrib();

}
