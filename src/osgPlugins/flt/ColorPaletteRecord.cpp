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
    // note, sizeof returns unsigned int, while getSize() etc returns
    // int, this correctly generates a warning when comparisons are made
    // under Linux.  This really needs to be fixed so getSize() returns
    // an unsigned int.  I won't do it now as it may well break code 
    // which I don't fully understand.  I've made the hacky use of an
    // (int) cast to fix the warning, I don't think this will cause an
    // problems. RO August 2001.
    if (getSize() > (int) sizeof(SOldColorPalette))
    {
        SColorPalette* pSColor = (SColorPalette*)getData();
        int nOffset = sizeof(SColorPalette);

        if (nOffset < getSize())
        {
            int n = 0;
            ENDIAN( pSColor->nNames );

            while ((n++ < pSColor->nNames) && (nOffset < getSize()))
            {
                SColorName* pName = (SColorName*)((char*)getData())+nOffset;
                ENDIAN( pName->swSize );
                ENDIAN( pName->nIndex );
                nOffset += pName->swSize;
            };
        }
    }
    else    // version 11, 12 & 13
    {
        SOldColorPalette* pSColor = (SOldColorPalette*)getData();
        unsigned int i;
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
