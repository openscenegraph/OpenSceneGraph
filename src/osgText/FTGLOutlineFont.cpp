#include    "FTGLOutlineFont.h"
#include    "FTGlyphContainer.h"
#include    "FTGL.h"
#include    "FTOutlineGlyph.h"


FTGLOutlineFont::FTGLOutlineFont()
{}


FTGLOutlineFont::~FTGLOutlineFont()
{}


// mrn@changes
bool FTGLOutlineFont::MakeGlyphList( unsigned int renderContext)
{
    FTGlyphContainer* glyphList=_contextGlyphList[renderContext];

    for( unsigned int n = 0; n < numGlyphs; ++n)
    {
        FT_Glyph* ftGlyph = face.Glyph( n, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
        
        if( ftGlyph)
        {
            FTOutlineGlyph* tempGlyph = new FTOutlineGlyph( *ftGlyph);
            glyphList->Add( tempGlyph);
        }
        else
        {
            err = face.Error();
        }
    }
    
    return !err;
}


// mrn@changes
void FTGLOutlineFont::render( const char* string, unsigned int renderContext)
{    
    glPushAttrib( GL_ENABLE_BIT | GL_HINT_BIT | GL_LINE_BIT | GL_PIXEL_MODE_BIT);
    
    glEnable( GL_LINE_SMOOTH);
    glHint( GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glEnable(GL_BLEND);
     glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // GL_ONE

    FTFont::render( string,renderContext);

    glPopAttrib();

}


// mrn@changes
void FTGLOutlineFont::render( const wchar_t* string, unsigned int renderContext)
{    
    glPushAttrib( GL_ENABLE_BIT | GL_HINT_BIT | GL_LINE_BIT | GL_PIXEL_MODE_BIT);
    
    glEnable( GL_LINE_SMOOTH);
    glHint( GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glEnable(GL_BLEND);
     glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // GL_ONE

    FTFont::render( string,renderContext);

    glPopAttrib();

}
