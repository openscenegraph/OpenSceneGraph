// BSPRecord.cpp
//
// Author: Michael M. Morrison
//

#include "flt.h"
#include "Registry.h"
#include "BSPRecord.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                          BSPRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<BSPRecord> g_BSPProxy;

BSPRecord::BSPRecord()
{
}


// virtual
BSPRecord::~BSPRecord()
{
}


void BSPRecord::endian()
{
    SBSP *pSBSP = (SBSP*)getData();

    ENDIAN( pSBSP->planeA );
    ENDIAN( pSBSP->planeB );
    ENDIAN( pSBSP->planeC );
    ENDIAN( pSBSP->planeD );
}
