// MultiTextureRecord.cpp

#include "flt.h"
#include "Registry.h"
#include "MultiTextureRecord.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                       MultiTextureRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<MultiTextureRecord> g_MultiTextureProxy;

MultiTextureRecord::MultiTextureRecord()
{
    CERR << "MultiTextureRecord created\n";
}


// virtual
MultiTextureRecord::~MultiTextureRecord()
{
}


// virtual
void MultiTextureRecord::endian()
{
    SMultiTexture *pSMultiTexture = (SMultiTexture*)getData();

    ENDIAN( pSMultiTexture->layers );
}
