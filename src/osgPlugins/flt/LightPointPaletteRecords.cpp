
#include "flt.h"
#include "Registry.h"
#include "LightPointPaletteRecords.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                     LightPointPaletteRecords
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<LtPtAppearancePaletteRecord> g_LtPtAppearancePaletteProxy;

LtPtAppearancePaletteRecord::LtPtAppearancePaletteRecord()
{
}


// virtual
LtPtAppearancePaletteRecord::~LtPtAppearancePaletteRecord()
{
}


// virtual
void LtPtAppearancePaletteRecord::endian()
{
    SLightPointAppearancePalette* ltPtApp = (SLightPointAppearancePalette*)getData();

    ENDIAN( ltPtApp->index );
    ENDIAN( ltPtApp->surfMatCode );
    ENDIAN( ltPtApp->featureID );
    ENDIAN( ltPtApp->backColor );
    ENDIAN( ltPtApp->displayMode );
    ENDIAN( ltPtApp->intensity );
    ENDIAN( ltPtApp->backIntensity );
    ENDIAN( ltPtApp->minDefocus );
    ENDIAN( ltPtApp->maxDefocus );
    ENDIAN( ltPtApp->fadeMode );
    ENDIAN( ltPtApp->fogPunch );
    ENDIAN( ltPtApp->dirMode );
    ENDIAN( ltPtApp->rangeMode );
    ENDIAN( ltPtApp->minPixelSize );
    ENDIAN( ltPtApp->maxPixelSize );
    ENDIAN( ltPtApp->actualSize );
    ENDIAN( ltPtApp->transFalloffPixelSize );
    ENDIAN( ltPtApp->transFalloffExp );
    ENDIAN( ltPtApp->transFalloffScalar );
    ENDIAN( ltPtApp->transFalloffClamp );
    ENDIAN( ltPtApp->fogScalar );
    ENDIAN( ltPtApp->fogIntensity );
    ENDIAN( ltPtApp->sizeDiffThreshold );
    ENDIAN( ltPtApp->directionality );
    ENDIAN( ltPtApp->horizLobeAngle );
    ENDIAN( ltPtApp->vertLobeAngle );
    ENDIAN( ltPtApp->lobeRollAngle );
    ENDIAN( ltPtApp->dirFalloffExp );
    ENDIAN( ltPtApp->dirAmbientIntensity );
    ENDIAN( ltPtApp->significance );
    ENDIAN( ltPtApp->flags );
    ENDIAN( ltPtApp->visRange );
    ENDIAN( ltPtApp->fadeRangeRatio );
    ENDIAN( ltPtApp->fadeInDurationSecs );
    ENDIAN( ltPtApp->adeOutDurationSecs );
    ENDIAN( ltPtApp->lodRangeRatio );
    ENDIAN( ltPtApp->lodScale );
}

