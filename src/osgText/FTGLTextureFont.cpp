#include	"FTGLTextureFont.h"
#include	"FTGlyphContainer.h"
#include	"FTTextureGlyph.h"

using namespace std;

typedef unsigned long	UInt32; // a mac thing?

inline UInt32 NextPowerOf2( UInt32 in)
{
     in -= 1;

     in |= in >> 16;
     in |= in >> 8;
     in |= in >> 4;
     in |= in >> 2;
     in |= in >> 1;

     return in + 1;
}


FTGLTextureFont::FTGLTextureFont()
:	numTextures(1),
	textMem(0),
	padding(1),
	tempGlyph(0),
	maxTextSize(0),
	textureWidth(0),
	textureHeight(0),
	glyphHeight(0),
	glyphWidth(0)
{}


FTGLTextureFont::~FTGLTextureFont()
{
	glDeleteTextures( numTextures, (const GLuint*)glTextureID);
}


bool FTGLTextureFont::MakeGlyphList()
{
	glEnable( GL_TEXTURE_2D);
	
	if( !maxTextSize)
		glGetIntegerv( GL_MAX_TEXTURE_SIZE, (GLint*)&maxTextSize);
		
	glyphHeight = ( charSize.Height()) + padding;
	glyphWidth = ( charSize.Width()) + padding;
	
	GetSize();
	int totalMem;
	
	if( textureHeight > maxTextSize)
	{
		numTextures = static_cast<int>( textureHeight / maxTextSize) + 1;
		if( numTextures > 15) // FIXME
			numTextures = 15;
		
		int heightRemain = NextPowerOf2( textureHeight % maxTextSize);
		totalMem = ((maxTextSize * ( numTextures - 1)) + heightRemain) * textureWidth;

		glGenTextures( numTextures, (GLuint*)&glTextureID[0]);

		textMem = new unsigned char[totalMem]; // GL_ALPHA texture;
		memset( textMem, 0, totalMem);
			
		unsigned int glyphNum = 0;
		unsigned char* currTextPtr = textMem;
		
		for( int x = 0; x < numTextures - 1; ++x)
		{
			glyphNum = FillGlyphs( glyphNum, glTextureID[x], textureWidth, maxTextSize, currTextPtr);
			
			CreateTexture( x, textureWidth, maxTextSize, currTextPtr);
			
			currTextPtr += ( textureWidth * maxTextSize);
			++glyphNum;
		}
		
		glyphNum = FillGlyphs( glyphNum, glTextureID[numTextures - 1], textureWidth, heightRemain, currTextPtr);
		CreateTexture( numTextures - 1, textureWidth, heightRemain, currTextPtr);
	}
	else
	{
		textureHeight = NextPowerOf2( textureHeight);
		totalMem = textureWidth * textureHeight;
		
		glGenTextures( numTextures, (GLuint*)&glTextureID[0]);

		textMem = new unsigned char[totalMem]; // GL_ALPHA texture;
		memset( textMem, 0, totalMem);

		FillGlyphs( 0, glTextureID[0], textureWidth, textureHeight, textMem);
		CreateTexture( 0, textureWidth, textureHeight, textMem);
	}

	delete [] textMem;
	return !err;
}


unsigned int FTGLTextureFont::FillGlyphs( unsigned int glyphStart, int id, int width, int height, unsigned char* textdata)
{
	int currentTextX = padding;
	int currentTextY = padding;// + padding;
	
	float currTextU = (float)padding / (float)width;
	float currTextV = (float)padding / (float)height;
	
//	numGlyphs = 256; // FIXME hack
	unsigned int n;
	
	for( n = glyphStart; n <= numGlyphs; ++n)
	{
		FT_Glyph* ftGlyph = face.Glyph( n, FT_LOAD_NO_HINTING);
		
		if( ftGlyph)
		{
			unsigned char* data = textdata + (( currentTextY * width) + currentTextX);
			
			currTextU = (float)currentTextX / (float)width;
			
			tempGlyph = new FTTextureGlyph( *ftGlyph, id, data, width, height, currTextU, currTextV);
			glyphList->Add( tempGlyph);

			currentTextX += glyphWidth;
			if( currentTextX > ( width - glyphWidth))
			{
				currentTextY += glyphHeight;
				if( currentTextY > ( height - glyphHeight))
					return n;
					
				currentTextX = padding;
				currTextV = (float)currentTextY / (float)height;
			}
		}
		else
		{
			err = face.Error();
		}
	}

	return n;
}


void FTGLTextureFont::GetSize()
{
	//work out the max width. Most likely maxTextSize
	textureWidth = NextPowerOf2( numGlyphs * glyphWidth);
	if( textureWidth > maxTextSize)
	{
		textureWidth = maxTextSize;
	}
	
	int h = static_cast<int>( textureWidth / glyphWidth);
	textureHeight = (( numGlyphs / h) + 1) * glyphHeight;
}


void FTGLTextureFont::CreateTexture( int id, int width, int height, unsigned char* data)
{
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1); //What does this do exactly?
	glBindTexture( GL_TEXTURE_2D, glTextureID[id]);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
}


void FTGLTextureFont::render( const char* string)
{	
	glPushAttrib( GL_ENABLE_BIT | GL_HINT_BIT | GL_LINE_BIT | GL_PIXEL_MODE_BIT);
	
	glEnable(GL_BLEND);
 	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // GL_ONE
 	
	glBindTexture( GL_TEXTURE_2D, (GLuint)FTTextureGlyph::activeTextureID);

 	// QUADS are faster!? Less function call overhead?
 	glBegin( GL_QUADS);
 		FTFont::render( string);
 	glEnd();
	
	glPopAttrib();
}


void FTGLTextureFont::render( const wchar_t* string)
{	
	glPushAttrib( GL_ENABLE_BIT | GL_HINT_BIT | GL_LINE_BIT | GL_PIXEL_MODE_BIT);
	
	glEnable(GL_BLEND);
 	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // GL_ONE
 	
	glBindTexture( GL_TEXTURE_2D, (GLuint)FTTextureGlyph::activeTextureID);

 	// QUADS are faster!? Less function call overhead?
 	glBegin( GL_QUADS);
 		FTFont::render( string);
 	glEnd();
	
	glPopAttrib();
}

