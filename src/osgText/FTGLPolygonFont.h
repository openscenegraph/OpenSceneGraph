#ifndef        __FTGLPolygonFont__
#define        __FTGLPolygonFont__

#include "FTGL.h"

#include    "FTFont.h"


class FTPolyGlyph;

/**
 * FTGLPolygonFont is a specialisation of the FTFont class for handling
 * tesselated Polygon Mesh fonts
 *
 * @see        FTFont
 */
class FTGL_EXPORT FTGLPolygonFont : public FTFont
{
    public:
        /**
         * Default Constructor
         */
        FTGLPolygonFont();
        
        /**
         * Destructor
         */
        ~FTGLPolygonFont();
        
    private:
        /**
         * Constructs the internal glyph cache.
         *
         * This a list of glyphs processed for openGL rendering NOT
         * freetype glyphs
         */
        // mrn@changes
        virtual bool MakeGlyphList( unsigned int renderContext=0);
        
};


#endif    //    __FTGLPolygonFont__

