// UVListRecord.cpp

#include "flt.h"
#include "Registry.h"
#include "UVListRecord.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                       UVListRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<UVListRecord> g_UVListProxy;

UVListRecord::UVListRecord()
{
    CERR << "UVListRecord created\n";
}


// virtual
UVListRecord::~UVListRecord()
{
}


// virtual
void UVListRecord::endian()
{

    SUVList *pSUVList = (SUVList*)getData();

    ENDIAN( pSUVList->layers );
}
