#include    "FTGLPixmapFont.h"
#include    "FTGlyphContainer.h"
#include    "FTPixmapGlyph.h"


FTGLPixmapFont::FTGLPixmapFont()
{}


FTGLPixmapFont::~FTGLPixmapFont()
{}


// OPSignature: bool FTGlyphContainer:MakeGlyphList() 
// mrn@changes
bool FTGLPixmapFont::MakeGlyphList(unsigned int renderContext)
{
    FTGlyphContainer* glyphList=_contextGlyphList[renderContext];

//    if( preCache)
    for( unsigned int c = 0; c < numGlyphs; ++c)
    {
        FT_Glyph* ftGlyph = face.Glyph( c, FT_LOAD_DEFAULT);
//        FT_HAS_VERTICAL(face)
    
        if( ftGlyph)
        {
            FTPixmapGlyph* tempGlyph = new FTPixmapGlyph( *ftGlyph);
            glyphList->Add( tempGlyph);
        }
        else
        {
            err = face.Error();
        }
    }
    
    return !err;
}


void FTGLPixmapFont::render( const char* string,unsigned int renderContext)
{    
    glPushAttrib( GL_ENABLE_BIT | GL_PIXEL_MODE_BIT);

    // Why is this modifying state here? - DB
    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    FTFont::render( string,renderContext);

    glPopAttrib();
}


void FTGLPixmapFont::render( const wchar_t* string,unsigned int renderContext)
{    
    glPushAttrib( GL_ENABLE_BIT | GL_PIXEL_MODE_BIT);

    // Why is this modifying state here? - DB
    glEnable(GL_BLEND);
     glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    FTFont::render( string,renderContext);

    glPopAttrib();

}

