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
}
