#ifndef        __FTGLPixmapFont__
#define        __FTGLPixmapFont__


#include "FTGL.h"

#include "FTFont.h"

class FTPixmapGlyph;

/**
 * FTGLPixmapFont is a specialisation of the FTFont class for handling
 * Pixmap (Grey Scale) fonts
 *
 * @see        FTFont
 */
class FTGL_EXPORT FTGLPixmapFont : public FTFont
{
    public:
        /**
         * Default Constructor
         */
        FTGLPixmapFont();
        
        /**
         * Destructor
         */
        ~FTGLPixmapFont();
        
        /**
         * Renders a string of characters
         * 
         * @param string    'C' style string to be output.     
         */
        // mrn@changes
        virtual void render( const char* string, unsigned int renderContext=0);
        
        /**
         * Renders a string of characters
         * 
         * @param string    wchar_t string to be output.     
         */
        // mrn@changes
        virtual void render( const wchar_t* string, unsigned int renderContext=0);


    private:
        /**
         * Constructs the internal glyph cache.
         *
         * This a list of glyphs processed for openGL rendering NOT
         * freetype glyphs
         */
        // mrn@changes
        virtual bool MakeGlyphList(unsigned int renderContext=0);
        
};


#endif    //    __FTGLPixmapFont__

