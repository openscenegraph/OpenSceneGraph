// ExtensionRecord.cpp

#include "flt.h"
#include "Registry.h"
#include "ExtensionRecord.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                          ExtensionRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<ExtensionRecord> g_ExtensionProxy;

ExtensionRecord::ExtensionRecord()
{
}


// virtual
ExtensionRecord::~ExtensionRecord()
{
}


void ExtensionRecord::endian()
{
    SExtension  *pExtension = (SExtension*)getData();

    //  VALID_RECORD(SHeader, pRecHdr)
    ENDIAN( pExtension->code );
}
