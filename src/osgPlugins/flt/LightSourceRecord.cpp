// LightSourceRecord.cpp

#include "flt.h"
#include "Registry.h"
#include "LightSourceRecord.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                          LightSourceRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<LightSourceRecord> g_LightSourceRecordProxy;

LightSourceRecord::LightSourceRecord()
{
}


// virtual
LightSourceRecord::~LightSourceRecord()
{
}


// virtual
void LightSourceRecord::endian()
{
    SLightSource *pSLightSource = (SLightSource*)getData();

    ENDIAN( pSLightSource->diReserved_1 );
    ENDIAN( pSLightSource->diIndex );
    ENDIAN( pSLightSource->diReserved_2 );
    ENDIAN( pSLightSource->dwFlags );
    ENDIAN( pSLightSource->diReserved_3 );
    pSLightSource->Coord.endian();
    ENDIAN( pSLightSource->sfYaw );
    ENDIAN( pSLightSource->sfPitch );
}
