#ifndef		__FTGLTextureFont
#define		__FTGLTextureFont
#include	"FTFont.h"

#include "FTGL.h"

class FTTextureGlyph;

/**
 * FTGLTextureFont is a specialisation of the FTFont class for handling
 * Texture mapped fonts
 *
 * @see		FTFont
 */
class FTGLTextureFont : public FTFont
{
	public:
		// methods
		FTGLTextureFont();
		virtual ~FTGLTextureFont();
		
		virtual int TextureWidth() const { return textureWidth;}
		virtual int TextureHeight() const { return textureHeight;}
		
		virtual void render( const char* string);
		virtual void render( const wchar_t* string);

		
	private:
		// attributes
		FTTextureGlyph* tempGlyph;
		
		long maxTextSize;
		int textureWidth;
		int textureHeight;
		
		unsigned long glTextureID[16];
		int numTextures;
		unsigned char* textMem;
		
		int glyphHeight;
		int glyphWidth;

		int padding;
		
		// methods
		bool MakeGlyphList();
		void CreateTexture( int id, int width, int height, unsigned char* data);
		void GetSize();
		unsigned int FillGlyphs( unsigned int glyphStart, int textID, int textureWidth, int textureHeight, unsigned char* textMem);
		
		
};
#endif
