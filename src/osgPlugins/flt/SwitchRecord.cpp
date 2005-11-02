// SwitchRecord.cpp

#include "flt.h"
#include "Registry.h"
#include "SwitchRecord.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                          SwitchRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<SwitchRecord> g_fltSwitchProxy;

SwitchRecord::SwitchRecord()
{
}


// virtual
SwitchRecord::~SwitchRecord()
{
}


void SwitchRecord::endian()
{
    SSwitch *pSSwitch = (SSwitch*)getData();

    ENDIAN( pSSwitch->nCurrentMask );
    ENDIAN( pSSwitch->nMasks );
    ENDIAN( pSSwitch->nWordsInMask );

    for (int m=0; m < pSSwitch->nMasks*pSSwitch->nWordsInMask; m++)
    {
        ENDIAN( pSSwitch->aMask[m] );
    }
}
