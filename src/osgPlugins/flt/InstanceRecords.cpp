// InstanceRecords.cpp

#include "flt.h"
#include "Registry.h"
#include "InstanceRecords.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                          InstanceDefinitionRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<InstanceDefinitionRecord> g_InstanceDefinitionProxy;

InstanceDefinitionRecord::InstanceDefinitionRecord()
{
}


// virtual
InstanceDefinitionRecord::~InstanceDefinitionRecord()
{
}


void InstanceDefinitionRecord::endian()
{
    SInstanceDefinition *pSInstDef = (SInstanceDefinition*)getData();

    ENDIAN( pSInstDef->iInstDefNumber );
}


////////////////////////////////////////////////////////////////////
//
//                          InstanceReferenceRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<InstanceReferenceRecord> g_InstanceReferenceProxy;

InstanceReferenceRecord::InstanceReferenceRecord()
{
}


// virtual
InstanceReferenceRecord::~InstanceReferenceRecord()
{
}


void InstanceReferenceRecord::endian()
{
    SInstanceReference *pSInstRef = (SInstanceReference*)getData();

    ENDIAN( pSInstRef->iInstDefNumber );
}
