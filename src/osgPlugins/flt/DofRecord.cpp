// DofRecord.cpp

#include "flt.h"
#include "Registry.h"
#include "DofRecord.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                              DofRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<DofRecord> g_DofProxy;

DofRecord::DofRecord()
{
}


// virtual
DofRecord::~DofRecord()
{
}


void DofRecord::endian()
{
    SDegreeOfFreedom *pSDof = (SDegreeOfFreedom*)getData();

    ENDIAN( pSDof->diReserved );
    pSDof->OriginLocalDOF.endian();
    pSDof->PointOnXaxis.endian();
    pSDof->PointInXYplane.endian();
    pSDof->dfZ.endian();
    pSDof->dfY.endian();
    pSDof->dfX.endian();
    pSDof->dfPitch.endian();
    pSDof->dfRoll.endian();
    pSDof->dfYaw.endian();
    pSDof->dfZscale.endian();
    pSDof->dfYscale.endian();
    pSDof->dfXscale.endian();
    ENDIAN( pSDof->dwFlags );
}
