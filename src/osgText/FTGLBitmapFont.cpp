#include    "FTGLBitmapFont.h"
#include    "FTGlyphContainer.h"
#include    "FTBitmapGlyph.h"


FTGLBitmapFont::FTGLBitmapFont()
{}


FTGLBitmapFont::~FTGLBitmapFont()
{}


// OPSignature: bool FTGlyphContainer:MakeGlyphList() 
// mrn@changes
bool FTGLBitmapFont::MakeGlyphList(unsigned int renderContext)
{
    FTGlyphContainer* glyphList=_contextGlyphList[renderContext];

    //    if( preCache)
    for( unsigned int c = 0; c < numGlyphs; ++c)
    {
        FT_Glyph* ftGlyph = face.Glyph( c, FT_LOAD_DEFAULT);
//        FT_HAS_VERTICAL(face)

        if( ftGlyph)
        {
            FTBitmapGlyph* tempGlyph = osgNew FTBitmapGlyph( *ftGlyph);
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
void FTGLBitmapFont::render( const char* string,unsigned int renderContext)
{    
    glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT);
    
    glPixelStorei( GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0);
#if (FREETYPE_MAJOR >= 2) && (FREETYPE_MINOR>=1)
    glPixelStorei( GL_UNPACK_ALIGNMENT, 2);
#else
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
#endif

    FTFont::render( string,renderContext);

    glPopClientAttrib();

}


// mrn@changes
void FTGLBitmapFont::render( const wchar_t* string,unsigned int renderContext)
{    
    glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT);
    
    glPixelStorei( GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1);

    FTFont::render( string,renderContext);

    glPopClientAttrib();

}
