// LightSourcePaletteRecord.cpp

#include "flt.h"
#include "Registry.h"
#include "LightSourcePaletteRecord.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                       LightSourcePaletteRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<LightSourcePaletteRecord> g_LightSourcePaletteProxy;

LightSourcePaletteRecord::LightSourcePaletteRecord()
{
}


// virtual
LightSourcePaletteRecord::~LightSourcePaletteRecord()
{
}


// virtual
void LightSourcePaletteRecord::endian()
{
    SLightSourcePalette *pSLightSourcePalette = (SLightSourcePalette*)getData();

    ENDIAN( pSLightSourcePalette->diReserved_1 );
    ENDIAN( pSLightSourcePalette->diIndex );
    ENDIAN( pSLightSourcePalette->diReserved_2 );
    ENDIAN( pSLightSourcePalette->sfAmbientRGBA[0] );
    ENDIAN( pSLightSourcePalette->sfAmbientRGBA[1] );
    ENDIAN( pSLightSourcePalette->sfAmbientRGBA[2] );
    ENDIAN( pSLightSourcePalette->sfAmbientRGBA[3] );
    ENDIAN( pSLightSourcePalette->sfDiffuseRGBA[0] );
    ENDIAN( pSLightSourcePalette->sfDiffuseRGBA[1] );
    ENDIAN( pSLightSourcePalette->sfDiffuseRGBA[2] );
    ENDIAN( pSLightSourcePalette->sfDiffuseRGBA[3] );
    ENDIAN( pSLightSourcePalette->sfSpecularRGBA[0] );
    ENDIAN( pSLightSourcePalette->sfSpecularRGBA[1] );
    ENDIAN( pSLightSourcePalette->sfSpecularRGBA[2] );
    ENDIAN( pSLightSourcePalette->sfSpecularRGBA[3] );
    ENDIAN( pSLightSourcePalette->sfDropoff );
    ENDIAN( pSLightSourcePalette->sfCutoff );
    ENDIAN( pSLightSourcePalette->sfYaw );
    ENDIAN( pSLightSourcePalette->sfPitch );
    ENDIAN( pSLightSourcePalette->sfConstantAttuenation );
    ENDIAN( pSLightSourcePalette->sfLinearAttuenation );
    ENDIAN( pSLightSourcePalette->sfQuadraticAttuenation );
}
