// TexturePaletteRecord.cpp

#include "flt.h"
#include "Registry.h"
#include "TexturePaletteRecord.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                       TexturePaletteRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<TexturePaletteRecord> g_TexturePaletteProxy;

TexturePaletteRecord::TexturePaletteRecord()
{
}


// virtual
TexturePaletteRecord::~TexturePaletteRecord()
{
}


// virtual
void TexturePaletteRecord::endian()
{
    STexturePalette *pSTexture = (STexturePalette*)getData();

    ENDIAN( pSTexture->diIndex );
    ENDIAN( pSTexture->diX );
    ENDIAN( pSTexture->diY );
}
