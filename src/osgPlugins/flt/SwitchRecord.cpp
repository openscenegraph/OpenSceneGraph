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

RegisterRecordProxy<SwitchRecord> g_SwitchProxy;

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

    ENDIAN( pSSwitch->dwCurrentMask );
    ENDIAN( pSSwitch->diWordsInMask );
    ENDIAN( pSSwitch->diMasks );
}
