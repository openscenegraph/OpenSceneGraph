#ifndef		__FTFont__
#define		__FTFont__

#include <string>

#include "FTGL.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "FTFace.h"
#include "FTGL.h"


class FTGlyphContainer;

using namespace std;



/**
 * FTFont is the public interface for the FTGL library.
 *
 * Specific font classes are derived from this class. It uses the helper
 * classes FTFace and FTSize to access the Freetype library. This class
 * is abstract and deriving classes must implement the protected
 * <code>MakeGlyphList</code> function to build a glyphList with the
 * appropriate glyph type.
 *
 * @see		FTFace
 * @see		FTSize
 * @see		FTGlyphContainer
 * @see		FTGlyph
 */
class FTGL_EXPORT FTFont
{
	public:
		/**
		 * Default Constructor
		 */
		FTFont();
		
		/**
		 * Destructor
		 */
		virtual ~FTFont();
		
		/**
		 * Opens and reads a font file.
		 *
		 * @param fontname	font file name.
		 * @return			<code>true</code> if file has opened
		 *					successfully.
		 */
		virtual bool Open( const char* fontname );
		
		/**
		 * Disposes of the font
		 */
		virtual void Close();
		
		/**
		 * Sets the char size for the current face.
		 *
		 * @param size		the face size in points (1/72 inch)
		 * @param res		the resolution of the target device.
		 * @return			<code>true</code> if size was set correctly
		 */
		virtual bool FaceSize( const unsigned int size, const unsigned int res = 72 );
		
		/**
		 * Sets the character map for the face.
		 *
		 * @param encoding		Freetype enumerate for char map code.
		 * @return				<code>true</code> if charmap was valid and
		 *						set correctly
		 */
		virtual bool CharMap( FT_Encoding encoding );
		
		/**
		 * Gets the global ascender height for the face.
		 *
		 * @return	Ascender height
		 */
		virtual int	Ascender() const;
		
		/**
		 * Gets the global descender height for the face.
		 *
		 * @return	Descender height
		 */
		virtual int	Descender() const;
		
		/**
		 * Gets the advance width for a string.
		 *
		 * param string	a wchar_t string
		 * @return		advance width
		 */
		float Advance( const wchar_t* string);

		/**
		 * Gets the advance width for a string.
		 *
		 * param string	a char string
		 * @return		advance width
		 */
		float Advance( const char* string);

		/**
		 * Renders a string of characters
		 * 
		 * @param string	'C' style string to be output.	 
		 */
		virtual void render( const char* string );

		/**
		 * Renders a string of characters
		 * 
		 * @param string	wchar_t string to be output.	 
		 */
		virtual void render( const wchar_t* string );

		/**
		 * Queries the Font for errors.
		 *
		 * @return	The current error code.
		 */
		virtual FT_Error Error() const { return err;}


	protected:
		/**
		 * Constructs the internal glyph cache.
		 *
		 * This a list of glyphs processed for openGL rendering NOT
		 * freetype glyphs
		 */
		virtual bool MakeGlyphList() = 0;
		
		/**
		 * Current face object
		 */
		FTFace face;
		
		/**
		 * Number of faces in this font
		 */
		unsigned int numFaces;
		
		/**
		 * Current size object
		 */
		FTSize charSize;

		/**
		 * An object that holds a list of glyphs
		 */
		FTGlyphContainer*	glyphList;
		
		/**
		 * The number of glyphs in this font
		 */
		unsigned int numGlyphs;
		
		/**
		 * Current pen or cursor position;
		 */
		FT_Vector pen;
		
		/**
		 * Current error code. Zero means no error.
		 */
		FT_Error err;
		
	private:

};


#endif	//	__FTFont__

