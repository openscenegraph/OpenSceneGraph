// ExternalRecord.cpp

#include "flt.h"
#include "Registry.h"
#include "FltFile.h"
#include "ExternalRecord.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                          ExternalRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<ExternalRecord> g_ExternalProxy;

ExternalRecord::ExternalRecord()
{
}


// virtual
ExternalRecord::~ExternalRecord()
{
}


void ExternalRecord::setExternal(FltFile* pExternal)
{
    _fltfile = pExternal;
}


void ExternalRecord::endian()
{
    SExternalReference *pSExternal = (SExternalReference*)getData();

    if (getSize() >= sizeof(SExternalReference))
    {
        ENDIAN( pSExternal->diFlags );
    }
    else
    {
        pSExternal->diFlags = 0;
    }
}
