#include	"FTFace.h"
#include	"FTFont.h"
#include	"FTGlyphContainer.h"
#include	"FTGL.h"


FTFont::FTFont()
:	numFaces(0),
	glyphList(0),
	err(0)
{
	pen.x = 0;
	pen.y = 0;
}


FTFont::~FTFont()
{
	Close();
}


bool FTFont::Open( const char* fontname )
{
	if( face.Open( fontname))
	{
		FT_Face* ftFace = face.Face();		
		numGlyphs = (*ftFace)->num_glyphs;
		
		return true;
	}
	else
	{
		err = face.Error();
		return false;
	}
}


void FTFont::Close()
{
	delete glyphList;
}


bool FTFont::FaceSize( const unsigned int size, const unsigned int res )
{
	charSize = face.Size( size, res);

	if( glyphList)
		delete glyphList;
	
	glyphList = new FTGlyphContainer( &face, numGlyphs);
	
	if( MakeGlyphList())
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool FTFont::CharMap( FT_Encoding encoding)
{
	err = face.CharMap( encoding);
	return !err;
}


int	FTFont::Ascender() const
{
	return charSize.Ascender();
}


int	FTFont::Descender() const
{
	return charSize.Descender();
}


float FTFont::Advance( const wchar_t* string)
{
	const wchar_t* c = string; // wchar_t IS unsigned?
	float width = 0;

	while( *c)
	{
		width += glyphList->Advance( *c, *(c + 1));	
		++c;
	}

	return width;
}


float FTFont::Advance( const char* string)
{
	const unsigned char* c = (unsigned char*)string; // This is ugly, what is the c++ way?
	float width = 0;

	while( *c)
	{
		width += glyphList->Advance( *c, *(c + 1));	
		++c;
	}

	return width;
}


void FTFont::render( const char* string )
{
	const unsigned char* c = (unsigned char*)string; // This is ugly, what is the c++ way?
	FT_Vector kernAdvance;
	pen.x = 0; pen.y = 0;

	while( *c)
	{
		kernAdvance = glyphList->render( *c, *(c + 1), pen);
		
		pen.x += kernAdvance.x;
		pen.y += kernAdvance.y;
		
		++c;
	}
}


void FTFont::render( const wchar_t* string )
{
	const wchar_t* c = string; // wchar_t IS unsigned?
	FT_Vector kernAdvance;
	pen.x = 0; pen.y = 0;

	while( *c)
	{
		kernAdvance = glyphList->render( *c, *(c + 1), pen);
		
		pen.x += kernAdvance.x;
		pen.y += kernAdvance.y;
		
		++c;
	}
}
