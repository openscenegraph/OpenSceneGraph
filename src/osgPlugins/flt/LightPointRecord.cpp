// LightPointRecord.cpp

#include "flt.h"
#include "Registry.h"
#include "LightPointRecord.h"
#include <osg/Notify>

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                          LightPointRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<LightPointRecord> g_LightPointProxy;

LightPointRecord::LightPointRecord()
{
}


// virtual
LightPointRecord::~LightPointRecord()
{
}

SLightPoint* LightPointRecord::getData() const
{
    if (LightPointRecord::getFlightVersion()>1551)
    {
        return (SLightPoint*)_pData;
    }
    else
    {
        // disable ligth point for older versions since they don't have
        // the same format as SLightPoint, this needs fixing but this will have
        // to wait till after OSG 1.0.  
        // Robert Osfield, 18th November 2005.
        return 0;
    }
}


void LightPointRecord::endian()
{
    SLightPoint *pSLightPoint = (SLightPoint*)getData();
    if (pSLightPoint)
    {
        ENDIAN( pSLightPoint->iMaterial );
        ENDIAN( pSLightPoint->iFeature );
        ENDIAN( pSLightPoint->diMode );
        ENDIAN( pSLightPoint->sfIntensityFront );
        ENDIAN( pSLightPoint->sfIntensityBack );
        ENDIAN( pSLightPoint->sfMinDefocus );
        ENDIAN( pSLightPoint->sfMaxDefocus );
        ENDIAN( pSLightPoint->diFadeMode );
        ENDIAN( pSLightPoint->diFogPunchMode );
        ENDIAN( pSLightPoint->diDirectionalMode );
        ENDIAN( pSLightPoint->diRangeMode );
        ENDIAN( pSLightPoint->sfMinPixelSize );
        ENDIAN( pSLightPoint->sfMaxPixelSize );
        ENDIAN( pSLightPoint->afActualPixelSize );
        ENDIAN( pSLightPoint->sfTranspFalloff );
        ENDIAN( pSLightPoint->sfTranspFalloffExponent );
        ENDIAN( pSLightPoint->sfTranspFalloffScalar );
        ENDIAN( pSLightPoint->sfTranspFalloffClamp );
        ENDIAN( pSLightPoint->sfFog );
        ENDIAN( pSLightPoint->sfReserved );
        ENDIAN( pSLightPoint->sfSize );
        ENDIAN( pSLightPoint->diDirection );
        ENDIAN( pSLightPoint->sfLobeHoriz );
        ENDIAN( pSLightPoint->sfLobeVert );
        ENDIAN( pSLightPoint->sfLobeRoll );
        ENDIAN( pSLightPoint->sfFalloff );
        ENDIAN( pSLightPoint->sfAmbientIntensity );
        ENDIAN( pSLightPoint->sfAnimPeriod );
        ENDIAN( pSLightPoint->sfAnimPhaseDelay );
        ENDIAN( pSLightPoint->sfAnimPeriodEnable );
        ENDIAN( pSLightPoint->sfSignificance );
        ENDIAN( pSLightPoint->sfDrawOrder );
        ENDIAN( pSLightPoint->sfFlags );
        ENDIAN( pSLightPoint->dwBackColor );
        pSLightPoint->animRot.endian();
    }

}



////////////////////////////////////////////////////////////////////
//
//                      Indexed LightPointRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<LightPointIndexRecord> g_LightPointIndexdProxy;

LightPointIndexRecord::LightPointIndexRecord()
{
}


// virtual
LightPointIndexRecord::~LightPointIndexRecord()
{
}


void LightPointIndexRecord::endian()
{
    SLightPointIndex *pSLightPointIndex = (SLightPointIndex*)getData();

    ENDIAN( pSLightPointIndex->iAppearanceIndex );
    ENDIAN( pSLightPointIndex->iAnimationIndex );
    ENDIAN( pSLightPointIndex->iDrawOrder );
}
