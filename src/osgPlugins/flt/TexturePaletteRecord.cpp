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

    int flightVersion = getFlightVersion();

    if (flightVersion > 13)
    {
        STexturePalette *pSTexture = (STexturePalette*)getData();

        ENDIAN( pSTexture->diIndex );
        ENDIAN( pSTexture->diX );
        ENDIAN( pSTexture->diY );
    }
    else    // version 11, 12 & 13
    {
        SOldTexturePalette *pSOldTexture = (SOldTexturePalette*)getData();

        ENDIAN( pSOldTexture->diIndex );
        ENDIAN( pSOldTexture->diX );
        ENDIAN( pSOldTexture->diY );
    }
}
