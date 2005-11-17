
#include "flt.h"
#include "Registry.h"
#include "LightPointPaletteRecords.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                     LightPointAppearancePaletteRecords
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




////////////////////////////////////////////////////////////////////
//
//                     LightPointAnimationPaletteRecords
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<LtPtAnimationPaletteRecord> g_LtPtAnimationPaletteProxy;

LtPtAnimationPaletteRecord::LtPtAnimationPaletteRecord()
{
}


// virtual
LtPtAnimationPaletteRecord::~LtPtAnimationPaletteRecord()
{
}


// virtual
void LtPtAnimationPaletteRecord::endian()
{
    SLightPointAnimationPalette* ltPtAnim = (SLightPointAnimationPalette*)getData();

    ENDIAN( ltPtAnim->index );
    ENDIAN( ltPtAnim->period );
    ENDIAN( ltPtAnim->phaseDelay );
    ENDIAN( ltPtAnim->enabledPeriod );
    ENDIAN( ltPtAnim->axis[0] );
    ENDIAN( ltPtAnim->axis[1] );
    ENDIAN( ltPtAnim->axis[2] );
    ENDIAN( ltPtAnim->flags );
    ENDIAN( ltPtAnim->animType );
    ENDIAN( ltPtAnim->morseTiming );
    ENDIAN( ltPtAnim->wordRate );
    ENDIAN( ltPtAnim->charRate );
    ENDIAN( ltPtAnim->numSequences );

    for (int idx=0; idx < ltPtAnim->numSequences; idx++)
    {
        SLightPointAnimationSequence* seq = sequence( idx );
        assert( seq );
        ENDIAN( seq->seqState );
        ENDIAN( seq->duration );
        ENDIAN( seq->seqColor );
    }
}

SLightPointAnimationSequence*
LtPtAnimationPaletteRecord::sequence( int idx )
{
    SLightPointAnimationPalette* ltPtAnim = (SLightPointAnimationPalette*)getData();
    if (idx >= ltPtAnim->numSequences)
        return NULL;

    SLightPointAnimationSequence* seq = (SLightPointAnimationSequence*)
        ( (char*)getData() + sizeof( SLightPointAnimationPalette ) );
    seq += idx;

    return seq;
}

