#include    "FTFace.h"
#include    "FTLibrary.h"
#include    "FTCharmap.h"
#include    "FTGL.h"


FTFace::FTFace():
    charMap(0),
    ftFace(0),
    numCharMaps(0),
    numGlyphs(0),
    err(0)
{
    kernAdvance.x = 0;
    kernAdvance.y = 0;
}


FTFace::~FTFace()
{
    osgDelete charMap;
    charMap = 0;
    Close();
}


bool FTFace::Open( const char* filename)
{
    ftFace = osgNew FT_Face;
    err = FT_New_Face( *FTLibrary::Instance().GetLibrary(), filename, 0, ftFace);

    if( err)
    {
        osgDelete ftFace;
        ftFace = 0;
        return false;
    }
    else
    {
        charMap = osgNew FTCharmap( *ftFace);
        return true;
    }
}


void FTFace::Close()
{
    if( ftFace)
    {
        FT_Done_Face( *ftFace);
        osgDelete ftFace;
        ftFace = 0;
    }
}


FTSize& FTFace::Size( const unsigned int size, const unsigned int res)
{
    if( !charSize.CharSize( ftFace, size, res, res))
    {
        err = charSize.Error();
    }
    
    return charSize;
}


bool FTFace::CharMap( FT_Encoding encoding)
{
    return charMap->CharMap( encoding);
}


unsigned int FTFace::CharIndex( unsigned int index) const
{
    return charMap->CharIndex( index);
}


FT_Vector& FTFace::KernAdvance( unsigned int index1, unsigned int index2)
{
    kernAdvance.x = 0; kernAdvance.y = 0;
    
    if( FT_HAS_KERNING((*ftFace)) && index1 && index2)
    {
        err = FT_Get_Kerning( *ftFace, index1, index2, ft_kerning_unfitted, &kernAdvance);
        if( !err)
        {    
            kernAdvance.x /= 64; kernAdvance.y /= 64;
        }
    }
    
    return kernAdvance;
}


FT_Glyph* FTFace::Glyph( unsigned int index, FT_Int load_flags)
{
    err = FT_Load_Glyph( *ftFace, index, load_flags);    
    err = FT_Get_Glyph( (*ftFace)->glyph, &ftGlyph);
        
    if( !err)
    {
        return &ftGlyph;
    }
    else
    {
        return NULL;
    }
}



