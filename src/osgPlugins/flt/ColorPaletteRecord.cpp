// ColorPaletteRecord.cpp

#include "flt.h"
#include "Input.h"
#include "Registry.h"
#include "ColorPaletteRecord.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                       MaterialPaletteRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<ColorPaletteRecord> g_ColorPaletteRecordProxy;

ColorPaletteRecord::ColorPaletteRecord()
{
}


// virtual
ColorPaletteRecord::~ColorPaletteRecord()
{
}


// virtual
void ColorPaletteRecord::endian()
{
    int flightVersion = getFlightVersion();

    if (flightVersion > 13)
    {
// TODO: May cause crash on win32
// It's only color names so we ignore
#if 0
        SColorPalette* pSColor = (SColorPalette*)getData();
        size_t nOffset = sizeof(SColorPalette);

        if ((nOffset < getSize())
        &&  (flightVersion >= 1500))
        {
            int n = 0;
            ENDIAN( pSColor->nNames );

            while ((n++ < pSColor->nNames) && (nOffset < getSize()))
            {
                SColorName* pName = (SColorName*)((char*)pSColor)+nOffset;
                ENDIAN( pName->swSize );
                ENDIAN( pName->nIndex );
                nOffset += pName->swSize;
            };
        }
#endif
    }
    else    // version 11, 12 & 13
    {
        SOldColorPalette* pSColor = (SOldColorPalette*)getData();
        size_t i;
        for (i=0; i < sizeof(pSColor->Colors)/sizeof(pSColor->Colors[0]); i++)
        {
            ENDIAN( pSColor->Colors[i]._red );
            ENDIAN( pSColor->Colors[i]._green );
            ENDIAN( pSColor->Colors[i]._blue );
        }

        for (i=0; i < sizeof(pSColor->FixedColors)/sizeof(pSColor->FixedColors[0]); i++)
        {
            ENDIAN( pSColor->FixedColors[i]._red );
            ENDIAN( pSColor->FixedColors[i]._green );
            ENDIAN( pSColor->FixedColors[i]._blue );
        }
    }
}
