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




