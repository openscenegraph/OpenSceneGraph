#include	"FTGlyphContainer.h"
#include	"FTGlyph.h"
#include	"FTFace.h"


FTGlyphContainer::FTGlyphContainer( FTFace* f, int g, bool p)
:	preCache( p),
	numGlyphs( g),
	face( f),
	err( 0)
{
	glyphs.reserve( g);
}



FTGlyphContainer::~FTGlyphContainer()
{
	vector<FTGlyph*>::iterator iter;
	for( iter = glyphs.begin(); iter != glyphs.end(); ++iter)
	{
		delete *iter;
	}
	
	glyphs.clear();
}


bool FTGlyphContainer::Add( FTGlyph* tempGlyph)
{
	// At the moment we are using a vector. Vectors don't return bool.
	glyphs.push_back( tempGlyph);
	return true;
}


float FTGlyphContainer::Advance( unsigned int index, unsigned int next)
{
	unsigned int left = face->CharIndex( index);
	unsigned int right = face->CharIndex( next);
	
	float width = face->KernAdvance( left, right).x;
	width += glyphs[left]->Advance();
	
	return width;
}


FT_Vector& FTGlyphContainer::render( unsigned int index, unsigned int next, FT_Vector pen)
{
	kernAdvance.x = 0; kernAdvance.y = 0;
	
	unsigned int left = face->CharIndex( index);
	unsigned int right = face->CharIndex( next);
	
	kernAdvance = face->KernAdvance( left, right);
		
	if( !face->Error())
	{
		advance = glyphs[left]->Render( pen);
	}
	
	kernAdvance.x = advance + kernAdvance.x; // FIXME float to long
//	kernAdvance.y = advance.y + kernAdvance.y;
	return kernAdvance;
}
