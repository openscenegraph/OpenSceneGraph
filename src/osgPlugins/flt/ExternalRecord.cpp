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
    _pExternal = NULL;
}


// virtual
ExternalRecord::~ExternalRecord()
{
    if (_pExternal)
        _pExternal->unref();
}


void ExternalRecord::setExternal(FltFile* pExternal)
{
    if (_pExternal)
        _pExternal->unref();

    _pExternal = pExternal;
    _pExternal->ref();
}


void ExternalRecord::endian()
{
	SExternalReference *pSExternal = (SExternalReference*)getData();

    ENDIAN( pSExternal->diFlags );
}



