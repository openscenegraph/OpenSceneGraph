// LodRecord.cpp

#include "flt.h"
#include "Registry.h"
#include "LodRecord.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                              LodRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<LodRecord> g_LodProxy;


LodRecord::LodRecord()
{
}


// virtual
LodRecord::~LodRecord()
{
}


void LodRecord::endian()
{
	SLevelOfDetail *pSLod = (SLevelOfDetail*)getData();

	ENDIAN( pSLod->dfSwitchInDist );
	ENDIAN( pSLod->dfSwitchOutDist );
	ENDIAN( pSLod->iSpecialId_1 );
	ENDIAN( pSLod->iSpecialId_2 );
	ENDIAN( pSLod->diFlags );
	pSLod->Center.endian();
	ENDIAN( pSLod->dfTransitionRange );
}



////////////////////////////////////////////////////////////////////
//
//                              OldLodRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<OldLodRecord> g_OldLodProxy;


OldLodRecord::OldLodRecord()
{
}


// virtual
OldLodRecord::~OldLodRecord()
{
}


void OldLodRecord::endian()
{
	SOldLOD *pSLod = (SOldLOD*)getData();

	ENDIAN( pSLod->dfSwitchInDist );
	ENDIAN( pSLod->dfSwitchOutDist );
	ENDIAN( pSLod->iSpecialId_1 );
	ENDIAN( pSLod->iSpecialId_2 );
	ENDIAN( pSLod->diFlags );
	pSLod->Center.endian();
	ENDIAN( pSLod->dfTransitionRange );
}



